from .base import Modifier as _BaseMod
from adventkai.typing import Entity, Vnum


class _Position(_BaseMod):
    category = "position"
    mod_id = -1


class Dead(_Position):
    mod_id = 0


class MortallyWounded(_Position):
    name = "Mortally Wounded"
    mod_id = 1


class Incapacitated(_Position):
    mod_id = 2


class Stunned(_Position):
    mod_id = 3


class Sleeping(_Position):
    mod_id = 4


class Resting(_Position):
    mod_id = 5


class Sitting(_Position):
    mod_id = 6


class Fighting(_Position):
    mod_id = 7


class Standing(_Position):
    mod_id = 8