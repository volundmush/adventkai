import asyncio
import logging
from sanic import Sanic
from ..core import Service
import pickle
from adventkai.game_session import ClientHello
from sanic_jwt import Initialize
from sanic_jwt import exceptions

from circlemud import authenticate


class WebService(Service):
    def __init__(self, core):
        super().__init__(core)
        self.app = Sanic(core.settings.NAME)
        Initialize(self.app, authenticate=authenticate)

    async def at_pre_start(self):
        app = self.app
        app.add_websocket_route(self.handle_ws, "/ws")

    async def start(self):
        server = await self.app.create_server(host=self.core.settings.WEBSERVER_INTERFACE,
                                              port=self.core.settings.WEBSERVER_PORT)
        await server.startup()
        await server.serve_forever()

    async def handle_ws(self, request, ws):
        message = await ws.recv()
        match message:
            case bytes():
                data = pickle.loads(message)
                await self.handle_opening_message(ws, data)

            case str():
                logging.error(f"Unexpected string data: {message}")

    async def handle_opening_message(self, ws, msg):
        match msg:
            case ClientHello():
                await self.core.handle_new_client(ws, msg)
