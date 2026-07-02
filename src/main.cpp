#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "appcontroller.h"
#include "bootstrap.h"
#include "thememanager.h"
#include "chocoadapter.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("YAS"));
    app.setApplicationName(QStringLiteral("yas-choco"));
    app.setApplicationDisplayName(QStringLiteral("Yet Another Store for Choco"));

    QQuickStyle::setStyle(QStringLiteral("Basic"));
    yas::loadBundledFonts();

    ChocoAdapter adapter;
    yas::AppController controller(&adapter);
    yas::ThemeManager themeManager;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("App"), &controller);
    engine.rootContext()->setContextProperty(QStringLiteral("YasManager"), &themeManager);
    engine.loadFromModule("YasChoco", "Main");
    return engine.rootObjects().isEmpty() ? 1 : app.exec();
}
