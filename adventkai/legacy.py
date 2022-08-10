from pathlib import Path
import adventkai
import logging
import snekmud
import asyncio
from random import randint
from collections import defaultdict
from dataclasses import dataclass
from dataclasses_json import dataclass_json
from mudforge import NET_CONNECTIONS
from snekmud import WORLD, COMPONENTS, EQUIP_SLOTS, OPERATIONS
from snekmud.utils import get_or_emplace
from snekmud.serialize import deserialize_entity, serialize_entity
from snekmud.db.accounts.models import Account
from snekmud.utils import read_json_file, write_json_file
from snekmud.modules import Module, Prototype
from adventkai.dgscripts.dgscripts import DGScript
from mudrich.circle import CircleStrip
from mudrich.circle import CircleToEvennia
from mudforge.utils import generate_name


def _broadcast(s: str):
    for k, v in NET_CONNECTIONS.items():
        v.send_line(s)


def convert_ansi(data):
    for x in ('Name', 'ShortDescription', 'RoomDescription', 'Description', 'ActionDescription'):
        if x in data:
            data[x] = CircleToEvennia(data[x])
    if 'ExDescriptions' in data:
        data["ExDescriptions"] = [[y[0], CircleToEvennia(y[1])] for y in data['ExDescriptions']]

    if (trig := data.get("Triggers", None)):
        variables = {k: v for k, v in trig.get("variables", list())}
        data["Triggers"]["variables"] = variables

    return data


WEARS = {0: "take", 1: "finger", 2: "neck", 3: "body", 4: "head", 5: "legs", 6: "feet", 7: "hands", 8: "arms",
             9: "shield", 10: "about", 11: "waist", 12: "wrist", 13: "wield", 14: "hold", 15: "back", 16: "ear",
             17: "shoulders", 18: "eyes"}

EQUIP = {1: "right_ring_finger", 2: "left_ring_finger", 3: "neck_1", 4: "neck_2", 5: "body", 6: "head", 7: "legs",
         8: "feet", 9: "hands", 10: "arms", 12: "about", 13: "waist", 14: "right_wrist", 15: "left_wrist",
         16: "wield_1",
         17: "wield_2", 18: "back", 19: "right_ear", 20: "left_ear", 21: "shoulders", 22: "eyes"}


