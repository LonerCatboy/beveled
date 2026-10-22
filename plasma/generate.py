#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later
"""Generates the Beveled Plasma styles (desktoptheme) as pixel-aligned SVG frames.

Plasma draws panels, task buttons, popups and tooltips with nine-patch SVG frames (FrameSvg):
elements named <prefix>-topleft ... <prefix>-bottomright plus <prefix>-hint-*-margin rects
that set the content margins. Every frame here is built pixel by pixel from the same two-ring
bevel rules as libbeveled, and written as 1x1 rectangles merged into horizontal runs, so the
artwork stays on the pixel grid at integer scales.

Colors follow the color scheme: the face is ColorScheme-Background, and the 3D shades are white
or black overlays on it (Plasma styles have no Light/Shadow color roles). The overlay strengths
match the classic shades on the classic face; the dark variant uses a softer highlight.

Usage: generate.py OUTPUT_DIR   writes OUTPUT_DIR/beveled and OUTPUT_DIR/beveled-dark
       generate.py --check DIR  validates a generated theme directory
"""

import gzip
import json
import sys
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from pathlib import Path

# Generated files are artwork: CC-BY-SA-4.0 (the generator itself is GPL).
# REUSE-IgnoreStart
ARTWORK_COPYRIGHT = "SPDX-FileCopyrightText: 2026 LonerCatboy"
ARTWORK_LICENSE = "SPDX-License-Identifier: CC-BY-SA-4.0"
# REUSE-IgnoreEnd
ARTWORK_LICENSE_XML = f"<!--\n{ARTWORK_COPYRIGHT}\n{ARTWORK_LICENSE}\n-->"

PARTS = ["topleft", "top", "topright", "left", "center", "right", "bottomleft", "bottom", "bottomright"]
SIDES = ["top", "bottom", "left", "right"]

# Overlays on the face that reproduce the classic shades: on the 212,208,200 face, black at
# 38.5% gives the 128 gray and black at 69% the 64 gray.
DARK_ALPHA = 0.385
SHADOW_ALPHA = 0.69


@dataclass(frozen=True)
class Variant:
    theme_id: str
    name: str
    description: str
    light_alpha: float  # white overlay for the Light (highlight) color


VARIANTS = [
    Variant("beveled", "Beveled", "Classic beveled panels and popups, for light color schemes", 1.0),
    Variant("beveled-dark", "Beveled Dark", "Classic beveled panels and popups, for dark color schemes", 0.45),
]

# Pixel tokens. Every pixel of a frame is painted face first, then an optional overlay.
FACE = "."  # face only (also Midlight, which equals the face in the classic look)
LIGHT = "W"
DARK = "s"
SHADOW = "#"
CLEAR = " "  # transparent
LIGHTER = "+"  # face with a soft white overlay (active task, like the classic dither)
HIGHLIGHT = "h"  # face tinted with the selection color (task asking for attention)
TEXT = "t"  # text color (tooltip border)

VIEW = "v"  # view background (text fields)
HOVER_TINT = "1"  # selection color at low opacity (list row under the mouse)
SELECTED_TINT = "2"  # selected list row
SELECTED_HOVER_TINT = "3"  # selected list row under the mouse

# Bevels as (outer top/left, outer bottom/right, inner top/left, inner bottom/right).
WINDOW = (FACE, SHADOW, LIGHT, DARK)
RAISED = (LIGHT, SHADOW, FACE, DARK)
PUSHED = (SHADOW, LIGHT, DARK, FACE)
PRESSED = (SHADOW, SHADOW, DARK, DARK)  # pressed push button
SUNKEN = (DARK, LIGHT, SHADOW, FACE)  # text fields
# "Hard" bevels: both rings in the same color, for a crisper, higher-contrast edge.
RAISED_HARD = (LIGHT, SHADOW, LIGHT, SHADOW)
PUSHED_HARD = (SHADOW, LIGHT, SHADOW, LIGHT)

