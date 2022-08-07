from snekmud.launcher import SnekLauncher as OldLauncher
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