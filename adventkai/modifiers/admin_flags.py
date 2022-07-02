from .base import Modifier as _BaseMod
from adventkai.typing import Entity


class _AdminFlag(_BaseMod):
    category = "AdminFlags"
    mod_id = -1


class TellAll(_AdminFlag):
    mod_id = 0


class SeeInvisible(_AdminFlag):
    mod_id = 1


class SeeSecret(_AdminFlag):
    mod_id = 2


class KnowWeather(_AdminFlag):
    mod_id = 3


class FullWhere(_AdminFlag):
    mod_id = 4


class Money(_AdminFlag):
    mod_id = 5


class EatAnything(_AdminFlag):
    mod_id = 6


class NoPoison(_AdminFlag):
    mod_id = 7


class WalkAnywhere(_AdminFlag):
    mod_id = 8


class NoKeys(_AdminFlag):
    mod_id = 9


class InstantKill(_AdminFlag):
    mod_id = 10


class NoSteal(_AdminFlag):
    mod_id = 11


class TransAll(_AdminFlag):
    mod_id = 12


class SwitchMortal(_AdminFlag):
    mod_id = 13


class ForceMass(_AdminFlag):
    mod_id = 14


class AllHouses(_AdminFlag):
    mod_id = 15


class NoDamage(_AdminFlag):
    mod_id = 16


class AllShops(_AdminFlag):
    mod_id = 17


class CEdit(_AdminFlag):
    mod_id = 18