# Task buttons always span the whole panel thickness (the task manager fills its area and ignores
# the panel margins), so their frames carry a transparent inset: the visible bevel then sits
# inside the panel's own bevel at any panel height.
TASK_INSET_X = 1  # half the gap between neighboring tasks
TASK_INSET_Y = 3  # panel bevel (2px) plus a 1px gap
SCROLLBAR_SIZE = 14  # thickness of popup scroll bars (classic: 16, a little slimmer here)


@dataclass
class Frame:
    prefix: str  # "" for frames without a prefix
    grid: list  # pixel map: left/top border + 1px center + right/bottom border
    border_x: int  # width of the corner and left/right elements
    border_y: int  # height of the corner and top/bottom elements
    margins: dict | None  # content margins per side, or None to write no margin hints
    hint_prefix: str | None = None  # prefix of the margin hints, when it differs from prefix


def ring_frame(prefix, rings, margins, center=FACE, inset_x=0, inset_y=0, open_side=None):
    """Concentric one-pixel rings, outer first, around a 1px center.

    Each ring is (top/left color, bottom/right color) and, as in libbeveled, the bottom/right
    color owns the corners. An optional transparent inset surrounds the rings and is added to the
    content margins. open_side drops the border on one side (tabs open towards their page): the
    two adjacent sides then run straight to that edge.
    """
    depth = len(rings)
    width, height = 2 * (inset_x + depth) + 1, 2 * (inset_y + depth) + 1
    grid = [[CLEAR] * width for _ in range(height)]
    for y in range(inset_y, height - inset_y):
        for x in range(inset_x, width - inset_x):
            grid[y][x] = center
    for ring, (top_left, bottom_right) in enumerate(rings):
        left, top = inset_x + ring, inset_y + ring
        right, bottom = width - 1 - left, height - 1 - top
        for x in range(left, right + 1):
            grid[top][x] = top_left
        for y in range(top, bottom + 1):
            grid[y][left] = top_left
        for x in range(left, right + 1):
            grid[bottom][x] = bottom_right
        for y in range(top, bottom + 1):
            grid[y][right] = bottom_right

    if open_side in ("top", "bottom"):
        rows = range(inset_y, inset_y + depth) if open_side == "top" else range(height - inset_y - depth, height - inset_y)
        for y in rows:
            for x in range(inset_x, width - inset_x):
                grid[y][x] = center
            for ring, (top_left, bottom_right) in enumerate(rings):
                grid[y][inset_x + ring] = top_left
                grid[y][width - 1 - inset_x - ring] = bottom_right
    elif open_side in ("left", "right"):
        cols = range(inset_x, inset_x + depth) if open_side == "left" else range(width - inset_x - depth, width - inset_x)
        for x in cols:
            for y in range(inset_y, height - inset_y):
                grid[y][x] = center
            for ring, (top_left, bottom_right) in enumerate(rings):
                grid[inset_y + ring][x] = top_left
                grid[height - 1 - inset_y - ring][x] = bottom_right

    insets = {"left": inset_x, "right": inset_x, "top": inset_y, "bottom": inset_y}
    if margins is not None:
        margins = {side: insets[side] + margins[side] for side in SIDES}
    return Frame(prefix, grid, inset_x + depth, inset_y + depth, margins)


def bevel_frame(prefix, bevel, margins, center=FACE, inset_x=0, inset_y=0, open_side=None):
    """A classic two-ring bevel: bevel is (outer TL, outer BR, inner TL, inner BR)."""
    rings = [(bevel[0], bevel[1]), (bevel[2], bevel[3])]
    return ring_frame(prefix, rings, margins, center, inset_x, inset_y, open_side)


def flat_frame(prefix, token, margins):
    """A flat fill with a 1px border of the same color."""
    return ring_frame(prefix, [(token, token)], margins, token)


def uniform(margin):
    return {side: margin for side in SIDES}


def tooltip_frame():
    return ring_frame("", [(TEXT, TEXT)], uniform(3))


def window_frame(bevel=WINDOW, margins=4):
    return bevel_frame("", bevel, uniform(margins))


