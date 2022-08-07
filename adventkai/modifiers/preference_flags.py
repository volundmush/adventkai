from .base import Modifier as _BaseMod


class _PreferenceFlag(_BaseMod):
    category = "PreferenceFlags"
    modifier_id = -1


class Brief(_PreferenceFlag):
    modifier_id = 0

class Compact(_PreferenceFlag):
    modifier_id = 1


class Deaf(_PreferenceFlag):
    modifier_id = 2


class NoTell(_PreferenceFlag):
    modifier_id = 3


class DisplayHP(_PreferenceFlag):
    modifier_id = 4


class DisplayKI(_PreferenceFlag):
    modifier_id = 5


class DisplayST(_PreferenceFlag):
    modifier_id = 6


class AutoExit(_PreferenceFlag):
    modifier_id = 7


class NoHassle(_PreferenceFlag):
    modifier_id = 8


class Quest(_PreferenceFlag):
    modifier_id = 9


class Summonable(_PreferenceFlag):
    modifier_id = 10


class NoRepeat(_PreferenceFlag):
    modifier_id = 11


class HolyLight(_PreferenceFlag):
    modifier_id = 12


class Color(_PreferenceFlag):
    modifier_id = 13


class Spare(_PreferenceFlag):
    modifier_id = 14


class NoWiz(_PreferenceFlag):
    modifier_id = 15


class Log1(_PreferenceFlag):
    modifier_id = 16


class Log2(_PreferenceFlag):
    modifier_id = 17


class NoAuction(_PreferenceFlag):
    modifier_id = 18


class NoGossip(_PreferenceFlag):
    modifier_id = 19


class NoGratz(_PreferenceFlag):
    modifier_id = 20


class RoomFlags(_PreferenceFlag):
    modifier_id = 21


class DisplayAuto(_PreferenceFlag):
    modifier_id = 22


class ClearScreen(_PreferenceFlag):
    modifier_id = 23


class BuildWalk(_PreferenceFlag):
    modifier_id = 24


class AFK(_PreferenceFlag):
    modifier_id = 25


class AutoLoot(_PreferenceFlag):
    modifier_id = 26


class AutoGold(_PreferenceFlag):
    modifier_id = 27


class AutoSplit(_PreferenceFlag):
    modifier_id = 28


class FullExit(_PreferenceFlag):
    modifier_id = 29


class AutoSacrifice(_PreferenceFlag):
    modifier_id = 30


class AutoMemorize(_PreferenceFlag):
    modifier_id = 31


class ViewOrder(_PreferenceFlag):
    modifier_id = 32


class NoCompress(_PreferenceFlag):
    modifier_id = 33


class AutoAssist(_PreferenceFlag):
    modifier_id = 34


class DisplayKIOld(_PreferenceFlag):
    modifier_id = 35


class DisplayExp(_PreferenceFlag):
    modifier_id = 36


class DisplayTNL(_PreferenceFlag):
    modifier_id = 37


class Test(_PreferenceFlag):
    modifier_id = 38


class Hide(_PreferenceFlag):
    modifier_id = 39


class NoMailWarning(_PreferenceFlag):
    modifier_id = 40


class Hints(_PreferenceFlag):
    modifier_id = 41


class Fury(_PreferenceFlag):
    modifier_id = 42


class NoDec(_PreferenceFlag):
    modifier_id = 43


class NoEqSee(_PreferenceFlag):
    modifier_id = 44


class NoMusic(_PreferenceFlag):
    modifier_id = 45


class LKeep(_PreferenceFlag):
    modifier_id = 46


class DisplayTime(_PreferenceFlag):
    modifier_id = 47


class DisplayGold(_PreferenceFlag):
    modifier_id = 48


class DisplayPractices(_PreferenceFlag):
    modifier_id = 49


class NoParry(_PreferenceFlag):
    modifier_id = 50


class DisplayHUTH(_PreferenceFlag):
    modifier_id = 51


class DisplayPercent(_PreferenceFlag):
    modifier_id = 52

class Carve(_PreferenceFlag):
    modifier_id = 53


class ArenaWatch(_PreferenceFlag):
    modifier_id = 54


class NoGive(_PreferenceFlag):
    modifier_id = 55


class Instruct(_PreferenceFlag):
    modifier_id = 56


class GHealth(_PreferenceFlag):
    modifier_id = 57


class IHealth(_PreferenceFlag):
    modifier_id = 58


class Energize(_PreferenceFlag):
    modifier_id = 59