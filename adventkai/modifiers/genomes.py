from .base import Modifier as _BaseMod
from adventkai.typing import Entity


class _Genome(_BaseMod):
    category = "Genomes"
    mod_id = -1
    description = ""


class Human(_Genome):
    mod_id = 1
    description = "Higher PS gains from fighting."


class Saiyan(_Genome):
    mod_id = 2
    description = "Saiyan Blood gains, halved"


class Namek(_Genome):
    mod_id = 3
    description = "No food needed."


class Icer(_Genome):
    mod_id = 4
    description = "+20% damage for Tier 4 Attacks"


class Tuffle(_Genome):
    mod_id = 5
    description = "Bonus to skill Auto-Train."


class Arlian(_Genome):
    mod_id = 6
    description = "Gains Arlian Adrenaline ability."


class Kai(_Genome):
    mod_id = 7
    description = "Start with Telepathy and Focus at SLVL 30"


class Konatsu(_Genome):
    mod_id = 8
    description = "+40% higher chance to multihit on physical attacks."
