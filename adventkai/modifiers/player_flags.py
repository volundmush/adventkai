from .base import Modifier as _BaseMod
from adventkai.typing import Entity


class _PlayerFlag(_BaseMod):
    category = "PlayerFlags"
    mod_id = -1


class Killer(_PlayerFlag):
    mod_id = 0


class Thief(_PlayerFlag):
    mod_id = 1


class Frozen(_PlayerFlag):
    mod_id = 2


class DontSet(_PlayerFlag):
    mod_id = 3


class Writing(_PlayerFlag):
    mod_id = 4


class Mailing(_PlayerFlag):
    mod_id = 5


class Crash(_PlayerFlag):
    mod_id = 6


class SiteOK(_PlayerFlag):
    mod_id = 7


class NoShout(_PlayerFlag):
    mod_id = 8


class NoTitle(_PlayerFlag):
    mod_id = 9


class Deleted(_PlayerFlag):
    mod_id = 10


class LoadRoom(_PlayerFlag):
    mod_id = 11


class NoWizList(_PlayerFlag):
    mod_id = 12


class NoDelete(_PlayerFlag):
    mod_id = 13


class InvisStart(_PlayerFlag):
    mod_id = 14


class Cryo(_PlayerFlag):
    mod_id = 15


class NotDeadYet(_PlayerFlag):
    mod_id = 16


class AgeMidG(_PlayerFlag):
    mod_id = 17


class AgeMidB(_PlayerFlag):
    mod_id = 18


class AgeOldG(_PlayerFlag):
    mod_id = 19


class AgeOldB(_PlayerFlag):
    mod_id = 20


class AgeEvenG(_PlayerFlag):
    mod_id = 21


class AgeEvenB(_PlayerFlag):
    mod_id = 22


class OldAge(_PlayerFlag):
    mod_id = 23


class RightArm(_PlayerFlag):
    mod_id = 24


class LeftArm(_PlayerFlag):
    mod_id = 25


class RightLeg(_PlayerFlag):
    mod_id = 26


class LeftLeg(_PlayerFlag):
    mod_id = 27


class Head(_PlayerFlag):
    mod_id = 28


class SaiyanTail(_PlayerFlag):
    mod_id = 29


class Tail(_PlayerFlag):
    mod_id = 30


class Piloting(_PlayerFlag):
    mod_id = 31


class SkilPoints(_PlayerFlag):
    mod_id = 32


class Spar(_PlayerFlag):
    mod_id = 33


class Charge(_PlayerFlag):
    mod_id = 34


class Trans1(_PlayerFlag):
    mod_id = 35


class Trans2(_PlayerFlag):
    mod_id = 36


class Trans3(_PlayerFlag):
    mod_id = 37


class Trans4(_PlayerFlag):
    mod_id = 38


class Trans5(_PlayerFlag):
    mod_id = 39


class Trans6(_PlayerFlag):
    mod_id = 40


class AbsorbModel(_PlayerFlag):
    mod_id = 41


class RepairModel(_PlayerFlag):
    mod_id = 42


class SenseModel(_PlayerFlag):
    mod_id = 43


class PoweringUp(_PlayerFlag):
    mod_id = 44


class KnockedOut(_PlayerFlag):
    mod_id = 45


class CyberRightArm(_PlayerFlag):
    mod_id = 46


class CyberLeftArm(_PlayerFlag):
    mod_id = 47


class CyberRightLeg(_PlayerFlag):
    mod_id = 48


class CyberLeftLeg(_PlayerFlag):
    mod_id = 49


class FullPowerSSJ(_PlayerFlag):
    mod_id = 50


class Immortal(_PlayerFlag):
    mod_id = 51


class EyesClosed(_PlayerFlag):
    mod_id = 52


class Disguised(_PlayerFlag):
    mod_id = 53


class Bandaged(_PlayerFlag):
    mod_id = 54


class PotentialReleased(_PlayerFlag):
    mod_id = 55


class HealTank(_PlayerFlag):
    mod_id = 56


class Fury(_PlayerFlag):
    mod_id = 57


class Pose(_PlayerFlag):
    mod_id = 58


class Oozaru(_PlayerFlag):
    mod_id = 59


class Absorbed(_PlayerFlag):
    mod_id = 60


class MultP(_PlayerFlag):
    mod_id = 61


class PDeath(_PlayerFlag):
    mod_id = 62


class ThanDW(_PlayerFlag):
    mod_id = 63


class SelfD(_PlayerFlag):
    mod_id = 64


class SelfD2(_PlayerFlag):
    mod_id = 65


class Spiral(_PlayerFlag):
    mod_id = 66


class BioGR(_PlayerFlag):
    mod_id = 67


class LegendarySSJ(_PlayerFlag):
    mod_id = 68


class RepLearn(_PlayerFlag):
    mod_id = 69


class Forget(_PlayerFlag):
    mod_id = 70


class Transmission(_PlayerFlag):
    mod_id = 71


class Fishing(_PlayerFlag):
    mod_id = 72


class Goop(_PlayerFlag):
    mod_id = 73


class MultiHit(_PlayerFlag):
    mod_id = 74


class AuraLight(_PlayerFlag):
    mod_id = 75


class RDisplay(_PlayerFlag):
    mod_id = 76


class Stolen(_PlayerFlag):
    mod_id = 77


class TailHide(_PlayerFlag):
    mod_id = 78


class NoGrow(_PlayerFlag):
    mod_id = 79

