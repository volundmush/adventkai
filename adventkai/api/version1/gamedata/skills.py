from sanic.response import json
from sanic import Sanic, response
from sanic import Blueprint

from sanic_jwt import Initialize
from sanic_jwt import exceptions, protected, inject_user

from adventkai import settings
from circlemud import skill_manager

skills = Blueprint("skills", url_prefix="/skills")

@skills.get("/<skill_id:int>")
@protected()
async def get_skill(request, skill_id):
    # Implement logic to retrieve a single skill by ID using skill_manager.get(skill_id)
    if skill := skill_manager.get(skill_id):
        return response.json(skill)
    else:
        return response.json({"error": "Skill not found"}, status=404)


@skills.get("/")
@protected()
async def get_skill_range(request):
    start = request.args.get("start", default=0, type=int)
    end = request.args.get("end", default=999, type=int)
    # Implement logic to retrieve a range of skills using skill_manager.get_range(start, end)
    skills = skill_manager.get_range(start, end)
    return response.json(skills)

@skills.post("/")
@protected()
async def get_multiple_skills(request):
    data = request.json
    ids = data.get("ids", [])
    # Implement logic to retrieve multiple skills by IDs using skill_manager.get_many(ids)
    skills = skill_manager.get_many(ids)
    return response.json(skills)