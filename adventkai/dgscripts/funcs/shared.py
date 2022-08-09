from snekmud import COMPONENTS, OPERATIONS, WORLD, GETTERS
from adventkai.utils import get_trait
from mudforge.utils import lazy_property
from .base import _Base, _IntTrait, _TraitBase, _CompCheck, _FlagCheck


class Id(_Base):

    async def execute(self):
        return self.ent


class Name(_Base):
    aliases = ["alias"]

    async def execute(self):
        return GETTERS["GetDisplayName"](self.script.handler.owner, self.ent).execute()


class VarExists(_Base):

    async def execute(self):
        if self.arg:
            if (con := self.trig_comp.variables.get(self.script.context, None)) is not None:
                if self.member.lower() in con:
                    return "1"
        return "0"


class Level(_IntTrait):
    trait_name = "Level"
    aliases = ["lvl"]


class Is_Pc(_CompCheck):
    comp_name = "PlayerCharacter"


class Is_NPC(_CompCheck):
    comp_name = "NPC"


class Alignment(_IntTrait):
    trait_name = "Alignment"
    aliases = ["align"]


class Affects(_FlagCheck):
    trait_name = "AffectFlags"
    aliases = ["affect"]


class CanBeSeen(_Base):

    async def execute(self):
        result = GETTERS["VisibleTo"](self.other_ent, self.ent).execute()
        return "1" if result else "0"


class Clan(_Base):
    async def execute(self):
        return "0"


class Equipment(_Base):
    aliases = ["eq"]

    async def execute(self):
        if not (eq := WORLD.try_component(self.ent, COMPONENTS["Equipment"])):
            return ""
        if self.arg == "*":
            return "1" if eq.all() else "0"
        elif self.arg.isnumeric():
            if (found := eq.equipment.get(int(self.arg), None)):
                return found.item
        return ""


class Inventory(_Base):

    async def execute(self):
        if not (inv := WORLD.try_component(self.ent, COMPONENTS["Inventory"])):
            return ""
        if not self.arg:
            for o in inv.inventory:
                return o
        try:
            vnum = int(self.arg)
        except ValueError as err:
            return ""
        v_comp = COMPONENTS["HasVnum"]
        for x in inv.inventory:
            if (v := WORLD.try_component(x, v_comp)):
                if v.vnum == vnum:
                    return "1"
        return "0"


class HasItem(_Base):
    aliases = ["has_item"]

    async def execute(self):
        try:
            vnum = int(self.arg)
        except ValueError as err:
            return ""
        contents = GETTERS["GetAllContainedEntities"](self.ent).execute()
        v_comp = COMPONENTS["HasVnum"]
        for x in contents:
            if (v := WORLD.try_component(x, v_comp)):
                if v.vnum == vnum:
                    return "1"
        return "0"


class Room(_Base):
    aliases = ["location"]

    async def execute(self):
        if (room := GETTERS["GetRoomLocation"](self.ent).execute()):
            return room
        return ""


class Weight(_IntTrait):
    trait_name = "Weight"
