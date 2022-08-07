from .base import Modifier as _BaseMod


class _ZoneFlag(_BaseMod):
    category = "ZoneFlags"
    modifier_id = -1


class Closed(_ZoneFlag):
    modifier_id = 0


class NoImmortal(_ZoneFlag):
    modifier_id = 1


class Quest(_ZoneFlag):
    modifier_id = 2


class DragonBalls(_ZoneFlag):
    modifier_id = 3
