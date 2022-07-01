from .components import Inventory, InInventory, InRoom, InSpace, Equipment, Equipped
from .components import RoomFlags, SectorType, ItemType, ItemValues

from .typing import Entity
from adventkai import WORLD
from .utils import get_or_emplace
import typing
from collections.abc import Iterable


def add_to_inventory(ent: typing.Union[Entity, Iterable[Entity]], dest: Entity, comp = InInventory):
    c = get_or_emplace(dest, Inventory)
    if not isinstance(ent, Iterable):
        ent = [ent]
    c.inventory.extend(ent)
    for e in ent:
        WORLD.add_component(e, comp(holder=dest))


def remove_from_inventory(ent: Entity, comp = InInventory):
    if (i := WORLD.get_component(ent, comp)):
        c = get_or_emplace(i.holder, Inventory)
        c.inventory.remove(ent)
        if not c.inventory:
            WORLD.remove_component(i.holder, Inventory)
        WORLD.remove_component(ent, comp)


def equip_to_entity(ent: Entity, dest: Entity, slot: int):
    e = get_or_emplace(dest, Equipment)
    e.equipment[slot] = ent
    WORLD.add_component(ent, Equipped(holder=dest, slot=slot))


def unequipFromEntity(ent: Entity):
    if (i := WORLD.get_component(ent, Equipped)):
        e = WORLD.get_component(i.holder)
        e.equipment.pop(i.slot, None)
        if not e.equipment:
            WORLD.remove_component(i.holder, Equipment)
        WORLD.remove_component(ent, Equipped)


def add_to_room(ent: typing.Union[Entity, Iterable[Entity]], dest: Entity, comp = InRoom):
    add_to_inventory(ent, dest, comp)


def remove_from_room(ent: Entity, comp = InRoom):
    remove_from_inventory(ent, comp)


def dump_inventory(ent: Entity, comp = InInventory) -> list[Entity]:
    i = get_or_emplace(ent, Inventory)
    WORLD.remove_component(ent, Inventory)
    for e in i.inventory:
        WORLD.remove_component(e, comp)
    return i.inventory


def dump_room(ent: Entity, comp = InRoom) -> list[Entity]:
    return dump_inventory(ent, comp)


def dump_equipment(ent: Entity) -> list[Entity]:
    i = get_or_emplace(ent, Equipment)
    for k, v in i.equipment.items():
        WORLD.remove_component(v, Equipped)
    WORLD.remove(ent, Equipment)
    return list(i.equipment.values())


def is_providing_light(ent: Entity) -> bool:
    it = WORLD.try_component(ent, ItemType)
    if it and it.type_flag == 1:
        i = WORLD.try_component(ent, ItemValues)
        if i and i.values.get(2, 0):
            return True

    rf = WORLD.try_component(ent, RoomFlags)
    dark = False
    if rf:
        for f in rf.flags:
            if f.is_providing_light(ent):
                return True
            if f.is_providing_darkness(ent):
                dark = True

    s = WORLD.try_component(ent, SectorType)
    if s:
        if s.is_providing_light(ent) and not dark:
            return True

    e = WORLD.try_component(ent, Equipment)
    if e:
        for eq in e.equipment.values():
            if is_providing_light(eq):
                return True

    return True


def is_illuminated(ent: Entity) -> bool:
    """
    Returns true if a room is lit enough to see in it.
    """
    if is_providing_light(ent):
        return True

    i = WORLD.try_component(ent, Inventory)
    if i:
        for e in i.inventory:
            if is_providing_light(e):
                return True
    return False