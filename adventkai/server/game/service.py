import time
import asyncio
from adventkai.core import Service


class GameService(Service):
    def __init__(self, core):
        super().__init__(core)

    async def at_pre_start(self):
        pass

    async def start(self):
        deltaTimeInSeconds = 0.1
        loop_frequency = 0.1
        last_time = time.perf_counter()

        while self.running:
            start = time.perf_counter()
            # comm.run_loop_once(deltaTimeInSeconds)
            end = time.perf_counter()

            duration = end - start
            wait_time = loop_frequency - duration
            if wait_time < 0:
                wait_time = 0.001

            await asyncio.sleep(wait_time)
            deltaTimeInSeconds = time.perf_counter() - start

    async def run_loop_once(self, deltaTime: float):
        pass