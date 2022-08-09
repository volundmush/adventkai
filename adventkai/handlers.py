from snekmud import WORLD, COMPONENTS, OPERATIONS, GETTERS
from adventkai import LEGACY_ROOMS

from snekmud.handlers import GameSessionHandler as GSH


class GameSessionHandler(GSH):

    async def at_start(self, copyover=None, cmdhandler: str = "Puppet"):
        await super().at_start(copyover=copyover, cmdhandler=cmdhandler)
        if (room := GETTERS["GetRoomLocation"](self.puppet).execute()):
            self.send(line=await OPERATIONS["DisplayRoom"](self.puppet, room).execute())

    async def find_start_room(self):
        if (leg_room := WORLD.try_component(self.character, COMPONENTS["SaveInLegacyRoom"])):
            if (ent := LEGACY_ROOMS.get(leg_room.vnum, None)) is not None:
                WORLD.remove_component(self.character, COMPONENTS["SaveInLegacyRoom"])
                return ent, self.loc_last
        return await super().find_start_room()