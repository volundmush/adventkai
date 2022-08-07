from .base import Modifier as _BaseMod


class _AndroidType(_BaseMod):
    category = "AndroidType"
    modifier_id = -1
    description = ""


class Absorb(_AndroidType):
    modifier_id = 1


class Repair(_AndroidType):
    modifier_id = 2


class Sense(_AndroidType):
    modifier_id = 3