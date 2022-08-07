import adventkai
import mudforge
import snekmud
from mudrich.circle import CircleToRich

async def pre_start(entrypoint=None, services=None):
    txt_paths = mudforge.CONFIG.TEXT_FILES

    for k, v in txt_paths.items():
        adventkai.TEXT_FILES[k] = open(v, mode="r").read()

    snekmud.STATIC_TEXT["greet"] = CircleToRich(adventkai.TEXT_FILES["greetansi"])

    snekmud.PY_DICT["adventkai"] = adventkai
