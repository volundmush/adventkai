from .base import Modifier as _BaseMod


class _Affect(_BaseMod):
    category = "AffectFlags"
    modifier_id = -1


class DontUse(_Affect):
    modifier_id = 0


class Blind(_Affect):
    modifier_id = 1


class Invisible(_Affect):
    modifier_id = 2


class DetectAlignment(_Affect):
    modifier_id = 3


class DetectInvisible(_Affect):
    modifier_id = 4


class DetectMagic(_Affect):
    modifier_id = 5


class SenseLife(_Affect):
    modifier_id = 6


class WaterWalk(_Affect):
    modifier_id = 7


class Sanctuary(_Affect):
    modifier_id = 8


class Group(_Affect):
    modifier_id = 9


class Curse(_Affect):
    modifier_id = 10


class Infravision(_Affect):
    modifier_id = 11


class Poison(_Affect):
    modifier_id = 12


class WeakenedState(_Affect):
    modifier_id = 13


class ProtectGood(_Affect):
    modifier_id = 14


class Sleep(_Affect):
    modifier_id = 15


class NoTrack(_Affect):
    modifier_id = 16


class Undead(_Affect):
    modifier_id = 17


class Paralyze(_Affect):
    modifier_id = 18


class Sneak(_Affect):
    modifier_id = 19


class Hide(_Affect):
    modifier_id = 20


class Unused21(_Affect):
    modifier_id = 21


class Charm(_Affect):
    modifier_id = 22


class Flying(_Affect):
    modifier_id = 23


class WaterBreath(_Affect):
    modifier_id = 24


class Angelic(_Affect):
    modifier_id = 25


class Ethereal(_Affect):
    modifier_id = 26


class MagicOnly(_Affect):
    modifier_id = 27


class NextPartial(_Affect):
    modifier_id = 28


class NextNoAction(_Affect):
    modifier_id = 29


class Stunned(_Affect):
    modifier_id = 30


class Tamed(_Affect):
    modifier_id = 31


class CreepingDeath(_Affect):
    modifier_id = 32


class Spirit(_Affect):
    modifier_id = 33


class StoneSkin(_Affect):
    modifier_id = 34


class Summoned(_Affect):
    modifier_id = 35


class Celestial(_Affect):
    modifier_id = 36


class Fiendish(_Affect):
    modifier_id = 37


class FireShield(_Affect):
    modifier_id = 38


class LowLight(_Affect):
    modifier_id = 39


class Zanzoken(_Affect):
    modifier_id = 40


class KnockedOut(_Affect):
    modifier_id = 41


class Might(_Affect):
    modifier_id = 42


class Flex(_Affect):
    modifier_id = 43


class Genisu(_Affect):
    modifier_id = 44


class Bless(_Affect):
    modifier_id = 45


class Burnt(_Affect):
    modifier_id = 46


class Burned(_Affect):
    modifier_id = 47


class MBreak(_Affect):
    modifier_id = 48


class Hasshuken(_Affect):
    modifier_id = 49


class FutureSight(_Affect):
    modifier_id = 50


class RealParalyze(_Affect):
    modifier_id = 51


class Infuse(_Affect):
    modifier_id = 52


class Enlighten(_Affect):
    modifier_id = 53


class Frozen(_Affect):
    modifier_id = 54


class FireShield2(_Affect):
    modifier_id = 55


class Ensnared(_Affect):
    modifier_id = 56


class Hayasa(_Affect):
    modifier_id = 57


class Pursuit(_Affect):
    modifier_id = 58


class Wither(_Affect):
    modifier_id = 59


class HydroZap(_Affect):
    modifier_id = 60


class Position(_Affect):
    modifier_id = 61


class Shocked(_Affect):
    modifier_id = 62


class Metamorph(_Affect):
    modifier_id = 63


class HealGlow(_Affect):
    modifier_id = 64


class EtherealArmor(_Affect):
    modifier_id = 65


class EtherealChains(_Affect):
    modifier_id = 66


class Wunjo(_Affect):
    modifier_id = 67


class Purisaz(_Affect):
    modifier_id = 78


class Ashed(_Affect):
    modifier_id = 69


class Puked(_Affect):
    modifier_id = 70


class Liquefied(_Affect):
    modifier_id = 71


class Shell(_Affect):
    modifier_id = 72


class Immunity(_Affect):
    modifier_id = 73


class SpiritControl(_Affect):
    modifier_id = 74