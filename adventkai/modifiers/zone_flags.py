from .base import Modifier as _BaseMod
from adventkai.typing import Entity


class _ZoneFlag(_BaseMod):
    category = "ZoneFlags"
    mod_id = -1


class Closed(_ZoneFlag):
    mod_id = 0


class NoImmortal(_ZoneFlag):
    mod_id = 1


class Quest(_ZoneFlag):
    mod_id = 2


class DragonBalls(_ZoneFlag):
    mod_id = 3
