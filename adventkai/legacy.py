from pathlib import Path
import adventkai
import logging
import snekmud
import asyncio
from random import randint

from mudforge import NET_CONNECTIONS
from snekmud import WORLD, COMPONENTS, EQUIP_SLOTS, OPERATIONS
from snekmud.utils import get_or_emplace
from snekmud.serialize import deserialize_entity, serialize_entity
from snekmud.db.accounts.models import Account
from snekmud.utils import read_json_file
from snekmud.modules import Module, Prototype

from mudrich.circle import CircleToEvennia


def _broadcast(s: str):
    for k, v in NET_CONNECTIONS.items():
        v.send_line(s)


def convert_ansi(data):
    for x in ('Name', 'ShortDescription', 'LongDescription', 'Description'):
        if x in data:
            data[x] = CircleToEvennia(data[x])
    if 'ExDescriptions' in data:
        data["ExDescriptions"] = [[y[0], CircleToEvennia(y[1])] for y in data['ExDescriptions']]

    return data

class LegacyModule(Module):

    def __init__(self, name: str, path: Path, save_path: Path):
        super().__init__(name, path, save_path)
        self.account_map = dict()
        self.system = None
        self.created_entities = set()
        self.legacy_path = Path("legacy")

    wears = {0: "take", 1: "finger", 2: "neck", 3: "body", 4: "head", 5: "legs", 6: "feet", 7: "hands", 8: "arms",
             9: "shield", 10: "about", 11: "waist", 12: "wrist", 13: "wield", 14: "hold", 15: "back", 16: "ear",
             17: "shoulders", 18: "eyes"}

    equip = {1: "right_ring_finger", 2: "left_ring_finger", 3: "neck_1", 4: "neck_2", 5: "body", 6: "head", 7: "legs",
             8: "feet", 9: "hands", 10: "arms", 12: "about", 13: "waist", 14: "right_wrist", 15: "left_wrist", 16: "wield_1",
             17: "wield_2", 18: "back", 19: "right_ear", 20: "left_ear", 21: "shoulders", 22: "eyes"}

    def convert_wears(self, wear_flags: list[int]) -> list[str]:
        return [self.wears[f] for f in wear_flags]

    async def load_init(self):
        self.system = snekmud.MODULES["system"]

        logging.info("Loading Legacy Zones...")
        await self.load_zones()
        logging.info(f"Loaded {len(adventkai.LEGACY_ZONES)} Legacy Zones from Files!")
        _broadcast(f"Loading {len(adventkai.LEGACY_ZONES)} Legacy Zones!")

        logging.info("Loading Legacy Triggers...")
        await self.load_triggers()
        logging.info(f"Loaded {len(adventkai.LEGACY_TRIGGERS)} Legacy Triggers from Files!")
        _broadcast(f"Loaded {len(adventkai.LEGACY_TRIGGERS)} Legacy Scripts!")

    async def load_maps(self):
        logging.info("Loading Legacy Rooms...")
        await self.load_rooms()
        logging.info(f"Loaded {len(adventkai.LEGACY_ROOMS)} Legacy Rooms from Files!")
        _broadcast(f"Loaded {len(adventkai.LEGACY_ROOMS)} Legacy Rooms!")

    async def load_zones(self):
        z_dir = self.legacy_path / "zones"

        for d in [d for d in z_dir.iterdir() if d.is_dir()]:
            await asyncio.sleep(0)
            zf_dir = d / "zone.json"
            if not (zf_dir.exists() and zf_dir.is_file()):
                continue
            if not (j := read_json_file(zf_dir)):
                continue

            ent = deserialize_entity(convert_ansi(j))
            vcomp = WORLD.component_for_entity(ent, COMPONENTS["HasVnum"])
            z = WORLD.component_for_entity(ent, COMPONENTS["Zone"])
            z.dir = d
            vn = vcomp.vnum
            adventkai.LEGACY_ZONES[vn] = ent

    async def load_triggers(self):
        for k, v in adventkai.LEGACY_ZONES.items():
            await asyncio.sleep(0)
            z = get_or_emplace(v, COMPONENTS["Zone"])
            zv = get_or_emplace(v, COMPONENTS["ZoneVnums"])
            tf_dir = z.dir / "triggers.json"
            if not (tf_dir.exists() and tf_dir.is_file()):
                continue
            for j in read_json_file(tf_dir):
                ent = deserialize_entity(convert_ansi(j))
                vcomp = WORLD.component_for_entity(ent, COMPONENTS["HasVnum"])
                vn = vcomp.vnum

                zv.triggers[vn] = ent
                adventkai.LEGACY_TRIGGERS[vn] = ent

    def _load_vars(self, ent, j):
        if "global_vars" in j:
            h = COMPONENTS["DgScriptHolder"]()
            for g in j.pop("global_vars"):
                if g[1] not in h.variables:
                    h.variables[g[1]] = dict()
                h.variables[g[1]][g[0]] = g[2]
            if h.variables:
                WORLD.add_component(ent, h)

    async def load_rooms(self):
        zone_vnums = sorted(adventkai.LEGACY_ZONES.keys())
        zone_total = len(zone_vnums)

        for znum, k in enumerate(zone_vnums):
            await asyncio.sleep(0)
            v = adventkai.LEGACY_ZONES[k]
            z = get_or_emplace(v, COMPONENTS["Zone"])
            zv = get_or_emplace(v, COMPONENTS["ZoneVnums"])
            zn = WORLD.try_component(v, COMPONENTS["Name"])
            rf_dir = z.dir / "rooms.json"
            if not (rf_dir.exists() and rf_dir.is_file()):
                continue
            rj = read_json_file(rf_dir)
            num_rooms = len(rj)
            logging.info(f"Loading Zone {znum+1} of {zone_total} - ({k}): {zn} ({num_rooms} rooms)")
            _broadcast(f"Loading {num_rooms} rooms from Zone {znum+1} of {zone_total}...")
            for num, j in enumerate(rj):
                ent = deserialize_entity(convert_ansi(j))
                vcomp = WORLD.component_for_entity(ent, COMPONENTS["HasVnum"])
                vn = vcomp.vnum
                zv.rooms[vn] = ent
                adventkai.LEGACY_ROOMS[vn] = ent
                self._load_vars(ent, j)

    async def load_prototypes(self):
        await super().load_prototypes()

        logging.info("Loading Legacy Objects...")
        await self.load_objects()
        logging.info(f"Loaded {len(adventkai.LEGACY_OBJECTS)} Legacy Objects from Files!")
        _broadcast(f"Loaded {len(adventkai.LEGACY_OBJECTS)} Legacy Items!")

        logging.info("Loading Legacy Mobiles...")
        await self.load_mobiles()
        logging.info(f"Loaded {len(adventkai.LEGACY_MOBILES)} Legacy Mobiles from Files!")
        _broadcast(f"Loaded {len(adventkai.LEGACY_MOBILES)} Legacy NPCs!")

    async def load_objects(self):
        for k, v in adventkai.LEGACY_ZONES.items():
            await asyncio.sleep(0)
            z = get_or_emplace(v, COMPONENTS["Zone"])
            zv = get_or_emplace(v, COMPONENTS["ZoneVnums"])
            of_dir = z.dir / "objects.json"
            if not (of_dir.exists() and of_dir.is_file()):
                continue
            for j in read_json_file(of_dir):
                j = convert_ansi(j)
                if (wears := j.pop("WearFlags", list())):
                    j["WearSlots"] = dict()
                    j["WearSlots"]["slots"] = self.convert_wears(wears)

                vn = j["HasVnum"]["vnum"]
                proto_key = f"obj_{vn}"
                j["Prototype"] = {"module_name": self.name, "prototype": proto_key}
                proto = Prototype(self, proto_key, j)
                self.prototypes[proto_key] = proto
                adventkai.LEGACY_OBJECTS[vn] = proto
                zv.objects[vn] = proto

    async def load_mobiles(self):
        for k, v in adventkai.LEGACY_ZONES.items():
            await asyncio.sleep(0)
            z = get_or_emplace(v, COMPONENTS["Zone"])
            zv = get_or_emplace(v, COMPONENTS["ZoneVnums"])
            of_dir = z.dir / "mobiles.json"
            if not (of_dir.exists() and of_dir.is_file()):
                continue
            for j in read_json_file(of_dir):
                j = convert_ansi(j)
                vn = j["HasVnum"]["vnum"]
                proto_key = f"mob_{vn}"
                j["Prototype"] = {"module_name": self.name, "prototype": proto_key}
                proto = Prototype(self, proto_key, j)
                self.prototypes[proto_key] = proto
                adventkai.LEGACY_MOBILES[vn] = proto
                zv.mobiles[vn] = proto

    async def load_userdata(self):
        logging.info("Loading Legacy Accounts...")
        await self.load_accounts()
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

        logging.info("Unloading userdata...")
        for e in self.created_entities:
            WORLD.delete_entity(e, immediate=True)
        self.created_entities.clear()
        logging.info("Entities removed from world...")

    async def load_accounts(self):
        a_dir = self.legacy_path / "accounts"

        for d in [d for d in a_dir.iterdir() if d.is_file()]:
            await asyncio.sleep(0)
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
        for i in data:
            if (wears := i.pop("WearFlags", list())):
                i["WearSlots"] = dict()
                i["WearSlots"]["slots"] = self.convert_wears(wears)
            ent = deserialize_entity(convert_ansi(i))
            self.created_entities.add(ent)
            proto_key = "obj"
            if (leg_vnum := WORLD.try_component(ent, COMPONENTS["HasLegacyObjProto"])):
                if leg_vnum.vnum not in (0, -1, 4294967295):
                    proto_key = f"obj_{leg_vnum.vnum}"
            self.assign_id(ent, proto_key, index=False)
            await OPERATIONS["AddToInventory"](ent, holder).execute()
            if "contents" in i:
                await self._load_contents(i.pop("contents"), ent)

    async def load_player_characters(self):
        c_dir = self.legacy_path / "characters"
        count = 0
        for d in [d for d in c_dir.iterdir() if d.is_file()]:
            await asyncio.sleep(0)
            if not (j := read_json_file(d)):
                continue
            acc_id = j["player_specials"].pop("account_id")
            acc = self.account_map[acc_id]
            ent = deserialize_entity(convert_ansi(j))
            self.system.assign_id(ent, "pc", index=False)
            WORLD.add_component(ent, COMPONENTS["AccountOwner"](account_id=acc.id))
            self.created_entities.add(ent)
            self._load_vars(ent, j)

            if "carrying" in j:
                await self._load_contents(j.pop("carrying"), ent)

            for k, v in j.pop("equipment", list()):
                if (wears := v.pop("WearFlags", list())):
                    v["WearSlots"] = dict()
                    v["WearSlots"]["slots"] = self.convert_wears(wears)
                eq_ent = deserialize_entity(convert_ansi(v))
                self.created_entities.add(eq_ent)
                proto_key = "obj"
                if (leg_vnum := WORLD.try_component(ent, COMPONENTS["HasLegacyObjProto"])):
                    if leg_vnum.vnum not in (0, -1, 4294967295):
                        proto_key = f"obj_{leg_vnum.vnum}"
                self.assign_id(eq_ent, proto_key, index=False)
                await OPERATIONS["EquipToEntity"](eq_ent, ent, EQUIP_SLOTS["circle"][self.equip[k]]).execute()
                if "contents" in v:
                    await self._load_contents(v.pop("contents"), eq_ent)

    async def save_userdata(self):
        for ent, (n, pc, acc_owner) in WORLD.get_components(COMPONENTS["Name"], COMPONENTS["PlayerCharacter"], COMPONENTS["AccountOwner"]):
            data = serialize_entity(ent)
            inventory = data.pop("Inventory", None)
            equipment = data.pop("Equipment", None)

            acc = Account.objects.get(id=acc_owner.account_id)
            c = acc.characters.create(id=pc.player_id, name=str(n), data=data, inventory=inventory, equipment=equipment)

    async def load_entities_initial(self):
        for zone_id in sorted(adventkai.LEGACY_ZONES.keys()):
            logging.info(f"ZONE RESET: {zone_id}")
            await self.zone_reset(adventkai.LEGACY_ZONES[zone_id])

    async def zone_reset(self, ent):
        #has_vnum = WORLD.component_for_entity(ent, COMPONENTS["HasVnum"])
        #name = WORLD.component_for_entity(ent, COMPONENTS["Name"])
        zone = WORLD.component_for_entity(ent, COMPONENTS["Zone"])
        zv = WORLD.component_for_entity(ent, COMPONENTS["ZoneVnums"])
        #vn = has_vnum.vnum
        # print(f"Resetting Zone: {vn}: {name}")
    
        last_loaded = None
        last_cmd = False
        # fully load all commands in one query.
    
        for line, cmd in enumerate(zone.commands):
            # print(f"Processing Line {line}: {cmd.command} {cmd.arg1} {cmd.arg2} {cmd.arg3} {cmd.arg4} {cmd.arg5}")
            await asyncio.sleep(0)
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
                        mob = deserialize_entity(proto.data)
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

                        item = deserialize_entity(proto.data)
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
                        item = deserialize_entity(proto.data)
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

                        wear_slot = self.equip[cmd.arg3]
                        # print(f"For the new {last_loaded}: Equipping Item {cmd.arg1}: {proto.data['Name']} to slot {wear_slot}")
                        item = deserialize_entity(proto.data)
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
    
        for room in zv.rooms.values():
            continue
            if hasattr(room.obj, "at_zone_reset"):
                room.obj.at_zone_reset()
                await asyncio.sleep(0.01)