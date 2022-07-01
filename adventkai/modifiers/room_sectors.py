from .base import Modifier as _BaseMod
from adventkai.typing import Entity


class _RoomSector(_BaseMod):
    category = "room_sectors"
    mod_id = -1


class Inside(_RoomSector):
    mod_id = 0

    def is_providing_light(self, ent: Entity) -> bool:
        return True


class City(Inside):
    mod_id = 1


class Field(_RoomSector):
    mod_id = 2


class Forest(_RoomSector):
    mod_id = 3


class Hills(_RoomSector):
    mod_id = 4


class Mountain(_RoomSector):
    mod_id = 5


class WaterSwim(_RoomSector):
    mod_id = 6


class WaterNoSwim(_RoomSector):
    mod_id = 7


class Flying(_RoomSector):
    mod_id = 8


class Underwater(_RoomSector):
    mod_id = 9


class Shop(_RoomSector):
    mod_id = 10


class Important(_RoomSector):
    mod_id = 11


class Desert(_RoomSector):
    mod_id = 12


class Space(_RoomSector):
    mod_id = 13


class Lava(_RoomSector):
    mod_id = 14
