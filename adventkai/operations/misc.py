class AtZoneReset:

    def __init__(self, zone, room, **kwargs):
        self.zone = zone
        self.room = room
        self.kwargs = kwargs

    async def execute(self):
        pass
