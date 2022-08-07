from .base import Modifier as _BaseMod


class _ItemType(_BaseMod):
    category = "ItemType"
    modifier_id = -1


class Light(_ItemType):
    modifier_id = 1


class Scroll(_ItemType):
    modifier_id = 2


class Wand(_ItemType):
    modifier_id = 3


class Staff(_ItemType):
    modifier_id = 4


class Weapon(_ItemType):
    modifier_id = 5


class FireWeapon(_ItemType):
    modifier_id = 6


class CampFire(_ItemType):
    modifier_id = 7


class Treasure(_ItemType):
    modifier_id = 8


class Armor(_ItemType):
    modifier_id = 9


class Potion(_ItemType):
    modifier_id = 10


class Worn(_ItemType):
    modifier_id = 11


class Other(_ItemType):
    modifier_id = 12


class Trash(_ItemType):
    modifier_id = 13


class Trap(_ItemType):
    modifier_id = 14


class Container(_ItemType):
    modifier_id = 15


class Note(_ItemType):
    modifier_id = 16


class DrinkContainer(_ItemType):
    modifier_id = 17


class Key(_ItemType):
    modifier_id = 18


class Food(_ItemType):
    modifier_id = 19


class Money(_ItemType):
    modifier_id = 20


class Pen(_ItemType):
    modifier_id = 21


class Boat(_ItemType):
    modifier_id = 22


class Fountain(_ItemType):
    modifier_id = 23


class Vehicle(_ItemType):
    modifier_id = 24


class Hatch(_ItemType):
    modifier_id = 25


class Window(_ItemType):
    modifier_id = 26


class Control(_ItemType):
    modifier_id = 27


class Portal(_ItemType):
    modifier_id = 28


class SpellBook(_ItemType):
    modifier_id = 29


class Board(_ItemType):
    modifier_id = 30


class Chair(_ItemType):
    modifier_id = 31


class Bed(_ItemType):
    modifier_id = 32


class Yum(_ItemType):
    modifier_id = 33


class Plant(_ItemType):
    modifier_id = 34


class FishPole(_ItemType):
    modifier_id = 35


class FishBait(_ItemType):
    modifier_id = 36