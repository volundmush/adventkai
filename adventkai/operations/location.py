from snekmud import WORLD, COMPONENTS, OPERATIONS, GETTERS
from adventkai.components import ExitDir, RoomExit
from snekmud.exceptions import CommandError
from adventkai import LEGACY_ROOMS
from snekmud.operations import location as old_loc


class AddToRoom(old_loc.AddToRoom):

    def __init__(self, ent, dest, **kwargs):
        super().__init__(ent, dest, **kwargs)
        if WORLD.has_component(self.dest, COMPONENTS["HasVnum"]):
            self.comp = "InLegacyRoom"
        self.contents = list()

    async def execute(self):
        self.contents = GETTERS["GetContents"](self.dest).execute()
        return await super().execute()

    async def at_receive_entity(self, ent):
        if not WORLD.has_component(ent, COMPONENTS["PlayerCharacter"]):
            return
        npcs = [x for x in self.contents if WORLD.has_component(x, COMPONENTS["NPC"])]
        print(f"AVAILABLE NPCS: {npcs}")
        script_vars = {"actor": ent, "direction": self.kwargs.pop("from_direction", "")}
        print(f"SCRIPT VARS: {script_vars}")
        for x in npcs:
            if GETTERS["VisibleTo"](x, ent).execute():
                await OPERATIONS["DgMobGreet"](x, script_vars).execute()


class RemoveFromRoom(old_loc.RemoveFromRoom):

    async def execute(self):
        if (await super().execute()):
            return True
        self.rev_comp = "InLegacyRoom"
        return await super().execute()


class TraverseExit:

    def __init__(self, ent, from_room, ex_dir, ex_data, move_type: str = "move", move_verb: str = "walks", quiet=False,
                 look_after=True, **kwargs):
        self.ent = ent
        self.from_room = from_room
        self.ex_dir = ex_dir
        self.ex_data = ex_data
        self.move_type = move_type
        self.move_verb = move_verb
        self.quiet = quiet
        self.look_after = look_after
        self.kwargs = kwargs

    async def execute(self):
        if (dest := await self.find_destination()) is None:
            raise CommandError("You can't go that way.")
        if not await self.can_traverse():
            return
        await OPERATIONS["RemoveFromRoom"](self.ent).execute()
        if not self.quiet:
            await self.announce_move_from()
            await self.announce_move_to()
        await OPERATIONS["AddToRoom"](self.ent, dest, from_direction=self.ex_dir.name.lower(),
                                      rev_direction=self.ex_dir.reverse().name.lower()).execute()
        await self.at_transition(self.from_room, dest)
        if self.look_after and (cmd := WORLD.try_component(self.ent, COMPONENTS["HasCmdHandler"])):
            cmd.send(line=await OPERATIONS["DisplayRoom"](self.ent, dest).execute())

    async def find_destination(self):
        if (ent := LEGACY_ROOMS.get(self.ex_data.to_room, None)) is not None:
            return ent

    async def can_traverse(self) -> bool:
        # TODO: make this respect locked status and etc.
        return True

    async def announce_move_from(self):
        pass

    async def announce_move_to(self):
        pass

    async def at_transition(self, from_room, to_room):
        pass


class RemoveFromLocation(old_loc.RemoveFromLocation):

    async def execute(self):
        await super().execute()
        if WORLD.has_component(self.ent, COMPONENTS["InLegacyRoom"]):
            await OPERATIONS["RemoveFromRoom"](self.ent).execute()