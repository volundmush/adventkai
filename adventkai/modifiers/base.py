from snekmud.modifiers import Modifier as BaseModifier


class Modifier(BaseModifier):

    def is_providing_light(self, obj) -> bool:
        return False

    def is_providing_darkness(self, obj) -> bool:
        return False

    def provides_gravity_tolerance(self, obj) -> int:
        return 0
