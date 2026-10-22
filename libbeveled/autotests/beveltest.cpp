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

    void sunkenHard()
    {
        check(QSize(6, 5), 1, [](QPainter *painter, const QPalette &p) { drawBevel(painter, QRect(0, 0, 6, 5), BevelStyle::SunkenHard, p); },
            {
                "sssssW",
                "s###WW",
                "s#??WW",
                "sWWWWW",
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

    // The gloss brightens the top half more than the bottom, with a sharp edge in the middle,
    // and stays inside its rectangle.
    void gloss()
    {
        for (const int scale : {1, 2}) {
            const QImage image = TestUtils::render(QSize(10, 20), scale, [](QPainter *painter) {
                painter->fillRect(QRect(0, 0, 10, 20), QColor(0, 0, 0));
                drawGloss(painter, QRect(0, 2, 10, 16), QColor(255, 255, 255));
            });
            const auto level = [&](int logicalY) { return qRed(image.pixel(5 * scale, logicalY * scale)); };
            QCOMPARE(level(0), 0);
            QCOMPARE(level(19), 0);
            QVERIFY(level(2) > level(9)); // reflection fades towards the middle
            QVERIFY(level(9) > level(10)); // sharp edge
            QVERIFY(level(17) > level(10)); // glow at the bottom
        }
    }

    // Rectangles too small for a bevel, empty or inverted (e.g. a window narrower than its
    // borders) must not draw outside themselves.
    void degenerate()
    {
        const QPalette p = TestUtils::testPalette();
        const QList<QRect> rects = {QRect(2, 2, 1, 1), QRect(2, 2, 3, 2), QRect(2, 2, 0, 0), QRect(4, 4, -3, -2)};
        for (const QRect &rect : rects) {
            for (const int scale : {1, 2}) {
                const QImage image = TestUtils::render(QSize(8, 8), scale, [&](QPainter *painter) {
                    drawBevel(painter, rect, BevelStyle::Window, p);
                    fillRect(painter, rect, p.color(QPalette::Button));
                });
                const QRect allowed = rect.isValid() ? QRect(rect.topLeft() * scale, rect.size() * scale) : QRect();
                for (int y = 0; y < image.height(); ++y) {
                    for (int x = 0; x < image.width(); ++x) {
                        if (!allowed.contains(x, y)) {
                            QVERIFY2(image.pixel(x, y) == TestUtils::Untouched.rgb(),
                                qPrintable(QStringLiteral("%1x%2 at scale %3: pixel %4,%5").arg(rect.width()).arg(rect.height()).arg(scale).arg(x).arg(y)));
                        }
                    }
                }
            }
        }
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
