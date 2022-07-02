from .base import Modifier as _BaseMod
from adventkai.typing import Entity


class _Mutation(_BaseMod):
    category = "Mutations"
    mod_id = -1
    description = ""


class ExtremeSpeed(_Mutation):
    name = "Extreme Speed"
    mod_id = 1
    description = "+30% to Speed Index."


class HealingFactor(_Mutation):
    name = "Healing Factor"
    mod_id = 2
    description = "LF regen refills %12 instead of 5%"


class ExtremeReflexes(_Mutation):
    name = "Extreme Reflexes"
    mod_id = 3
    description = "+10 to parry, block, and dodge. +10 agility at creation."


class Infravision(_Mutation):
    mod_id = 4
    description = "+5 to spot hiding, can see in dark"


class NaturalCamo(_Mutation):
    name = "Natural Camouflage"
    mod_id = 5
    description = "+10 to hide/sneak rolls"


class LimbRegen(_Mutation):
    name = "Limb Regen"
    mod_id = 6
    description = "Limbs regen almost instantly."


class Poisonous(_Mutation):
    mod_id = 7
    description = "Immune to toxins, venomous bite attack."


class RubberyBody(_Mutation):
    mod_id = 8
    name = "Rubbery Body"
    description = "10% of physical dmg to you is reduced and attacker takes that much loss in stamina."


class InnateTelepathy(_Mutation):
    mod_id = 9
    description = "Start with telepathy at SLVL 50"


class NaturalEnergy(_Mutation):
    name = "Natural Energy"
    mod_id = 10
    description = "Get 5% of your ki damage refunded back into your current ki total."