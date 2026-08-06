#!/usr/bin/env python3
# Generates TrashSortingMenu.layout from the pixel geometry in
# DESIGN_REFERENCE/TRASH_SORTING_FINAL/dayz_layout_spec.md.
#
# The spec gives absolute pixels on a 1920x1080 canvas. DayZ layouts position a
# child as a fraction of its PARENT, so every coordinate here is converted once,
# in one place, instead of being eyeballed 200 times by hand.

W, H = 1920.0, 1080.0
OUT = []

FONT_BOLD = "gui/fonts/metron-bold14"
FONT_BODY = "gui/fonts/etelkatextpro22"


def rgb(h, a=1.0):
    h = h.lstrip('#')
    r, g, b = int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16)
    return "%s %s %s %s" % (f(a), f(r / 255.0), f(g / 255.0), f(b / 255.0))


def f(v):
    s = "%.6f" % v
    s = s.rstrip('0').rstrip('.')
    return s if s else "0"


# palette (from DESIGN_REFERENCE/TRASH_SORTING_FINAL/README.md)
C_MAIN = "#16181A"
C_HEADER = "#1C1E1C"
C_INSTR = "#131513"
C_WORK = "#1D201F"
C_BINAREA = "#20241A"
C_BIN = "#1A1C19"
C_CARD = "#131513"
C_FOOTER = "#181A18"
C_BTN = "#292D28"
C_B_DIM = "#30352F"
C_B_BASE = "#383D38"
C_B_BRIGHT = "#535A50"
C_T_PRIMARY = "#EDEFEA"
C_T_SECOND = "#A4A9A1"
C_T_MUTED = "#737971"
C_ACCENT = "#A3B877"
C_DANGER = "#A95D5D"
C_MARKER = {"Wood": "#76644E", "Plastic": "#566978", "Metal": "#666B68"}


class Ctx:
    """A parent box in absolute pixels; converts a child box to parent fractions."""

    def __init__(self, x, y, w, h):
        self.x, self.y, self.w, self.h = float(x), float(y), float(w), float(h)

    def rel(self, x, y, w, h):
        return (f((x - self.x) / self.w), f((y - self.y) / self.h),
                f(w / self.w), f(h / self.h))

    def sub(self, x, y, w, h):
        return Ctx(x, y, w, h)


def emit(depth, s):
    OUT.append(" " * depth + s)


def widget(depth, cls, name, ctx, box, *, color=None, textcolor=None,
           font=None, valign=None, halign=None, ignore=None, visible=None,
           exact_text=True, children=None):
    # A container must stay pointer-transparent-but-traversable: marking it
    # ignorepointer would also cut off the buttons nested inside it. Only leaf
    # decoration (backgrounds, labels, stripes, the dim layer) opts out.
    if ignore is None:
        ignore = children is None
    px, py, pw, ph = ctx.rel(*box)
    emit(depth, "%s %s {" % (cls, name))
    d = depth + 1
    if font:
        emit(d, 'font "%s"' % font)
    if textcolor:
        emit(d, '"text color" %s' % textcolor)
    if valign:
        emit(d, '"text valign" %s' % valign)
    if halign:
        emit(d, '"text halign" %s' % halign)
    emit(d, "position %s %s" % (px, py))
    emit(d, "size %s %s" % (pw, ph))
    if color is not None:
        emit(d, "color %s" % color)
    emit(d, "hexactpos 0")
    emit(d, "vexactpos 0")
    emit(d, "hexactsize 0")
    emit(d, "vexactsize 0")
    if ignore:
        emit(d, "ignorepointer 1")
    if visible is not None:
        emit(d, "visible %d" % (1 if visible else 0))
    if font and exact_text:
        emit(d, '"exact text" 1')
    if children:
        emit(d, "{")
        children(d + 1)
        emit(d, "}")
    emit(depth, "}")


