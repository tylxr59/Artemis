#include "app/AppController.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QQuickStyle>

int main(int argc, char *argv[])
{
    QGuiApplication::setOrganizationName(QStringLiteral("Artemis"));
    QGuiApplication::setOrganizationDomain(QStringLiteral("artemis.local"));
    QGuiApplication::setApplicationName(QStringLiteral("Artemis"));
    QGuiApplication::setApplicationVersion(QStringLiteral(ARTEMIS_VERSION));

    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));

    Artemis::AppController controller;
    controller.initialize();

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, &app,
                     [](const QList<QQmlError> &warnings) {
        for (const auto &warning : warnings)
            qCritical().noquote() << warning.toString();
    });
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &controller);
    engine.loadFromModule(QStringLiteral("org.artemis"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Artemis failed to load its QML root object";
        return 1;
    }
    return app.exec();
}
