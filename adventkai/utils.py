from snekmud.components import _ModBase
from snekmud import COMPONENTS, WORLD, OPERATIONS
from adventkai import TRAITS


def modifiers_for_entity(ent, check_equipment=True):
    eq = COMPONENTS["Equipment"]
    for comp in WORLD.components_for_entity(ent):
        if isinstance(comp, eq) and check_equipment:
            for x in comp.all():
                for modifier in modifiers_for_entity(x, check_equipment=False):
                    yield modifier
        elif isinstance(comp, _ModBase):
            for mod in comp.all():
                yield mod


def get_trait(ent, stat_name: str):
    if (found := TRAITS["universal"].get(stat_name, None)):
        return found(ent)
    if not (m := WORLD.try_component(ent, COMPONENTS["MetaTypes"])):
        return None
    for t in m.types:
        if (found := TRAITS[t].get(stat_name, None)):
            return found(ent)
    return None