def bordered(depth, name, ctx, box, border, fill, body, *, visible=None):
    """Root panel carries the border colour; a 1px-inset panel carries the fill.

    Making the root the border is what lets the script recolour a card or bin
    on hover / correct / wrong with a single SetColor on one widget."""
    x, y, w, h = box
    inner = Ctx(x, y, w, h)

    def kids(d):
        widget(d, "PanelWidgetClass", name + "Bg", inner,
               (x + 1, y + 1, w - 2, h - 2), color=rgb(fill))
        body(d, inner)

    widget(depth, "PanelWidgetClass", name, ctx, box,
           color=rgb(border), visible=visible, ignore=False, children=kids)


# ---------------------------------------------------------------- geometry
ROOT = Ctx(0, 0, W, H)

MP = (320, 160, 1280, 760)          # main panel
HDR = (320, 160, 1280, 84)          # header
INS = (320, 244, 1280, 56)          # instruction strip
WRK = (320, 300, 880, 558)          # work area
BINA = (1200, 300, 400, 558)        # bin column
FTR = (320, 858, 1280, 62)          # footer

GRID = (342, 340, 836, 490)
CARD_W, CARD_H, GAP = 271, 155, 12

emit(0, "PanelWidgetClass Root {")
emit(1, "position 0 0")
emit(1, "size 1 1")
emit(1, "color 0 0 0 0")
emit(1, "hexactpos 0")
emit(1, "vexactpos 0")
emit(1, "hexactsize 0")
emit(1, "vexactsize 0")
emit(1, "{")

D = 2

# world dim
widget(D, "PanelWidgetClass", "DarkOverlay", ROOT, (0, 0, W, H),
       color=rgb("#060705", 0.86), ignore=False)


