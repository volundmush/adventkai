from .utils import callables_from_module
import adventkai
import mudforge


async def pre_start(entrypoint=None, services=None):

    for mod_path in mudforge.CONFIG.get("modifiers", list()):
        for c in callables_from_module(mod_path):
            modifier = c()
            adventkai.MODIFIERS[modifier.category][str(modifier)] = modifier