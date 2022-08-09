from snekmud.launcher import SnekLauncher as OldLauncher
import adventkai
import os


class Launcher(OldLauncher):
    name = "AdventKai"
    cmdname = "advent"
    root = os.path.abspath(os.path.dirname(adventkai.__file__))
    game_template = os.path.abspath(
        os.path.join(
            os.path.abspath(os.path.dirname(adventkai.__file__)), "profile_template"
        )
    )

    def __init__(self):
        super().__init__()
        self.operations["legacy"] = self.operation_legacy
        self.choices.append("legacy")
        self.known_operations.append("legacy")

    def operation_legacy(self, op, args, unknown):
        self.ready_local(args)
        from mudforge.startup import main
        main(setup_only=True)
        from adventkai.legacy import LegacyImporter
        importer = LegacyImporter()
        importer.run()
