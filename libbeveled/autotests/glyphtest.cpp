// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include <beveled/bevel.h>
#include <beveled/glyphs.h>

#include "testutils.h"

#include <QTest>

using namespace Beveled;

// Caption buttons compared with the reference screenshot (riferimenti/reference1.png), with
// Midlight shown as 'l' because the test palette keeps it distinct from Button.
class GlyphTest : public QObject
{
    Q_OBJECT

private:
    static void check(Glyph glyph, bool enabled, const TestUtils::Map &expected)
    {
        const QPalette p = TestUtils::testPalette();
        for (const int scale : {1, 2, 3}) {
            const QImage image = TestUtils::render(CaptionButtonSize, scale, [&](QPainter *painter) {
                const QRect button(QPoint(0, 0), CaptionButtonSize);
                painter->fillRect(button, p.color(QPalette::Button));
                drawBevel(painter, button, BevelStyle::Raised, p);
                drawCaptionGlyph(painter, button, glyph, p, enabled);
            });
            const QString error = TestUtils::compare(image, expected, scale, p);
            QVERIFY2(error.isEmpty(), qPrintable(QStringLiteral("scale %1: %2").arg(scale).arg(error)));
        }
    }

private Q_SLOTS:
    // Pressed glyphs move by one unit (scale pixels) down and right.
    void pressedOffset()
    {
        const QPalette p = TestUtils::testPalette();
        for (const int scale : {1, 2, 3}) {
            const auto glyphImage = [&](GlyphOffset offset) {
                return TestUtils::render(CaptionButtonSize, scale, [&](QPainter *painter) {
                    const QRect button(QPoint(0, 0), CaptionButtonSize);
                    painter->fillRect(button, p.color(QPalette::Button));
                    drawCaptionGlyph(painter, button, Glyph::Close, p, true, offset);
                });
            };
            const QImage rest = glyphImage(GlyphOffset::None);
            {
                const int shift = scale;
                const QImage moved = glyphImage(GlyphOffset::Pressed);
                QVERIFY(moved != rest);
                for (int y = shift; y < rest.height(); ++y) {
                    for (int x = shift; x < rest.width(); ++x) {
                        QVERIFY2(moved.pixel(x, y) == rest.pixel(x - shift, y - shift),
                            qPrintable(QStringLiteral("scale %1 shift %2 at %3,%4").arg(scale).arg(shift).arg(x).arg(y)));
                    }
                }
            }
        }
    }

    void minimize()
    {
        check(Glyph::Minimize, true,
            {
                "WWWWWWWWWWWWWWW#",
                "Wllllllllllllls#",
                "Wl............s#",
                "Wl............s#",
                "Wl............s#",
                "Wl............s#",
                "Wl............s#",
                "Wl............s#",
                "Wl............s#",
                "Wl..kkkkkk....s#",
                "Wl..kkkkkk....s#",
                "Wl............s#",
                "Wssssssssssssss#",
                "################",
            });
    }

    void maximize()
    {
        check(Glyph::Maximize, true,
            {
                "WWWWWWWWWWWWWWW#",
                "Wllllllllllllls#",
                "Wl.kkkkkkkkk..s#",
                "Wl.kkkkkkkkk..s#",
                "Wl.k.......k..s#",
                "Wl.k.......k..s#",
                "Wl.k.......k..s#",
                "Wl.k.......k..s#",
                "Wl.k.......k..s#",
                "Wl.k.......k..s#",
                "Wl.kkkkkkkkk..s#",
                "Wl............s#",
                "Wssssssssssssss#",
                "################",
            });
    }

    void restore()
    {
        check(Glyph::Restore, true,
            {
                "WWWWWWWWWWWWWWW#",
                "Wllllllllllllls#",
                "Wl...kkkkkk...s#",
                "Wl...kkkkkk...s#",
                "Wl...k....k...s#",
                "Wl.kkkkkk.k...s#",
                "Wl.kkkkkk.k...s#",
                "Wl.k....kkk...s#",
                "Wl.k....k.....s#",
                "Wl.k....k.....s#",
                "Wl.kkkkkk.....s#",
                "Wl............s#",
                "Wssssssssssssss#",
                "################",
            });
    }

    void close()
    {
        check(Glyph::Close, true,
            {
                "WWWWWWWWWWWWWWW#",
                "Wllllllllllllls#",
                "Wl............s#",
                "Wl..kk....kk..s#",
                "Wl...kk..kk...s#",
                "Wl....kkkk....s#",
                "Wl.....kk.....s#",
                "Wl....kkkk....s#",
                "Wl...kk..kk...s#",
                "Wl..kk....kk..s#",
                "Wl............s#",
                "Wl............s#",
                "Wssssssssssssss#",
                "################",
            });
    }

    // Disabled MDI close button from the reference: embossed glyph.
    void closeDisabled()
    {
        check(Glyph::Close, false,
            {
                "WWWWWWWWWWWWWWW#",
                "Wllllllllllllls#",
                "Wl............s#",
                "Wl..ss....ss..s#",
                "Wl...ss..ssWW.s#",
                "Wl....ssssWW..s#",
                "Wl.....ssWW...s#",
                "Wl....ssss....s#",
                "Wl...ssWWss...s#",
                "Wl..ssWW..ss..s#",
                "Wl...WW....WW.s#",
                "Wl............s#",
                "Wssssssssssssss#",
                "################",
            });
    }
};

QTEST_MAIN(GlyphTest)

#include "glyphtest.moc"
