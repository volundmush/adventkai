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


def _read_json(p: Path):
    return orjson.loads(open(p, mode='rb').read())


async def _broadcast(s: str):
    for k, v in NET_CONNECTIONS.items():
        await v.send_line(s)


class LegacyLoader:

    def __init__(self, p: Path):
        self.path = p

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
            ent = WORLD.create_entity()
            vn = j.pop("number")
            if "name" in j:
                WORLD.add_component(ent, cm.Name(j.pop("name")))
            WORLD.add_component(ent, cm.HasVnum(vnum=vn))
            if "builders" in j:
                j["legacy_builders"] = j.pop("builders").split()
            if "cmd" in j:
                j["commands"] = j.pop("cmd")
            j["dir"] = d

            self._load_modifiers(ent, j, "zone_flags", "zone_flags", cm.ZoneFlags)

            WORLD.add_component(ent, cm.Zone.from_dict(j))
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
                ent = WORLD.create_entity()
                vn = j.pop("nr")
                zv.triggers[vn] = ent
                adventkai.LEGACY_TRIGGERS[vn] = ent

                WORLD.add_component(ent, cm.HasVnum(vnum=vn))
                WORLD.add_component(ent, cm.Name(j.pop("name")))
                WORLD.add_component(ent, cm.DgScriptProto.from_dict(j))

    def _load_modifiers(self, ent, j, j_field, category, component):
        if j_field in j:
            choices = adventkai.MODIFIERS_ID[category]
            cmp = component()
            for i in j.pop(j_field):
                if i in choices:
                    cmp.flags[str(choices[i])] = choices[i]
            if cmp.flags:
                WORLD.add_component(ent, cmp)

    def _load_exdesc(self, ent, j):
        if "ex_description" in j:
            exd = get_or_emplace(ent, cm.ExDescriptions)
            for edesc in j.pop("ex_description"):
                exd.ex_descriptions.append((edesc[0], edesc[1]))

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

                ent = WORLD.create_entity()
                vn = j.pop("number")
                zv.rooms[vn] = ent
                adventkai.LEGACY_ROOMS[vn] = ent

                if "name" in j:
                    WORLD.add_component(ent, cm.Name(j.pop("name")))
                if "description" in j:
                    WORLD.add_component(ent, cm.Description(j.pop("description")))

                self._load_modifiers(ent, j, "room_flags", "room_flags", cm.RoomFlags)

                if "sector_type" in j:
                    sect = adventkai.MODIFIERS_ID["room_sectors"]
                    if (found := sect.get(j.pop("sector_type"), None)):
                        WORLD.add_component(ent, cm.SectorType(sector=found))

                self._load_triggers(ent, j)

                if "dir_option" in j:
                    ex = get_or_emplace(ent, cm.Exits)
                    for e in j.pop("dir_option"):
                        di = cm.ExitDir(e[0])
                        if not e[1]:
                            continue
                        edata = e[1]
                        if "general_description" in edata:
                            edata["description"] = edata.pop("general_description")
                        if "failroom" in edata:
                            edata["fail_room"] = edata.pop("failroom")
                        if "totalfailroom" in edata:
                            edata["total_fail_room"] = edata.pop("totalfailroom")
                        ex.exits[di] = cm.RoomExit.from_dict(edata)

                self._load_exdesc(ent, j)

    def _load_physics(self, ent, j):
        physics = dict()
        if "size" in j:
            physics["size"] = j.pop("size")
        if "weight" in j:
            physics["weight"] = j.pop("weight")
        if "height" in j:
            physics["height"] = j.pop("height")

        if physics:
            WORLD.add_component(ent, cm.Physics.from_dict(physics))

    def _load_triggers(self, ent, j):
        if "proto_script" in j:
            trig = get_or_emplace(ent, cm.Triggers)
            for i in j.pop("proto_script"):
                trig.triggers.append(i)

    def _load_object_base(self, ent, j):
        if "name" in j:
            WORLD.add_component(ent, cm.Name(j.pop("name")))
        if "description" in j:
            WORLD.add_component(ent, cm.Description(j.pop("description")))
        if "short_description" in j:
            WORLD.add_component(ent, cm.ShortDescription(j.pop("short_description")))

        self._load_physics(ent, j)

        if "type_flag" in j:
            tflags = adventkai.MODIFIERS_ID["item_types"]
            if (found := tflags.get(j.pop("type_flag"), None)):
                WORLD.add_component(ent, cm.ItemType(type_flag=found))

        self._load_modifiers(ent, j, "wear_flags", "wear_flags", cm.WearFlags)
        self._load_modifiers(ent, j, "extra_flags", "item_flags", cm.ItemFlags)

        if "value" in j:
            iv = get_or_emplace(ent, cm.ItemValues)
            for i, v in j.pop("value"):
                iv.values[i] = v

    async def load_objects(self):
        for k, v in adventkai.LEGACY_ZONES.items():
            await asyncio.sleep(0)
            z = get_or_emplace(v, cm.Zone)
            zv = get_or_emplace(v, cm.ZoneVnums)
            of_dir = z.dir / "objects.json"
            if not (of_dir.exists() and of_dir.is_file()):
                continue
            for j in _read_json(of_dir):
                ent = WORLD.create_entity()
                vn = j.pop("item_number")
                zv.objects[vn] = ent
                adventkai.LEGACY_OBJECTS[vn] = ent
                self._load_object_base(ent, j)
                self._load_triggers(ent, j)

    def _load_race(self, ent, j):
        if "race" in j:
            races = adventkai.MODIFIERS_ID["race"]
            if (found := races.get(j.pop("race"), None)):
                WORLD.add_component(ent, cm.Race(race=found))

    def _load_sensei(self, ent, j):
        if "chclass" in j:
            races = adventkai.MODIFIERS_ID["sensei"]
            if (found := races.get(j.pop("chclass"), None)):
                WORLD.add_component(ent, cm.Sensei(sensei=found))

    def _load_stats(self, ent, j):
        if "real_abils" in j:
            s = j.pop("real_abils")
            stats = cm.Stats()
            stats.strength = s["str"]
            stats.constitution = s["con"]
            stats.agility = s["dex"]
            stats.intelligence = s["intel"]
            stats.wisdom = s["wis"]
            stats.speed = s["cha"]
            WORLD.add_component(ent, stats)

    def _load_char_strings(self, ent, j):
        if "name" in j:
            WORLD.add_component(ent, cm.Name(j.pop("name")))
        if "description" in j:
            WORLD.add_component(ent, cm.Description(j.pop("description")))
        if "short_descr" in j:
            WORLD.add_component(ent, cm.ShortDescription(j.pop("short_descr")))
        if "long_descr" in j:
            WORLD.add_component(ent, cm.LongDescription(j.pop("long_descr")))

    def _load_physiology(self, ent, j):
        phy = cm.Physiology()
        if "sex" in j:
            phy.sex = j.pop("sex")
        if "hairl" in j:
            phy.hair_length = j.pop("hairl")
        if "hairs" in j:
            phy.hair_style = j.pop("hairs")
        if "hairc" in j:
            phy.hair_color = j.pop("hairc")
        if "skin" in j:
            phy.skin_color = j.pop("skin")
        if "eye" in j:
            phy.eye_color = j.pop("eye")
        if "aura" in j:
            phy.aura_color = j.pop("aura")
        if "distfea" in j:
            phy.distinguishing_feature = j.pop("distfea")
        WORLD.add_component(ent, phy)

    def _load_power(self, ent, j):
        p = cm.PowerStats()
        if "basepl" in j:
            p.power = j.pop("basepl")
        if "baseki" in j:
            p.ki = j.pop("baseki")
        if "basest" in j:
            p.stamina = j.pop("basest")
        WORLD.add_component(ent, p)

    def _load_character_base(self, ent, j):
        self._load_char_strings(ent, j)
        self._load_physics(ent, j)
        self._load_triggers(ent, j)
        self._load_stats(ent, j)
        self._load_modifiers(ent, j, "affected_by", "affects", cm.AffectFlags)
        self._load_race(ent, j)
        self._load_sensei(ent, j)
        self._load_physiology(ent, j)
        self._load_power(ent, j)

    async def load_mobiles(self):
        for k, v in adventkai.LEGACY_ZONES.items():
            await asyncio.sleep(0)
            z = get_or_emplace(v, cm.Zone)
            zv = get_or_emplace(v, cm.ZoneVnums)
            of_dir = z.dir / "mobiles.json"
            if not (of_dir.exists() and of_dir.is_file()):
                continue
            for j in _read_json(of_dir):
                ent = WORLD.create_entity()
                vn = j.pop("nr")
                zv.mobiles[vn] = ent
                adventkai.LEGACY_MOBILES[vn] = ent
                self._load_character_base(ent, j)
                self._load_modifiers(ent, j, "act", "mob_flags", cm.MobFlags)

    async def load_userdata(self):
        pass