def main_panel_body(d, mp):
    # ---- header ----
    def header_body(d2, hdr):
        widget(d2, "TextWidgetClass", "TitleText", hdr, (348, 174, 620, 32),
               font=FONT_BOLD, textcolor=rgb(C_T_PRIMARY), valign="center")
        widget(d2, "TextWidgetClass", "ZoneText", hdr, (348, 208, 620, 24),
               font=FONT_BODY, textcolor=rgb(C_T_MUTED), valign="center")
        widget(d2, "TextWidgetClass", "ProgressText", hdr, (1050, 174, 380, 24),
               font=FONT_BOLD, textcolor=rgb(C_T_SECOND), valign="center",
               halign="right")
        # 9 progress pips, 26x6 with a 30px stride, right-aligned under the label
        for i in range(9):
            widget(d2, "PanelWidgetClass", "ProgressPip%d" % i, hdr,
                   (1164 + i * 30, 208, 26, 6), color=rgb(C_B_DIM))

        # close button: border root, fill, label, transparent hit area on top
        def close_body(d3, box):
            widget(d3, "TextWidgetClass", "CloseButtonText", box,
                   (1470, 190, 118, 36), font=FONT_BOLD,
                   textcolor=rgb(C_T_PRIMARY), valign="center", halign="center")
            widget(d3, "ButtonWidgetClass", "CloseButton", box,
                   (1470, 190, 118, 36), color="0 0 0 0", ignore=False)

        bordered(d2, "CloseButtonFrame", hdr, (1470, 190, 118, 36),
                 C_B_BRIGHT, C_BTN, close_body)

    widget(d, "PanelWidgetClass", "HeaderPanel", mp, HDR,
           color=rgb(C_HEADER), children=lambda dd: header_body(dd, Ctx(*HDR)))

    # ---- instruction strip ----
    def instr_body(d2, ins):
        widget(d2, "TextWidgetClass", "InstructionText", ins,
               (348, 254, 720, 36), font=FONT_BODY,
               textcolor=rgb(C_T_SECOND), valign="center")
        widget(d2, "TextWidgetClass", "HintText", ins,
               (1080, 254, 492, 36), font=FONT_BODY,
               textcolor=rgb(C_T_MUTED), valign="center", halign="right")

    widget(d, "PanelWidgetClass", "InstructionPanel", mp, INS,
           color=rgb(C_INSTR), children=lambda dd: instr_body(dd, Ctx(*INS)))

    # ---- work area with the 3x3 card grid ----
    def work_body(d2, wrk):
        widget(d2, "TextWidgetClass", "WorkAreaTitle", wrk, (342, 312, 400, 22),
               font=FONT_BOLD, textcolor=rgb(C_T_MUTED), valign="center")

        def grid_body(d3, grid):
            for i in range(9):
                col, row = i % 3, i // 3
                cx = GRID[0] + col * (CARD_W + GAP)
                cy = GRID[1] + row * (CARD_H + GAP)

                def card_body(d4, card, idx=i, ox=cx, oy=cy):
                    # 3D model host: the mod ships no textures, so the picture
                    # is a live ItemPreviewWidget created by the menu script.
                    widget(d4, "PanelWidgetClass", "TrashPreviewHost%d" % idx,
                           card, (ox + 76, oy + 14, 120, 84), color="0 0 0 0")
                    # shown only when the preview entity cannot be created
                    widget(d4, "TextWidgetClass", "TrashFallback%d" % idx, card,
                           (ox + 8, oy + 40, 255, 32), font=FONT_BODY,
                           textcolor=rgb(C_T_MUTED), valign="center",
                           halign="center", visible=False)
                    widget(d4, "TextWidgetClass", "TrashName%d" % idx, card,
                           (ox + 8, oy + 108, 255, 34), font=FONT_BODY,
                           textcolor=rgb(C_T_SECOND), valign="center",
                           halign="center")
                    widget(d4, "ButtonWidgetClass", "TrashItem%d" % idx, card,
                           (ox, oy, CARD_W, CARD_H), color="0 0 0 0",
                           ignore=False)

                bordered(d3, "TrashCard%d" % i, grid,
                         (cx, cy, CARD_W, CARD_H), C_B_BASE, C_CARD, card_body)

        widget(d2, "PanelWidgetClass", "TrashGrid", wrk, GRID, color="0 0 0 0",
               children=lambda dd: grid_body(dd, Ctx(*GRID)))

    widget(d, "PanelWidgetClass", "WorkArea", mp, WRK,
           color=rgb(C_WORK), children=lambda dd: work_body(dd, Ctx(*WRK)))

    # ---- bin column ----
    def bina_body(d2, ba):
        widget(d2, "TextWidgetClass", "BinAreaTitle", ba, (1222, 312, 300, 22),
               font=FONT_BOLD, textcolor=rgb(C_T_MUTED), valign="center")

        for idx, key in enumerate(["Wood", "Plastic", "Metal"]):
            by = 340 + idx * 172

            def bin_body(d3, box, k=key, y0=by):
                # 3px material stripe along the top edge
                widget(d3, "PanelWidgetClass", "Bin%sMarker" % k, box,
                       (1222, y0, 356, 3), color=rgb(C_MARKER[k]))
                widget(d3, "PanelWidgetClass", "Bin%sPreviewHost" % k, box,
                       (1238, y0 + 32, 96, 96), color="0 0 0 0")
                widget(d3, "TextWidgetClass", "Bin%sFallback" % k, box,
                       (1238, y0 + 64, 96, 32), font=FONT_BODY,
                       textcolor=rgb(C_T_MUTED), valign="center",
                       halign="center", visible=False)
                widget(d3, "TextWidgetClass", "Bin%sLabel" % k, box,
                       (1350, y0 + 28, 210, 30), font=FONT_BOLD,
                       textcolor=rgb(C_T_PRIMARY), valign="center")
                widget(d3, "TextWidgetClass", "Bin%sCounter" % k, box,
                       (1350, y0 + 64, 210, 24), font=FONT_BODY,
                       textcolor=rgb(C_T_MUTED), valign="center")
                widget(d3, "TextWidgetClass", "Bin%sStatus" % k, box,
                       (1350, y0 + 96, 210, 22), font=FONT_BODY,
                       textcolor=rgb(C_T_MUTED), valign="center")

            bordered(d2, "Bin%s" % key, ba, (1222, by, 356, 160),
                     C_B_BASE, C_BIN, bin_body)

    widget(d, "PanelWidgetClass", "BinArea", mp, BINA,
           color=rgb(C_BINAREA), children=lambda dd: bina_body(dd, Ctx(*BINA)))

    # ---- footer ----
    def footer_body(d2, ftr):
        widget(d2, "TextWidgetClass", "TaskText", ftr, (348, 878, 420, 24),
               font=FONT_BODY, textcolor=rgb(C_T_SECOND), valign="center")
        widget(d2, "TextWidgetClass", "SortedText", ftr, (820, 878, 300, 24),
               font=FONT_BODY, textcolor=rgb(C_T_SECOND), valign="center",
               halign="center")
        widget(d2, "TextWidgetClass", "RemainingText", ftr,
               (1140, 878, 200, 24), font=FONT_BODY,
               textcolor=rgb(C_T_MUTED), valign="center", halign="center")

        def cancel_body(d3, box):
            widget(d3, "TextWidgetClass", "CancelButtonText", box,
                   (1352, 871, 220, 36), font=FONT_BOLD,
                   textcolor=rgb(C_T_SECOND), valign="center", halign="center")
            widget(d3, "ButtonWidgetClass", "CancelButton", box,
                   (1352, 871, 220, 36), color="0 0 0 0", ignore=False)

        bordered(d2, "CancelButtonFrame", ftr, (1352, 871, 220, 36),
                 C_B_BRIGHT, C_BTN, cancel_body)

    widget(d, "PanelWidgetClass", "FooterPanel", mp, FTR,
           color=rgb(C_FOOTER), children=lambda dd: footer_body(dd, Ctx(*FTR)))


