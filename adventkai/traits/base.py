from snekmud import WORLD, COMPONENTS, OPERATIONS
from adventkai.utils import modifiers_for_entity
from snekmud.utils import get_or_emplace
from mudforge.utils import lazy_property


class BaseHandler:

    def __init__(self, ent):
        self.ent = ent


class CompHandler(BaseHandler):
    comp_name = None
    prop_name = None

    @lazy_property
    def comp(self):
        return get_or_emplace(self.ent, COMPONENTS[self.comp_name])

    def raw(self):
        return getattr(self.comp, self.prop_name)

    def raw_set(self, value):
        setattr(self.comp, self.prop_name, value)
        return value


class PercentHandler(CompHandler):

    def set(self, value: float) -> float:
        return self.raw_set(max(0.0, min(1.0, value)))

    def mod(self, value: float) -> float:
        return self.set(self.raw() + value)


class IntHandler(CompHandler):

    def set(self, value: int) -> int:
        return self.raw_set(value)

    def mod(self, value: int) -> int:
        return self.set(self.raw() + value)

    def mult(self) -> float:
        out = 1.0
        for m in modifiers_for_entity(self.ent):
            out += m.stat_multiplier(self.ent, self.prop_name)
        return out

    def bonus(self) -> int:
        out = 0
        for m in modifiers_for_entity(self.ent):
            out += m.stat_bonus(self.ent, self.prop_name)
        return out

    def get_bonuses(self) -> (int, float):
        bonus = 0
        mult = 1.0
        for m in modifiers_for_entity(self.ent):
            bonus += m.stat_bonus(self.ent, self.prop_name)
            mult += m.stat_multiplier(self.ent, self.prop_name)
        return (bonus, mult)

    def effective(self) -> int:
        bonus, mult = self.get_bonuses()
        return round((self.raw() + bonus) * mult)


class MaxIntHandler(IntHandler):
    max_value = None

    def set(self, value: int) -> int:
        return self.raw_set(min(self.max_value, value))


class MinIntHandler(IntHandler):
    min_value = None

    def set(self, value: int) -> int:
        return self.raw_set(max(self.min_value, value))


class BoundedIntHandler(IntHandler):
    min_value = None
    max_value = None

    def set(self, value: int) -> int:
        return self.raw_set(max(self.min_value, min(self.max_value, value)))
