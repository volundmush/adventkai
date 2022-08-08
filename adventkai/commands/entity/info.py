from snekmud import OPERATIONS, COMPONENTS, WORLD
from .base import _DeadCommand


class Look(_DeadCommand):
    name = "look"

    async def execute(self):
        if not (room := await OPERATIONS["GetRoomLocation"](self.entity).execute()):
            raise self.ex("There's not much to see here... wherever 'here' is.")
        self.send(line=await OPERATIONS["DisplayRoom"](self.entity, room).execute())