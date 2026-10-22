// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <KCoreConfigSkeleton>

#include <QString>
#include <QStringList>

class QCheckBox;
class QComboBox;
class QFormLayout;
class QSlider;
class QWidget;

namespace Beveled {

// Rows for settings forms. Widgets are named "kcfg_<Entry>", so KCModule loads, saves and resets
// them through KConfigXT without further code.

// Combo box indexes follow the order of the entry's choices in the .kcfg file.
QComboBox *addChoice(QFormLayout *layout, const QString &label, const QString &entry, const QStringList &choices);

// The slider range comes from the <min>/<max> of the entry, so it cannot drift from the schema.
QSlider *addSlider(QFormLayout *layout, const QString &label, const KCoreConfigSkeleton::ItemInt *item, int pageStep, const QString &unit);

QCheckBox *addCheck(QFormLayout *layout, const QString &text, const QString &entry);

// Half a line of vertical space between groups of rows.
void addSpacer(QFormLayout *layout);

// Generic access to the value of a choice, check box or slider (-1 for other widgets).
int widgetValue(const QWidget *widget);
void setWidgetValue(QWidget *widget, int value);

} // namespace Beveled
