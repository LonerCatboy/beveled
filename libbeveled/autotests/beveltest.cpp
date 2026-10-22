// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include <beveled/bevel.h>

#include "testutils.h"

#include <QTest>

using namespace Beveled;

class BevelTest : public QObject
{
    Q_OBJECT

private:
    static void check(const QSize &size, int scale, const std::function<void(QPainter *, const QPalette &)> &paint, const TestUtils::Map &expected)
    {
        const QPalette p = TestUtils::testPalette();
        const QImage image = TestUtils::render(size, scale, [&](QPainter *painter) { paint(painter, p); });
        const QString error = TestUtils::compare(image, expected, scale, p);
        QVERIFY2(error.isEmpty(), qPrintable(error));
    }

private Q_SLOTS:
    void scales_data()
    {
        QTest::addColumn<int>("scale");
        QTest::newRow("100%") << 1;
        QTest::newRow("200%") << 2;
        QTest::newRow("300%") << 3;
    }

    // Window border as in the reference: Midlight/Shadow outside, Light/Dark inside.
    void window_data() { scales_data(); }
    void window()
    {
        QFETCH(int, scale);
        check(QSize(8, 6), scale, [](QPainter *painter, const QPalette &p) { drawBevel(painter, QRect(0, 0, 8, 6), BevelStyle::Window, p); },
            {
                "lllllll#",
                "lWWWWWs#",
                "lW????s#",
                "lW????s#",
                "lssssss#",
                "########",
            });
    }

    // Push and caption buttons: Light/Shadow outside, Midlight/Dark inside.
    void raised_data() { scales_data(); }
    void raised()
    {
        QFETCH(int, scale);
        check(QSize(8, 6), scale, [](QPainter *painter, const QPalette &p) { drawBevel(painter, QRect(0, 0, 8, 6), BevelStyle::Raised, p); },
            {
                "WWWWWWW#",
                "Wllllls#",
                "Wl????s#",
                "Wl????s#",
                "Wssssss#",
                "########",
            });
    }

    void pressed()
    {
        check(QSize(6, 5), 1, [](QPainter *painter, const QPalette &p) { drawBevel(painter, QRect(0, 0, 6, 5), BevelStyle::Pressed, p); },
            {
                "######",
                "#ssss#",
                "#s??s#",
                "#ssss#",
                "######",
            });
    }

    void pushed()
    {
        check(QSize(6, 5), 1, [](QPainter *painter, const QPalette &p) { drawBevel(painter, QRect(0, 0, 6, 5), BevelStyle::Pushed, p); },
            {
                "#####W",
                "#ssslW",
                "#s??lW",
                "#llllW",
                "WWWWWW",
            });
    }

    void raisedHard()
    {
        check(QSize(6, 5), 1, [](QPainter *painter, const QPalette &p) { drawBevel(painter, QRect(0, 0, 6, 5), BevelStyle::RaisedHard, p); },
            {
                "WWWWW#",
                "WWWW##",
                "WW??##",
                "W#####",
                "######",
            });
    }

    void pushedHard()
    {
        check(QSize(6, 5), 1, [](QPainter *painter, const QPalette &p) { drawBevel(painter, QRect(0, 0, 6, 5), BevelStyle::PushedHard, p); },
            {
                "#####W",
                "####WW",
                "##??WW",
                "#WWWWW",
                "WWWWWW",
            });
    }

    void sunken()
    {
        check(QSize(6, 5), 1, [](QPainter *painter, const QPalette &p) { drawBevel(painter, QRect(0, 0, 6, 5), BevelStyle::Sunken, p); },
            {
                "sssssW",
                "s###lW",
                "s#??lW",
                "sllllW",
                "WWWWWW",
            });
    }

    void etched()
    {
        check(QSize(6, 5), 1, [](QPainter *painter, const QPalette &p) { drawBevel(painter, QRect(0, 0, 6, 5), BevelStyle::Etched, p); },
            {
                "sssssW",
                "sWWWsW",
                "sW??sW",
                "sssssW",
                "WWWWWW",
            });
    }

    // Rings wider than one pixel: each ring is a solid band, corners stay blocky.
    void ringWidth()
    {
        check(QSize(10, 8), 1, [](QPainter *painter, const QPalette &p) { drawBevel(painter, QRect(0, 0, 10, 8), BevelStyle::Raised, p, 2); },
            {
                "WWWWWWWW##",
                "WWWWWWWW##",
                "WWllllss##",
                "WWllllss##",
                "WWssssss##",
                "WWssssss##",
                "##########",
                "##########",
            });
        QCOMPARE(bevelThickness(2), 4);
    }

    // A bevel drawn on a translated painter (e.g. inside a child widget).
    void translated()
    {
        check(QSize(6, 5), 2,
            [](QPainter *painter, const QPalette &p) {
                painter->translate(1, 1);
                drawBevel(painter, QRect(0, 0, 4, 3), BevelStyle::Raised, p);
            },
            {
                "??????",
                "?WWW#?",
                "?Wss#?",
                "?####?",
                "??????",
            });
    }

    // Toolbar separator from the reference: Dark then Light, centered.
    void etchedLine()
    {
        check(QSize(4, 3), 2, [](QPainter *painter, const QPalette &p) { drawEtchedLine(painter, QRect(0, 0, 4, 3), Qt::Vertical, p); },
            {
                "?sW?",
                "?sW?",
                "?sW?",
            });
        check(QSize(3, 2), 1, [](QPainter *painter, const QPalette &p) { drawEtchedLine(painter, QRect(0, 0, 3, 2), Qt::Horizontal, p); },
            {
                "sss",
                "WWW",
            });
    }
};

QTEST_MAIN(BevelTest)

#include "beveltest.moc"
