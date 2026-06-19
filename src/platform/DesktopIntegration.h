#pragma once

#include <QString>
#include <QVariantList>

namespace Artemis::DesktopIntegration {

QVariantList availableEditors();
QVariantList availableTerminals();
bool openFolder(const QString &path);
bool openEditor(const QString &desktopId, const QString &path, QString *error = nullptr);
bool openTerminal(const QString &desktopId, const QString &path, QString *error = nullptr);
void showNotification(const QString &title, const QString &text);

}
