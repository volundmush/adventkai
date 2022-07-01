from .base import Modifier as _BaseMod
from adventkai.typing import Entity


class _ItemType(_BaseMod):
    category = "item_types"
    mod_id = -1


class Light(_ItemType):
    mod_id = 1


class Scroll(_ItemType):
    mod_id = 2


class Wand(_ItemType):
    mod_id = 3


class Staff(_ItemType):
    mod_id = 4


class Weapon(_ItemType):
    mod_id = 5


class FireWeapon(_ItemType):
    mod_id = 6


class CampFire(_ItemType):
    mod_id = 7


class Treasure(_ItemType):
    mod_id = 8


class Armor(_ItemType):
    mod_id = 9


class Potion(_ItemType):
    mod_id = 10


class Worn(_ItemType):
    mod_id = 11


class Other(_ItemType):
    mod_id = 12


class Trash(_ItemType):
    mod_id = 13


class Trap(_ItemType):
    mod_id = 14


class Container(_ItemType):
    mod_id = 15


class Note(_ItemType):
    mod_id = 16


class DrinkContainer(_ItemType):
    mod_id = 17


class Key(_ItemType):
    mod_id = 18


class Food(_ItemType):
    mod_id = 19


class Money(_ItemType):
    mod_id = 20


class Pen(_ItemType):
    mod_id = 21


class Boat(_ItemType):
    mod_id = 22


class Fountain(_ItemType):
    mod_id = 23


class Vehicle(_ItemType):
    mod_id = 24


class Hatch(_ItemType):
    mod_id = 25


class Window(_ItemType):
    mod_id = 26


class Control(_ItemType):
    mod_id = 27


class Portal(_ItemType):
    mod_id = 28


class SpellBook(_ItemType):
    mod_id = 29


class Board(_ItemType):
    mod_id = 30


class Chair(_ItemType):
    mod_id = 31


class Bed(_ItemType):
    mod_id = 32


class Yum(_ItemType):
    mod_id = 33


class Plant(_ItemType):
    mod_id = 34


class FishPole(_ItemType):
    mod_id = 35


class FishBait(_ItemType):
    mod_id = 36