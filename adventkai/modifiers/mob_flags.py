from .base import Modifier as _BaseMod
from adventkai.typing import Entity


class _MobFlag(_BaseMod):
    category = "mob_flags"
    mod_id = -1


class Spec(_MobFlag):
    mod_id = 0


class Sentinel(_MobFlag):
    mod_id = 1


class NoScavenger(_MobFlag):
    mod_id = 2


class IsNPC(_MobFlag):
    mod_id = 3


class Aware(_MobFlag):
    mod_id = 4


class Aggressive(_MobFlag):
    mod_id = 5


class StayZone(_MobFlag):
    mod_id = 6


class Wimpy(_MobFlag):
    mod_id = 7


class AggrEvil(_MobFlag):
    mod_id = 8


class AggrGood(_MobFlag):
    mod_id = 9


class AggrNeutral(_MobFlag):
    mod_id = 10


class Memory(_MobFlag):
    mod_id = 11


class Helper(_MobFlag):
    mod_id = 12


class NoCharm(_MobFlag):
    mod_id = 13


class NoSummon(_MobFlag):
    mod_id = 14


class NoSleep(_MobFlag):
    mod_id = 15


class AutoBalance(_MobFlag):
    mod_id = 16


class NoBlind(_MobFlag):
    mod_id = 17


class NoKill(_MobFlag):
    mod_id = 18


class NotDeadYet(_MobFlag):
    mod_id = 19


class Mountable(_MobFlag):
    mod_id = 20


class RightArm(_MobFlag):
    mod_id = 21


class LeftArm(_MobFlag):
    mod_id = 22


class RightLeg(_MobFlag):
    mod_id = 23


class LeftLeg(_MobFlag):
    mod_id = 24


class Head(_MobFlag):
    mod_id = 25


class JustDesc(_MobFlag):
    mod_id = 26


class Husk(_MobFlag):
    mod_id = 27


class Spar(_MobFlag):
    mod_id = 28


class Dummy(_MobFlag):
    mod_id = 29


class AbsorbModel(_MobFlag):
    mod_id = 30


class RepairModel(_MobFlag):
    mod_id = 31


class NoPoison(_MobFlag):
    mod_id = 32


class KnowKaioken(_MobFlag):
    mod_id = 33


class PoweringUp(_MobFlag):
    mod_id = 34