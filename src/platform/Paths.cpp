#include "platform/Paths.h"

#include <QDir>
#include <QStandardPaths>

namespace Artemis::Paths {

QString dataRoot()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString stateRoot()
{
    const auto generic = QStandardPaths::writableLocation(QStandardPaths::GenericStateLocation);
    return QDir(generic).filePath(QStringLiteral("artemis"));
}

QString databasePath()
{
    return QDir(dataRoot()).filePath(QStringLiteral("artemis.sqlite3"));
}

QString logRoot()
{
    return QDir(stateRoot()).filePath(QStringLiteral("logs"));
}

bool ensureRuntimeDirectories(QString *error)
{
    for (const auto &path : {dataRoot(), logRoot()}) {
        if (!QDir().mkpath(path)) {
            if (error)
                *error = QStringLiteral("Could not create %1").arg(path);
            return false;
        }
    }
    return true;
}

} // namespace Artemis::Paths
