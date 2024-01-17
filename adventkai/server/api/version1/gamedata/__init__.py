from sanic.response import json
from sanic import Sanic
from sanic import Blueprint

from sanic_jwt import Initialize
from sanic_jwt import exceptions, protected, inject_user

from adventkai import settings
from .skills import skills as skills_bp

gamedata = Blueprint.group(skills_bp, url_prefix="/gamedata")