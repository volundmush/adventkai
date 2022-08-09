from snekmud import getters as old_g
from snekmud import WORLD, COMPONENTS, OPERATIONS, GETTERS
from snekmud.typing import Entity


class GetRoomLocation(old_g.GetRoomLocation):

    def execute(self):
        if (leg := WORLD.try_component(self.ent, COMPONENTS["InLegacyRoom"])):
            return leg.holder
        return super().execute()


class GetAdminLevel:

    def __init__(self, ent):
        self.ent = ent

    def execute(self):
        return 0


class IsProvidingLight:

    def __init__(self, ent: Entity):
        self.ent = ent

    def execute(self) -> bool:
        it = WORLD.try_component(self.ent, ItemType)
        if it and it.type_flag == 1:
            i = WORLD.try_component(self.ent, ItemValues)
            if i and i.values.get(2, 0):
                return True

        rf = WORLD.try_component(self.ent, RoomFlags)
        dark = False
        if rf:
            for f in rf.flags:
                if f.is_providing_light(self.ent):
                    return True
                if f.is_providing_darkness(self.ent):
                    dark = True

        s = WORLD.try_component(self.ent, SectorType)
        if s:
            if s.is_providing_light(self.ent) and not dark:
                return True

        e = WORLD.try_component(self.ent, Equipment)
        if e:
            for eq in e.equipment.values():
                if is_providing_light(eq):
                    return True

        return True


class IsIlluminated:

    def __init__(self, ent: Entity):
        self.ent = ent

    def execute(self) -> bool:
        """
        Returns true if a room is lit enough to see in it.
        """
        providing = GETTERS["IsProvidingLight"]
        if providing(self.ent).execute():
            return True

        i = WORLD.try_component(self.ent, Inventory)
        if i:
            for e in i.inventory:
                if providing(e).execute():
                    return True
        return False