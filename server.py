from sanic import Sanic
from sanic_jwt import Initialize
import socketio

import circlemud
from circlemud import account_manager

from adventkai import settings
from adventkai import api

sio = socketio.AsyncServer(async_mode="sanic", namespaces='*')
app = Sanic(settings.NAME)
Initialize(app, claim_aud=settings.HOSTNAME, authenticate=account_manager.authenticate, retrieve_user=account_manager.retrieve_user)
sio.attach(app)
app.blueprint(api.api)

# Link in the C++ game library.
@app.before_server_start
def init_game(app, loop):
    circlemud.initialize()

app.add_task(circlemud.run_game_loop())

CONNECTIONS = dict()


@sio.on('connect')
async def connect_handler(sid, environ):
    new_conn = circlemud.GameSession(sid, sio)
    CONNECTIONS[sid] = new_conn
    app.add_task(new_conn.run(), name=f"SocketIO {sid}")


@sio.on('disconnect')
async def disconnect_handler(sid):
    if conn := CONNECTIONS.pop(sid, None):
        await conn.handle_disconnect()
    else:
        print(f"disconnect_handler: No connection for sid {sid}")

@sio.on('*')
async def message_handler(event, sid, message):
    if conn := CONNECTIONS.get(sid, None):
        await conn.handle_event(event, message)
    else:
        print(f"message_handler: No connection for sid {sid}")


# Finally, run.
# SCREAMING NOTE: DO NOT RUN AS MULTI-PROCESS IT WILL FUCK *EVERYTHING* UP.
if __name__ == "__main__":
    app.run(host=settings.WEBSERVER_INTERFACE, port=settings.WEBSERVER_PORT,
            single_process=True, workers=0)