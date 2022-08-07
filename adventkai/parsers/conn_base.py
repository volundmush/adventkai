from mudrich.evennia import EvenniaToRich
from mudrich.circle import CircleToRich


class ConnParser:

    def __init__(self, conn):
        self.conn = conn

    def send(self, s: str):
        self.conn.send_line(s)

    def send_ev(self, s):
        self.conn.send_line(EvenniaToRich(s))

    def send_circle(self, s):
        self.conn.send_line(CircleToRich(s))

    async def start(self):
        pass

    async def close(self):
        pass

    async def parse(self, s: str):
        pass
