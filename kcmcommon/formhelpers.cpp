// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "formhelpers.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QSpacerItem>

namespace Beveled {

QComboBox *addChoice(QFormLayout *layout, const QString &label, const QString &entry, const QStringList &choices)
{
    auto combo = new QComboBox(layout->parentWidget());
    combo->setObjectName(QStringLiteral("kcfg_") + entry);
    combo->addItems(choices);
    layout->addRow(label, combo);
    return combo;
}

QSlider *addSlider(QFormLayout *layout, const QString &label, const KCoreConfigSkeleton::ItemInt *item, int pageStep, const QString &unit)
{
    QWidget *parent = layout->parentWidget();
    auto slider = new QSlider(Qt::Horizontal, parent);
    slider->setObjectName(QStringLiteral("kcfg_") + item->name());
    slider->setRange(item->minValue().toInt(), item->maxValue().toInt());
    slider->setPageStep(pageStep);
    slider->setTickInterval(pageStep);
    slider->setTickPosition(QSlider::TicksBelow);

    auto value = new QLabel(parent);
    const auto showValue = [value, unit](int v) { value->setText(QStringLiteral("%1 %2").arg(v).arg(unit)); };
    QObject::connect(slider, &QSlider::valueChanged, value, showValue);
    showValue(slider->value());

    auto row = new QHBoxLayout;
    row->addWidget(slider);
    row->addWidget(value);
    layout->addRow(label, row);
    return slider;
}

QCheckBox *addCheck(QFormLayout *layout, const QString &text, const QString &entry)
{
    auto check = new QCheckBox(text, layout->parentWidget());
    check->setObjectName(QStringLiteral("kcfg_") + entry);
    layout->addRow(QString(), check);
    return check;
}

void addSpacer(QFormLayout *layout)
{
    layout->addItem(new QSpacerItem(0, layout->parentWidget()->fontMetrics().height() / 2));
}

int widgetValue(const QWidget *widget)
{
    if (auto combo = qobject_cast<const QComboBox *>(widget)) {
        return combo->currentIndex();
    }
    if (auto check = qobject_cast<const QCheckBox *>(widget)) {
        return check->isChecked();
    }
    if (auto slider = qobject_cast<const QSlider *>(widget)) {
        return slider->value();
    }
    return -1;
}

void setWidgetValue(QWidget *widget, int value)
{
    if (auto combo = qobject_cast<QComboBox *>(widget)) {
        combo->setCurrentIndex(value);
    } else if (auto check = qobject_cast<QCheckBox *>(widget)) {
        check->setChecked(value != 0);
    } else if (auto slider = qobject_cast<QSlider *>(widget)) {
        slider->setValue(value);
    }
}

} // namespace Beveled
