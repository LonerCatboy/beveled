// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QImage>
#include <QPainter>
#include <QPalette>
#include <QString>

#include <functional>
#include <string_view>
#include <vector>

// Helpers to render primitives into an image and compare it with an ASCII map.
//
// Map legend (one character per logical pixel, expanded to scale x scale device pixels):
//   '.' Button   'l' Midlight   'W' Light   's' Dark   '#' Shadow   'k' ButtonText
//   '?' untouched (left at the background color)
namespace TestUtils {

using Map = std::vector<std::string_view>;

inline const QColor Untouched(0xff, 0x00, 0xff);

// Every role gets a distinct color so that a wrong role shows up as a mismatch.
inline QPalette testPalette()
{
    QPalette p;
    p.setColor(QPalette::Button, QColor(0xd4, 0xd0, 0xc8));
    p.setColor(QPalette::Midlight, QColor(0xdf, 0xdf, 0xdf));
    p.setColor(QPalette::Light, QColor(0xff, 0xff, 0xff));
    p.setColor(QPalette::Dark, QColor(0x80, 0x80, 0x80));
    p.setColor(QPalette::Mid, QColor(0x90, 0x90, 0x90));
    p.setColor(QPalette::Shadow, QColor(0x40, 0x40, 0x40));
    p.setColor(QPalette::ButtonText, QColor(0x00, 0x00, 0x00));
    return p;
}

inline QColor colorFor(char c, const QPalette &p)
{
    switch (c) {
    case '.':
        return p.color(QPalette::Button);
    case 'l':
        return p.color(QPalette::Midlight);
    case 'W':
        return p.color(QPalette::Light);
    case 's':
        return p.color(QPalette::Dark);
    case '#':
        return p.color(QPalette::Shadow);
    case 'k':
        return p.color(QPalette::ButtonText);
    default:
        return Untouched;
    }
}

inline QChar charFor(QRgb rgb, const QPalette &p)
{
    for (const char c : {'.', 'l', 'W', 's', '#', 'k', '?'}) {
        if (colorFor(c, p).rgb() == rgb) {
            return QLatin1Char(c);
        }
    }
    return QLatin1Char('X'); // a color outside the palette, e.g. from antialiasing
}

// Renders at an integer or fractional scale into an image of logicalSize * scale device pixels.
inline QImage render(const QSize &logicalSize, qreal scale, const std::function<void(QPainter *)> &paint)
{
    QImage image((QSizeF(logicalSize) * scale).toSize(), QImage::Format_RGB32);
    image.setDevicePixelRatio(scale);
    image.fill(Untouched);
    QPainter painter(&image);
    paint(&painter);
    painter.end();
    return image;
}

// Returns an empty string when the image matches the map expanded by scale, otherwise a
// readable description including the actual image as a map (one character per device pixel).
inline QString compare(const QImage &image, const Map &rows, int scale, const QPalette &p)
{
    QString error;
    if (image.height() != int(rows.size()) * scale || image.width() != int(rows.front().size()) * scale) {
        error = QStringLiteral("size mismatch");
    }
    for (int y = 0; error.isEmpty() && y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const char expected = rows.at(y / scale).at(x / scale);
            if (image.pixel(x, y) != colorFor(expected, p).rgb()) {
                error = QStringLiteral("first mismatch at device pixel (%1, %2): expected '%3', got '%4'")
                            .arg(x)
                            .arg(y)
                            .arg(QLatin1Char(expected))
                            .arg(charFor(image.pixel(x, y), p));
                break;
            }
        }
    }
    if (error.isEmpty()) {
        return {};
    }
    error += QStringLiteral("\nactual:\n");
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            error += charFor(image.pixel(x, y), p);
        }
        error += QLatin1Char('\n');
    }
    return error;
}

} // namespace TestUtils
