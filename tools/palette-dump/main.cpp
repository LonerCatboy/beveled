// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

// Prints the QPalette that KDE derives from a .colors file, i.e. what applications and the
// window decoration will actually receive. Light/Midlight/Mid/Dark/Shadow are computed by
// KColorScheme, not read from the file, so this is the only reliable way to know them.
//
// Usage: palette-dump <file.colors>

#include <KColorScheme>
#include <KSharedConfig>

#include <QGuiApplication>
#include <QMetaEnum>
#include <QPalette>
#include <QTextStream>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTextStream out(stdout);
    if (argc != 2) {
        out << "usage: palette-dump <file.colors>\n";
        return 1;
    }

    const KSharedConfigPtr config = KSharedConfig::openConfig(QString::fromLocal8Bit(argv[1]), KConfig::SimpleConfig);
    const QPalette palette = KColorScheme::createApplicationPalette(config);

    const QList<QPalette::ColorRole> roles = {QPalette::Window, QPalette::WindowText, QPalette::Button, QPalette::ButtonText, QPalette::Light,
        QPalette::Midlight, QPalette::Mid, QPalette::Dark, QPalette::Shadow, QPalette::Base, QPalette::Text, QPalette::Highlight, QPalette::HighlightedText};
    const QMetaEnum roleEnum = QMetaEnum::fromType<QPalette::ColorRole>();

    out << qSetFieldWidth(16) << Qt::left << "role" << "Active" << "Inactive" << "Disabled" << qSetFieldWidth(0) << "\n";
    for (const QPalette::ColorRole role : roles) {
        out << qSetFieldWidth(16) << roleEnum.valueToKey(role);
        for (const QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
            out << palette.color(group, role).name();
        }
        out << qSetFieldWidth(0) << "\n";
    }
    return 0;
}
