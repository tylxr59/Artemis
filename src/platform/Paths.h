#pragma once

#include <QString>

namespace Artemis::Paths {
QString dataRoot();
QString stateRoot();
QString databasePath();
QString logRoot();
QString attachmentRoot();
bool ensureRuntimeDirectories(QString *error = nullptr);
}
