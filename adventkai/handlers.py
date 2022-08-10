from snekmud import WORLD, COMPONENTS, OPERATIONS, GETTERS
from adventkai import LEGACY_ROOMS

from snekmud.handlers import GameSessionHandler as GSH
from adventkai.utils import get_trait


class GameSessionHandler(GSH):
    text_kwargs = ["text", "line", "python"]

    def __init__(self, owner):
        super().__init__(owner)
        self.prompt_countdown = 0.0

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

    def reset_prompt(self):
        self.prompt_countdown = 0.1

    def generate_prompt(self):
        if not (meta := WORLD.try_component(self.puppet, COMPONENTS["MetaTypes"])):
            return
        for x in meta.types:
            if (func := getattr(self, f"render_prompt_{x}", None)):
                super().send(prompt=f"\n\n{func()}\n")
                return

    def render_prompt_character(self):
        prompt_sections = list()
        power = get_trait(self.puppet, "PowerLevel")
        prompt_sections.append(f"|W[|rPL|y: |C{power.effective():,}|W]")
        ki = get_trait(self.puppet, "Ki")
        prompt_sections.append(f"|W[|cKI|y: |C{ki.effective():,}|W]")
        stamina = get_trait(self.puppet, "Stamina")
        prompt_sections.append(f"|W[|gST|y: |C{stamina.effective():,}|W]")
        return "|n".join(prompt_sections)

    async def update(self, interval: float):
        self.update_prompt(interval)

    def update_prompt(self, interval: float):
        if self.prompt_countdown > 0.0:
            self.prompt_countdown = max(0.0, self.prompt_countdown - interval)
            if self.prompt_countdown <= 0.0:
                self.generate_prompt()

    def send(self, **kwargs):
        for x in self.text_kwargs:
            if x in kwargs:
                self.reset_prompt()
                break
        super().send(**kwargs)
