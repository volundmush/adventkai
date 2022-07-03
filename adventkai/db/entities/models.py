from django.db import models
from yamlfield.fields import YAMLField


class PlayerCharacter(models.Model):
    player_id = models.PositiveIntegerField(null=False, blank=False, unique=True)
    name = models.CharField(max_length=30, blank=False, null=False, unique=True)
    account = models.ForeignKey("accounts.Account", related_name="characters", on_delete=models.PROTECT)
    data = YAMLField(null=False, blank=False)
