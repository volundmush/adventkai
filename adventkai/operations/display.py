from snekmud import WORLD, COMPONENTS, OPERATIONS, GETTERS
from adventkai.components import ExitDir
from mudrich.evennia import EvenniaToRich
from collections import defaultdict
import adventkai
from rich.text import Text
from rich.console import Group
from rich.style import NULL_STYLE
from snekmud import MODIFIERS_NAMES, MODIFIERS_ID
from rich.table import Table
from snekmud.operations.display import DisplayRoom as _DR
from mudforge.utils import lazy_property


COMPASS_TEMPLATE = """||{N:^3}||
||{NW:>3}|| ||{U:^3}|| ||{NE:<3}||
||{W:>3}|| ||{I:^3}|| ||{E:<3}||
||{SW:>3}|| ||{D:^3}|| ||{SE:<3}||
||{S:^3}||
"""


class DisplayRoom(_DR):

    @lazy_property
    def exits(self):
        if(exits := WORLD.try_component(self.room, COMPONENTS["Exits"])):
            return exits.exits
        return dict()

    @lazy_property
    def sector(self):
        return WORLD.try_component(self.room, COMPONENTS["SectorType"])

    def generate_automap(self, min_y=-2, max_y=2, min_x=-2, max_x=2):
        visited = set()
        cur_map = defaultdict(lambda: defaultdict(lambda: " "))

        def scan(room, cur_x, cur_y):
            if room in visited:
                return
            visited.add(room)

            if (sect := WORLD.try_component(room, COMPONENTS["SectorType"])) and sect.modifier:
                cur_map[cur_y][cur_x] = sect.modifier.map_key
            else:
                cur_map[cur_y][cur_x] = "o"
            
            if not (exits := WORLD.try_component(room, COMPONENTS["Exits"])) and exits.exits:
                return
            
            for ex, data in exits.exits.items():
                if data.to_room is None:
                    continue
                
                if (destination_ent := adventkai.LEGACY_ROOMS.get(data.to_room, None)) is None:
                    continue
                
                match ex:
                    case ExitDir.NORTH:
                        if (cur_y + 1) <= max_y:
                            scan(destination_ent, cur_x, cur_y + 1)
                    case ExitDir.SOUTH:
                        if (cur_y - 1) >= min_y:
                            scan(destination_ent, cur_x, cur_y - 1)
                    case ExitDir.EAST:
                        if (cur_x + 1) <= max_x:
                            scan(destination_ent, cur_x + 1, cur_y)
                    case ExitDir.WEST:
                        if (cur_x - 1) >= min_x:
                            scan(destination_ent, cur_x - 1, cur_y)
                    case ExitDir.NORTHEAST:
                        if ((cur_y + 1) <= max_y) and ((cur_x + 1) <= max_x):
                            scan(destination_ent, cur_x + 1, cur_y + 1)
                    case ExitDir.NORTHWEST:
                        if ((cur_y + 1) <= max_y) and ((cur_x - 1) >= min_x):
                            scan(destination_ent, cur_x - 1, cur_y + 1)
                    case ExitDir.SOUTHEAST:
                        if ((cur_y - 1) >= min_y) and ((cur_x + 1) <= max_x):
                            scan(destination_ent, cur_x + 1, cur_y - 1)
                    case ExitDir.SOUTHWEST:
                        if ((cur_y - 1) >= min_y) and ((cur_x - 1) >= min_x):
                            scan(destination_ent, cur_x - 1, cur_y - 1)

        scan(self.room, 0, 0)

        cur_map[0][0] = "|rX|n"

        return cur_map

    header_line = Text("O----------------------------------------------------------------------O")
    subheader_line = Text("------------------------------------------------------------------------")
    
    async def get_planet(self):
        pass

    def generate_compass(self):
        compass_dict = defaultdict(str)

        for ex, data in self.exits.items():
            upper = ex.name.upper()
            match ex:
                case ExitDir.NORTH | ExitDir.SOUTH:
                    compass_dict[upper[0]] = f" |c{upper[0]}|n "
                case ExitDir.UP | ExitDir.DOWN:
                    compass_dict[upper[0]] = f" |y{upper[0]}|n "
                case ExitDir.EAST:
                    compass_dict["E"] = "|cE|n  "
                case ExitDir.WEST:
                    compass_dict["W"] = "  |cW|n"
                case ExitDir.NORTHWEST | ExitDir.SOUTHWEST:
                    compass_dict[upper[0] + upper[5]] = f" |c{upper[0] + upper[5]}|n"
                case ExitDir.NORTHEAST | ExitDir.SOUTHEAST:
                    compass_dict[upper[0] + upper[5]] = f"|c{upper[0] + upper[5]}|n "
                case ExitDir.INWARDS:
                    compass_dict["I"] = f" |MI|n "
                case ExitDir.OUTWARDS:
                    compass_dict["I"] = "|MOUT|n"

        return EvenniaToRich(COMPASS_TEMPLATE.format_map(compass_dict))

    async def execute(self):

        out = list()

        builder = True

        out.append(self.header_line)

        out.append(EvenniaToRich(f"Location: {GETTERS['GetDisplayName'](self.viewer, self.room).execute()}"))

        if (planet := await self.get_planet()):
            out.append(EvenniaToRich(f"Planet: {planet}"))
        out.append(EvenniaToRich(f"Gravity: Normal"))

        if builder:
            build_info = []
            if (rflags := WORLD.try_component(self.room, COMPONENTS["RoomFlags"])) and rflags.flags:
                build_info.append(f"Flags: [ |g{' '.join(str(r) for r in rflags.flags.all())} |n]")
            if (sector := WORLD.try_component(self.room, COMPONENTS["SectorType"])) and sector.modifier:
                build_info.append(f"Sector: [ |g{str(sector.modifier)} |n ]")
            if build_info:
                out.append(EvenniaToRich(" ".join(build_info)))

        out.append(self.header_line)

        if (desc := WORLD.try_component(self.room, COMPONENTS["Description"])) and desc.plain:
            out.append(desc.rich)
        else:
            out.append(EvenniaToRich("You see nothing special."))

        out.append(self.subheader_line)

        y_coor = [2, 1, 0, -1, -2]
        x_coor = [-4, -3, -2, -1, 0, 1, 2, 3, 4]
        automap = self.generate_automap(min_x=-4, max_x=4)
        col_automap = EvenniaToRich("\r\n".join(["".join([automap[y][x] for x in x_coor]) for y in y_coor]))

        map_legend = [f"{x.map_key}: {x.get_map_name()}" for x in
                      sorted(MODIFIERS_NAMES["SectorType"].values(), key=lambda y: y.modifier_id)]
        map_legend.append("|rX|n: You")

        table = Table(box=None)
        table.add_column("Compass", width=17, header_style=NULL_STYLE, justify="center")
        table.add_column("Auto-Map", width=10, header_style=NULL_STYLE)
        table.add_column("Map Key", width=37, header_style=NULL_STYLE)
        table.add_row(EvenniaToRich("|r---------"), EvenniaToRich("|r----------"),
                      EvenniaToRich("|r-----------------------------"))
        table.add_row(self.generate_compass(), col_automap, EvenniaToRich(", ".join(map_legend)))

        out.append(table)

        out.append(self.subheader_line)

        # contents
        room_format = OPERATIONS["DisplayInRoom"]
        for c in GETTERS["VisibleContents"](self.viewer, self.room, **self.kwargs).execute():
            if c == self.viewer:
                continue
            results = await room_format(self.viewer, self.room, c, **self.kwargs).execute()
            out.append(results)

        return Group(*out)

