from django.db import models
from yamlfield.fields import YAMLField


class Module(models.Model):
    name = models.CharField(max_length=20, blank=False, null=False, unique=True)


class Prototype(models.Model):
    module = models.ForeignKey(Module, related_name="prototypes", on_delete=models.PROTECT)
    name = models.CharField(max_length=20, blank=False, null=False)

    class Meta:
        unique_together = (("module", "name"),)


class Entity(models.Model):
    prototype = models.ForeignKey(Prototype, related_name="entities", on_delete=models.PROTECT)
    ent_id = models.CharField(max_length=40, blank=False, null=False)
    data = YAMLField(null=False, blank=False)

    class Meta:
        unique_together = (("prototype", "ent_id"),)
