#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "appcontroller.h"
#include "bootstrap.h"
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

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("App"), &controller);
    engine.loadFromModule("YasChoco", "Main");
    return engine.rootObjects().isEmpty() ? 1 : app.exec();
}
