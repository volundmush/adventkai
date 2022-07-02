from pathlib import Path
import orjson
import mudforge
import adventkai
import logging
from adventkai import WORLD
from mudforge import NET_CONNECTIONS
from .utils import get_or_emplace
from adventkai import components as cm
import asyncio
from adventkai.serialize import serialize_entity, deserialize_entity

from adventkai.db.accounts.models import Account


def _read_json(p: Path):
    return orjson.loads(open(p, mode='rb').read())


async def _broadcast(s: str):
    for k, v in NET_CONNECTIONS.items():
        await v.send_line(s)


class LegacyLoader:

    def __init__(self, p: Path):
        self.path = p
        self.account_map = dict()

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
            if not (j := _read_json(zf_dir)):
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
            for j in _read_json(tf_dir):
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
            rj = _read_json(rf_dir)
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
            for j in _read_json(of_dir):
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
            for j in _read_json(of_dir):
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
        await _broadcast(f"Loading {count} Legacy Accounts!")

    async def load_accounts(self):
        a_dir = self.path / "accounts"

        for d in [d for d in a_dir.iterdir() if d.is_file()]:
            await asyncio.sleep(0)
            if not (j := _read_json(d)):
                continue
            acc = Account.objects.create_user(j.pop("name"), email=j.pop("email", None), password=j.pop("password", None))
            logging.info(f"Loading Legacy User: {acc.username}")
            if acc.username in ("Wayland", "Virtus", "Volund"):
                acc.is_superuser = True
                acc.save()
            self.account_map[j.pop("account_id")] = acc


    async def load_player_characters(self):
        c_dir = self.path / "characters"

        for d in [d for d in c_dir.iterdir() if d.is_file()]:
            await asyncio.sleep(0)
            if not (j := _read_json(d)):
                continue
            acc_id = j["player_specials"].pop("account_id")
            acc = self.account_map[acc_id]
            ent = deserialize_entity(j)
            p_id = WORLD.component_for_entity(ent, cm.PlayerCharacter)
            player_id = p_id.player_id
            adventkai.PLAYER_ID[player_id] = ent
            self._load_vars(ent, j)


