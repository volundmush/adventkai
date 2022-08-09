from snekmud import OPERATIONS, COMPONENTS, WORLD
from .base import _DeadCommand
from adventkai import LEGACY_ROOMS

class Goto(_DeadCommand):
    name = "goto"
    admin_level = 2

    async def execute(self):
        if not self.args:
            raise self.ex("Usage: goto <vnum>")
        try:
            vnum = int(self.args)
        except ValueError:
            raise
        if not (ent := LEGACY_ROOMS.get(vnum, None)):
            raise self.ex(f"Legacy Room {vnum} not found.")
        await OPERATIONS["RemoveFromLocation"](self.entity, move_type="goto").execute()
        await OPERATIONS["AddToRoom"](self.entity, ent, move_type="goto").execute()
        self.send(line=await OPERATIONS["DisplayRoom"](self.entity, ent).execute())



