from .base import Modifier as _BaseMod
from adventkai.typing import Entity


class _WearFlag(_BaseMod):
    category = "admin_flags"
    mod_id = -1



class Take(_WearFlag):
    mod_id = 0


class Finger(_WearFlag):
    mod_id = 1


class Neck(_WearFlag):
    mod_id = 2


class Body(_WearFlag):
    mod_id = 3


class Head(_WearFlag):
    mod_id = 4


class Legs(_WearFlag):
    mod_id = 5


class Feet(_WearFlag):
    mod_id = 6


class Hands(_WearFlag):
    mod_id = 7


class Arms(_WearFlag):
    mod_id = 8


class Shield(_WearFlag):
    mod_id = 9


class About(_WearFlag):
    mod_id = 10


class Waist(_WearFlag):
    mod_id = 11


class Wrist(_WearFlag):
    mod_id = 12


class Wield(_WearFlag):
    mod_id = 13


class Hold(_WearFlag):
    mod_id = 14


class Pack(_WearFlag):
    mod_id = 15


class Ear(_WearFlag):
    mod_id = 16


class Wings(_WearFlag):
    mod_id = 17


class Eye(_WearFlag):
    mod_id = 18