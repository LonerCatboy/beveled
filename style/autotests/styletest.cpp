// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include <beveled/shades.h>

#include <QApplication>
#include <QPushButton>
#include <QStyle>
#include <QStyleFactory>
#include <QTest>

#include <memory>

class StyleTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void found()
    {
        QVERIFY(QStyleFactory::keys().contains(QStringLiteral("Beveled"), Qt::CaseInsensitive));
        std::unique_ptr<QStyle> style(QStyleFactory::create(QStringLiteral("beveled")));
        QVERIFY(style);
    }

    // Buttons are more compact than with the plain Windows style, but still fit their label.
    void compactButtons()
    {
        std::unique_ptr<QStyle> beveled(QStyleFactory::create(QStringLiteral("Beveled")));
        std::unique_ptr<QStyle> windows(QStyleFactory::create(QStringLiteral("Windows")));
        QVERIFY(beveled && windows);

        QPushButton button(QStringLiteral("OK"));
        button.setStyle(windows.get());
        const QSize windowsSize = button.sizeHint();
        button.setStyle(beveled.get());
        const QSize beveledSize = button.sizeHint();
        QVERIFY2(beveledSize.width() < windowsSize.width(), qPrintable(QStringLiteral("%1 vs %2").arg(beveledSize.width()).arg(windowsSize.width())));
        QVERIFY(beveledSize.height() <= windowsSize.height());

        const QSize text = button.fontMetrics().size(Qt::TextShowMnemonic, button.text());
        QVERIFY(beveledSize.width() >= text.width() + 4);
        QVERIFY(beveledSize.height() >= text.height() + 4);
        button.setStyle(nullptr);
    }

    // The application palette gets Beveled's 3D shades, as in the window decoration. Qt polishes
    // the base palette (the one from the platform theme, i.e. the KDE color scheme) with the
    // application style.
    void classicShades()
    {
        QApplication::setStyle(QStringLiteral("Beveled"));
        const QPalette applied = QApplication::palette();
        const Beveled::Shades expected = Beveled::shadesForFace(applied.color(QPalette::Button));
        QCOMPARE(applied.color(QPalette::Light), expected.light);
        QCOMPARE(applied.color(QPalette::Dark), expected.dark);
        QCOMPARE(applied.color(QPalette::Mid), expected.dark);
        QCOMPARE(applied.color(QPalette::Shadow), expected.shadow);

        // Not what the plain Windows style would give, or the check above proves nothing.
        QApplication::setStyle(QStringLiteral("Windows"));
        QVERIFY(QApplication::palette().color(QPalette::Dark) != expected.dark);
    }
};

QTEST_MAIN(StyleTest)

#include "styletest.moc"
