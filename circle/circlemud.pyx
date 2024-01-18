from cython.operator cimport dereference as deref, preincrement as inc
from libcpp.memory cimport shared_ptr
from libcpp.string cimport string

cimport comm
cimport net
cimport db
cimport structs
cimport accounts
cimport spells
cimport utils

import os
import asyncio
import time
import logging
import traceback
import pickle
import orjson


from adventkai.portal.game_session import Capabilities
from adventkai.core import Service
from adventkai.server.database import dump_all

from adventkai.game_session import (
    GameSession as BaseGameSession,
    ClientHello,
    ClientCommand,
    ClientGMCP,
    ClientUpdate,
    ClientDisconnect,
    ServerDisconnect,
    ServerUserdata,
    ServerMSSP,
    ServerText,
    ServerGMCP
)

cdef class GameSession:
    cdef shared_ptr[net.Connection] conn
    cdef object core
    cdef object capabilities
    cdef object userdata
    cdef object ws
    cdef object running
    cdef object outgoing_queue

    def __cinit__(self):
        self.conn = net.newConnection()

    def __init__(self, core, ws, data: ClientHello):
        self.core = core
        self.capabilities = data.capabilities
        self.userdata = data.userdata
        self.ws = ws
        self.running = True
        # This contains arbitrary data sent by the server which will be sent on a reconnect.
        self.outgoing_queue = asyncio.Queue()

    async def run(self):
        await self.start()
        await asyncio.gather(*[self.run_reader(), self.run_writer(), self.conn_pusher()])

    async def run_writer(self):
        while data := await self.outgoing_queue.get():
            try:
                await self.ws.send(pickle.dumps(data))
            except Exception as err:
                logging.error(traceback.format_exc())
                logging.error(err)

    async def run_reader(self):
        async for message in self.ws:
            if isinstance(message, bytes):
                msg = pickle.loads(message)
                await self.handle_incoming_message(msg)

    async def start(self):
        if self.userdata:
            await self.start_resume()
        else:
            await self.start_fresh()

    async def start_resume(self):
        pass

    async def start_fresh(self):
        pass

    async def change_capabilities(self, changed: dict[str, "Any"]):
        for k, v in changed.items():
            setattr(self.capabilities, k, v)
            await self.at_capability_change(k, v)

    async def at_capability_change(self, capability: str, value):
        pass

    async def handle_incoming_message(self, msg):
        if (meth := getattr(msg, "at_server_receive", None)):
            await meth(self)

    async def handle_command(self, text: str):
        message = {
            "type": "Game.Command",
            "data": text
        }
        self.conn.get().inQueue.push_back(orjson.dumps(message))

    async def handle_gmcp(self, cmd: str, gmcp: str):
        message = {"type": "Game.GMCP",
                   "cmd": str}
        if gmcp:
            message["data"] = gmcp
        self.conn.get().inQueue.push_back(orjson.dumps(message))

    async def conn_pusher(self):
        while True:
            if self.conn.get().outQueue.empty():
                await asyncio.sleep(0.05)
                continue

            message = self.conn.get().outQueue.front()
            self.conn.get().outQueue.pop_front()

            decoded = message.decode("UTF-8")
            await self.outgoing_queue.put(decoded)


def initialize():
    comm.init_locale()
    comm.init_log()
    if not comm.init_sodium():
        raise Exception("Sodium failed to initialize!")
    db.load_config()
    os.chdir("lib")
    comm.init_database()
    comm.init_zones()

def run_loop_once(deltaTime: float):
    comm.run_loop_once(deltaTime)

def trigger_save():
    save_all()


cdef save_rooms(dump_data: dict):
    # rooms
    it = db.world.begin()
    end = db.world.end()

    dumped = list()

    while it != end:
        dumped.append({"id": deref(it).first,
                       "data": utils.rjdump(deref(it).second.rserialize()).decode("UTF-8")})
        inc(it)
    dump_data["rooms"] = dumped

cdef save_shops(dump_data: dict):
    # shops
    it = db.shop_index.begin()
    end = db.shop_index.end()

    dumped = list()

    while it != end:
        dumped.append({"id": deref(it).first,
                       "data": utils.jdump(deref(it).second.serialize()).decode("UTF-8")})
        inc(it)
    dump_data["shops"] = dumped

cdef save_guilds(dump_data: dict):
    # guilds
    it = db.guild_index.begin()
    end = db.guild_index.end()

    dumped = list()

    while it != end:
        dumped.append({"id": deref(it).first,
                       "data": utils.jdump(deref(it).second.serialize()).decode("UTF-8")})
        inc(it)
    dump_data["guilds"] = dumped

