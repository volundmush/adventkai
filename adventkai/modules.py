import sys
from pathlib import Path
import asyncio
from adventkai.db.entities.models import PlayerCharacter
from adventkai.typing import Entity
from mudforge.utils import generate_name
from adventkai import components as cm
from adventkai.utils import read_data_file
from adventkai.serialize import deserialize_entity
from adventkai import WORLD
import adventkai
import logging


class Prototype:

    def __init__(self, module, name, ent: Entity):
        self.module = module
        self.name = name
        self.entities = dict()
        self.ent = ent


class Module:

    def __init__(self, name: str, path: Path, save_path: Path):
        self.name = sys.intern(name)
        self.maps: dict[str, Entity] = dict()
        self.prototypes: dict[str, Prototype] = dict()
        self.entities: dict[str, Entity] = dict()
        self.path = path
        self.save_path = save_path

    def __str__(self):
        return self.name


    async def load_maps(self):
        m_dir = self.path / "maps"
        if not m_dir.exists():
            return

        if not m_dir.is_dir():
            return

        for d in [d for d in m_dir.iterdir() if d.is_file()]:
            key, ext = d.name.split(".", 1)
            data = read_data_file(d)
            if not data:
                continue
            map_ent = WORLD.create_entity()
            WORLD.add_component(map_ent, cm.Name(key))
            self.maps[key] = map_ent
            continue

            grid = cm.GridMap()
            for d in data:
                if "Coordinates" not in data:
                    continue
                coordinates = data.pop("Coordinates")
                room_ent = deserialize_entity(data)
                grid.rooms.add(cm.PointHolder(coordinates, room_ent))

    async def load_prototypes(self):
        p_dir = self.path / "prototypes"
        if not p_dir.exists():
            return

        if not p_dir.is_dir():
            return

        for d in [d for d in p_dir.iterdir() if d.is_file()]:
            key, ext = d.name.split(".", 1)
            data = read_data_file(d)
            if not data:
                continue
            p_ent = deserialize_entity(data)
            WORLD.add_component(p_ent, cm.Prototype(name=key))
            self.prototypes[key] = Prototype(self, key, p_ent)

    async def load_entities_initial(self):
        pass

    async def load_entities_finalize(self):
        pass

    def assign_id(self, ent: Entity, proto: str, index: bool = True):
        p = self.prototypes[proto]
        p_ent = p.ent
        new_id = generate_name(proto, p.entities.keys())
        WORLD.add_component(ent, cm.EntityID(module_name=self.name, prototype=proto, ent_id=new_id))
        if index:
            self.entities[new_id] = ent
            p.entities[new_id] = ent