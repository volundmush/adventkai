from snekmud import COMPONENTS, OPERATIONS, WORLD
from adventkai.utils import get_trait
from mudforge.utils import lazy_property


class _Base:

    def __init__(self, trig_comp, script, member, call, arg):
        self.trig_comp = trig_comp
        self.ent = trig_comp.owner
        self.other_ent = script.handler.owner
        self.script = script
        self.member = member
        self.call = call
        self.arg = arg

    async def execute(self):
        pass


class _CompCheck(_Base):
    comp_name = None

    async def execute(self):
        return "1" if WORLD.has_component(self.ent, COMPONENTS[self.comp_name]) else "0"


class _TraitBase(_Base):
    trait_name = None

    @lazy_property
    def trait(self):
        return get_trait(self.ent, self.trait_name)

    def mod(self, value):
        return self.trait.mod(value)


class _FlagCheck(_TraitBase):

    async def execute(self):
        if self.arg:
            return "1" if self.trait.has(self.arg) else "0"
        else:
            return " ".join(self.trait.names())


class _SpecFlagCheck(_FlagCheck):
    mod_name = None

    async def execute(self):
        self.arg = self.mod_name
        return await super().execute()


class _IntTrait(_TraitBase):

    async def execute(self):
        if self.arg:
            try:
                value = int(self.arg)
            except ValueError as err:
                return ""
            return str(self.mod(value))
        else:
            return str(self.trait.get())


class _StringTrait(_TraitBase):

    async def execute(self):
        return str(self.trait.get())