cdef save_zones(dump_data: dict):
    # zones
    it = db.zone_table.begin()
    end = db.zone_table.end()

    dumped = list()

    while it != end:
        dumped.append({"id": deref(it).first,
                       "data": utils.jdump(deref(it).second.serialize()).decode("UTF-8")})
        inc(it)
    dump_data["zones"] = dumped

cdef save_areas(dump_data: dict):
    # areas
    it = db.areas.begin()
    end = db.areas.end()

    dumped = list()

    while it != end:
        dumped.append({"id": deref(it).first,
                       "data": utils.jdump(deref(it).second.serialize()).decode("UTF-8")})
        inc(it)
    dump_data["areas"] = dumped

cdef save_accounts(dump_data: dict):
    # accounts
    it = accounts.accounts.begin()
    end = accounts.accounts.end()

    dumped = list()

    while it != end:
        dumped.append({"id": deref(it).first,
                       "data": utils.jdump(deref(it).second.serialize()).decode("UTF-8")})
        inc(it)
    dump_data["accounts"] = dumped

cdef save_players(dump_data: dict):
    # players
    it = db.players.begin()
    end = db.players.end()

    dumped = list()

    while it != end:
        dumped.append({"id": deref(it).first,
                       "data": utils.jdump(deref(it).second.serialize()).decode("UTF-8")})
        inc(it)
    dump_data["players"] = dumped

cdef save_item_prototypes(dump_data: dict):
    # itemPrototypes
    it = db.obj_proto.begin()
    end = db.obj_proto.end()

    dumped = list()

    while it != end:
        dumped.append({"id": deref(it).first,
                       "data": utils.jdump(deref(it).second.serializeProto()).decode("UTF-8")})
        inc(it)
    dump_data["itemPrototypes"] = dumped

cdef save_npc_prototypes(dump_data: dict):
    # npcPrototypes
    it = db.mob_proto.begin()
    end = db.mob_proto.end()

    dumped = list()

    while it != end:
        dumped.append({"id": deref(it).first,
                       "data": utils.jdump(deref(it).second.serializeProto()).decode("UTF-8")})
        inc(it)
    dump_data["npcPrototypes"] = dumped

cdef save_dgscript_prototypes(dump_data: dict):
    # dgScriptPrototypes
    it = db.trig_index.begin()
    end = db.trig_index.end()

    dumped = list()

    while it != end:
        dumped.append({"id": deref(it).first,
                       "data": utils.jdump(deref(it).second.serializeProto()).decode("UTF-8")})
        inc(it)
    dump_data["dgScriptPrototypes"] = dumped

cdef save_characters(dump_data: dict):
    # characters
    it = db.uniqueCharacters.begin()
    end = db.uniqueCharacters.end()

    dumped = list()

    while it != end:
        ref = deref(it)
        gen = ref.second.first
        c = ref.second.second
        dumped.append({"id": ref.first, "generation": gen, "vnum": c.vn,
                       "data": utils.jdump(c.serializeInstance()).decode("UTF-8"),
                       "location": utils.jdump(c.serializeLocation()).decode("UTF-8"),
                       "relations": utils.jdump(c.serializeRelations()).decode("UTF-8"),
                       "name": c.name.decode("UTF-8") if c.name is not NULL else "",
                       "shortDesc": c.short_description if c.short_description is not NULL else ""})
        inc(it)
    dump_data["characters"] = dumped

cdef save_items(dump_data: dict):
    # items
    it = db.uniqueObjects.begin()
    end = db.uniqueObjects.end()

    dumped = list()

    while it != end:
        ref = deref(it)
        gen = ref.second.first
        c = ref.second.second
        dumped.append({"id": ref.first, "generation": gen, "vnum": c.vn,
                       "data": utils.jdump(c.serializeInstance()).decode("UTF-8"),
                       "location": c.serializeLocation().decode("UTF-8"), "slot": c.worn_on,
                       "relations": utils.jdump(c.serializeRelations()).decode("UTF-8"),
                       "name": c.name.decode("UTF-8") if c.name is not NULL else "",
                       "shortDesc": c.short_description if c.short_description is not NULL else ""})
        inc(it)
    dump_data["items"] = dumped

