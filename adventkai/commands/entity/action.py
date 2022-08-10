from snekmud import OPERATIONS, COMPONENTS, WORLD, GETTERS
from .base import _DeadCommand, _FightCommand
from adventkai import LEGACY_ROOMS, TRAITS
from snekmud.utils import get_or_emplace
from adventkai.utils import get_trait
from mudforge.utils import partial_match


class Transform(_FightCommand):
    name = "transform"
    min_text = "tran"

    async def display_transformations(self, available):
        self.send(line=f"TRANSFORM TABLE HERE! {available}")

    async def execute(self):
        race = get_trait(self.entity, "Race")
        if not (available := race.get().get_available_transformations(self.entity)):
            self.send(line="You... have transformations?")
            return

        if not self.args:
            await self.display_transformations(available)
            return

        arg_lower = self.args.lower()

        trans = get_trait(self.entity, "Transformation")

        cur_form = trans.get()

        if arg_lower == "revert":
            if not cur_form:
                self.send(line="You are not transformed!")
                return
            if not cur_form.can_revert:
                self.send(line="That would be unthinkable!")
                return

            await trans.revert()
            return

        if not (found := partial_match(self.args, available, key=lambda x: x.get_name())):
            self.send(line="You don't know how to become that.")
            return

        if not found.can_transform(self.entity):
            self.send(line="You're not able to handle that form!")
            return

        await trans.transform_to(found)
        #self.handler.cmdqueue.set_wait(0.5)