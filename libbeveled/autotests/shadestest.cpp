// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include <beveled/shades.h>

#include <QPalette>
#include <QTest>

using namespace Beveled;

class ShadesTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // The classic face must give the classic 3D colors exactly.
    void classicFace()
    {
        const Shades s = shadesForFace(QColor(212, 208, 200));
        QCOMPARE(s.light, QColor(255, 255, 255));
        QCOMPARE(s.midlight, QColor(212, 208, 200));
        QCOMPARE(s.dark, QColor(128, 128, 128));
        QCOMPARE(s.shadow, QColor(64, 64, 64));
    }

    void depth()
    {
        const QColor face(212, 208, 200);
        const Shades flat = shadesForFace(face, 0.0);
        QCOMPARE(flat.dark, QColor(208, 208, 208));
        QCOMPARE(flat.shadow, QColor(208, 208, 208));

        const Shades deep = shadesForFace(face, 1.5);
        QVERIFY(deep.dark.red() < 128);
        QVERIFY(deep.shadow.red() < deep.dark.red());

        const Shades extreme = shadesForFace(face, 10.0);
        QCOMPARE(extreme.shadow, QColor(0, 0, 0));
    }

    // Dark faces: a softer highlight that keeps the order light > face > dark > shadow.
    void darkFace()
    {
        const QColor face(60, 60, 64);
        const Shades s = shadesForFace(face);
        QVERIFY(s.light.red() > face.red() + 40);
        QVERIFY(s.light.red() < 200);
        QVERIFY(s.light.blue() >= s.light.red()); // the face's hue is kept
        QCOMPARE(s.midlight, face);
        QVERIFY(s.dark.red() < face.red());
        QVERIFY(s.shadow.red() < s.dark.red());
    }

    // Faces lighter than the classic gray keep a pure white highlight.
    void lightFace() { QCOMPARE(shadesForFace(QColor(240, 240, 240)).light, QColor(255, 255, 255)); }

    void mixColors()
    {
        QCOMPARE(mix(QColor(0, 0, 0), QColor(255, 255, 255), 0.0), QColor(0, 0, 0));
        QCOMPARE(mix(QColor(0, 0, 0), QColor(255, 255, 255), 1.0), QColor(255, 255, 255));
        QCOMPARE(mix(QColor(0, 100, 200), QColor(100, 200, 0), 0.5), QColor(50, 150, 100));
        QCOMPARE(mix(QColor(10, 10, 10, 77), QColor(255, 255, 255), 0.0).alpha(), 77);
    }

    void applyToPalette()
    {
        QPalette p;
        p.setColor(QPalette::Active, QPalette::Button, QColor(212, 208, 200));
        p.setColor(QPalette::Disabled, QPalette::Button, QColor(212, 208, 200));
        applyShades(p);
        QCOMPARE(p.color(QPalette::Active, QPalette::Dark), QColor(128, 128, 128));
        QCOMPARE(p.color(QPalette::Active, QPalette::Mid), QColor(128, 128, 128));
        QCOMPARE(p.color(QPalette::Disabled, QPalette::Shadow), QColor(64, 64, 64));
    }
};

QTEST_MAIN(ShadesTest)

#include "shadestest.moc"
