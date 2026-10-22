// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include <KCModule>
#include <KConfigGroup>
#include <KPluginFactory>
#include <KPluginMetaData>
#include <KSharedConfig>

#include <QComboBox>
#include <QFile>
#include <QSlider>
#include <QStandardPaths>
#include <QTest>

#include <memory>

class KcmTest : public QObject
{
    Q_OBJECT

    // Stands in for the System Settings page that hosts the module and owns its widgets.
    std::unique_ptr<QWidget> m_host;
    std::unique_ptr<KCModule> m_kcm;

    template<typename T>
    T *child(const char *name)
    {
        T *widget = m_kcm->widget()->findChild<T *>(QLatin1String(name));
        if (!widget) {
            qFatal("missing widget %s", name);
        }
        return widget;
    }

    static KConfigGroup group()
    {
        KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("beveledrc"));
        config->reparseConfiguration();
        return config->group(QStringLiteral("Decoration"));
    }

private Q_SLOTS:
    void initTestCase()
    {
        // Guard against writing to the real user configuration.
        QVERIFY(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).contains(QLatin1String("autotests")));
        QFile::remove(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/beveledrc"));

        m_host = std::make_unique<QWidget>();
        const auto result = KPluginFactory::instantiatePlugin<KCModule>(KPluginMetaData(QStringLiteral(KCM_PLUGIN_PATH)), m_host.get());
        QVERIFY2(result, qPrintable(result.errorString));
        m_kcm.reset(result.plugin);
        m_kcm->load();
    }

    void defaults()
    {
        QCOMPARE(child<QComboBox>("kcfg_TitleBarStyle")->currentIndex(), 0);
        QCOMPARE(child<QComboBox>("kcfg_ButtonBevel")->currentIndex(), 0);
        QCOMPARE(child<QComboBox>("kcfg_FrameBevel")->currentIndex(), 0);
        QCOMPARE(child<QComboBox>("kcfg_ButtonHover")->currentIndex(), 0);
        QCOMPARE(child<QComboBox>("kcfg_ShadowStyle")->currentIndex(), 1);
        QCOMPARE(child<QSlider>("kcfg_ShadowDistance")->value(), 4);
        QCOMPARE(child<QSlider>("kcfg_ShadeDepth")->value(), 100);
        QCOMPARE(child<QSlider>("kcfg_ButtonMargin")->value(), 2);
        QCOMPARE(child<QSlider>("kcfg_ButtonMargin")->minimum(), 0);
        QCOMPARE(child<QSlider>("kcfg_ShadeDepth")->minimum(), 50);
        QCOMPARE(child<QSlider>("kcfg_ShadeDepth")->maximum(), 150);
        auto padding = child<QSlider>("kcfg_ButtonPadding");
        QCOMPARE(padding->value(), 0);
        QCOMPARE(padding->minimum(), 0);
        QCOMPARE(padding->maximum(), 4);
    }

    void save()
    {
        child<QComboBox>("kcfg_TitleBarStyle")->setCurrentIndex(1);
        child<QComboBox>("kcfg_ButtonBevel")->setCurrentIndex(1);
        child<QSlider>("kcfg_ButtonPadding")->setValue(3);
        child<QSlider>("kcfg_ButtonMargin")->setValue(0);
        child<QComboBox>("kcfg_ButtonHover")->setCurrentIndex(2);
        child<QComboBox>("kcfg_ShadowStyle")->setCurrentIndex(2);
        child<QSlider>("kcfg_ShadeDepth")->setValue(130);
        QVERIFY(m_kcm->needsSave());
        m_kcm->save();

        const KConfigGroup g = group();
        QCOMPARE(g.readEntry("TitleBarStyle"), QStringLiteral("Solid"));
        QCOMPARE(g.readEntry("ButtonBevel"), QStringLiteral("Hard"));
        QCOMPARE(g.readEntry("ButtonPadding", -1), 3);
        QCOMPARE(g.readEntry("ButtonMargin", -1), 0);
        QCOMPARE(g.readEntry("ButtonHover"), QStringLiteral("Nudge"));
        QCOMPARE(g.readEntry("ShadowStyle"), QStringLiteral("Solid"));
        QCOMPARE(g.config()->group(QStringLiteral("General")).readEntry("ShadeDepth", -1), 130);
    }

    // The distance slider only makes sense with a shadow.
    void distanceDisabledWithoutShadow()
    {
        auto shadow = child<QComboBox>("kcfg_ShadowStyle");
        shadow->setCurrentIndex(0);
        QVERIFY(!child<QSlider>("kcfg_ShadowDistance")->isEnabled());
        shadow->setCurrentIndex(1);
        QVERIFY(child<QSlider>("kcfg_ShadowDistance")->isEnabled());
    }

    // Values outside the schema range are clamped when read back.
    void clamped()
    {
        KConfigGroup g = group();
        g.writeEntry("ButtonPadding", 99);
        g.sync();
        m_kcm->load();
        QCOMPARE(child<QSlider>("kcfg_ButtonPadding")->value(), 4);
    }

    void restoreDefaults()
    {
        m_kcm->defaults();
        m_kcm->save();
        const KConfigGroup g = group();
        QVERIFY(!g.hasKey("TitleBarStyle"));
        QVERIFY(!g.hasKey("ButtonPadding"));
    }

    void cleanupTestCase()
    {
        m_kcm.reset();
        m_host.reset();
    }
};

QTEST_MAIN(KcmTest)

#include "kcmtest.moc"
