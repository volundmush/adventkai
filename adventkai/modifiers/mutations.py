from .base import Modifier as _BaseMod


class _Mutation(_BaseMod):
    category = "Mutations"
    modifier_id = -1
    description = ""


class ExtremeSpeed(_Mutation):
    name = "Extreme Speed"
    modifier_id = 1
    description = "+30% to Speed Index."


class HealingFactor(_Mutation):
    name = "Healing Factor"
    modifier_id = 2
    description = "LF regen refills %12 instead of 5%"


class ExtremeReflexes(_Mutation):
    name = "Extreme Reflexes"
    modifier_id = 3
    description = "+10 to parry, block, and dodge. +10 agility at creation."


class Infravision(_Mutation):
    modifier_id = 4
    description = "+5 to spot hiding, can see in dark"


class NaturalCamo(_Mutation):
    name = "Natural Camouflage"
    modifier_id = 5
    description = "+10 to hide/sneak rolls"


class LimbRegen(_Mutation):
    name = "Limb Regen"
    modifier_id = 6
    description = "Limbs regen almost instantly."


class Poisonous(_Mutation):
    modifier_id = 7
    description = "Immune to toxins, venomous bite attack."


class RubberyBody(_Mutation):
    modifier_id = 8
    name = "Rubbery Body"
    description = "10% of physical dmg to you is reduced and attacker takes that much loss in stamina."


class InnateTelepathy(_Mutation):
    modifier_id = 9
    description = "Start with telepathy at SLVL 50"


class NaturalEnergy(_Mutation):
    name = "Natural Energy"
    modifier_id = 10
    description = "Get 5% of your ki damage refunded back into your current ki total."