// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include <beveled/shadow.h>

#include <QColor>
#include <QTest>

using namespace Beveled;

class ShadowTest : public QObject
{
    Q_OBJECT

private:
    static int alphaAt(const ShadowTexture &s, int x, int y) { return qAlpha(s.image.pixel(x, y)); }

    // The window stand-in sits at the padding offset and spans the image minus the padding.
    static QRect windowRect(const ShadowTexture &s) { return QRect(QPoint(s.padding.left(), s.padding.top()), s.image.size().shrunkBy(s.padding)); }

    // Every pixel under the window is transparent.
    static void checkWindowCleared(const ShadowTexture &s)
    {
        const QRect w = windowRect(s);
        for (int y = w.top(); y <= w.bottom(); ++y) {
            for (int x = w.left(); x <= w.right(); ++x) {
                QVERIFY2(alphaAt(s, x, y) == 0, qPrintable(QStringLiteral("pixel %1,%2 under the window").arg(x).arg(y)));
            }
        }
    }

    // The nine-patch corners must fit in the image, and its stretched center inside the window.
    static void checkNinePatch(const ShadowTexture &s)
    {
        QVERIFY(QRect(QPoint(0, 0), s.image.size()).contains(s.innerRect));
        QVERIFY(windowRect(s).contains(s.innerRect));
        QCOMPARE(s.innerRect.size(), QSize(1, 1));
    }

private Q_SLOTS:
    // Right and bottom bands of exactly distance pixels, flat color, nothing on the left or top.
    void solid()
    {
        const QColor color(64, 64, 64, 100);
        for (int d : {1, 4, 16}) {
            const ShadowTexture s = solidShadow(d, color);
            QCOMPARE(s.padding, QMargins(0, 0, d, d));
            checkNinePatch(s);
            checkWindowCleared(s);

            const QRect w = windowRect(s);
            const QRgb expected = qPremultiply(color.rgba());
            for (int y = 0; y < s.image.height(); ++y) {
                for (int x = 0; x < s.image.width(); ++x) {
                    const bool inShadow = w.translated(d, d).contains(x, y) && !w.contains(x, y);
                    QCOMPARE(s.image.pixel(x, y), inShadow ? expected : qRgba(0, 0, 0, 0));
                }
            }
        }
    }

    void clampedDistance()
    {
        QCOMPARE(solidShadow(0, Qt::black).padding, QMargins(0, 0, MinShadowDistance, MinShadowDistance));
        QCOMPARE(solidShadow(999, Qt::black).padding, QMargins(0, 0, MaxShadowDistance, MaxShadowDistance));
        QCOMPARE(softShadow(-5, Qt::black).padding, softShadow(MinShadowDistance, Qt::black).padding);
    }

    // Symmetric left/right, darker below than above, fading out towards the image border.
    void soft()
    {
        const QColor color(0, 0, 0, 120);
        for (int d : {1, 4, 16}) {
            const ShadowTexture s = softShadow(d, color);
            checkNinePatch(s);
            checkWindowCleared(s);
            QVERIFY(s.padding.bottom() > s.padding.top());
            QCOMPARE(s.padding.left(), s.padding.right());

            const QRect w = windowRect(s);
            const int midX = s.innerRect.x();
            const int midY = s.innerRect.y();
            const int width = s.image.width();
            for (int y = 0; y < s.image.height(); ++y) {
                for (int x = 0; x < width; ++x) {
                    QCOMPARE(alphaAt(s, x, y), alphaAt(s, width - 1 - x, y));
                    QVERIFY(alphaAt(s, x, y) <= color.alpha());
                }
            }
            QVERIFY(alphaAt(s, midX, w.bottom() + 1) > alphaAt(s, midX, std::max(0, w.top() - 1)));
            QVERIFY(alphaAt(s, midX, s.image.height() - 1) < 4);
            QVERIFY(alphaAt(s, 0, midY) < 4);
            // Monotonic fade from the window edge outwards.
            for (int x = w.left() - 1; x > 0; --x) {
                QVERIFY(alphaAt(s, x - 1, midY) <= alphaAt(s, x, midY));
            }
            // The stretched edge row matches a row next to it: the corners hold the whole falloff.
            for (int x = 0; x < width; ++x) {
                QVERIFY(std::abs(alphaAt(s, x, midY) - alphaAt(s, x, midY - 1)) <= 1);
            }
        }
    }
};

QTEST_MAIN(ShadowTest)

#include "shadowtest.moc"
