from adventkai.typing import Entity


class Modifier:
    mod_id = -1

    def __str__(self):
        if hasattr(self.__class__, "name"):
            return self.name
        return self.__class__.__name__

    def __int__(self):
        return self.mod_id

    def is_providing_light(self, ent: Entity) -> bool:
        return False

    def is_providing_darkness(self, ent: Entity) -> bool:
        return False

    def provides_gravity_tolerance(self, ent: Entity) -> int:
        return 0
