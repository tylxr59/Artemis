#pragma once

#include <QString>

namespace Artemis::Paths {
QString dataRoot();
QString stateRoot();
QString databasePath();
QString worktreeRoot();
QString logRoot();
bool ensureRuntimeDirectories(QString *error = nullptr);
}