def task_frames():
    inset = {"inset_x": TASK_INSET_X, "inset_y": TASK_INSET_Y}
    frames = [bevel_frame(prefix, RAISED_HARD, uniform(3), **inset) for prefix in ["normal", "hover", "minimized"]]
    # The active task looks pushed in, lighter, with its content moved down and right by a pixel.
    pushed = {"top": 4, "left": 4, "bottom": 2, "right": 2}
    frames.append(bevel_frame("focus", PUSHED_HARD, pushed, LIGHTER, **inset))
    frames.append(bevel_frame("attention", RAISED_HARD, uniform(3), HIGHLIGHT, **inset))
    # Progress is drawn over the task: a flat bar in the selection color, inside the bevel.
    bar_x, bar_y = TASK_INSET_X + 2, TASK_INSET_Y + 2
    grid = [[CLEAR] * (2 * bar_x + 1) for _ in range(2 * bar_y + 1)]
    grid[bar_y][bar_x] = HIGHLIGHT  # only the stretched center is painted
    frames.append(Frame("progress", grid, bar_x, bar_y, uniform(0)))
    return frames


# The frames below are used inside popups (application launcher, system tray, calendar, ...).
# Their content margins follow the default style, so layouts keep their usual sizes.


def viewitem_frames():
    # Highlighted list rows: flat, square tints of the selection color.
    return [
        flat_frame("hover", HOVER_TINT, uniform(4)),
        flat_frame("selected", SELECTED_TINT, uniform(4)),
        flat_frame("selected+hover", SELECTED_HOVER_TINT, uniform(4)),
    ]


def lineedit_frames():
    # Text fields: classic sunken field on the view background.
    return [bevel_frame("base", SUNKEN, uniform(6), VIEW)]


def button_frames():
    return [
        bevel_frame("normal", RAISED, uniform(6)),
        bevel_frame("pressed", PRESSED, {"top": 7, "left": 7, "bottom": 5, "right": 5}),
        # Flat tool buttons: a thin raised edge on hover, a thin sunken edge when pressed.
        ring_frame("toolbutton-hover", [(LIGHT, DARK)], uniform(4), CLEAR),
        ring_frame("toolbutton-pressed", [(DARK, LIGHT)], {"top": 5, "left": 5, "bottom": 3, "right": 3}, CLEAR),
    ]


def tabbar_frames():
    # The active tab is raised and open towards its page; inactive tabs are not drawn.
    margins = {"left": 6, "right": 6, "top": 10, "bottom": 10}
    sideways = {"left": 8, "right": 4, "top": 6, "bottom": 6}
    return [
        bevel_frame("north-active-tab", RAISED, margins, open_side="bottom"),
        bevel_frame("south-active-tab", RAISED, margins, open_side="top"),
        bevel_frame("west-active-tab", RAISED, {**sideways, "left": 4, "right": 8}, open_side="right"),
        bevel_frame("east-active-tab", RAISED, sideways, open_side="left"),
    ]


def plasmoidheading_frames():
    # Popup header and footer: the face with an etched line (Dark over Light) towards the
    # content. Both share the unprefixed margin hints, written once with the header.
    face_rows = [[FACE] * 5 for _ in range(3)]
    etched_line = [[DARK] * 5, [LIGHT] * 5]
    header = Frame("header", face_rows + etched_line, 2, 2, uniform(6), hint_prefix="")
    footer = Frame("footer", etched_line + face_rows, 2, 2, None)
    return [header, footer]


def scrollbar_frames():
    # Classic scroll bar: a light track and a raised slider, no arrows (Plasma has none).
    return [
        flat_frame("background-vertical", LIGHTER, None),
        flat_frame("background-horizontal", LIGHTER, None),
        bevel_frame("slider", RAISED, None),
        bevel_frame("mouseover-slider", RAISED, None),
    ]


