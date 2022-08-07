from .base import Modifier as _BaseMod


class _MobFlag(_BaseMod):
    category = "MobFlags"
    modifier_id = -1


class Spec(_MobFlag):
    modifier_id = 0


class Sentinel(_MobFlag):
    modifier_id = 1


class NoScavenger(_MobFlag):
    modifier_id = 2


class IsNPC(_MobFlag):
    modifier_id = 3


class Aware(_MobFlag):
    modifier_id = 4


class Aggressive(_MobFlag):
    modifier_id = 5


class StayZone(_MobFlag):
    modifier_id = 6


class Wimpy(_MobFlag):
    modifier_id = 7


class AggrEvil(_MobFlag):
    modifier_id = 8


class AggrGood(_MobFlag):
    modifier_id = 9


class AggrNeutral(_MobFlag):
    modifier_id = 10


class Memory(_MobFlag):
    modifier_id = 11


class Helper(_MobFlag):
    modifier_id = 12


class NoCharm(_MobFlag):
    modifier_id = 13


class NoSummon(_MobFlag):
    modifier_id = 14


class NoSleep(_MobFlag):
    modifier_id = 15


class AutoBalance(_MobFlag):
    modifier_id = 16


class NoBlind(_MobFlag):
    modifier_id = 17


class NoKill(_MobFlag):
    modifier_id = 18


class NotDeadYet(_MobFlag):
    modifier_id = 19


class Mountable(_MobFlag):
    modifier_id = 20


class RightArm(_MobFlag):
    modifier_id = 21


class LeftArm(_MobFlag):
    modifier_id = 22


class RightLeg(_MobFlag):
    modifier_id = 23


class LeftLeg(_MobFlag):
    modifier_id = 24


class Head(_MobFlag):
    modifier_id = 25


class JustDesc(_MobFlag):
    modifier_id = 26


class Husk(_MobFlag):
    modifier_id = 27


class Spar(_MobFlag):
    modifier_id = 28


class Dummy(_MobFlag):
    modifier_id = 29


class AbsorbModel(_MobFlag):
    modifier_id = 30


class RepairModel(_MobFlag):
    modifier_id = 31


class NoPoison(_MobFlag):
    modifier_id = 32


class KnowKaioken(_MobFlag):
    modifier_id = 33


class PoweringUp(_MobFlag):
    modifier_id = 34