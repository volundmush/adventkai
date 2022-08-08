import adventkai
import mudforge
import snekmud
from mudrich.circle import CircleToRich
from snekmud.utils import callables_from_module


def early_launch():
    txt_paths = mudforge.CONFIG.TEXT_FILES

    for k, v in txt_paths.items():
        adventkai.TEXT_FILES[k] = open(v, mode="r").read()

    snekmud.STATIC_TEXT["greet"] = CircleToRich(adventkai.TEXT_FILES["greetansi"])

    snekmud.PY_DICT["adventkai"] = adventkai

    for path in mudforge.CONFIG.DG_VARS:
        for k, v in callables_from_module(path).items():
            if not hasattr(v, "execute"):
                continue
            adventkai.DG_VARS[k.lower()] = v

    for category, paths in mudforge.CONFIG.DG_FUNCTIONS.items():
        for path in paths:
            for k, v in callables_from_module(path).items():
                if not hasattr(v, "execute"):
                    continue
                adventkai.DG_FUNCTIONS[category][k.lower()] = v
                for x in getattr(v, "aliases", list()):
                    adventkai.DG_FUNCTIONS[category][x.lower()] = v

    for category, paths in mudforge.CONFIG.STATS.items():
        for path in paths:
            adventkai.STATS[category].update(callables_from_module(path))
