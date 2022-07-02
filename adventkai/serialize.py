import adventkai
from adventkai.typing import Entity
from adventkai import WORLD, COMPONENTS
from adventkai import components as cm
from adventkai.db.entities.models import Module as ModDB, Prototype as ProDB, Entity as EnDB


def serialize_entity(ent: Entity) -> dict:
    data = {}

    for c in WORLD.components_for_entity(ent):
        if not c:
            continue
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


def save_entity(ent: Entity):
    e = WORLD.component_for_entity(ent, cm.EntityID)
    m, created = ModDB.objects.get_or_create(name=e.module_name)
    p, created2 = ProDB.objects.get_or_create(module=m, name=e.prototype)
    data = serialize_entity(ent)
    en, created3 = EnDB.objects.update_or_create(prototype=p, ent_id=e.ent_id, data=data)

    return en