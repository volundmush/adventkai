from snekmud import OPERATIONS, COMPONENTS, WORLD, GETTERS
from .base import _DeadCommand, _RestingCommand
from adventkai import LEGACY_ROOMS
from snekmud.utils import get_or_emplace


class Get(_RestingCommand):
    name = "get"
    min_text = "ge"

    async def execute(self):
        if not self.args:
            raise self.ex("Get what?")
        args = self.args.split()
        search_get = GETTERS["SearchEntities"]
        con_get = GETTERS["VisibleContents"]
        match len(args):
            case 1:
                containers = list()
                if (room := GETTERS["GetRoomLocation"](self.entity).execute()):
                    containers.append(room)
            case 2:
                candidates = GETTERS["VisibleNearbyMeta"](self.entity).execute()
                containers = search_get(self.entity, candidates, args[1]).execute()
            case _:
                raise self.ex("Usage: get <target>[ <from>]")

        final_candidates = list()
        for container in containers:
            final_candidates.extend(con_get(self.entity, container).execute())
        if self.entity in final_candidates:
            final_candidates.remove(self.entity)

        if not (items := search_get(self.entity, final_candidates, args[0]).execute()):
            raise self.ex("Nothing by that name.")

        await OPERATIONS["GetItems"](self.entity, items).execute()


class Drop(_RestingCommand):
    name = "drop"
    min_text = "dro"

    async def execute(self):
        if not self.args:
            raise self.ex("Drop what?")

        inv = GETTERS["VisibleContents"](self.entity, self.entity).execute()

        if not (items := GETTERS["SearchEntities"](self.entity, inv, self.args).execute()):
            raise self.ex("Nothing by that name.")

        await OPERATIONS["DropItems"](self.entity, items).execute()