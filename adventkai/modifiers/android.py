from .base import Modifier as _BaseMod
from adventkai.typing import Entity


class _AndroidType(_BaseMod):
    category = "AndroidType"
    mod_id = -1
    description = ""


class Absorb(_AndroidType):
    mod_id = 1


class Repair(_AndroidType):
    mod_id = 2


class Sense(_AndroidType):
    mod_id = 3