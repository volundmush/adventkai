from .base import Modifier as _BaseMod


class _Bonus(_BaseMod):
    category = "Bonuses"
    modifier_id = -1
    cost = 0
    description = ""
    is_flaw = False
    excludes = ()


class Thrifty(_Bonus):
    modifier_id = 0
    cost = 2
    description = "-10% Shop Buy Cost and +10% Shop Sell Cost"
    excludes = (28,)


class Prodigy(_Bonus):
    modifier_id = 1
    cost = 5
    description = "+25% Experience Gained Until Level 80"
    excludes = (40,)


class QuickStudy(_Bonus):
    name = "Quick Study"
    modifier_id = 2
    cost = 3
    description = "Character auto-trains skills faster"


class DieHard(_Bonus):
    name = "Die Hard"
    modifier_id = 3
    cost = 6
    description = "Life Force's PL regen doubled, but cost is the same"


class Brawler(_Bonus):
    modifier_id = 4
    cost = 4
    description = "Physical attacks do 20% more damage"


class Destroyer(_Bonus):
    modifier_id = 5
    cost = 3
    description = "Damaged Rooms act as regen rooms for you"


class HardWorker(_Bonus):
    name = "Hard Worker"
    modifier_id = 6
    cost = 3
    description = "Physical rewards better + activity drains less stamina"
    excludes = (39,)


class Healer(_Bonus):
    modifier_id = 7
    cost = 3
    description = "Heal/First-aid/Vigor/Repair restore +10%"


class Loyal(_Bonus):
    modifier_id = 8
    cost = 2
    description = "+20% Experience When Grouped As Follower"
    excludes = (50,)


class Brawny(_Bonus):
    modifier_id = 9
    cost = 5
    description = "Strength gains +2 every 10 levels, Train STR + 75%"
    excludes = (43,)


class Scholarly(_Bonus):
    modifier_id = 10
    cost = 5
    description = "Intelligence gains +2 every 10 levels, Train INT + 75%"
    excludes = (44,)


class Sage(_Bonus):
    modifier_id = 11
    cost = 5
    description = "Wisdom gains +2 every 10 levels, Train WIS + 75%"
    excludes = (45, )


class Agile(_Bonus):
    modifier_id = 12
    cost = 4
    description = "Agility gains +2 every 10 levels, Train AGL + 75%"
    excludes = (46,)


class Quick(_Bonus):
    modifier_id = 13
    cost = 6
    description = "Speed gains +2 every 10 levels, Train SPD + 75%"
    excludes = (47,)


class Sturdy(_Bonus):
    modifier_id = 14
    cost = 5
    description = "Constitution +2 every 10 levels, Train CON + 75%"
    excludes = (48,)


class ThickSkin(_Bonus):
    name = "Thick Skin"
    modifier_id = 15
    cost = 5
    description = "-20% Physical and -10% ki dmg received"
    excludes = (33, )


class IronChef(_Bonus):
    name = "Iron Chef"
    modifier_id = 16
    cost = 2
    description = "Food cooked by you lasts longer/heals better"


class Fireproof(_Bonus):
    modifier_id = 17
    cost = 4
    description = "-50% Fire Dmg taken, -10% ki, immunity to burn"
    excludes = (34,)


class Powerhitter(_Bonus):
    modifier_id = 18
    cost = 4
    description = "15% critical hits will be x4 instead of x2"


class Healthy(_Bonus):
    modifier_id = 19
    cost = 3
    description = "40% chance to recover from ill effects when sleeping"
    excludes = (20, 29)


class Insomniac(_Bonus):
    modifier_id = 20
    cost = 2
    description = "Can't Sleep. Immune to yoikominminken and paralysis"
    excludes = (27, 19)


class Evasive(_Bonus):
    modifier_id = 21
    cost = 3
    description = "+15% to dodge rolls"
    excludes = (30,)


class TheWall(_Bonus):
    name = "The Wall"
    modifier_id = 22
    cost = 3
    description = "+20% chance to block"
    excludes = (31, )


class Accurate(_Bonus):
    modifier_id = 23
    cost = 4
    description = "+20% chance to hit physical, +10% to hit with ki"
    excludes = (32,)


