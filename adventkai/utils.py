from snekmud.components import _SingleModifier
from adventkai.components import FlagBase
from snekmud import COMPONENTS, WORLD, OPERATIONS
from adventkai import STATS


def modifiers_for_entity(ent, check_equipment=True):
    eq = COMPONENTS["Equipment"]
    for comp in WORLD.components_for_entity(ent):
        if isinstance(comp, eq) and check_equipment:
            for x in eq.equipment.all():
                for modifier in modifiers_for_entity(x, check_equipment=False):
                    yield modifier
        elif isinstance(comp, (_SingleModifier, FlagBase)):
            for mod in comp.all():
                yield mod


def get_stat(ent, stat_name: str):
    if (found := STATS["universal"].get(stat_name, None)):
        return found(ent)
    if not (m := WORLD.try_component(ent, COMPONENTS["MetaTypes"])):
        return None
    for t in m.types:
        if (found := STATS[t].get(stat_name, None)):
            return found(ent)
    return None
