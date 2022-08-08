from snekmud import WORLD, COMPONENTS, OPERATIONS
from adventkai.utils import modifiers_for_entity, get_stat
from snekmud.utils import get_or_emplace
from mudforge.utils import lazy_property

import sys

from .base import BaseHandler, BoundedIntHandler, MinIntHandler, PercentHandler


class _StatHandler(BoundedIntHandler):
    min_value = 1
    max_value = 80
    comp_name = "Stats"


_me = sys.modules[__name__]
for x in ("Strength", "Intelligence", "Wisdom", "Agility", "Constitution", "Speed", "Luck"):
    setattr(_me, x, type(x, (_StatHandler,), {"prop_name": x.lower()}))
del _me


class _PowerStatHandler(MinIntHandler):
    comp_name = "PowerStats"
    perc_handler = None

    def __init__(self, ent):
        super().__init__(ent)
        self.perc = self.perc_handler(ent)

    def current(self) -> int:
        perc = self.perc.get()
        return round(self.effective() * perc)

    def mod_current(self, value: int):
        """
        This will adjust the underlying percent.
        """


class Suppress(BoundedIntHandler):
    comp_name = "Suppress"
    prop_name = "suppressed"
    min_value = 1
    max_value = 100


class MaxCarry(BaseHandler):

    def calculate(self, exist_value=None) -> float:
        if exist_value is None:
            exist_value = get_stat(self.ent, "PowerLevel").effective()
        return (exist_value / 200.0) + (get_stat(self.ent, "Strength").get() * 50.0)


class Speednar(BaseHandler):

    def calculate(self, exist_value=None) -> float:
        if exist_value is None:
            exist_value = get_stat(self.ent, "PowerLevel").effective()
        ratio = get_stat(self.ent, "Weight").burden() - get_stat(self.ent, "MaxCarry").calculate(exist_value=exist_value)
        if ratio >= .05:
            return max(0.01, min(1.0, 1.0-ratio))
        return 1.0


class Stamina(_PowerStatHandler):
    perc_handler = type("StaminaPercent", (PercentHandler,), {"comp_name": "Health", "prop_name": "stamina"})
    prop_name = "stamina"


class Ki(_PowerStatHandler):
    perc_handler = type("KiPercent", (PercentHandler,), {"comp_name": "Health", "prop_name": "energy"})
    prop_name = "stamina"


class PowerLevel(_PowerStatHandler):
    perc_handler = type("HealthPercent", (PercentHandler,), {"comp_name": "Health", "prop_name": "health"})
    prop_name = "power"

    def current(self) -> int:
        perc = self.perc.get()
        eff = self.effective_max()
        return eff * min(get_stat(self.ent, "Suppress").get(), perc)

    def effective_max(self) -> int:
        eff = self.effective()
        speednar = get_stat(self.ent, "Speednar").calculate(exist_value=eff)
        return eff * speednar