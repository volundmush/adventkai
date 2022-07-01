from .utils import callables_from_module
import adventkai
import mudforge
import os
import sys


def early_launch():
    os.environ["DJANGO_ALLOW_ASYNC_UNSAFE"] = "true"
    import django
    from django.conf import settings
    from game_ext import django_settings
    settings.configure(default_settings=django_settings)
    django.setup()



async def pre_start(entrypoint=None, services=None):
    mod_paths = mudforge.CONFIG.get("modifiers", list())

    for mod_path in mod_paths:
        for k, v in callables_from_module(mod_path).items():
            modifier = v()
            adventkai.MODIFIERS_NAMES[modifier.category][str(modifier)] = modifier
            adventkai.MODIFIERS_ID[modifier.category][int(modifier)] = modifier