class LegacyImporter:



    def convert_wears(self, wear_flags: list[int]) -> list[str]:
        return [WEARS[f] for f in wear_flags]

    def __init__(self):
        self.leg_path = Path("legacy")
        self.mod_path = Path("modules")
        self.account_map = dict()
        self.zones = dict()
        self.leg_zones = dict()
        self.id_storage = defaultdict(lambda: defaultdict(set))

    def gen_id(self, mod_name: str, proto: str):
        new_id = generate_name(proto, self.id_storage[mod_name][proto])
        self.id_storage[mod_name][proto].add(new_id)
        return {"module_name": mod_name, "prototype": proto, "ent_id": new_id}

    def load_maps(self):
        logging.info("Loading Legacy Rooms...")
        self.load_rooms()
        logging.info(f"Loaded {len(adventkai.LEGACY_ROOMS)} Legacy Rooms from Files!")
        _broadcast(f"Loaded {len(adventkai.LEGACY_ROOMS)} Legacy Rooms!")

    def load_rooms(self):
        for k, v in self.leg_zones.items():
            rf_dir = v / "rooms.json"
            if not (rf_dir.exists() and rf_dir.is_file()):
                continue
            rj = read_json_file(rf_dir)
            num_rooms = len(rj)
            if not num_rooms:
                continue
            room_data = list()
            room_folder = self.zones[k] / "rooms"
            room_folder.mkdir(exist_ok=True)
            logging.info(f"Loading Zone {k}: ({num_rooms} rooms)")
            for j in rj:
                j = convert_ansi(j)
                vn = j.pop("vnum")
                j["LegacyRoom"] = {"vnum": vn}
                j["MetaTypes"] = {"types": ["room", "legacy_room"]}
                write_json_file(room_folder / f"room_{vn}.json", j)

    def load_zones(self):
        z_dir = self.leg_path / "zones"

        for d in [d for d in z_dir.iterdir() if d.is_dir()]:
            zf_dir = d / "zone.json"
            if not (zf_dir.exists() and zf_dir.is_file()):
                continue
            if not (j := read_json_file(zf_dir)):
                continue

            j["color_name"] = CircleToEvennia(j["color_name"])
            j["class"] = "adventkai.legacy.LegacyModule"
            vn = j["vnum"]
            zone_mod_dir = self.mod_path / f"zone_{vn}"
            zone_mod_dir.mkdir(exist_ok=True)
            write_json_file(zone_mod_dir / "meta.json", j)
            self.zones[vn] = zone_mod_dir
            self.leg_zones[vn] = d

    def load_triggers(self):
        for k, v in self.leg_zones.items():
            tf_dir = v / "triggers.json"
            if not (tf_dir.exists() and tf_dir.is_file()):
                continue
            trig_folder = self.zones[k] / "triggers"
            trig_folder.mkdir(exist_ok=True)
            for j in read_json_file(tf_dir):
                j["name"] = CircleStrip(j["name"])
                j["cmdlist"] = [CircleToEvennia(x) for x in j["cmdlist"]]
                dg = DGScript.from_dict(j)
                dvn = j["vnum"]

                write_json_file(trig_folder / f"{dvn}.json", dg.to_dict())

    def load_init(self):
        logging.info("Loading Legacy Zones...")
        self.load_zones()
        logging.info(f"Loaded {len(adventkai.LEGACY_ZONES)} Legacy Zones from Files!")
        _broadcast(f"Loading {len(adventkai.LEGACY_ZONES)} Legacy Zones!")

        logging.info("Loading Legacy Triggers...")
        self.load_triggers()
        logging.info(f"Loaded {len(adventkai.LEGACY_TRIGGERS)} Legacy Triggers from Files!")
        _broadcast(f"Loaded {len(adventkai.LEGACY_TRIGGERS)} Legacy Scripts!")


    def load_prototypes(self):

        logging.info("Loading Legacy Objects...")
        self.load_objects()
        logging.info(f"Loaded {len(adventkai.LEGACY_OBJECTS)} Legacy Objects from Files!")
        _broadcast(f"Loaded {len(adventkai.LEGACY_OBJECTS)} Legacy Items!")

        logging.info("Loading Legacy Mobiles...")
        self.load_mobiles()
        logging.info(f"Loaded {len(adventkai.LEGACY_MOBILES)} Legacy Mobiles from Files!")
        _broadcast(f"Loaded {len(adventkai.LEGACY_MOBILES)} Legacy NPCs!")

    def load_objects(self):
        for k, v in self.leg_zones.items():
            of_dir = v / "objects.json"
            if not (of_dir.exists() and of_dir.is_file()):
                continue
            if not (obj_data := read_json_file(of_dir)):
                continue
            proto_dir = self.zones[k] / "prototypes"
            proto_dir.mkdir(exist_ok=True)
            obj_pro = {"Name": "Legacy Object"}
            write_json_file(proto_dir / "obj.json", obj_pro)
            for j in obj_data:
                j = convert_ansi(j)
                if (wears := j.pop("WearFlags", list())):
                    j["WearSlots"] = dict()
                    j["WearSlots"]["slots"] = self.convert_wears(wears)
                vn = j.pop("vnum")
                j["LegacyItem"] = {"vnum": vn}
                j["MetaTypes"] = {"types": ["item", "legacy_item"]}
                proto_key = f"obj_{vn}"
                write_json_file(proto_dir / f"{proto_key}.json", j)

    def load_mobiles(self):
        for k, v in self.leg_zones.items():
            of_dir = v / "mobiles.json"
            if not (of_dir.exists() and of_dir.is_file()):
                continue
            if not (mob_data := read_json_file(of_dir)):
                continue
            proto_dir = self.zones[k] / "prototypes"
            proto_dir.mkdir(exist_ok=True)
            mob_pro = {"Name": "Legacy NPC"}
            write_json_file(proto_dir / "mob.json", mob_pro)
            for j in mob_data:
                j = convert_ansi(j)
                j["NPC"] = dict()
                vn = j.pop("vnum")
                j["LegacyNPC"] = {"vnum": vn}
                j["MetaTypes"] = {"types": ["character", "npc", "legacy_character"]}
                proto_key = f"mob_{vn}"
                write_json_file(proto_dir / f"{proto_key}.json", j)

    async def load_userdata(self):
        logging.info("Loading Legacy Accounts...")
        self.load_accounts()
        count = Account.objects.count()
        logging.info(f"Loaded {count} Legacy Accounts from Files!")
        _broadcast(f"Loaded {count} Legacy Accounts!")

        logging.info("Loading Legacy Player Characters...")
        await self.load_player_characters()
        count = len(WORLD.get_component(COMPONENTS["PlayerCharacter"]))
        logging.info(f"Loaded {count} Legacy Player Characters from Files!")
        _broadcast(f"Loaded {count} Legacy Player characters!")

        logging.info("Saving converted User data...")
        await self.save_userdata()
        logging.info("Saved all User Data complete!")

    def load_accounts(self):
        a_dir = self.leg_path / "accounts"

        for d in [d for d in a_dir.iterdir() if d.is_file()]:
            if not (j := read_json_file(d)):
                continue
            acc = Account.objects.create_user(j.pop("name"), email=j.pop("email", None), password=j.pop("password", None))
            logging.info(f"Loading Legacy User: {acc.username}")

            if acc.username in ("Wayland", "Virtus", "Volund"):
                acc.is_superuser = True

            extra = dict()
            for x in ("current_rpp", "earned_rpp", "max_slots"):
                if x in j:
                    extra[x] = j.pop(x)

            if extra:
                acc.extra = extra

            acc.save()
            self.account_map[j.pop("account_id")] = acc

    async def _load_contents(self, data, holder):
        pro_comp = COMPONENTS["Prototype"]
        for i in data:
            if (wears := i.pop("WearFlags", list())):
                i["WearSlots"] = dict()
                i["WearSlots"]["slots"] = self.convert_wears(wears)

            if (vn := i.pop("vnum", None)) is not None:
                i["LegacyItem"] = {"vnum": vn}
            i["MetaTypes"] = {"types": ["item", "legacy_item"]}

            ent = deserialize_entity(convert_ansi(i))
            mod_name = "legacy"
            proto_key = "obj"
            if (proto_obj := WORLD.try_component(ent, pro_comp)):
                mod_name = proto_obj.module_name
                proto_key = proto_obj.prototype
                WORLD.remove_component(ent, pro_comp)
            WORLD.add_component(ent, COMPONENTS["EntityID"].deserialize(self.gen_id(mod_name, proto_key), ent))
            await OPERATIONS["AddToInventory"](ent, holder).execute()
            if "contents" in i:
                await self._load_contents(i.pop("contents"), ent)

    async def load_player_characters(self):
        c_dir = self.leg_path / "characters"
        count = 0
        for d in [d for d in c_dir.iterdir() if d.is_file()]:
            if not (j := read_json_file(d)):
                continue
            acc_id = j["player_specials"].pop("account_id")
            acc = self.account_map[acc_id]
            ent = deserialize_entity(convert_ansi(j))
            WORLD.add_component(ent, COMPONENTS["EntityID"].deserialize(self.gen_id("system", "pc"), ent))
            WORLD.add_component(ent, COMPONENTS["AccountOwner"](account_id=acc.id))

            if "carrying" in j:
                await self._load_contents(j.pop("carrying"), ent)

            pro_comp = COMPONENTS["Prototype"]
            for k, v in j.pop("equipment", list()):
                if (wears := v.pop("WearFlags", list())):
                    v["WearSlots"] = dict()
                    v["WearSlots"]["slots"] = self.convert_wears(wears)

                if (vn := v.pop("vnum", None)) is not None:
                    v["LegacyItem"] = {"vnum": vn}
                v["MetaTypes"] = {"types": ["item", "legacy_item"]}

                eq_ent = deserialize_entity(convert_ansi(v))
                proto_key = "obj"
                mod_name = "legacy"
                if (proto_obj := WORLD.try_component(ent, pro_comp)):
                    mod_name = proto_obj.module_name
                    proto_key = proto_obj.prototype
                    WORLD.remove_component(ent, pro_comp)
                WORLD.add_component(ent, COMPONENTS["EntityID"].deserialize(self.gen_id(mod_name, proto_key), ent))
                await OPERATIONS["EquipToEntity"](eq_ent, ent, EQUIP_SLOTS["circle"][EQUIP[k]]).execute()
                if "contents" in v:
                    await self._load_contents(v.pop("contents"), eq_ent)

    async def save_userdata(self):
        for ent, (n, pc, acc_owner) in WORLD.get_components(COMPONENTS["Name"], COMPONENTS["PlayerCharacter"], COMPONENTS["AccountOwner"]):
            data = serialize_entity(ent)
            inventory = data.pop("Inventory", None)
            equipment = data.pop("Equipment", None)

            acc = Account.objects.get(id=acc_owner.account_id)
            c = acc.characters.create(id=pc.player_id, name=str(n), data=data, inventory=inventory, equipment=equipment)

    def run(self):
        self.load_init()
        self.load_maps()
        self.load_prototypes()
        asyncio.run(self.load_userdata())


