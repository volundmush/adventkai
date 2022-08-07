from snekmud import WORLD, COMPONENTS, OPERATIONS
from snekmud.typing import Entity
from snekmud.operations.search import GetRoomLocation as _GRL


class GetRoomLocation(_GRL):

    async def execute(self):
        if (leg := WORLD.try_component(self.ent, COMPONENTS["InLegacyRoom"])):
            return leg.holder
        return await super().execute()