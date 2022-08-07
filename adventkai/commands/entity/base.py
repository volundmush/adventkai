from snekmud import COMPONENTS, WORLD, OPERATIONS
from snekmud.commands.base import Command
from snekmud.commands.ent_play import _PlayCommand, EntityPlayCmdHandler as EPCH


class ActionPuppetCmdHandler(EPCH):
    pass


class _DeadCommand(_PlayCommand):
    min_position = 0
    admin_level = 0

    @classmethod
    async def access(cls, **kwargs):
        if (acc := kwargs.get("account", None)):
            return acc.handler.get_admin_level() >= cls.admin_level
        admin_level = await OPERATIONS["GetAdminLevel"](kwargs["entity"]).execute()
        return admin_level >= cls.admin_level

    async def at_pre_execute(self):
        if (pos := WORLD.try_component(self.entity, COMPONENTS["Position"])) and pos.modifier:
            if not pos.modifier.modifier_id <= self.min_position:
                self.send(line=pos.modifier.incap_message)
                return False
        return True


class _RestingCOmmand(_DeadCommand):
    min_position = 5


class _StandCommand(_DeadCommand):
    min_position = 8


class _FightCommand(_DeadCommand):
    min_position = 7