class EnergyLeech(_Bonus):
    modifier_id = 24
    cost = 5
    description = "(lvl/5)*+2% ki dmg res, if sufficient charge capacity."
    excludes = (35,)


class GoodMemory(_Bonus):
    name = "Good Memory"
    modifier_id = 25
    cost = 6
    description = "+2 Skill Slots initially, +1 every 20 levels after"
    excludes = (51,)


class _Flaw(_Bonus):
    is_flaw = True


class SoftTouch(_Flaw):
    name = "Soft Touch"
    modifier_id = 26
    cost = 5
    description = "Half Damage for all hit locations"


class LateSleeper(_Flaw):
    name = "Late Sleeper"
    modifier_id = 27
    description = "Can only wake automatically. 33% every hour if maxed"
    cost = 5
    excludes = (20,)


class ImpulseShopper(_Flaw):
    name = "Impulse Shopper"
    modifier_id = 28
    description = "+25% shop costs"
    cost = 3
    excludes = (0,)


class Sickly(_Flaw):
    modifier_id = 29
    cost = 5
    description = "Suffer from harmful effects longer"
    excludes = (19,)


class PunchingBag(_Flaw):
    name = "Punching Bag"
    modifier_id = 30
    cost = 3
    description = "-15% to dodge rolls"
    excludes = (21,)


class Pushover(_Flaw):
    name = "Pushover"
    modifier_id = 31
    cost = 3
    description = "-20% block chance"


class Inaccurate(_Flaw):
    modifier_id = 32
    description = "-20% hit chance with physical, -10% with ki"
    cost = 4
    excludes = (23,)


class ThinSkin(_Flaw):
    modifier_id = 33
    cost = 4
    description = "+20% physical and +10% ki damage received"


class Fireprone(_Flaw):
    modifier_id = 34
    cost = 5
    description = "+50% Fire Dmg taken, +10% ki, always burned"
    excludes = (17,)


class EnergyIntensive(_Flaw):
    modifier_id = 35
    name = "Energy Intensive"
    cost = 6
    description = "(lvl/5)*-2% ki damage, 10% chance of charge backlash."
    excludes = {24,}


class Coward(_Flaw):
    modifier_id = 36
    cost = 6
    description = "Can't attack enemy with 150%+ your powerlevel."


class Arrogant(_Flaw):
    modifier_id = 37
    cost = 1
    description = "Cannot suppress"


class Unfocused(_Flaw):
    modifier_id = 38
    cost = 3
    description = "Charge concentration randomly breaks"


class Slacker(_Flaw):
    modifier_id = 39
    cost = 3
    excludes = (6,)
    description = "Physical activity drains more stamina"


class SlowLearner(_Flaw):
    modifier_id = 40
    name = "Slow Learner"
    excludes = (2,)


class Masochistic(_Flaw):
    modifier_id = 41
    cost = 5
    description = "Defense Skills cap at 75"


class Mute(_Flaw):
    modifier_id = 42
    cost = 4
    description = "Can't use IC speech related commands"


class Wimp(_Flaw):
    modifier_id = 43
    cost = 6
    description = "Strength is capped at 45"
    excludes = (5,)


class Dull(_Flaw):
    modifier_id = 44
    cost = 6
    description = "Intelligence is capped at 45"
    excludes = (10,)


class Foolish(_Flaw):
    modifier_id = 45
    cost = 6
    description = "Wisdom is capped at 45"
    excludes = (11,)


class Clumsy(_Flaw):
    modifier_id = 46
    cost = 3
    description = "Agility is capped at 45"
    excludes = (12,)


class Slow(_Flaw):
    modifier_id = 47
    cost = 6
    description = "Speed is capped at 45"
    excludes = (13,)


class Frail(_Flaw):
    modifier_id = 48
    cost = 4
    excludes = (14,)
    description = "Constitution is capped at 45"


class Sadistic(_Flaw):
    modifier_id = 49
    cost = 3
    description = "Half experience gained for quick skills."


class Loner(_Flaw):
    modifier_id = 50
    cost = 2
    excludes = (8,)
    description = "Can't group, +5% train gains, +10% to physical gains"


class BadMemory(_Flaw):
    modifier_id = 51
    cost = 6
    excludes = (25,)
    description = "-5 Skill Slots"