def fills(token, variant):
    """SVG fills for a pixel token, bottom layer first: (class, fill, opacity)."""
    face = ("ColorScheme-Background", "currentColor", 1.0)
    return {
        FACE: [face],
        LIGHT: [face, (None, "#ffffff", variant.light_alpha)],
        DARK: [face, (None, "#000000", DARK_ALPHA)],
        SHADOW: [face, (None, "#000000", SHADOW_ALPHA)],
        LIGHTER: [face, (None, "#ffffff", variant.light_alpha * 0.5)],
        HIGHLIGHT: [face, ("ColorScheme-Highlight", "currentColor", 0.5)],
        TEXT: [("ColorScheme-Text", "currentColor", 1.0)],
        VIEW: [("ColorScheme-ViewBackground", "currentColor", 1.0)],
        HOVER_TINT: [("ColorScheme-Highlight", "currentColor", 0.25)],
        SELECTED_TINT: [("ColorScheme-Highlight", "currentColor", 0.45)],
        SELECTED_HOVER_TINT: [("ColorScheme-Highlight", "currentColor", 0.55)],
        CLEAR: [],
    }[token]


def part_rects(frame, part):
    """The pixel rectangle (x, y, w, h) of a nine-patch part inside the frame grid."""
    bx, by = frame.border_x, frame.border_y
    cx, cy = len(frame.grid[0]) - 2 * bx, len(frame.grid) - 2 * by
    col = {"left": (0, bx), "center": (bx, cx), "right": (bx + cx, bx)}
    row = {"top": (0, by), "center": (by, cy), "bottom": (by + cy, by)}
    vertical = "top" if part.startswith("top") else "bottom" if part.startswith("bottom") else "center"
    horizontal = "left" if part.endswith("left") else "right" if part.endswith("right") else "center"
    (x, w), (y, h) = col[horizontal], row[vertical]
    return x, y, w, h


def element_name(frame, name):
    return f"{frame.prefix}-{name}" if frame.prefix else name


def hint_name(frame, side):
    prefix = frame.prefix if frame.hint_prefix is None else frame.hint_prefix
    return f"{prefix}-hint-{side}-margin" if prefix else f"hint-{side}-margin"


def svg_document(frames, variant, extra_hints=()):
    """Lays the frames out side by side and writes every part as merged pixel runs."""
    out = [
        ARTWORK_LICENSE_XML,
        '<svg xmlns="http://www.w3.org/2000/svg" version="1.1" shape-rendering="crispEdges">',
        '<style id="current-color-scheme" type="text/css">'
        ".ColorScheme-Background { color:#d4d0c8; } "
        ".ColorScheme-Text { color:#000000; } "
        ".ColorScheme-Highlight { color:#0a246a; } "
        ".ColorScheme-ViewBackground { color:#ffffff; }"
        "</style>",
    ]
    x0 = 0
    for frame in frames:
        width, height = len(frame.grid[0]), len(frame.grid)
        for part in PARTS:
            px, py, pw, ph = part_rects(frame, part)
            out.append(f'<g id="{element_name(frame, part)}">')
            # A transparent rect fixes the element's bounds even when it has no ink.
            out.append(f'<rect x="{x0 + px}" y="{py}" width="{pw}" height="{ph}" fill="none"/>')
            for y in range(py, py + ph):
                x = px
                while x < px + pw:
                    token = frame.grid[y][x]
                    end = x
                    while end < px + pw and frame.grid[y][end] == token:
                        end += 1
                    for css, fill, opacity in fills(token, variant):
                        cls = f' class="{css}"' if css else ""
                        out.append(f'<rect{cls} x="{x0 + x}" y="{y}" width="{end - x}" height="1" fill="{fill}" fill-opacity="{opacity:g}"/>')
                    x = end
            out.append("</g>")
        # Margin hints: rects whose width (left/right) or height (top/bottom) is the margin.
        hy = height + 1
        for i, side in enumerate(SIDES if frame.margins else []):
            m = max(frame.margins[side], 0)
            w, h = (m, 1) if side in ("left", "right") else (1, m)
            if m > 0:
                out.append(f'<rect id="{hint_name(frame, side)}" x="{x0 + i * 6}" y="{hy}" width="{w}" height="{h}" fill="#f0f"/>')
        x0 += max(width, 24) + 4
    # Standalone hints (e.g. the scroll bar thickness): sizes read by Plasma, never drawn.
    for i, (hint, w, h) in enumerate(extra_hints):
        out.append(f'<rect id="{hint}" x="{x0 + i * 20}" y="0" width="{w}" height="{h}" fill="#f0f"/>')
    out.append("</svg>")
    return "\n".join(out) + "\n"


