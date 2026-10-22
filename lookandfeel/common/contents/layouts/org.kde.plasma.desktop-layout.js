// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

// Desktop layout of the Beveled global theme, used only when "Use desktop layout from theme" is
// checked in the Global Theme page: a classic taskbar at the bottom, docked (not floating),
// 28px high, with window titles in the task buttons.

var panel = new Panel;
panel.location = "bottom";
panel.height = 28;
panel.floating = false;

panel.addWidget("org.kde.plasma.kickoff");
panel.addWidget("org.kde.plasma.taskmanager");
panel.addWidget("org.kde.plasma.marginsseparator");
panel.addWidget("org.kde.plasma.systemtray");
panel.addWidget("org.kde.plasma.digitalclock");

var desktopsArray = desktopsForActivity(currentActivity());
for (var j = 0; j < desktopsArray.length; j++) {
    desktopsArray[j].wallpaperPlugin = "org.kde.image";
}
