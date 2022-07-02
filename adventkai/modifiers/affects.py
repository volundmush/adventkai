from .base import Modifier as _BaseMod
from adventkai.typing import Entity


class _Affect(_BaseMod):
    category = "AffectFlags"
    mod_id = -1


class DontUse(_Affect):
    mod_id = 0


class Blind(_Affect):
    mod_id = 1


class Invisible(_Affect):
    mod_id = 2


class DetectAlignment(_Affect):
    mod_id = 3


class DetectInvisible(_Affect):
    mod_id = 4


class DetectMagic(_Affect):
    mod_id = 5


class SenseLife(_Affect):
    mod_id = 6


class WaterWalk(_Affect):
    mod_id = 7


class Sanctuary(_Affect):
    mod_id = 8


class Group(_Affect):
    mod_id = 9


class Curse(_Affect):
    mod_id = 10


class Infravision(_Affect):
    mod_id = 11


class Poison(_Affect):
    mod_id = 12


class WeakenedState(_Affect):
    mod_id = 13


class ProtectGood(_Affect):
    mod_id = 14


class Sleep(_Affect):
    mod_id = 15


class NoTrack(_Affect):
    mod_id = 16


class Undead(_Affect):
    mod_id = 17


class Paralyze(_Affect):
    mod_id = 18


class Sneak(_Affect):
    mod_id = 19


class Hide(_Affect):
    mod_id = 20


class Unused21(_Affect):
    mod_id = 21


class Charm(_Affect):
    mod_id = 22


class Flying(_Affect):
    mod_id = 23


class WaterBreath(_Affect):
    mod_id = 24


class Angelic(_Affect):
    mod_id = 25


class Ethereal(_Affect):
    mod_id = 26


class MagicOnly(_Affect):
    mod_id = 27


class NextPartial(_Affect):
    mod_id = 28


class NextNoAction(_Affect):
    mod_id = 29


class Stunned(_Affect):
    mod_id = 30


class Tamed(_Affect):
    mod_id = 31


class CreepingDeath(_Affect):
    mod_id = 32


class Spirit(_Affect):
    mod_id = 33


class StoneSkin(_Affect):
    mod_id = 34


class Summoned(_Affect):
    mod_id = 35


class Celestial(_Affect):
    mod_id = 36


class Fiendish(_Affect):
    mod_id = 37


class FireShield(_Affect):
    mod_id = 38


class LowLight(_Affect):
    mod_id = 39


class Zanzoken(_Affect):
    mod_id = 40


class KnockedOut(_Affect):
    mod_id = 41


class Might(_Affect):
    mod_id = 42


class Flex(_Affect):
    mod_id = 43


class Genisu(_Affect):
    mod_id = 44


class Bless(_Affect):
    mod_id = 45


class Burnt(_Affect):
    mod_id = 46


class Burned(_Affect):
    mod_id = 47


class MBreak(_Affect):
    mod_id = 48


class Hasshuken(_Affect):
    mod_id = 49


class FutureSight(_Affect):
    mod_id = 50


class RealParalyze(_Affect):
    mod_id = 51


class Infuse(_Affect):
    mod_id = 52


class Enlighten(_Affect):
    mod_id = 53


class Frozen(_Affect):
    mod_id = 54


class FireShield2(_Affect):
    mod_id = 55


class Ensnared(_Affect):
    mod_id = 56


class Hayasa(_Affect):
    mod_id = 57


class Pursuit(_Affect):
    mod_id = 58


class Wither(_Affect):
    mod_id = 59


class HydroZap(_Affect):
    mod_id = 60


class Position(_Affect):
    mod_id = 61


class Shocked(_Affect):
    mod_id = 62


class Metamorph(_Affect):
    mod_id = 63


class HealGlow(_Affect):
    mod_id = 64


class EtherealArmor(_Affect):
    mod_id = 65


class EtherealChains(_Affect):
    mod_id = 66


class Wunjo(_Affect):
    mod_id = 67


class Purisaz(_Affect):
    mod_id = 78


class Ashed(_Affect):
    mod_id = 69


class Puked(_Affect):
    mod_id = 70


class Liquefied(_Affect):
    mod_id = 71


class Shell(_Affect):
    mod_id = 72


class Immunity(_Affect):
    mod_id = 73


class SpiritControl(_Affect):
    mod_id = 74