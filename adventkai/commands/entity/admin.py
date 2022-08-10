from snekmud import OPERATIONS, COMPONENTS, WORLD, GETTERS
from .base import _DeadCommand
from adventkai import LEGACY_ROOMS
from snekmud.utils import get_or_emplace


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


class Tel(_DeadCommand):
    name = "@telelport"
    min_text = "@tel"
    admin_level = 2

    async def execute(self):
        if not (self.lsargs and self.rsargs):
            raise self.ex("Usage: @tel <target>=<room>")
        try:
            target = int(self.lsargs)
            dest = int(self.rsargs)
        except ValueError as err:
            raise self.ex("Target and Room must be Entity IDs")
        if not WORLD.entity_exists(target):
            raise self.ex("Target not found.")
        if not WORLD.entity_exists(dest):
            raise self.ex("Destination room not found.")
        if "room" not in GETTERS["GetMetaTypes"](dest).execute():
            raise self.ex("Destination is not a room.")

        await OPERATIONS["RemoveFromLocation"](target, move_type="goto").execute()
        await OPERATIONS["AddToRoom"](target, dest, move_type="goto").execute()
        c = get_or_emplace(target, COMPONENTS["HasCmdHandler"])
        c.send(line=await OPERATIONS["DisplayRoom"](target, dest).execute())


class AdmExamine(_DeadCommand):
    name = "@examine"
    min_text = "@ex"
    admin_level = 2

    async def execute(self):
        if not self.args:
            raise self.ex("Usage: @examine <target>")
        try:
            target = int(self.args)
        except ValueError as err:
            raise self.ex("Target must be Entity ID")
        if not WORLD.entity_exists(target):
            raise self.ex("Target not found.")

        await OPERATIONS["DisplayExamine"](self.entity, target).execute()


class AdmWhereIs(_DeadCommand):
    name = "@whereis"
    min_text = "@whe"
    admin_level = 2

    async def execute(self):
        if not self.args:
            raise self.ex("Usage: @whereis <name>")
        search = self.args.lower()
        results = []
        disp_name = GETTERS["GetDisplayName"]
        for ent, (meta,) in WORLD.get_components(COMPONENTS["MetaTypes"]):
            if "room" in meta.types:
                continue
            name = disp_name(self.entity, ent, plain=True).execute()
            if search in name.lower():
                results.append(ent)
        if not results:
            raise self.ex("Nothing found.")
        for ent in results:
            name = disp_name(self.entity, ent).execute()
            name_disp = f"{name} |g[ENT: {ent}]|n"
            loc = None
            loc_display = None
            if (room := GETTERS["GetRoomLocation"](ent).execute()):
                loc = room
                loc_display = f"Room: {disp_name(self.entity, loc, build=True).execute()}"
            elif (inv := WORLD.try_component(ent, COMPONENTS["InInventory"])):
                loc = inv.holder
                loc_display = f"Inv of {disp_name(self.entity, loc, build=True).execute()}"
            elif (eq := WORLD.try_component(ent, COMPONENTS["Equipped"])):
                loc = eq.holder
                loc_display = f"Equipped by {disp_name(self.entity, loc, build=True).execute()} at {eq.slot}"
            else:
                loc_display = "Nowhere!"
            self.send(line=f"{name_disp} -- {loc_display}")

class CmdCheck(_DeadCommand):
    name = "@check"

    async def execute(self):
        act = COMPONENTS["ActionDescription"]

        for ent, (act_desc,) in WORLD.get_components(act):
            self.send(python=WORLD.components_for_entity(ent))
