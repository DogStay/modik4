#!/usr/bin/env python3
"""Сверяет контракт между скриптами меню и раскладками TFL_Pasport.

Две проверки, каждая ловит реальный класс поломок:

1. Каждое имя, которое скрипт ищет через FindAnyWidget от корня меню,
   должно существовать в раскладке этого меню. Если имени нет, скрипт
   получает null и блок просто не появляется на экране.

2. Кнопка с текстом не должна иметь нулевую альфу фона. В вебе
   "прозрачный фон" оставляет подпись видимой, а в Enfusion альфа
   кнопки гасит и её подпись — кнопка исчезает целиком.

Запуск из корня репозитория:  python3 TFL_Pasport/check_widget_contract.py
"""

import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
LAYOUTS = os.path.join(HERE, "gui", "layouts")
SCRIPTS = os.path.join(HERE, "scripts")

# Раскладка каждого меню и те раскладки, что монтируются прямо в её корень:
# FindAnyWidget от корня видит и их тоже.
MENUS = {
    "TFLVPPAdmin/MenuTFLAdmin.c": (
        "faction_admin_v2.layout",
        ["TFL_TerritoryChainEditor.layout", "TFL_TerritoryRewardPresetEditor.layout"],
    ),
}

ROOT_RECEIVERS = {"layoutRoot", "M_SUB_WIDGET", "s_Root", "m_Root"}

WIDGET_DECL = re.compile(r"(\w+WidgetClass)\s+([A-Za-z_]\w*)")
FIND_CALL = re.compile(r'(\w+)\s*\.\s*FindAnyWidget\s*\(\s*"([^"]+)"\s*\)')
BUTTON = re.compile(r"ButtonWidgetClass\s+(\w+)\s*\{([^}]*)\}")
BG_COLOR = re.compile(r"(?<!text )color\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)")
BUTTON_TEXT = re.compile(r'\stext\s+"([^"]*)"')


def read(path):
    with open(path, encoding="utf-8", errors="replace") as handle:
        return handle.read()


def widget_names(layout_file):
    text = read(os.path.join(LAYOUTS, layout_file))
    return set(match.group(2) for match in WIDGET_DECL.finditer(text))


def check_contract():
    """Имена, которые скрипт ищет, но которых нет в его раскладке."""
    missing_total = 0
    for script, (main_layout, mounted) in MENUS.items():
        path = os.path.join(SCRIPTS, script)
        if not os.path.exists(path):
            print(f"!! нет файла скрипта: {script}")
            continue

        available = widget_names(main_layout)
        for extra in mounted:
            available |= widget_names(extra)

        looked_up = FIND_CALL.findall(read(path))
        missing = sorted(
            set(name for recv, name in looked_up if recv in ROOT_RECEIVERS and name not in available)
        )
        if missing:
            print(f"### {script} -> {main_layout}: отсутствует {len(missing)}")
            for name in missing:
                print(f"      {name}")
            missing_total += len(missing)
    return missing_total


def check_invisible_buttons():
    """Кнопки с подписью и нулевой альфой фона — невидимы в игре."""
    found = 0
    for layout_file in sorted(os.listdir(LAYOUTS)):
        if not layout_file.endswith(".layout"):
            continue
        text = read(os.path.join(LAYOUTS, layout_file))
        for match in BUTTON.finditer(text):
            body = match.group(2)
            color = BG_COLOR.search(body)
            label = BUTTON_TEXT.search(body)
            if color and float(color.group(4)) == 0.0 and label and label.group(1).strip():
                print(f'### {layout_file}: {match.group(1)} "{label.group(1)}" — альфа 0, кнопка не видна')
                found += 1
    return found


def main():
    missing = check_contract()
    invisible = check_invisible_buttons()
    print()
    print(f"имён без виджета в раскладке: {missing}")
    print(f"кнопок с подписью и альфой 0: {invisible}")
    return 1 if (missing or invisible) else 0


if __name__ == "__main__":
    sys.exit(main())