@dataclass_json
@dataclass
class ZoneResetCmd:
    """
    Commands:
    M - Read a mobile
    O - Read an Object
    G - Give obj to mob
    P - put obj in obj
    G - obj to Char
    E - obj to char equip
    D - set state of door
    T - trigger command
    V - assign variable
    """
    command: str = "-"
    if_flag: bool = False
    arg1: int = 0
    arg2: int = 0
    arg3: int = 0
    arg4: int = 0
    arg5: int = 0
    sarg1: str = ""
    sarg2: str = ""


class LegacyModule(Module):

    def __init__(self, name: str, path: Path, save_path: Path, meta=None):
        super().__init__(name, path, save_path, meta=meta)
        self.legacy_builders = list()
        self.age = 0
        self.bot = 0
        self.top = 0
        self.reset_mode = 0
        self.min_level = 0
        self.max_level = 0
        self.commands = list()
        self.vnum = -1

        self.rooms = dict()

        for x in ("color_name", "vnum", "legacy_builders", "lifespan", "age", "bot", "top", "reset_mode", "min_level", "max_level"):
            if x in meta:
                setattr(self, x, meta[x])
        self.sort_order = self.vnum
        adventkai.LEGACY_ZONES[self.vnum] = self
        for cmd in meta.pop("commands", list()):
            self.commands.append(ZoneResetCmd.from_dict(cmd))

    async def load_triggers(self):
        trigger_folder = self.path / "triggers"
        if not trigger_folder.exists():
            return

        for p in trigger_folder.iterdir():
            if p.is_file() and p.name.lower().endswith(".json"):
                if not (data := read_json_file(p)):
                    continue
                dg = DGScript.from_dict(data)
                adventkai.LEGACY_TRIGGERS[dg.vnum] = dg

    async def load_init(self):
        await super().load_init()
        await self.load_triggers()

    async def load_rooms(self):
        room_folder = self.path / "rooms"
        if not room_folder.exists():
            return

        for p in room_folder.iterdir():
            if p.is_file() and p.name.lower().endswith(".json"):
                if not (data := read_json_file(p)):
                    continue
                vn = data["LegacyRoom"]["vnum"]
                ent = deserialize_entity(data)
                adventkai.LEGACY_ROOMS[vn] = ent
                self.rooms[vn] = ent

    async def load_maps(self):
        await super().load_maps()
        await self.load_rooms()

    async def load_prototypes(self):
        await super().load_prototypes()
        for k, v in self.prototypes.items():
            if "_" not in k:
                continue
            prefix, val = k.split("_")
            val = int(val)
            match prefix:
                case "obj":
                    adventkai.LEGACY_OBJECTS[val] = v
                case "mob":
                    adventkai.LEGACY_MOBILES[val] = v

    async def load_entities_initial(self):
        await self.zone_reset()

    async def zone_reset(self):
        logging.info(f"Resetting Zone: {self.vnum}")
    
        last_loaded = None
        last_cmd = False
        # fully load all commands in one query.
    
        for line, cmd in enumerate(self.commands):
            # print(f"Processing Line {line}: {cmd.command} {cmd.arg1} {cmd.arg2} {cmd.arg3} {cmd.arg4} {cmd.arg5}")
            if cmd.if_flag:
                if not (last_cmd or last_loaded):
                    continue
            else:
                last_loaded = None
    
            try:
                match cmd.command:
                    # Comment
                    case "*":
                        last_cmd = False
                        continue
    
                    # Load a Mobile.
                    case "M":
                        if not (room := adventkai.LEGACY_ROOMS.get(cmd.arg3, None)):
                            last_cmd = False
                            last_loaded = None
                            continue

                        if not (proto := adventkai.LEGACY_MOBILES.get(cmd.arg1, None)):
                            last_cmd = False
                            last_loaded = None
                            continue

                        # Roll the chance dice. if arg5 is 0 this will always succeed.
                        if not randint(1, 100) >= cmd.arg5:
                            last_cmd = False
                            last_loaded = None
                            continue
    
                        # arg4 is the max amount of mobiles that can have loaded with this room as a home.
                        if cmd.arg4 > 0:
                            total_amount = 0
                            if (inv := WORLD.try_component(room, COMPONENTS["Inventory"])):
                                sp_comp = COMPONENTS["SpawnRoom"]
                                pro_comp = COMPONENTS["Prototype"]
                                for e in inv.inventory:
                                    if not ((pro := WORLD.try_component(e,
                                                                        pro_comp)) and pro.module_name == "legacy" and pro.prototype == f"mob_{cmd.arg1}"):
                                        continue
                                    if (spawn := WORLD.try_component(e, sp_comp)) and spawn.room == room:
                                        total_amount += 1
                            if total_amount >= cmd.arg4:
                                continue
    
                        # print(f"For Room {cmd.arg3}: Loading Mobile {cmd.arg1}: {proto.data['Name']}")
                        mob = deserialize_entity(proto.get())
                        WORLD.add_component(mob, COMPONENTS["SpawnRoom"](room=room))
                        await OPERATIONS["AddToRoom"](mob, room).execute()

                        last_loaded = mob
                        last_cmd = True
    
                    # Load an Object / Item.
                    case "O":
                        if not (room := adventkai.LEGACY_ROOMS.get(cmd.arg3, None)):
                            last_cmd = False
                            last_loaded = None
                            continue

                        if not (proto := adventkai.LEGACY_OBJECTS.get(cmd.arg1, None)):
                            last_cmd = False
                            last_loaded = None
                            continue
    
                        # Roll the chance dice. if arg5 is 0 this will always succeed.
                        if not randint(1, 100) >= cmd.arg5:
                            last_cmd = False
                            last_loaded = None
                            continue
    
                        # arg4 is the max amount of mobiles that can be in this room.
                        if cmd.arg4 > 0:
                            total_amount = 0
                            if (inv := WORLD.try_component(room, COMPONENTS["Inventory"])):
                                sp_comp = COMPONENTS["SpawnRoom"]
                                pro_comp = COMPONENTS["Prototype"]
                                for e in inv.inventory:
                                    if not ((pro := WORLD.try_component(e, pro_comp)) and pro.module_name == "legacy" and pro.prototype == f"obj_{cmd.arg1}"):
                                        continue
                                    if (spawn := WORLD.try_component(e, sp_comp)) and spawn.room == room:
                                        total_amount += 1

                            if total_amount >= cmd.arg4:
                                continue
    
                        # print(f"For Room {cmd.arg3}: Loading Item {cmd.arg1}: {proto.data['Name']}")

                        item = deserialize_entity(proto.get())
                        WORLD.add_component(item, COMPONENTS["SpawnRoom"](room=room))
                        await OPERATIONS["AddToRoom"](item, room).execute()
                        last_loaded = item
                        last_cmd = True
    
                    # Create an Item and add it to last loaded object's Inventory.
                    case "G" | "P":
                        if not last_loaded:
                            continue

                        if not (proto := adventkai.LEGACY_OBJECTS.get(cmd.arg1, None)):
                            last_cmd = False
                            last_loaded = None
                            continue
    
                        # Roll the chance dice. if arg5 is 0 this will always succeed.
                        if not randint(1, 100) >= cmd.arg5:
                            last_cmd = False
                            last_loaded = None
                            continue
    
                        # print(f"For the new {last_loaded}: Giving Item {cmd.arg1}: {proto.data['Name']}")
                        item = deserialize_entity(proto.get())
                        await OPERATIONS["AddToInventory"](item, last_loaded).execute()
                        last_loaded = item
                        last_cmd = True
    
                    # Create an Item and equip it to the last loaded object's equipment.
                    case "E":
                        if not last_loaded:
                            continue

                        if not (proto := adventkai.LEGACY_OBJECTS.get(cmd.arg1, None)):
                            last_cmd = False
                            last_loaded = None
                            continue
    
                        # Roll the chance dice. if arg5 is 0 this will always succeed.
                        if not randint(1, 100) >= cmd.arg5:
                            last_cmd = False
                            last_loaded = None
                            continue

                        wear_slot = EQUIP[cmd.arg3]
                        # print(f"For the new {last_loaded}: Equipping Item {cmd.arg1}: {proto.data['Name']} to slot {wear_slot}")
                        item = deserialize_entity(proto.get())
                        await OPERATIONS["EquipToEntity"](item, last_loaded, EQUIP_SLOTS["circle"][wear_slot]).execute()
                        last_cmd = True
    
                    # Set State of door.
                    case "D":
                        pass
    
                    # Assign a Trigger.
                    case "T":
                        if not last_loaded:
                            continue
                        # print(f"For the new {last_loaded.key}: Assigning Trigger {cmd.arg2}")
                        #last_loaded.dgscripts.attach(cmd.arg2)
    
                    # Assign a variable.
                    case "V":
                        if not last_loaded:
                            continue
                        # print(f"For the new {last_loaded.key}: Setting Global var {cmd.arg3} {cmd.sarg1} to: {cmd.sarg2}")
                        #last_loaded.dgscripts.vars[cmd.arg3][cmd.sarg1] = cmd.sarg2
    
            except KeyError as err:
                last_cmd = False
                last_loaded = None
                continue
    
        for room in self.rooms.values():
            await OPERATIONS["AtZoneReset"](self, room).execute()
