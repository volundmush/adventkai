import os
import sys
from collections import defaultdict

# The game name
NAME = "adventkai"

# TLS data - this must be paths to PEM and KEY files.
TLS = {"ca": "ca.pem", "cert": "cert.pem", "key": "key.key"}

# Interfaces - This will be used for Telnet, SSH, and similar outward-facing listeners
LISTEN_INTERFACE = "0.0.0.0"

# external ports used by telnet connections.
# Omit them to disable.
TELNET = {
    "plain": 1280,
    #    "tls": 7998
}

# external port used by SSH. This doesn't have a TLS version because
# SSH has its own encryption.
# Omit to disable.
SSH = 7995

# external ports used by the webserver
# Omit them to disable.
WEBSERVER_INTERFACE = "0.0.0.0"
WEBSERVER_PORT = 8000

# PORTAL_WS
PORTAL_URL_TO_WS = "http://127.0.0.1:8000/"

# The hostname to use for the website.
HOSTNAME = "example.com"
SCHEME = "http"


CORES = {
    "portal": "adventkai.portal.Core",
}

PORTAL_SERVICES = {
    "telnet": "adventkai.telnet.TelnetService",
    "telnets": "adventkai.telnet.TLSTelnetService",
}

PORTAL_CLASSES = {"telnet_protocol": "adventkai.telnet.TelnetProtocol"}

# Place to put log files, how often to rotate the log and how big each log file
# may become before rotating.
LOG_DIR = "logs"
SERVER_LOG_DAY_ROTATION = 7
SERVER_LOG_MAX_SIZE = 1000000

CLIENT_DEFAULT_WIDTH = 78

