// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QFile>
#include <QJSEngine>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QTest>

#include <memory>

class LookAndFeelTest : public QObject
{
    Q_OBJECT

    static QString packageFile(const char *path)
    {
        return QStringLiteral(PACKAGE_DIR "/") + QLatin1String(path);
    }

private Q_SLOTS:
    // The splash screen loads without QML errors, fades in at stage 2 and its blocks move.
    void splash()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine, QUrl::fromLocalFile(packageFile("contents/splash/Splash.qml")));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> root(component.create());
        QVERIFY2(root, qPrintable(component.errorString()));
        auto item = qobject_cast<QQuickItem *>(root.get());
        QVERIFY(item);
        item->setSize(QSizeF(800, 600));

        QObject *lane = nullptr;
        for (QObject *child : root->findChildren<QObject *>()) {
            if (child->property("offset").isValid()) {
                lane = child;
            }
        }
        QVERIFY(lane);
        const int start = lane->property("offset").toInt();
        root->setProperty("stage", 2);
        QTRY_VERIFY(lane->property("offset").toInt() != start);
        // Whole-pixel steps keep the blocks crisp.
        const int step = lane->property("step").toInt();
        QVERIFY(step >= 1);
        QCOMPARE((lane->property("offset").toInt() - start) % step, 0);
    }

    // The layout script runs against a stand-in for Plasma's scripting API and builds a docked
    // 32px bottom panel with the classic set of widgets.
    void layoutScript()
    {
        QFile file(packageFile("contents/layouts/org.kde.plasma.desktop-layout.js"));
        QVERIFY(file.open(QIODevice::ReadOnly));

        QJSEngine engine;
        const QJSValue setup = engine.evaluate(QStringLiteral(R"(
            var panels = [];
            function Panel() { this.widgets = []; panels.push(this); }
            Panel.prototype.addWidget = function (name) { this.widgets.push(name); return {}; };
            function currentActivity() { return "activity"; }
            function desktopsForActivity(activity) { return [{}]; }
        )"));
        QVERIFY(!setup.isError());
        const QJSValue result = engine.evaluate(QString::fromUtf8(file.readAll()));
        QVERIFY2(!result.isError(), qPrintable(result.toString()));

        const QJSValue panel = engine.globalObject().property(QStringLiteral("panels")).property(0);
        QCOMPARE(panel.property(QStringLiteral("location")).toString(), QStringLiteral("bottom"));
        QCOMPARE(panel.property(QStringLiteral("height")).toInt(), 32);
        QCOMPARE(panel.property(QStringLiteral("floating")).toBool(), false);
        const QStringList widgets = panel.property(QStringLiteral("widgets")).toVariant().toStringList();
        QCOMPARE(widgets.first(), QStringLiteral("org.kde.plasma.kickoff"));
        QVERIFY(widgets.contains(QStringLiteral("org.kde.plasma.taskmanager")));
        QVERIFY(widgets.contains(QStringLiteral("org.kde.plasma.systemtray")));
        QVERIFY(widgets.contains(QStringLiteral("org.kde.plasma.digitalclock")));
    }
};

QTEST_MAIN(LookAndFeelTest)

#include "lookandfeeltest.moc"
