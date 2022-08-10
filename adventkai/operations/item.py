from snekmud import WORLD, COMPONENTS, OPERATIONS, GETTERS


class GetItems:

    def __init__(self, ent, entities, **kwargs):
        self.ent = ent
        self.entities = entities
        self.kwargs = kwargs

    async def execute(self):
        pass


class DropItems:

    def __init__(self, ent, entities, **kwargs):
        self.ent = ent
        self.entities = entities
        self.kwargs = kwargs

    async def execute(self):
        pass


