from snekmud import OPERATIONS, COMPONENTS, WORLD
from .base import _DeadCommand, _RestingCOmmand
from adventkai.components import ExitDir

class _ExitCommand(_RestingCOmmand):
    ex_dir = None
    priority = -100

    async def get_move_type(self):
        return "walk"

    async def get_move_verb(self):
        return "walks"

    async def execute(self):
        if (room := await OPERATIONS["GetRoomLocation"](self.entity).execute()):
            if (exits := WORLD.try_component(room, COMPONENTS["Exits"])):
                if (ex := exits.exits.get(self.ex_dir, None)):
                    await OPERATIONS["TraverseExit"](self.entity, room, self.ex_dir, ex,
                                                     await self.get_move_type(), await self.get_move_verb()).execute()
                    return
        raise self.ex("You can't go that way.")


class North(_ExitCommand):
    name = "north"
    min_text = "n"
    ex_dir = ExitDir.NORTH


class East(_ExitCommand):
    name = "east"
    min_text = "e"
    ex_dir = ExitDir.EAST


class South(_ExitCommand):
    name = "south"
    min_text = "s"
    ex_dir = ExitDir.SOUTH


class West(_ExitCommand):
    name = "west"
    min_text = "w"
    ex_dir = ExitDir.WEST


class Up(_ExitCommand):
    name = "up"
    min_text = "u"
    ex_dir = ExitDir.UP


class Down(_ExitCommand):
    name = "down"
    min_text = "d"
    ex_dir = ExitDir.DOWN


class NorthWest(_ExitCommand):
    name = "northwest"
    aliases = ["nw"]
    min_text = "northw"
    ex_dir = ExitDir.NORTHWEST


class NorthEast(_ExitCommand):
    name = "northeast"
    aliases = ["ne"]
    min_text = "northe"
    ex_dir = ExitDir.NORTHEAST


class SouthWest(_ExitCommand):
    name = "southwest"
    aliases = ["sw"]
    min_text = "southw"
    ex_dir = ExitDir.SOUTHWEST


class SouthEast(_ExitCommand):
    name = "southeast"
    aliases = ["se"]
    min_text = "southe"
    ex_dir = ExitDir.SOUTHEAST


class Inside(_ExitCommand):
    name = "inside"
    min_text = ["in"]
    ex_dir = ExitDir.INWARDS


class Outside(_ExitCommand):
    name = "outside"
    min_text = ["o"]
    ex_dir = ExitDir.OUTWARDS
