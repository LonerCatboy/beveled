// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

// Entry point loaded by KWin: a factory that creates one Decoration per window.

#include "decoration.h"

#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(BeveledDecorationFactory, "beveled.json", registerPlugin<Beveled::Decoration>();)

#include "plugin.moc"
