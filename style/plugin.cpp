// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "style.h"

#include <QStylePlugin>

namespace Beveled {

// Entry point Qt uses to find the style by name in <qt plugins>/styles/.
class StylePlugin : public QStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QStyleFactoryInterface_iid FILE "beveled.json")

public:
    QStyle *create(const QString &key) override { return key.compare(QLatin1String("beveled"), Qt::CaseInsensitive) == 0 ? new Style : nullptr; }
};

} // namespace Beveled

#include "plugin.moc"
