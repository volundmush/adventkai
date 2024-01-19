from sanic.response import json
from sanic import Sanic
from sanic import Blueprint

from sanic_jwt import Initialize
from sanic_jwt import exceptions, protected, inject_user

from adventkai import settings

from .version1.gamedata import gamedata as gamedata_v1

api = Blueprint.group(gamedata_v1, version=1)

