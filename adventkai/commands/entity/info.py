from snekmud import OPERATIONS, COMPONENTS, WORLD, GETTERS
from .base import _RestingCommand


class Look(_RestingCommand):
    name = "look"
    min_text = "l"

    async def execute(self):
        if not (room := GETTERS["GetRoomLocation"](self.entity).execute()):
            raise self.ex("There's not much to see here... wherever 'here' is.")
        self.send(line=await OPERATIONS["DisplayRoom"](self.entity, room).execute())