FILES = {
    "widgets/panel-background": lambda: [window_frame(RAISED_HARD, margins=3)],
    "widgets/background": lambda: [window_frame()],
    "dialogs/background": lambda: [window_frame()],
    "widgets/tooltip": lambda: [tooltip_frame()],
    "widgets/tasks": task_frames,
    "widgets/viewitem": viewitem_frames,
    "widgets/lineedit": lineedit_frames,
    "widgets/button": button_frames,
    "widgets/tabbar": tabbar_frames,
    "widgets/plasmoidheading": plasmoidheading_frames,
    "widgets/scrollbar": scrollbar_frames,
}
EXTRA_HINTS = {
    "widgets/scrollbar": [("hint-scrollbar-size", SCROLLBAR_SIZE, SCROLLBAR_SIZE)],
}
# Plasma switches the panel, popups and tooltips to these folders depending on the opacity mode;
# without copies there it would fall back to the default style.
MODE_FOLDERS = ["", "solid/", "opaque/", "translucent/"]
MODE_FILES = ["widgets/panel-background", "widgets/background", "dialogs/background", "widgets/tooltip"]

PLASMARC = f"""# {ARTWORK_COPYRIGHT}
# {ARTWORK_LICENSE}

[AdaptiveTransparency]
enabled=false

[BlurBehindEffect]
enabled=false

[ContrastEffect]
enabled=false
"""


def write_theme(root, variant):
    theme = root / variant.theme_id
    for name, build in FILES.items():
        svg = svg_document(build(), variant, EXTRA_HINTS.get(name, ())).encode()
        folders = MODE_FOLDERS if name in MODE_FILES else [""]
        for folder in folders:
            path = theme / f"{folder}{name}.svgz"
            path.parent.mkdir(parents=True, exist_ok=True)
            # mtime=0 keeps the output byte-identical between builds.
            path.write_bytes(gzip.compress(svg, mtime=0))
    metadata = {
        "KPlugin": {
            "Authors": [{"Name": "LonerCatboy"}],
            "Description": variant.description,
            "Id": variant.theme_id,
            "License": "CC-BY-SA-4.0",
            "Name": variant.name,
            "Version": "0.1",
        },
        "X-Plasma-API": "5.0",
    }
    (theme / "metadata.json").write_text(json.dumps(metadata, indent=4) + "\n")
    (theme / "plasmarc").write_text(PLASMARC)


def check(theme):
    """Every frame has all nine parts, in-bounds geometry, and margin hints."""
    errors = []
    for name, build in FILES.items():
        root = ET.fromstring(gzip.decompress((theme / f"{name}.svgz").read_bytes()))
        ids = {e.get("id") for e in root.iter() if e.get("id")}
        for frame in build():
            for part in PARTS:
                if element_name(frame, part) not in ids:
                    errors.append(f"{name}: missing {element_name(frame, part)}")
            for side in SIDES if frame.margins else []:
                hint = hint_name(frame, side)
                if frame.margins[side] > 0 and hint not in ids:
                    errors.append(f"{name}: missing {hint}")
        all_ids = [e.get("id") for e in root.iter() if e.get("id")]
        for duplicate in {i for i in all_ids if all_ids.count(i) > 1}:
            errors.append(f"{name}: duplicate id {duplicate}")
        for rect in root.iter("{http://www.w3.org/2000/svg}rect"):
            for attr in ("x", "y", "width", "height"):
                value = float(rect.get(attr))
                if value != int(value) or value < 0:
                    errors.append(f"{name}: off-grid {attr}={value}")
    for file in ["metadata.json", "plasmarc"]:
        if not (theme / file).exists():
            errors.append(f"missing {file}")
    return errors


def main(argv):
    if len(argv) == 3 and argv[1] == "--check":
        errors = check(Path(argv[2]))
        for error in errors:
            print(error, file=sys.stderr)
        return 1 if errors else 0
    if len(argv) != 2:
        print(__doc__, file=sys.stderr)
        return 2
    for variant in VARIANTS:
        write_theme(Path(argv[1]), variant)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
