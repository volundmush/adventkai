from sanic import Sanic, response
from sanic_jwt import Initialize
import socketio
import os

import circlemud
from circlemud import account_manager

from adventkai import settings
from adventkai import api

# Get the absolute path to the 'lib/webroot/' directory
webroot_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'lib', 'webroot'))

sio = socketio.AsyncServer(async_mode="sanic", namespaces='*')
app = Sanic(settings.NAME)
Initialize(app, claim_aud=settings.HOSTNAME, authenticate=account_manager.authenticate, retrieve_user=account_manager.retrieve_user)
sio.attach(app)
app.blueprint(api.api)

# Static file serving
app.static('/static', os.path.join(webroot_path, 'static'))

# Route to serve index.html
@app.route('/')
async def index(request):
    return await response.file(os.path.join(webroot_path, 'index.html'))


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
    app.add_task(new_conn.run(), name=f"Connection {sid}")

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