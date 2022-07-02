from .base import Modifier as _BaseMod
from adventkai.typing import Entity


class _ItemFlag(_BaseMod):
    category = "ItemFlags"
    mod_id = -1


class Glow(_ItemFlag):
    mod_id = 0


class Hum(_ItemFlag):
    mod_id = 1


class NoRent(_ItemFlag):
    mod_id = 2


class NoDonate(_ItemFlag):
    mod_id = 3


class NoInvis(_ItemFlag):
    mod_id = 4


class Invisible(_ItemFlag):
    mod_id = 5


class Magic(_ItemFlag):
    mod_id = 6


class NoDrop(_ItemFlag):
    mod_id = 7


class Bless(_ItemFlag):
    mod_id = 8


class AntiGood(_ItemFlag):
    mod_id = 9


class AntiEvil(_ItemFlag):
    mod_id = 10


class AntiNeutral(_ItemFlag):
    mod_id = 11


class AntiWizard(_ItemFlag):
    mod_id = 12


class AntiCleric(_ItemFlag):
    mod_id = 13


class AntiRogue(_ItemFlag):
    mod_id = 14


class AntiFighter(_ItemFlag):
    mod_id = 15


class NoSell(_ItemFlag):
    mod_id = 16


class AntiDruid(_ItemFlag):
    mod_id = 17


class TwoHander(_ItemFlag):
    mod_id = 18


class AntiBard(_ItemFlag):
    mod_id = 19


class AntiRanger(_ItemFlag):
    mod_id = 20


class AntiPaladin(_ItemFlag):
    mod_id = 21


class AntiHuman(_ItemFlag):
    mod_id = 22


class AntiIcer(_ItemFlag):
    mod_id = 23


class AntiSaiyan(_ItemFlag):
    mod_id = 24


class AntiKonatsu(_ItemFlag):
    mod_id = 25


class UniqueSave(_ItemFlag):
    mod_id = 26


class Broken(_ItemFlag):
    mod_id = 27


class Unbreakable(_ItemFlag):
    mod_id = 28


class AntiMonk(_ItemFlag):
    mod_id = 29


class AntiBarbarian(_ItemFlag):
    mod_id = 30


class AntiSorcerer(_ItemFlag):
    mod_id = 31


class Double(_ItemFlag):
    mod_id = 32


class OnlyWizard(_ItemFlag):
    mod_id = 33


class OnlyCleric(_ItemFlag):
    mod_id = 34


class OnlyRogue(_ItemFlag):
    mod_id = 35


class OnlyFighter(_ItemFlag):
    mod_id = 36


class OnlyDruid(_ItemFlag):
    mod_id = 37


class OnlyBard(_ItemFlag):
    mod_id = 38


class OnlyRanger(_ItemFlag):
    mod_id = 39


class OnlyPaladin(_ItemFlag):
    mod_id = 40


class OnlyHuman(_ItemFlag):
    mod_id = 41


class OnlyIcer(_ItemFlag):
    mod_id = 42


class OnlySaiyan(_ItemFlag):
    mod_id = 43


class OnlyKonatsu(_ItemFlag):
    mod_id = 44


class OnlyMonk(_ItemFlag):
    mod_id = 45


class OnlyBarbarian(_ItemFlag):
    mod_id = 46


class OnlySorcerer(_ItemFlag):
    mod_id = 47


class AntiArcaneArcher(_ItemFlag):
    mod_id = 48


class AntiArcaneTrickster(_ItemFlag):
    mod_id = 49


class AntiArchmage(_ItemFlag):
    mod_id = 50


class AntiAssassin(_ItemFlag):
    mod_id = 51


class AntiBlackguard(_ItemFlag):
    mod_id = 52


class AntiDragonDisciple(_ItemFlag):
    mod_id = 53


class AntiDuelist(_ItemFlag):
    mod_id = 54


class AntiDwarvenDefender(_ItemFlag):
    mod_id = 55


class AntiEldritchKnight(_ItemFlag):
    mod_id = 56


class AntiHierophant(_ItemFlag):
    mod_id = 57


class AntiHorizonWalker(_ItemFlag):
    mod_id = 58


class AntiLoremaster(_ItemFlag):
    mod_id = 59


class AntiMysticTheurge(_ItemFlag):
    mod_id = 60


class AntiShadowmancer(_ItemFlag):
    mod_id = 61


class AntiThaumaturgist(_ItemFlag):
    mod_id = 62


class BScouter(_ItemFlag):
    mod_id = 63


class MScouter(_ItemFlag):
    mod_id = 64


class AScouter(_ItemFlag):
    mod_id = 65


class UScouter(_ItemFlag):
    mod_id = 66

class WeapLvl1(_ItemFlag):
    mod_id = 67


class WeapLvl2(_ItemFlag):
    mod_id = 68


class WeapLvl3(_ItemFlag):
    mod_id = 69


class Weaplvl4(_ItemFlag):
    mod_id = 70


class WeapLvl5(_ItemFlag):
    mod_id = 71


class CBoard(_ItemFlag):
    mod_id = 72


class Forged(_ItemFlag):
    mod_id = 73


class OnlyJinto(_ItemFlag):
    mod_id = 74


class Buried(_ItemFlag):
    mod_id = 76


class Slot1(_ItemFlag):
    mod_id = 77


class Slot2(_ItemFlag):
    mod_id = 78


class Token(_ItemFlag):
    mod_id = 79


class SlotOne(_ItemFlag):
    mod_id = 80


class SlotsFilled(_ItemFlag):
    mod_id = 81


class Restring(_ItemFlag):
    mod_id = 82


class Custom(_ItemFlag):
    mod_id = 83


class NoRandom(_ItemFlag):
    mod_id = 85


class Throw(_ItemFlag):
    mod_id = 86


class Hot(_ItemFlag):
    mod_id = 87


class Purge(_ItemFlag):
    mod_id = 88


class Ice(_ItemFlag):
    mod_id = 89


class Duplicate(_ItemFlag):
    mod_id = 90


class Mature(_ItemFlag):
    mod_id = 91


class CardCase(_ItemFlag):
    mod_id = 92


class NoPickUp(_ItemFlag):
    mod_id = 93


class NoSteal(_ItemFlag):
    mod_id = 94