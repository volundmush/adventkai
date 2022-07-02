from mudrich.evennia import EvenniaToRich
from mudrich.circle import CircleToRich

class ConnParser:

    def __init__(self, conn):
        self.conn = conn

    async def send(self, s: str):
        await self.conn.send_ev(s)

    async def send_ev(self, s):
        await self.conn.send_line(EvenniaToRich(s))

    async def send_circle(self, s):
        await self.conn.send_line(CircleToRich(s))

    async def start(self):
        pass

    async def close(self):
        pass

    async def parse(self, s: str):
        pass
