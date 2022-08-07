from .base import Modifier as _BaseMod


class _RoomSector(_BaseMod):
    category = "SectorType"
    modifier_id = -1
    map_key = "o"
    map_name = None

    @classmethod
    def get_map_name(cls):
        if cls.map_name:
            return cls.map_name
        return cls.get_name()


class Inside(_RoomSector):
    modifier_id = 0
    map_key = "|xI|n"

    def is_providing_light(self, obj) -> bool:
        return True


class City(Inside):
    modifier_id = 1
    map_key = "|wC|n"


class Field(_RoomSector):
    modifier_id = 2
    map_key = "|gP|n"
    map_name = "Plain"


class Forest(_RoomSector):
    modifier_id = 3
    map_key = "|GF|n"


class Hills(_RoomSector):
    modifier_id = 4
    map_key = "|YH|n"


class Mountain(_RoomSector):
    modifier_id = 5
    map_key = "|XM|n"


class WaterSwim(_RoomSector):
    modifier_id = 6
    map_key = "|C~|n"
    map_name = "Shallow Water"


class WaterNoSwim(_RoomSector):
    modifier_id = 7
    map_key = "|bW|n"
    map_name = "Water"


class Flying(_RoomSector):
    modifier_id = 8
    map_key = "|cS|n"
    map_name = "Sky"


class Underwater(_RoomSector):
    modifier_id = 9
    map_key = "|BU|n"


class Shop(_RoomSector):
    modifier_id = 10
    map_key = "|M$|n"


class Important(_RoomSector):
    modifier_id = 11
    map_key = "|m#|n"


class Desert(_RoomSector):
    modifier_id = 12
    map_key = "|yD|n"


class Space(_RoomSector):
    modifier_id = 13


class Lava(_RoomSector):
    modifier_id = 14
    map_key = "|rL|n"
