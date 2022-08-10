from snekmud import getters as old_g
from snekmud import WORLD, COMPONENTS, OPERATIONS, GETTERS
from snekmud.typing import Entity
from rich.text import Text
from mudrich.evennia import strip_ansi
import re



class SearchEntities:
    re_match = re.compile(r"^(?:(?P<number>(\d+|all|\*))\.)?(?P<name>.+?)?$")

    def __init__(self, viewer: Entity, targets: list[Entity], pattern: str, **kwargs):
        self.viewer = viewer
        self.targets = targets
        self.pattern = pattern
        self.kwargs = kwargs

    def execute(self) -> list[Entity]:
        out = list()
        disp = GETTERS["GetDisplayName"]
        if not (match := self.re_match.match(self.pattern)):
            return []
        check_name = match.group("name").lower()
        if (get_num := match.group("number")):
            try:
                get_num = int(match.group("number"))
                if get_num <= 0:
                    get_num = 1
            except ValueError as err:
                get_num = None
        else:
            get_num = None
        for target in self.targets:
            name = disp(self.viewer, target, plain=True).execute()
            if check_name == "*" or check_name in name:
                out.append(target)
                if get_num:
                    if len(out) == get_num:
                        return [target, ]
        return out


class GetDisplayName(old_g.GetDisplayName):

    def execute(self):
        out = f"Entity {self.target}"

        if (name := WORLD.try_component(self.target, COMPONENTS["Name"])):
            if self.rich:
                out = name.rich
            elif self.plain:
                out = name.plain
            else:
                out = name.color
        elif (comp := WORLD.try_component(self.target, COMPONENTS["EntityID"])):
            out = f"{comp.module_name}/{comp.ent_id}"

        if self.rich and not hasattr(out, "__rich_console__"):
            out = Text(out)

        suf_string = []
        suffix = ''

        if (build := self.kwargs.pop("build", False)):
            suf_string.append(f" |g[ENT: {self.target}]|n")

        if suf_string:
            suffix = " ".join(suf_string)
            if self.plain:
                suffix = strip_ansi(suffix)

        if suffix and hasattr(out, "__rich_console__"):
            return Text(" ").join([out, suffix])
        elif suffix:
            return out + " " + suffix
        else:
            return out


class GetRoomLocation(old_g.GetRoomLocation):

    def execute(self):
        if (leg := WORLD.try_component(self.ent, COMPONENTS["InLegacyRoom"])):
            return leg.holder
        return super().execute()


class GetMetaTypes:

    def __init__(self, ent):
        self.ent = ent

    def execute(self):
        if (meta := WORLD.try_component(self.ent, COMPONENTS["MetaTypes"])):
            return meta.types
        return []


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