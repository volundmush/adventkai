from mudforge.launcher import Launcher as OldLauncher
import adventkai
import os
import sys


class Launcher(OldLauncher):
    name = "AdventKai"
    cmdname = "advent"
    root = os.path.abspath(os.path.dirname(adventkai.__file__))
    game_template = os.path.abspath(
        os.path.join(
            os.path.abspath(os.path.dirname(adventkai.__file__)), "profile_template"
        )
    )

    def operation_passthru(self, op, args, unknown):
        self.set_profile_path(args)
        os.chdir(self.profile_path)
        sys.path.insert(0, os.getcwd())

        import django
        from django.conf import settings
        from game_ext import django_settings
        settings.configure(default_settings=django_settings)
        django.setup()
        from django.core.management import call_command
        call_command(op, *unknown)