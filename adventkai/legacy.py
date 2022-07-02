from pathlib import Path
import adventkai
import logging
from adventkai import WORLD
from mudforge import NET_CONNECTIONS
from .utils import get_or_emplace
from adventkai import components as cm
import asyncio
from adventkai.serialize import deserialize_entity, save_entity
from adventkai import api
from adventkai.db.accounts.models import Account
from adventkai.utils import read_json_file


async def _broadcast(s: str):
    for k, v in NET_CONNECTIONS.items():
        await v.send_line(s)


class LegacyLoader:

    def __init__(self, p: Path):
        self.path = p
        self.account_map = dict()
        self.system = adventkai.MODULES["system"]
        self.legacy = adventkai.MODULES["legacy"]
        self.created_entities = set()

    async def load_assets(self):
        logging.info("Loading Legacy Zones...")
        await self.load_zones()
        logging.info(f"Loaded {len(adventkai.LEGACY_ZONES)} Legacy Zones from Files!")
        await _broadcast(f"Loading {len(adventkai.LEGACY_ZONES)} Legacy Zones!")

        logging.info("Loading Legacy Triggers...")
        await self.load_triggers()
        logging.info(f"Loaded {len(adventkai.LEGACY_TRIGGERS)} Legacy Triggers from Files!")
        await _broadcast(f"Loaded {len(adventkai.LEGACY_TRIGGERS)} Legacy Scripts!")

        logging.info("Loading Legacy Rooms...")
        await self.load_rooms()
        logging.info(f"Loaded {len(adventkai.LEGACY_ROOMS)} Legacy Rooms from Files!")
        await _broadcast(f"Loaded {len(adventkai.LEGACY_ROOMS)} Legacy Rooms!")

        logging.info("Loading Legacy Objects...")
        await self.load_objects()
        logging.info(f"Loaded {len(adventkai.LEGACY_OBJECTS)} Legacy Objects from Files!")
        await _broadcast(f"Loaded {len(adventkai.LEGACY_OBJECTS)} Legacy Items!")

        logging.info("Loading Legacy Mobiles...")
        await self.load_mobiles()
        logging.info(f"Loaded {len(adventkai.LEGACY_MOBILES)} Legacy Mobiles from Files!")
        await _broadcast(f"Loaded {len(adventkai.LEGACY_MOBILES)} Legacy NPCs!")

        # TODO: Shops and guilds

    async def load_zones(self):
        z_dir = self.path / "zones"

        for d in [d for d in z_dir.iterdir() if d.is_dir()]:
            await asyncio.sleep(0)
            zf_dir = d / "zone.json"
            if not (zf_dir.exists() and zf_dir.is_file()):
                continue
            if not (j := read_json_file(zf_dir)):
                continue

            ent = deserialize_entity(j)
            vcomp = WORLD.component_for_entity(ent, cm.HasVnum)
            z = WORLD.component_for_entity(ent, cm.Zone)
            z.dir = d
            vn = vcomp.vnum
            adventkai.LEGACY_ZONES[vn] = ent

    async def load_triggers(self):
        for k, v in adventkai.LEGACY_ZONES.items():
            await asyncio.sleep(0)
            z = get_or_emplace(v, cm.Zone)
            zv = get_or_emplace(v, cm.ZoneVnums)
            tf_dir = z.dir / "triggers.json"
            if not (tf_dir.exists() and tf_dir.is_file()):
                continue
            for j in read_json_file(tf_dir):
                ent = deserialize_entity(j)
                vcomp = WORLD.component_for_entity(ent, cm.HasVnum)
                vn = vcomp.vnum

                zv.triggers[vn] = ent
                adventkai.LEGACY_TRIGGERS[vn] = ent

    def _load_vars(self, ent, j):
        if "global_vars" in j:
            h = cm.DgScriptHolder()
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
            z = get_or_emplace(v, cm.Zone)
            zv = get_or_emplace(v, cm.ZoneVnums)
            zn = WORLD.try_component(v, cm.Name)
            rf_dir = z.dir / "rooms.json"
            if not (rf_dir.exists() and rf_dir.is_file()):
                continue
            rj = read_json_file(rf_dir)
            num_rooms = len(rj)
            logging.info(f"Loading Zone {znum+1} of {zone_total} - ({k}): {zn} ({num_rooms} rooms)")
            await _broadcast(f"Loading {num_rooms} rooms from Zone {znum+1} of {zone_total}...")
            for num, j in enumerate(rj):

                ent = deserialize_entity(j)
                vcomp = WORLD.component_for_entity(ent, cm.HasVnum)
                vn = vcomp.vnum
                zv.rooms[vn] = ent
                adventkai.LEGACY_ROOMS[vn] = ent
                self._load_vars(ent, j)


    async def load_objects(self):
        for k, v in adventkai.LEGACY_ZONES.items():
            await asyncio.sleep(0)
            z = get_or_emplace(v, cm.Zone)
            zv = get_or_emplace(v, cm.ZoneVnums)
            of_dir = z.dir / "objects.json"
            if not (of_dir.exists() and of_dir.is_file()):
                continue
            for j in read_json_file(of_dir):
                ent = deserialize_entity(j)
                vcomp = WORLD.component_for_entity(ent, cm.HasVnum)
                vn = vcomp.vnum

                zv.objects[vn] = ent
                adventkai.LEGACY_OBJECTS[vn] = ent

    async def load_mobiles(self):
        for k, v in adventkai.LEGACY_ZONES.items():
            await asyncio.sleep(0)
            z = get_or_emplace(v, cm.Zone)
            zv = get_or_emplace(v, cm.ZoneVnums)
            of_dir = z.dir / "mobiles.json"
            if not (of_dir.exists() and of_dir.is_file()):
                continue
            for j in read_json_file(of_dir):
                ent = deserialize_entity(j)
                vcomp = WORLD.component_for_entity(ent, cm.HasVnum)
                vn = vcomp.vnum

                zv.mobiles[vn] = ent
                adventkai.LEGACY_MOBILES[vn] = ent

    async def load_userdata(self):
        logging.info("Loading Legacy Accounts...")
        await self.load_accounts()
        count = Account.objects.count()
        logging.info(f"Loaded {count} Legacy Accounts from Files!")
        await _broadcast(f"Loaded {count} Legacy Accounts!")

        logging.info("Loading Legacy Player Characters...")
        await self.load_player_characters()
        count = len(WORLD.get_component(cm.PlayerCharacter))
        logging.info(f"Loaded {count} Legacy Player Characters from Files!")
        await _broadcast(f"Loaded {count} Legacy Player characters!")

        logging.info("Saving converted User data...")
        await self.save_userdata()
        logging.info("Saved all User Data complete!")


    async def load_accounts(self):
        a_dir = self.path / "accounts"

        for d in [d for d in a_dir.iterdir() if d.is_file()]:
            await asyncio.sleep(0)
            if not (j := read_json_file(d)):
                continue
            acc = Account.objects.create_user(j.pop("name"), email=j.pop("email", None), password=j.pop("password", None))
            logging.info(f"Loading Legacy User: {acc.username}")
            if acc.username in ("Wayland", "Virtus", "Volund"):
                acc.is_superuser = True
                acc.save()
            self.account_map[j.pop("account_id")] = acc

    def _load_contents(self, data, holder):
        for i in data:
            ent = deserialize_entity(i)
            self.created_entities.add(ent)
            self.legacy.assign_id(ent, "obj")
            WORLD.add_component(ent, cm.IsPersistent())
            api.add_to_inventory(ent, holder)
            if "contents" in i:
                self._load_contents(i.pop("contents"), ent)


    async def load_player_characters(self):
        c_dir = self.path / "characters"
        count = 0
        for d in [d for d in c_dir.iterdir() if d.is_file()]:
            await asyncio.sleep(0)
            if not (j := read_json_file(d)):
                continue
            acc_id = j["player_specials"].pop("account_id")
            acc = self.account_map[acc_id]
            ent = deserialize_entity(j)
            new_id = self.system.assign_id(ent, "pc")
            WORLD.add_component(ent, cm.IsPersistent())
            WORLD.add_component(ent, cm.AccountOwner(account_id=acc.id))
            self.created_entities.add(ent)
            self._load_vars(ent, j)

            if "carrying" in j:
                self._load_contents(j.pop("carrying"), ent)

            for k, v in j.pop("equipment", list()):
                eq_ent = deserialize_entity(v)
                self.created_entities.add(eq_ent)
                self.legacy.assign_id(eq_ent, "obj")
                api.equip_to_entity(eq_ent, ent, k)
                WORLD.add_component(eq_ent, cm.IsPersistent())
                if "contents" in v:
                    self._load_contents(v.pop("contents"), eq_ent)

    async def save_userdata(self):
        for ent, pers in WORLD.get_component(cm.IsPersistent):
            en = save_entity(ent)

            if (ao := WORLD.try_component(ent, cm.AccountOwner)):
                acc = Account.objects.get(id=ao.account_id)
                acc.characters.add(en)
