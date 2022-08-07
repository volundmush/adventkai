from snekmud import WORLD, COMPONENTS, OPERATIONS
from adventkai import LEGACY_ROOMS

from snekmud.handlers import GameSessionHandler as GSH

class GameSessionHandler(GSH):

    async def find_start_room(self):
        if (leg_room := WORLD.try_component(self.character, COMPONENTS["SaveInLegacyRoom"])):
            if (ent := LEGACY_ROOMS.get(leg_room.vnum, None)) is not None:
                WORLD.remove_component(self.character, COMPONENTS["SaveInLegacyRoom"])
                return ent, self.loc_last
        return await super().find_start_room()