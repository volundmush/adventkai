from snekmud import COMPONENTS, OPERATIONS, WORLD


class _Base:

    def __init__(self, trig_comp, script, member, call, arg):
        self.trig_comp = trig_comp
        self.ent = trig_comp.owner
        self.script = script
        self.member = member
        self.call = call
        self.arg = arg

    async def execute(self):
        pass


class Id(_Base):

    async def execute(self):
        return self.ent


class Name(_Base):

    async def execute(self):
        return OPERATIONS["GetDisplayName"](self.script.handler.owner, self.ent).execute()


class VarExists(_Base):

    async def execute(self):
        if self.arg:
            if (con := self.trig_comp.variables.get(self.script.context, None)) is not None:
                if self.member.lower() in con:
                    return "1"
        return "0"


class Level(_Base):
    aliases = ["lvl"]




def level(obj, script, arg):
    return stat_get_set(obj, script, "level", "level", arg)


def is_pc(obj, script, arg):
    return "1" if WORLD.has_component() else "0"


class Alignment(_Base):
    aliases = ["align"]


class Affects(_Base):
    aliases = ["affect"]


def affect(obj, script, arg):
    return "1" if obj.affect_flags.has(arg) else "0"


affects = affect


def alias(obj, script, arg):
    return obj.get_display_name(looker=script.handler.owner)


def canbeseen(obj, script, arg):
    return "1" if script.handler.owner.can_see(obj) else "0"


def clan(obj, script, arg):
    return "0"


def eq(obj, script, arg):
    if arg == "*":
        return "1" if obj.equipment.all() else "0"
    if arg.isnumeric():
        if (found := obj.equipment.get(int(arg))):
            return found.dbref
    return ""


def has_item(obj, script, arg):
    if arg.isnumeric():
        return "1" if obj.has_item(int(arg)) else "0"
    return "0"


def inventory(obj, script, arg):
    if not arg:
        for o in obj.inventory.all():
            return o.dbref
    if arg and arg.isnumeric():
        vnum = int(arg)
        for o in obj.inventory.all():
            if o.item_vnum == vnum:
                return o.dbref
    return ""


def room(obj, script, arg):
    if (r := obj.location) and r.obj_type == "room":
        return r.dbref
    return ""


def weight(obj, script, arg):
    return str(obj.weight.total())