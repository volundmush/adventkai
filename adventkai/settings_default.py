from snekmud.settings_default import *

NAME = "AdventKai"

SERVICES["game"] = "adventkai.game.GameService"

CLASSES["GameSessionHandler"] = "adventkai.handlers.GameSessionHandler"

MODIFIERS.extend([f"adventkai.modifiers.{x}" for x in [
    "admin_flags", "affects", "android", "bonuses", "genomes", "item_flags", "item_types", "mob_flags",
    "mutations", "player_flags", "positions", "preference_flags", "races", "room_flags", "room_sectors",
    "sensei", "transformations", "wear_flags", "zone_flags"
]])

HOOKS["early_launch"].append("adventkai.hooks.early_launch")


COMPONENTS.extend([
    "adventkai.components"
])

TEXT_FILES = {x: f"legacy/text/{x}" for x in [
    "greetansi", "greetings", "motd", "wizlist"
]}

EQUIP_CLASS_PATHS.extend([
    "adventkai.equip"
])

OPERATION_CLASS_PATHS.extend([
    "adventkai.operations.misc", "adventkai.operations.display", "adventkai.operations.search",
    "adventkai.operations.location", "adventkai.operations.triggers"
])

COMMAND_PATHS.extend([
    "adventkai.commands.entity.admin", "adventkai.commands.entity.info", "adventkai.commands.entity.movement",
    "adventkai.commands.entity.comm", "adventkai.commands.entity.action"
])

CMDHANDLERS["Entity"]["Puppet"] = "adventkai.commands.entity.base.ActionPuppetCmdHandler"

DG_VARS = ["adventkai.dgscripts.dgvars"]


DG_FUNCTIONS = defaultdict(list)
DG_FUNCTIONS["shared"].append("adventkai.dgscripts.funcs.shared")
DG_FUNCTIONS["item"].append("adventkai.dgscripts.funcs.item")
DG_FUNCTIONS["character"].append("adventkai.dgscripts.funcs.character")
DG_FUNCTIONS["room"].append("adventkai.dgscripts.funcs.room")


TRAITS = defaultdict(list)
TRAITS["universal"].append("adventkai.traits.universal")
TRAITS["character"].append("adventkai.traits.character")

GETTER_PATHS.append("adventkai.getters")
