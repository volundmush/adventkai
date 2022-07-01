from .base import Modifier as _BaseMod
from adventkai.typing import Entity, Vnum
from adventkai import WORLD
from adventkai.components import Physiology as _phys


class _Sensei(_BaseMod):
    category = "sensei"
    mod_id = -1
    pc_ok = False
    abbr = "--"
    arts_name = "Like a Bum"
    location = 300
    home_room = 300

    def can_pick(self, ent: Entity) -> bool:
        return True

    def rpp_cost(self, ent: Entity) -> int:
        return 0


class Commoner(_Sensei):
    mod_id = 255


class _PC(_Sensei):
    pc_ok = True

    def can_pick(self, ent: Entity) -> bool:
        r = WORLD.get_component(ent, _phys)
        return str(r.race) != "Android"


class Roshi(_PC):
    mod_id = 0
    abbr = "Ro"
    arts_name = "Kame Arts"
    location = 1131
    home_room = 1130


class Piccolo(_PC):
    mod_id = 1
    abbr = "Pi"
    arts_name = "Demon Taijutsu"
    location = 1662
    home_room = 1659


class Krane(_PC):
    mod_id = 2
    abbr = "Kr"
    arts_name = "Crane Arts"
    location = 13012
    home_room = 13009


class Nail(_PC):
    mod_id = 3
    abbr = "Na"
    arts_name = "Tranquil Palm"
    location = 11683
    home_room = 11683


class Bardock(_PC):
    mod_id = 4
    abbr = "Ba"
    arts_name = "Brutal Beast"
    location = 2267
    home_room = 2268

    def provides_gravity_tolerance(self, ent: Entity) -> int:
        return 10


class Ginyu(_PC):
    mod_id = 5
    abbr = "Gi"
    arts_name = "Flaunted Style"
    location = 4290
    home_room = 4289


class Frieza(_PC):
    mod_id = 6
    abbr = "Fr"
    arts_name = "Frozen Fist"
    location = 4283
    home_room = 4282


class Tapion(_PC):
    mod_id = 7
    abbr = "Ta"
    arts_name = "Shadow Grappling"
    location = 8233
    home_room = 8231


class Sixteen(_PC):
    mod_id = 8
    abbr = "16"
    arts_name = "Iron Hand"
    location = 1714
    home_room = 1713

    def can_pick(self, ent: Entity) -> bool:
        r = WORLD.get_component(ent, _phys)
        return str(r.race) == "Android"


class Dabura(_PC):
    mod_id = 9
    abbr = "Da"
    arts_name = "Devil Dance"
    location = 6487
    home_room = 6486

    def can_pick(self, ent: Entity) -> bool:
        r = WORLD.get_component(ent, _phys)
        return str(r.race) == "Demon"


class Kibito(_PC):
    mod_id = 10
    abbr = "Ki"
    arts_name = "Gentle Fist"
    location = 12098
    home_room = 12098

    def rpp_cost(self, ent: Entity) -> int:
        r = WORLD.get_component(ent, _phys)
        if str(r.race) != "Kai":
            return 10
        return 0


class Jinto(_PC):
    mod_id = 11
    abbr = "Ji"
    arts_name = "Star's Radiance"
    location = 3499
    home_room = 3499

    def can_pick(self, ent: Entity) -> bool:
        r = WORLD.get_component(ent, _phys)
        return str(r.race) == "Hoshijin"


class Tsuna(_PC):
    mod_id = 12
    abbr = "Ts"
    arts_name = "Sacred Tsunami"
    location = 15009
    home_room = 15009

    def can_pick(self, ent: Entity) -> bool:
        r = WORLD.get_component(ent, _phys)
        return str(r.race) == "Kanassan"


class Kurzak(_PC):
    mod_id = 13
    abbr = "Ku"
    arts_name = "Adaptive Taijutsu"
    location = 16100
    home_room = 16100

    def can_pick(self, ent: Entity) -> bool:
        r = WORLD.get_component(ent, _phys)
        return str(r.race) == "Arlian"