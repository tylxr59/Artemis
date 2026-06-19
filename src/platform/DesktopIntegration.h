#pragma once

#include <QString>
#include <QVariantList>

namespace Artemis::DesktopIntegration {

QVariantList availableEditors();
bool openFolder(const QString &path);
bool openEditor(const QString &desktopId, const QString &path, QString *error = nullptr);
bool openTerminal(const QString &path, QString *error = nullptr);

}
