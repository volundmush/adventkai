from django.conf.global_settings import *

# Change this to False in production.
DEBUG = True

USE_TZ = True

DATABASES = {
    "default": {
        "ENGINE": "django.db.backends.sqlite3",
        "NAME": "advent.sqlite",
        "USER": "",
        "PASSWORD": "",
        "HOST": "",
        "PORT": "",
    }
}

# CHANGE THIS IN PRODUCTION!
SECRET_KEY = "advent"

INSTALLED_APPS = [
    "django.contrib.auth",
    "django.contrib.contenttypes",
    "django.contrib.sessions",
    "django.contrib.admin",
    "django.contrib.admindocs",
    "django.contrib.flatpages",
    "django.contrib.sites",
    "django.contrib.staticfiles",
    "django.contrib.messages",
    "adventkai.db.accounts",
    "adventkai.db.entities"
]

AUTH_USER_MODEL = "accounts.Account"
