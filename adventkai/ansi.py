"""
This was coded based on the ANSI implementation of Dragon Ball Advent Truth, I'm not sure if it represents all of CircleMUD
"""

import re
import random

from rich.text import Text
from rich.ansi import AnsiDecoder

DEFAULT_COLORS = {
    "0": "0",      # Normal
    "1": "0;36",   # Roomname
    "2": "0;32",   # Roomobjs
    "3": "0;33",   # Roompeople
    "4": "0;31",   # Hityou
    "5": "0;32",   # Youhit
    "6": "0;33",   # Otherhit
    "7": "1;33",   # Critical
    "8": "1;33",   # Holler
    "9": "1;33",   # Shout
    "10": "0;33",  # Gossip
    "11": "0;36",  # Auction
    "12": "0;32",  # Congrat
    "13": "0;31",  # Tell
    "14": "0;36",  # Yousay
    "15": "0:37"   # Roomsay
}

COLOR_MAP = {
    "n": "0",

    "d": "0;30",
    "b": "0;34",
    "g": "0;32",
    "c": "0;36",
    "r": "0;31",
    "m": "0;35",
    "y": "0;33",
    "w": "0;37",

    "D": "1;30",
    "B": "1;34",
    "G": "1;32",
    "C": "1;36",
    "R": "1;31",
    "M": "1;35",
    "Y": "1;33",
    "W": "1;37",

    "0": "40",
    "1": "44",
    "2": "42",
    "3": "46",
    "4": "41",
    "5": "45",
    "6": "43",
    "7": "47",

    "l": "5",
    "u": "4",
    "o": "1",
    "e": "7"
}

RANDOM_CODES = ["b", "g", "c", "r", "m", "y", "w", "B", "G", "C", "R", "M", "W", "Y"]

RE_COLOR = re.compile(r"@(n|d|D|b|B|g|G|c|C|r|R|m|M|y|Y|w|W|x|0|1|2|3|4|5|6|7|l|o|u|e|@|\[\d+\])")

def circle_to_ansi(entry: str, colors: dict = None) -> str:
    custom_colors = DEFAULT_COLORS.copy()
    if colors:
        custom_colors.update(colors)

    def replace_color(match_obj):
        m = match_obj.group(1)
        match m:
            case "@":
                return "@"
            case "x":
                code = random.choice(RANDOM_CODES)
                ansi_codes = COLOR_MAP[code]
            case _:
                if m.startswith("["):
                    code = m[1:][:-1]
                    if code in custom_colors:
                        ansi_codes = custom_colors[code]
                    else:
                        return m.group(0)
                else:
                    ansi_codes = COLOR_MAP[m]
        return f"\x1b[{ansi_codes}m"

    return RE_COLOR.sub(replace_color, entry)


def circle_to_rich(entry: str, colors: dict = None) -> Text:
    return Text("\n").join(AnsiDecoder().decode(circle_to_ansi(entry, colors=colors)))


def CircleStrip(entry: str) -> str:

    def replace_color(match_obj):
        m = match_obj.group(1)
        match m:
            case "@":
                return "@"
            case _:
                return ""

    return RE_COLOR.sub(replace_color, entry)