import adventkai
from adventkai.typing import Entity
from adventkai import WORLD, COMPONENTS


def serialize_entity(ent: Entity) -> dict:
    data = {}

    for c in WORLD.components_for_entity(ent):
        if c.should_save():
            data[c.save_name()] = c.export()

    return data


def deserialize_entity(data: dict) -> Entity:
    ent = WORLD.create_entity()

    for k, v in COMPONENTS.items():
        if k not in data:
            continue
        WORLD.add_component(ent, v.deserialize(data.pop(k)))

    return ent