bordered(D, "MainPanel", ROOT, MP, C_B_BASE, C_MAIN, main_panel_body)


# ---- toast (transient feedback, centred, y=212) ----
def toast_body(d, box):
    widget(d, "TextWidgetClass", "ToastTitle", box, (750, 224, 420, 26),
           font=FONT_BOLD, textcolor=rgb(C_ACCENT), valign="center",
           halign="center")
    widget(d, "TextWidgetClass", "ToastSub", box, (750, 250, 420, 22),
           font=FONT_BODY, textcolor=rgb(C_T_SECOND), valign="center",
           halign="center")


bordered(D, "ToastPanel", ROOT, (750, 212, 420, 64), C_B_BRIGHT, C_CARD,
         toast_body, visible=False)


# ---- result modal ----
def result_root(d, _ctx):
    widget(d, "PanelWidgetClass", "ResultDim", ROOT, (0, 0, W, H),
           color=rgb("#060705", 0.82), ignore=False)

    def result_body(d2, box):
        widget(d2, "TextWidgetClass", "ResultTitle", box, (712, 400, 496, 40),
               font=FONT_BOLD, textcolor=rgb(C_T_PRIMARY), valign="center")
        widget(d2, "PanelWidgetClass", "ResultRule", box, (712, 452, 496, 1),
               color=rgb(C_B_BASE))
        widget(d2, "TextWidgetClass", "ResultBody", box, (712, 466, 496, 52),
               font=FONT_BODY, textcolor=rgb(C_T_SECOND), valign="center")
        widget(d2, "TextWidgetClass", "ResultScore", box, (712, 526, 496, 78),
               font=FONT_BOLD, textcolor=rgb(C_ACCENT), valign="center",
               exact_text=False)

        def finish_body(d3, b2):
            widget(d3, "TextWidgetClass", "FinishButtonText", b2,
                   (712, 622, 496, 46), font=FONT_BOLD,
                   textcolor=rgb(C_ACCENT), valign="center", halign="center")
            widget(d3, "ButtonWidgetClass", "FinishButton", b2,
                   (712, 622, 496, 46), color="0 0 0 0", ignore=False)

        bordered(d2, "FinishButtonFrame", box, (712, 622, 496, 46),
                 C_ACCENT, C_BTN, finish_body)

    bordered(d, "ResultPanel", ROOT, (680, 360, 560, 360), C_B_BRIGHT, C_MAIN,
             result_body)


