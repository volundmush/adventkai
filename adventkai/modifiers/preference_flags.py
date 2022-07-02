from .base import Modifier as _BaseMod
from adventkai.typing import Entity


class _PreferenceFlag(_BaseMod):
    category = "PreferenceFlags"
    mod_id = -1


class Brief(_PreferenceFlag):
    mod_id = 0

class Compact(_PreferenceFlag):
    mod_id = 1


class Deaf(_PreferenceFlag):
    mod_id = 2


class NoTell(_PreferenceFlag):
    mod_id = 3


class DisplayHP(_PreferenceFlag):
    mod_id = 4


class DisplayKI(_PreferenceFlag):
    mod_id = 5


class DisplayST(_PreferenceFlag):
    mod_id = 6


class AutoExit(_PreferenceFlag):
    mod_id = 7


class NoHassle(_PreferenceFlag):
    mod_id = 8


class Quest(_PreferenceFlag):
    mod_id = 9


class Summonable(_PreferenceFlag):
    mod_id = 10


class NoRepeat(_PreferenceFlag):
    mod_id = 11


class HolyLight(_PreferenceFlag):
    mod_id = 12


class Color(_PreferenceFlag):
    mod_id = 13


class Spare(_PreferenceFlag):
    mod_id = 14


class NoWiz(_PreferenceFlag):
    mod_id = 15


class Log1(_PreferenceFlag):
    mod_id = 16


class Log2(_PreferenceFlag):
    mod_id = 17


class NoAuction(_PreferenceFlag):
    mod_id = 18


class NoGossip(_PreferenceFlag):
    mod_id = 19


class NoGratz(_PreferenceFlag):
    mod_id = 20


class RoomFlags(_PreferenceFlag):
    mod_id = 21


class DisplayAuto(_PreferenceFlag):
    mod_id = 22


class ClearScreen(_PreferenceFlag):
    mod_id = 23


class BuildWalk(_PreferenceFlag):
    mod_id = 24


class AFK(_PreferenceFlag):
    mod_id = 25


class AutoLoot(_PreferenceFlag):
    mod_id = 26


class AutoGold(_PreferenceFlag):
    mod_id = 27


class AutoSplit(_PreferenceFlag):
    mod_id = 28


class FullExit(_PreferenceFlag):
    mod_id = 29


class AutoSacrifice(_PreferenceFlag):
    mod_id = 30


class AutoMemorize(_PreferenceFlag):
    mod_id = 31


class ViewOrder(_PreferenceFlag):
    mod_id = 32


class NoCompress(_PreferenceFlag):
    mod_id = 33


class AutoAssist(_PreferenceFlag):
    mod_id = 34


class DisplayKIOld(_PreferenceFlag):
    mod_id = 35


class DisplayExp(_PreferenceFlag):
    mod_id = 36


class DisplayTNL(_PreferenceFlag):
    mod_id = 37


class Test(_PreferenceFlag):
    mod_id = 38


class Hide(_PreferenceFlag):
    mod_id = 39


class NoMailWarning(_PreferenceFlag):
    mod_id = 40


class Hints(_PreferenceFlag):
    mod_id = 41


class Fury(_PreferenceFlag):
    mod_id = 42


class NoDec(_PreferenceFlag):
    mod_id = 43


class NoEqSee(_PreferenceFlag):
    mod_id = 44


class NoMusic(_PreferenceFlag):
    mod_id = 45


class LKeep(_PreferenceFlag):
    mod_id = 46


class DisplayTime(_PreferenceFlag):
    mod_id = 47


class DisplayGold(_PreferenceFlag):
    mod_id = 48


class DisplayPractices(_PreferenceFlag):
    mod_id = 49


class NoParry(_PreferenceFlag):
    mod_id = 50


class DisplayHUTH(_PreferenceFlag):
    mod_id = 51


class DisplayPercent(_PreferenceFlag):
    mod_id = 52

class Carve(_PreferenceFlag):
    mod_id = 53


class ArenaWatch(_PreferenceFlag):
    mod_id = 54


class NoGive(_PreferenceFlag):
    mod_id = 55


class Instruct(_PreferenceFlag):
    mod_id = 56


class GHealth(_PreferenceFlag):
    mod_id = 57


class IHealth(_PreferenceFlag):
    mod_id = 58


class Energize(_PreferenceFlag):
    mod_id = 59