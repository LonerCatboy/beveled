// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include <beveled/pixelpainter.h>

#include "testutils.h"

#include <QTest>

using namespace Beveled;

class PixelPainterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void unit_data()
    {
        QTest::addColumn<qreal>("scale");
        QTest::addColumn<int>("unit");
        QTest::newRow("100%") << 1.0 << 1;
        QTest::newRow("125%") << 1.25 << 1;
        QTest::newRow("150%") << 1.5 << 1;
        QTest::newRow("200%") << 2.0 << 2;
        QTest::newRow("300%") << 3.0 << 3;
    }

    void unit()
    {
        QFETCH(qreal, scale);
        QFETCH(int, unit);
        TestUtils::render(QSize(4, 4), scale, [&](QPainter *painter) {
            PixelPainter pp(painter);
            QVERIFY(pp.isPixelAligned());
            QCOMPARE(pp.unit(), unit);
        });
    }

    // Adjacent logical rectangles must map to adjacent device rectangles at any scale.
    void adjacentRectsMeet_data()
    {
        QTest::addColumn<qreal>("scale");
        QTest::newRow("125%") << 1.25;
        QTest::newRow("150%") << 1.5;
        QTest::newRow("175%") << 1.75;
    }

    void adjacentRectsMeet()
    {
        QFETCH(qreal, scale);
        TestUtils::render(QSize(32, 4), scale, [&](QPainter *painter) {
            PixelPainter pp(painter);
            for (int x = 0; x < 30; ++x) {
                const QRect a = pp.toDevice(QRectF(x, 0, 1, 1));
                const QRect b = pp.toDevice(QRectF(x + 1, 0, 1, 1));
                QCOMPARE(a.right() + 1, b.left());
            }
        });
    }

    // A translated painter (as used when painting child widgets) still lands on the grid.
    void followsWorldTransform()
    {
        TestUtils::render(QSize(10, 10), 2.0, [&](QPainter *painter) {
            painter->translate(3, 2);
            PixelPainter pp(painter);
            QCOMPARE(pp.toDevice(QRectF(1, 1, 2, 3)), QRect(8, 6, 4, 6));
        });
    }

    // Surfaces painted upside down (e.g. OpenGL framebuffers in previews) have a negative
    // vertical scale: rects must still land on the grid, with their edges swapped back.
    void mirroredTransform()
    {
        const QPalette p = TestUtils::testPalette();
        const QImage image = TestUtils::render(QSize(4, 5), 2.0, [&](QPainter *painter) {
            painter->setWorldTransform(QTransform(1, 0, 0, -1, 0, 5)); // logical y -> 5 - y
            PixelPainter pp(painter);
            QCOMPARE(pp.unit(), 2);
            QVERIFY(pp.isPixelAligned());
            QCOMPARE(pp.toDevice(QRectF(1, 1, 2, 1)), QRect(2, 6, 4, 2));
            pp.fillRect(pp.toDevice(QRectF(1, 1, 2, 1)), p.color(QPalette::Shadow));
        });
        QCOMPARE(image.pixel(2, 6), p.color(QPalette::Shadow).rgb());
        QCOMPARE(image.pixel(5, 7), p.color(QPalette::Shadow).rgb());
        QCOMPARE(image.pixel(2, 5), TestUtils::Untouched.rgb());
        QCOMPARE(image.pixel(2, 8), TestUtils::Untouched.rgb());
    }

    void fillsDevicePixels()
    {
        const QPalette p = TestUtils::testPalette();
        const QImage image = TestUtils::render(QSize(3, 2), 2.0, [&](QPainter *painter) {
            PixelPainter pp(painter);
            pp.fillRect(QRect(1, 1, 3, 2), p.color(QPalette::Shadow));
        });
        QCOMPARE(image.pixel(0, 0), TestUtils::Untouched.rgb());
        QCOMPARE(image.pixel(1, 1), p.color(QPalette::Shadow).rgb());
        QCOMPARE(image.pixel(3, 2), p.color(QPalette::Shadow).rgb());
        QCOMPARE(image.pixel(4, 2), TestUtils::Untouched.rgb());
        QCOMPARE(image.pixel(3, 3), TestUtils::Untouched.rgb());
    }

    void restoresPainterState()
    {
        TestUtils::render(QSize(4, 4), 2.0, [&](QPainter *painter) {
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->translate(1, 1);
            const QTransform before = painter->worldTransform();
            {
                PixelPainter pp(painter);
            }
            QCOMPARE(painter->worldTransform(), before);
            QVERIFY(painter->testRenderHint(QPainter::Antialiasing));
        });
    }
};

QTEST_MAIN(PixelPainterTest)

#include "pixelpaintertest.moc"
