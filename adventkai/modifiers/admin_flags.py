from .base import Modifier as _BaseMod



class _AdminFlag(_BaseMod):
    category = "AdminFlags"
    modifier_id = -1


class TellAll(_AdminFlag):
    modifier_id = 0


class SeeInvisible(_AdminFlag):
    modifier_id = 1


class SeeSecret(_AdminFlag):
    modifier_id = 2


class KnowWeather(_AdminFlag):
    modifier_id = 3


class FullWhere(_AdminFlag):
    modifier_id = 4


class Money(_AdminFlag):
    modifier_id = 5


class EatAnything(_AdminFlag):
    modifier_id = 6


class NoPoison(_AdminFlag):
    modifier_id = 7


class WalkAnywhere(_AdminFlag):
    modifier_id = 8


class NoKeys(_AdminFlag):
    modifier_id = 9


class InstantKill(_AdminFlag):
    modifier_id = 10


class NoSteal(_AdminFlag):
    modifier_id = 11


class TransAll(_AdminFlag):
    modifier_id = 12


class SwitchMortal(_AdminFlag):
    modifier_id = 13


class ForceMass(_AdminFlag):
    modifier_id = 14


class AllHouses(_AdminFlag):
    modifier_id = 15


class NoDamage(_AdminFlag):
    modifier_id = 16


class AllShops(_AdminFlag):
    modifier_id = 17


class CEdit(_AdminFlag):
    modifier_id = 18
