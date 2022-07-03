from django.db import models
from django.contrib.auth.base_user import AbstractBaseUser
from django.contrib.auth.models import PermissionsMixin, UserManager
from django.utils.translation import gettext_lazy as _
from django.utils import timezone
from django.core import validators


class SimpleUserNameValidator(validators.RegexValidator):
    regex = r"^\w+$"
    message = _(
        "Enter a valid username. This value may contain only letters, "
        "numbers, and underscores."
    )
    flags = 0


class Account(AbstractBaseUser, PermissionsMixin):
    username_validator = SimpleUserNameValidator()
    objects = UserManager()

    username = models.CharField(
        _("username"),
        max_length=150,
        unique=True,
        help_text=_(
            "Required. 150 characters or fewer. Letters, digits and @/./+/-/_ only."
        ),
        validators=[username_validator],
        error_messages={
            "unique": _("A user with that username already exists."),
        },
    )

    is_staff = models.BooleanField(
        _("staff status"),
        default=False,
        help_text=_("Designates whether the user can log into this admin site."),
    )
    is_active = models.BooleanField(
        _("active"),
        default=True,
        help_text=_(
            "Designates whether this user should be treated as active. "
            "Unselect this instead of deleting accounts."
        ),
    )

    email = models.EmailField(_("email address"), blank=True)

    date_joined = models.DateTimeField(_("date joined"), default=timezone.now)

    rpp_current = models.PositiveIntegerField(_("roleplay points"), null=False, blank=False, default=0)
    rpp_total = models.PositiveIntegerField(_("roleplay total"), null=False, blank=False, default=0)
    max_slots = models.PositiveIntegerField(_("character slots"), null=False, blank=False, default=3)

    class Meta:
        verbose_name = _("user")
        verbose_name_plural = _("users")
        swappable = "AUTH_USER_MODEL"