widget(D, "PanelWidgetClass", "ResultLayer", ROOT, (0, 0, W, H),
       color="0 0 0 0", ignore=False, visible=False,
       children=lambda dd: result_root(dd, ROOT))


# ---- confirm modal ----
def confirm_root(d, _ctx):
    widget(d, "PanelWidgetClass", "ConfirmDim", ROOT, (0, 0, W, H),
           color=rgb("#060705", 0.78), ignore=False)

    def confirm_body(d2, box):
        widget(d2, "TextWidgetClass", "ConfirmTitle", box, (734, 474, 452, 34),
               font=FONT_BOLD, textcolor=rgb(C_T_PRIMARY), valign="center")
        widget(d2, "TextWidgetClass", "ConfirmBody", box, (734, 516, 452, 48),
               font=FONT_BODY, textcolor=rgb(C_T_SECOND), valign="center")

        def yes_body(d3, b2):
            widget(d3, "TextWidgetClass", "ConfirmYesText", b2,
                   (734, 578, 220, 42), font=FONT_BOLD,
                   textcolor=rgb(C_DANGER), valign="center", halign="center")
            widget(d3, "ButtonWidgetClass", "ConfirmYes", b2,
                   (734, 578, 220, 42), color="0 0 0 0", ignore=False)

        def no_body(d3, b2):
            widget(d3, "TextWidgetClass", "ConfirmNoText", b2,
                   (966, 578, 220, 42), font=FONT_BOLD,
                   textcolor=rgb(C_T_PRIMARY), valign="center", halign="center")
            widget(d3, "ButtonWidgetClass", "ConfirmNo", b2,
                   (966, 578, 220, 42), color="0 0 0 0", ignore=False)

        bordered(d2, "ConfirmYesFrame", box, (734, 578, 220, 42),
                 C_DANGER, C_BTN, yes_body)
        bordered(d2, "ConfirmNoFrame", box, (966, 578, 220, 42),
                 C_B_BRIGHT, C_BTN, no_body)

    bordered(d, "ConfirmPanel", ROOT, (700, 440, 520, 200), C_B_BRIGHT, C_MAIN,
             confirm_body)


widget(D, "PanelWidgetClass", "ConfirmLayer", ROOT, (0, 0, W, H),
       color="0 0 0 0", ignore=False, visible=False,
       children=lambda dd: confirm_root(dd, ROOT))


# ---- drag ghost: follows the cursor, so its position is set from script ----
def ghost_body(d, box):
    widget(d, "PanelWidgetClass", "DragGhostPreviewHost", box,
           (0, 0, 150, 120), color="0 0 0 0")
    widget(d, "TextWidgetClass", "DragGhostFallback", box, (0, 44, 150, 32),
           font=FONT_BODY, textcolor=rgb(C_T_MUTED), valign="center",
           halign="center", visible=False)


bordered(D, "DragGhost", ROOT, (0, 0, 150, 120), C_ACCENT, C_CARD,
         ghost_body, visible=False)

emit(1, "}")
emit(0, "}")

import io
import sys

path = sys.argv[1]
with io.open(path, "w", encoding="utf-8", newline="\n") as fh:
    fh.write("\n".join(OUT) + "\n")

opens = sum(l.count("{") for l in OUT)
closes = sum(l.count("}") for l in OUT)
print("wrote %s  lines=%d  braces %d/%d %s"
      % (path, len(OUT), opens, closes, "OK" if opens == closes else "MISMATCH"))
