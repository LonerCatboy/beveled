// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include <beveled/shades.h>

#include <KCModule>
#include <KConfigGroup>
#include <KPluginFactory>
#include <KPluginMetaData>
#include <KSharedConfig>

#include <QApplication>
#include <QFrame>
#include <QLibrary>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QSignalSpy>
#include <QSlider>
#include <QStandardPaths>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleOption>
#include <QTabBar>
#include <QTest>

#include <memory>

class StyleTest : public QObject
{
    Q_OBJECT

    static void writeOption(const char *key, const QString &value)
    {
        KConfigGroup group = KSharedConfig::openConfig(QStringLiteral("beveledrc"))->group(QStringLiteral("Style"));
        group.writeEntry(key, value);
        group.sync();
    }

    // Renders a sunken frame as applications create it (QFrame's default line width, as in
    // scroll areas and list views) and returns its pixels.
    static QImage renderSunkenFrame(QStyle *style)
    {
        QFrame frame;
        frame.setStyle(style);
        frame.setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        frame.resize(20, 20);
        return frame.grab().toImage();
    }

private Q_SLOTS:
    void initTestCase()
    {
        // Guard against reading or writing the real user configuration.
        QVERIFY(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).contains(QLatin1String("autotests")));
        QFile::remove(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/beveledrc"));
    }

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

    // Layout margins and spacing come from the options, not from the Windows style's 9-11px.
    void layoutMetrics()
    {
        writeOption("LayoutMargin", QStringLiteral("5"));
        writeOption("LayoutSpacing", QStringLiteral("99")); // out of range: clamped to 12
        std::unique_ptr<QStyle> style(QStyleFactory::create(QStringLiteral("Beveled")));
        QCOMPARE(style->pixelMetric(QStyle::PM_LayoutLeftMargin), 5);
        QCOMPARE(style->pixelMetric(QStyle::PM_LayoutBottomMargin), 5);
        QCOMPARE(style->pixelMetric(QStyle::PM_LayoutHorizontalSpacing), 12);
        QCOMPARE(style->pixelMetric(QStyle::PM_LayoutVerticalSpacing), 12);
        writeOption("LayoutMargin", QStringLiteral("6"));
        writeOption("LayoutSpacing", QStringLiteral("4"));
    }

    // Soft sunken frames show one light pixel at bottom/right (the inner ring is Midlight); the
    // hard variant shows two.
    void sunkenFrames()
    {
        writeOption("SunkenFrame", QStringLiteral("Soft"));
        std::unique_ptr<QStyle> soft(QStyleFactory::create(QStringLiteral("Beveled")));
        writeOption("SunkenFrame", QStringLiteral("Hard"));
        std::unique_ptr<QStyle> hard(QStyleFactory::create(QStringLiteral("Beveled")));
        writeOption("SunkenFrame", QStringLiteral("Soft"));

        const QPalette palette = QApplication::palette();
        const QImage softImage = renderSunkenFrame(soft.get());
        const QImage hardImage = renderSunkenFrame(hard.get());
        const int last = softImage.width() - 1;
        const int mid = softImage.height() / 2;
        QCOMPARE(softImage.pixel(last, mid), palette.color(QPalette::Light).rgb());
        QCOMPARE(softImage.pixel(last - 1, mid), palette.color(QPalette::Midlight).rgb());
        QCOMPARE(hardImage.pixel(last, mid), palette.color(QPalette::Light).rgb());
        QCOMPARE(hardImage.pixel(last - 1, mid), palette.color(QPalette::Light).rgb());
        QCOMPARE(hardImage.pixel(1, mid), palette.color(QPalette::Shadow).rgb());
    }

    // Dolphin's location bar draws its field with a bare PE_FrameLineEdit and hovers its path
    // buttons with a flat PE_PanelButtonCommand: a sunken, filled field, and a thin raised edge
    // inside it that leaves the field visible.
    void locationBar()
    {
        std::unique_ptr<QStyle> style(QStyleFactory::create(QStringLiteral("Beveled")));
        QPalette palette;
        palette.setColor(QPalette::Base, QColor(1, 1, 1));
        palette.setColor(QPalette::Window, QColor(2, 2, 2));
        palette.setColor(QPalette::Light, QColor(3, 3, 3));
        palette.setColor(QPalette::Dark, QColor(4, 4, 4));
        QImage image(30, 20, QImage::Format_RGB32);
        image.fill(palette.color(QPalette::Window));
        QPainter painter(&image);

        QStyleOptionFrame field;
        field.rect = image.rect();
        field.palette = palette;
        field.lineWidth = 2;
        field.state = QStyle::State_Enabled | QStyle::State_MouseOver;
        style->drawPrimitive(QStyle::PE_FrameLineEdit, &field, &painter);
        QStyleOptionButton hover;
        hover.rect = image.rect();
        hover.palette = palette;
        hover.features = QStyleOptionButton::Flat;
        hover.state = QStyle::State_Enabled | QStyle::State_MouseOver;
        style->drawPrimitive(QStyle::PE_PanelButtonCommand, &hover, &painter);
        painter.end();

        QCOMPARE(image.pixel(0, 10), palette.color(QPalette::Dark).rgb()); // sunken field edge
        QCOMPARE(image.pixel(2, 10), palette.color(QPalette::Light).rgb()); // hover edge inside it
        QCOMPARE(image.pixel(27, 10), palette.color(QPalette::Dark).rgb());
        QCOMPARE(image.pixel(15, 10), palette.color(QPalette::Base).rgb()); // field still shows
    }

