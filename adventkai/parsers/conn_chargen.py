from adventkai.exceptions import CommandError
from adventkai.db.accounts.models import Account
from django.core.exceptions import ValidationError
import re
from .conn_base import ConnParser
import adventkai
from adventkai import components as cm
from adventkai.typing import Entity
from mudforge.utils import lazy_property, partial_match
from rich.columns import Columns
from mudrich.circle import CircleToRich
from adventkai import WORLD


class Node:

    def __init__(self, cg):
        self.cg = cg

    async def start(self):
        await self.display_full()

    async def close(self):
        pass

    async def switch(self, node_class):
        await self.cg.switch(node_class)

    async def send(self, s):
        await self.cg.send(s)

    async def send_circle(self, s):
        await self.cg.send_circle(s)

    async def send_ev(self, s):
        await self.cg.send_ev(s)

    async def display(self):
        pass

    async def display_sub(self):
        self.send_circle("@w\r\nMake a selection:@n")

    async def display_full(self):
        await self.display()
        await self.display_sub()

    async def parse(self, s: str):
        pass

    async def check(self) -> typing.Optional[Node]:
        return None


class Mutations(Node):
    pass


class Genome(Node):
    pass


class AndroidType(Node):

    async def check(self):
        race = WORLD.component_for_entity(self.cg.ent, cm.Race)
        if str(race.modifier) != "Android":
            return Genome

    async def display(self):
        self.send_circle("\r\n@YChoose your model type.")
        for c in sorted(adventkai.MODIFIERS_ID["AndroidType"].values(), key=lambda x: x.mod_id):
            self.send_circle(f"@B{c.mod_id}@W)@C {c} Model")

    async def parse(self, s: str):
        if s.isdigit():
            if (choice := adventkai.MODIFIERS_ID["AndroidType"].get(int(s), None)):
                WORLD.add_component(self.cg.ent, cm.AndroidType(modifier=choice))
                await self.switch(Genome)
                return
            else:
                self.send("That's not a valid model, try again.")
                await self.start()
                return
        else:
            if (choice := partial_match(s, adventkai.MODIFIERS_ID["AndroidType"].values(), key=lambda x: x.name, exact=True)):
                WORLD.add_component(self.cg.ent, cm.AndroidType(modifier=choice))
                await self.switch(Genome)
                return
            else:
                self.send("That's not a valid model, try again.")
                await self.start()
                return

class Seeming(Node):

    @lazy_property
    def races(self):
        return sorted([r for r in adventkai.MODIFIERS_NAMES["Race"].values() if r.seeming_ok], key=lambda x: str(x))

    @lazy_property
    def formatted(self):
        out = list()
        for r in self.races:
            out.append(CircleToRich(f"@C{r:<15}@n"))
        return Columns(out, equal=True, width=self.cg.conn.details.width)

    async def check(self):
        race = WORLD.component_for_entity(self.cg.ent, cm.Race)
        if not race.modifier.has_seeming:
            return AndroidType

    async def display(self):
        self.send_circle("\r\n@Seeming SELECTION menu:\r\n@D---------------------------------------")
        self.send(self.formatted)

    async def display_sub(self):
        race = WORLD.component_for_entity(self.cg.ent, cm.Race)
        self.send_circle(
            f"@WEnter the race this {race.modifier} will appear as to others.")
        self.send_circle("@WAppears as:")

    async def parse(self, s: str):
        if not (r := partial_match(s, self.races, exact=True)):
            self.send("Sorry but that's not a viable seeming. Please try again.")
            await self.display_full()
        WORLD.add_component(self.cg.ent, cm.Seeming(seeming=str(r)))
        await self.switch(AndroidType)

class Race(Node):

    @lazy_property
    def races(self):
        return sorted([r for r in adventkai.MODIFIERS_NAMES["Race"].values() if r.pc_ok], key=lambda x: str(x))

    @lazy_property
    def formatted(self):
        out = list()
        for r in self.races:
            if r.rpp_cost():
                out.append(CircleToRich(f"@C{r:<15}@D[@R{r.rpp_cost()} RPP@D]@n"))
            else:
                out.append(CircleToRich(f"@C{r:<15}@n"))
        return Columns(out, equal=True, width=self.cg.conn.details.width)

    async def display(self):
        self.send_circle("\r\n@YRace SELECTION menu:\r\n@D---------------------------------------")
        self.send(self.formatted)

    async def display_sub(self):
        self.send_circle("@WEnter your preferred race. Check details using 'help <race>' or 'random' for random select.")
        self.send_circle("@WRace:")

    async def parse(self, s: str):
        if not (r := partial_match(s, self.races, exact=True)):
            self.send("Sorry but that's not a race. Please try again.")
            await self.display_full()
        WORLD.add_component(self.cg.ent, cm.Race(modifier=r))
        await self.switch(Seeming)




_RE_NAME = re.compile(r"^[A-Za-z]+$")

class Name(Node):

    def __init__(self, cg):
        super().__init__(cg)
        self.name = None

    async def display(self):
        self.send("Pick a good name!")

    async def display_sub(self):
        self.send("Name?")

    async def parse(self, s: str):
        if self.name:
            if self.name == s:
                WORLD.add_component(self.cg.ent, cm.Name(s))
                await self.switch(Race)
                return
            else:
                self.send("That didn't match!")
                self.name = None
                await self.start()
                return
        elif _RE_NAME.match(s):
            self.name = s
            self.send(f"{s}. Are you sure? Enter again to confirm.")
        else:
            self.send("That is not a good name. Names must be simple alphabetical, single words.")
            await self.start()
            return


class Chargen(ConnParser):

    def __init__(self, conn):
        super().__init__(conn)
        self.ent = adventkai.WORLD.create_entity()
        self.node = None

    async def start(self):
        await self.switch(Name)

    async def switch(self, node_class):
        if self.node:
            await self.node.close()
        new_node = node_class(self)
        if (bypass := await new_node.check()):
            await self.switch(bypass)
            return
        self.node = node_class(self)
        await self.node.start()
