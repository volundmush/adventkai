from snekmud import WORLD, COMPONENTS, OPERATIONS
from adventkai.components import ExitDir, RoomExit

class TraverseExit:

    def __init__(self, ent, from_room, ex_dir, ex_data, move_type: str = "move", move_verb: str = "walks", **kwargs):
        self.ent = ent
        self.from_room = from_room
        self.ex_dir = ex_dir
        self.ex_data = ex_dir
        self.move_type = move_type
        self.move_verb = move_verb
        self.kwargs = kwargs

    async def execute(self):
        pass