cdef save_scripts(dump_data: dict):
    it = db.uniqueScripts.begin()
    end = db.uniqueScripts.end()

    dumped = list()

    while it != end:
        ref = deref(it)
        gen = ref.second.first
        c = ref.second.second
        dumped.append({"id": ref.first, "generation": gen, "vnum": c.vn,
                       "data": utils.jdump(c.serializeInstance()).decode("UTF-8"),
                       "location": c.serializeLocation().decode("UTF-8"),
                       "name": c.name.decode("UTF-8") if c.name is not NULL else "",
                       "num": c.order})
        inc(it)
    dump_data["dgscripts"] = dumped

cdef save_all():
        dump_data = dict()
        start_time = time.perf_counter()

        save_rooms(dump_data)
        save_shops(dump_data)
        save_guilds(dump_data)
        save_zones(dump_data)
        save_areas(dump_data)
        save_accounts(dump_data)
        save_players(dump_data)
        save_item_prototypes(dump_data)
        save_npc_prototypes(dump_data)
        save_dgscript_prototypes(dump_data)
        save_characters(dump_data)
        save_items(dump_data)
        save_scripts(dump_data)

        end_time = time.perf_counter()
        print(f"Elapsed time to dump game: {end_time - start_time}")

        asyncio.create_task(dump_all(dump_data))


class GameService(Service):
    def __init__(self, core):
        super().__init__(core)
        self.running: bool = True

    async def at_pre_start(self):
        initialize()

    async def start(self):
        deltaTimeInSeconds: float = 0.1
        loop_frequency: float = 0.1
        save_timer: float = 60.0 * 5.0
        last_time = time.perf_counter()

        while self.running:
            start = time.perf_counter()
            comm.run_loop_once(deltaTimeInSeconds)
            end = time.perf_counter()

            save_timer -= deltaTimeInSeconds
            if save_timer <= 0.0:
                save_all()
                save_timer = 60.0 * 5.0

            duration = end - start
            wait_time = loop_frequency - duration
            if wait_time < 0:
                wait_time = 0.001

            await asyncio.sleep(wait_time)
            deltaTimeInSeconds = time.perf_counter() - start







cdef class _AccountManager:
    async def retrieve_user(self, request, payload, *args, **kwargs):
        if payload:
            if not (user_id := payload.get("user_id", None)):
                return None
            found = accounts.accounts.find(user_id)
            if found == accounts.accounts.end():
                return None
            user = deref(found).second

            out = {"user_id": user.vn, "username": user.name.decode("UTF-8"), "adminLevel": user.adminLevel}
            if not user.email.empty():
                out["email"] = user.email.decode("UTF-8")

            return out

        return None

    async def authenticate(self, request, *args, **kwargs):
        from sanic_jwt import exceptions
        username = request.json.get("username", None)
        password = request.json.get("password", None)

        if not username or not password:
            raise exceptions.AuthenticationFailed("Missing username or password.")

        user = accounts.findAccount(username.encode())

        if user is NULL:
            raise exceptions.AuthenticationFailed("Incorrect credentials.")

        if not user.checkPassword(password.encode()):
            raise exceptions.AuthenticationFailed("Incorrect credentials.")

        out = {"user_id": user.vn, "username": user.name.decode("UTF-8"), "adminLevel": user.adminLevel}
        if not user.email.empty():
            out["email"] = user.email.decode("UTF-8")
        if not user.characters.empty():
            out["characters"] = [x for x in user.characters]

        return out

account_manager = _AccountManager()

# Keeping this here as an example of how to iterate through stuff.
def print_account_names():
    it = accounts.accounts.begin()
    end = accounts.accounts.end()

    while it != end:
        print(deref(it).second.name)
        inc(it)


cdef class _SkillManager:
    min_id = 0
    max_id = 999

    def direct_get(self, num: int):
        out = {
            "id": num
        }
        spell: spells.spell_info_type = spells.spell_info[num]
        if spell.name is not NULL:
            out["name"] = spell.name.decode("UTF-8")
        return out

    def get_range(self, start: int, end: int):
        order = sorted([start, end])
        true_start = max(self.min_id, order[0])
        true_end = min(self.max_id, order[1])

        out = list()

        for i in range(true_start, true_end):
            out.append([i, self.direct_get(i)])

        return out

    def get_many(self, ids: list[int,...]):
        out = list()
        for num in ids:
            if (found := self.get(num)):
                out.append([num, found])
        return out

    def get(self, num: int):
        if num > self.max_id:
            return dict()
        if num < self.min_id:
            return dict()
        return self.direct_get(num)


skill_manager = _SkillManager()