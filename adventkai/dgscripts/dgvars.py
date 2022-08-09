from snekmud import OPERATIONS, WORLD, COMPONENTS


class _DgVars:

    def __init__(self, script, member: str = "", call: bool = False, arg: str = ""):
        self.script = script
        self.member = member
        self.call = call
        self.arg = arg

    async def execute(self):
        pass


class Self(_DgVars):

    async def execute(self):
        return self.script.handler.owner


class This(Self):
    pass


class Me(Self):
    pass


class Here(_DgVars):

    async def execute(self):
        ent = self.script.handler.owner
        if (meta := WORLD.try_component(ent, COMPONENTS["MetaTypes"])):
            if "room" in meta.types:
                return ent
        if (loc := GETTERS["GetRoomLocation"](ent).execute()) is not None:
            return loc
        return ""


class FindMob(_DgVars):
    pass


class FindObj(_DgVars):
    pass


class Time(_DgVars):
    pass


class CTime(_DgVars):
    pass


class Global(_DgVars):
    pass


class People(_DgVars):
    pass
