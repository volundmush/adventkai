from snekmud import WORLD, COMPONENTS, OPERATIONS
from adventkai.utils import modifiers_for_entity, get_stat
from snekmud.utils import get_or_emplace
from mudforge.utils import lazy_property

from .base import BaseHandler, BoundedIntHandler, MinIntHandler


class Level(BoundedIntHandler):
    comp_name = "Level"
    prop_name = "level"
    min_value = 1
    max_value = 100


class Weight(MinIntHandler):
    comp_name = "Physics"
    prop_name = "weight"

    def personal(self):
        bonus, mult = self.get_bonuses()
        return (self.raw() + bonus) * mult

    def total(self) -> float:
        return self.personal() + self.burden()

    def burden(self) -> float:
        return self.equipped() + self.carried()

    def equipped(self) -> float:
        out = 0.0
        if (eq := WORLD.try_component(self.ent, COMPONENTS["Equipment"])):
            for o in eq.all():
                out += get_stat(o, "Weight").total()
        return out

    def carried(self) -> float:
        out = 0.0
        if (inv := WORLD.try_component(self.ent, COMPONENTS["Inventory"])):
            for o in inv.inventory:
                out += get_stat(o, "Weight").total()
        return out