    // Translucent menus (Plasma's desktop menu) are opaque all the way to their frame.
    void translucentMenu()
    {
        std::unique_ptr<QStyle> style(QStyleFactory::create(QStringLiteral("Beveled")));
        QMenu menu;
        menu.setStyle(style.get());
        menu.setAttribute(Qt::WA_TranslucentBackground);
        menu.addAction(QStringLiteral("One"));
        menu.addAction(QStringLiteral("Two"));
        menu.resize(menu.sizeHint());
        const QImage image = menu.grab().toImage();
        menu.setStyle(nullptr);
        for (int x = 0; x < image.width(); ++x) {
            QCOMPARE(qAlpha(image.pixel(x, image.height() / 2)), 255);
        }
        for (int y = 0; y < image.height(); ++y) {
            QCOMPARE(qAlpha(image.pixel(image.width() / 2, y)), 255);
        }
    }

    // The settings module loads and its sliders take their range from the schema.
    void settingsModule()
    {
        QWidget host;
        const auto result = KPluginFactory::instantiatePlugin<KCModule>(KPluginMetaData(QStringLiteral(STYLE_KCM_PATH)), &host);
        QVERIFY2(result, qPrintable(result.errorString));
        std::unique_ptr<KCModule> kcm(result.plugin);
        kcm->load();
        auto margin = kcm->widget()->findChild<QSlider *>(QStringLiteral("kcfg_LayoutMargin"));
        QVERIFY(margin);
        QCOMPARE(margin->value(), 6);
        QCOMPARE(margin->maximum(), 12);
        QVERIFY(kcm->widget()->findChild<QWidget *>(QStringLiteral("kcfg_SunkenFrame")));
    }

    // The page System Settings actually shows: the Application Style page resolves
    // allocate_kstyle_config() and connects to changed(bool), save() and defaults() by name.
    void configPage()
    {
        QLibrary library(QStringLiteral(STYLE_KCM_PATH));
        QVERIFY2(library.load(), qPrintable(library.errorString()));
        using Allocate = QWidget *(*)(QWidget *);
        const auto allocate = reinterpret_cast<Allocate>(library.resolve("allocate_kstyle_config"));
        QVERIFY(allocate);

        std::unique_ptr<QWidget> page(allocate(nullptr));
        QVERIFY(page);
        for (const char *signature : {SIGNAL(changed(bool))}) {
            QVERIFY(page->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(signature + 1).constData()) >= 0);
        }
        for (const char *signature : {SLOT(save()), SLOT(defaults())}) {
            QVERIFY(page->metaObject()->indexOfSlot(QMetaObject::normalizedSignature(signature + 1).constData()) >= 0);
        }

        QSignalSpy changed(page.get(), SIGNAL(changed(bool)));
        auto spacing = page->findChild<QSlider *>(QStringLiteral("kcfg_LayoutSpacing"));
        QVERIFY(spacing);
        QCOMPARE(spacing->value(), 4);
        spacing->setValue(7);
        QVERIFY(!changed.isEmpty());
        QCOMPARE(changed.last().at(0).toBool(), true);

        QMetaObject::invokeMethod(page.get(), "save");
        KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("beveledrc"));
        config->reparseConfiguration();
        QCOMPARE(config->group(QStringLiteral("Style")).readEntry("LayoutSpacing", -1), 7);

        QMetaObject::invokeMethod(page.get(), "defaults");
        QCOMPARE(spacing->value(), 4);
        QMetaObject::invokeMethod(page.get(), "save");
    }

    // Square tabs: the selected tab's left edge is Light right below the top edge, where the
    // classic tab clips the corner (window color there).
    void squareTabs()
    {
        const auto renderTabs = [](const QString &shape) {
            writeOption("TabShape", shape);
            std::unique_ptr<QStyle> style(QStyleFactory::create(QStringLiteral("Beveled")));
            QTabBar tabs;
            tabs.setStyle(style.get());
            tabs.addTab(QStringLiteral("One"));
            tabs.addTab(QStringLiteral("Two"));
            tabs.resize(tabs.sizeHint());
            const QRect first = tabs.tabRect(0);
            const QImage image = tabs.grab().toImage();
            tabs.setStyle(nullptr);
            return image.pixel(first.topLeft() + QPoint(0, 1));
        };
        const QPalette palette = QApplication::palette();
        QCOMPARE(renderTabs(QStringLiteral("Square")), palette.color(QPalette::Light).rgb());
        QVERIFY(renderTabs(QStringLiteral("Classic")) != palette.color(QPalette::Light).rgb());
        writeOption("TabShape", QStringLiteral("Square"));
    }

    // When the options change, the application's style is replaced by a fresh instance, so
    // running applications pick up the change.
    void liveReload()
    {
        writeOption("LayoutSpacing", QStringLiteral("4"));
        QApplication::setStyle(QStringLiteral("Beveled"));
        QStyle *before = QApplication::style();
        QCOMPARE(before->pixelMetric(QStyle::PM_LayoutHorizontalSpacing), 4);

        writeOption("LayoutSpacing", QStringLiteral("9"));
        QVERIFY(QMetaObject::invokeMethod(before, "reparseConfiguration"));
        QTRY_VERIFY(QApplication::style() != before);
        QCOMPARE(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing), 9);
        writeOption("LayoutSpacing", QStringLiteral("4"));
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
