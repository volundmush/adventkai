from snekmud import COMPONENTS, WORLD, OPERATIONS
from adventkai import LEGACY_TRIGGERS
from adventkai.dgscripts.dgscripts import DGScriptInstance, DGState, MobTriggers, ItemTriggers, RoomTriggers


class _Trigger:
    trig_type = None

    def __init__(self, ent, script_vars: dict, **kwargs):
        self.ent = ent
        self.script_vars = script_vars
        self.kwargs = kwargs
        self.comp = None

    def get_valid_scripts(self):
        for vnum in self.comp.triggers:
            if (proto := LEGACY_TRIGGERS.get(vnum, None)) is None:
                continue
            if proto.trigger_type & self.trig_type:
                yield vnum

    def get_ready_scripts(self):
        for vnum in self.get_valid_scripts():
            if (script_instance := self.comp.scripts.get(vnum, None)) is not None:
                if script_instance.state == DGState.DORMANT:
                    yield vnum, script_instance
            else:
                if (proto := LEGACY_TRIGGERS.get(vnum, None)) is None:
                    continue
                script_instance = DGScriptInstance(self.comp, proto)
                self.comp.scripts[vnum] = script_instance
                yield vnum, script_instance

    async def execute(self):
        if not (t := WORLD.try_component(self.ent, COMPONENTS["Triggers"])):
            return None
        self.comp = t
        return await self.run_scripts()

    async def run_scripts(self):
        for vnum, script in self.get_ready_scripts():
            print(f"trying trigger {vnum} - {script}")
            return await script.execute()


class DgMobGreet(_Trigger):
    trig_type = MobTriggers.GREET
