from snekmud import OPERATIONS, COMPONENTS, WORLD, GETTERS
from .base import _RestingCommand


class Say(_RestingCommand):
    name = "say"

    async def execute(self):
        if not self.args:
            raise self.ex("What will you say?")
        await OPERATIONS["Say"](self.entity, self.args).execute()