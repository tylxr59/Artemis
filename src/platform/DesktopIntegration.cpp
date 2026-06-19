#include "platform/DesktopIntegration.h"

#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSettings>
#include <QSet>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>

#include <algorithm>

namespace Artemis::DesktopIntegration {
namespace {

QString defaultEditorDesktopId()
{
    QStringList candidates;
    for (const auto &path :
         QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation)) {
        candidates.append(QDir(path).filePath(QStringLiteral("mimeapps.list")));
    }
    for (const auto &path :
         QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        candidates.append(QDir(path).filePath(QStringLiteral("applications/mimeapps.list")));
    }
    for (const auto &candidate : candidates) {
        if (!QFileInfo::exists(candidate))
            continue;
        QSettings settings(candidate, QSettings::IniFormat);
        settings.beginGroup(QStringLiteral("Default Applications"));
        const auto desktopIds = settings.value(QStringLiteral("text/plain"))
                                    .toString()
                                    .split(QChar(u';'), Qt::SkipEmptyParts);
        settings.endGroup();
        if (!desktopIds.isEmpty())
            return desktopIds.constFirst();
    }
    return {};
}

QString desktopFilePath(const QString &desktopId)
{
    if (desktopId.isEmpty())
        return {};
    return QStandardPaths::locate(
        QStandardPaths::GenericDataLocation,
        QStringLiteral("applications/%1").arg(desktopId));
}

QVariantMap desktopEditor(const QString &path)
{
    QFile desktopFile(path);
    if (!desktopFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    QHash<QString, QString> values;
    QTextStream stream(&desktopFile);
    bool inDesktopEntry = false;
    while (!stream.atEnd()) {
        const auto line = stream.readLine().trimmed();
        if (line.startsWith(QChar(u'['))) {
            if (inDesktopEntry)
                break;
            inDesktopEntry = line == QStringLiteral("[Desktop Entry]");
            continue;
        }
        if (!inDesktopEntry || line.isEmpty() || line.startsWith(QChar(u'#')))
            continue;
        const auto separator = line.indexOf(QChar(u'='));
        if (separator > 0)
            values.insert(line.left(separator), line.mid(separator + 1));
    }

    if (values.value(QStringLiteral("Type")) != QStringLiteral("Application")
        || values.value(QStringLiteral("Hidden")).compare(
               QStringLiteral("true"), Qt::CaseInsensitive) == 0
        || values.value(QStringLiteral("NoDisplay")).compare(
               QStringLiteral("true"), Qt::CaseInsensitive) == 0
        || values.value(QStringLiteral("Terminal")).compare(
               QStringLiteral("true"), Qt::CaseInsensitive) == 0) {
        return {};
    }

    const auto categories = values.value(QStringLiteral("Categories"))
                                .split(QChar(u';'), Qt::SkipEmptyParts);
    if (!categories.contains(QStringLiteral("TextEditor"))
        && !categories.contains(QStringLiteral("IDE"))) {
        return {};
    }

    const auto name = values.value(QStringLiteral("Name")).trimmed();
    const auto desktopId = QFileInfo(path).fileName();
    if (name.isEmpty() || desktopId.isEmpty())
        return {};
    return {{QStringLiteral("name"), name}, {QStringLiteral("id"), desktopId}};
}

bool launchDesktopApplication(const QString &desktopId, const QString &path)
{
    const auto desktopFile = desktopFilePath(desktopId);
    const auto gio = QStandardPaths::findExecutable(QStringLiteral("gio"));
    return !desktopFile.isEmpty() && !gio.isEmpty()
        && QProcess::startDetached(
            gio, {QStringLiteral("launch"), desktopFile, path}, path);
}

}

QVariantList availableEditors()
{
    QVariantList editors;
    QSet<QString> seenIds;
    for (const auto &dataPath :
         QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        const auto applicationsPath = QDir(dataPath).filePath(QStringLiteral("applications"));
        QDirIterator iterator(applicationsPath, {QStringLiteral("*.desktop")},
                              QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            const auto editor = desktopEditor(iterator.next());
            const auto desktopId = editor.value(QStringLiteral("id")).toString();
            if (desktopId.isEmpty() || seenIds.contains(desktopId))
                continue;
            seenIds.insert(desktopId);
            editors.append(editor);
        }
    }
    std::sort(editors.begin(), editors.end(), [](const QVariant &left, const QVariant &right) {
        return left.toMap().value(QStringLiteral("name")).toString().localeAwareCompare(
                   right.toMap().value(QStringLiteral("name")).toString()) < 0;
    });

    QString defaultLabel = QStringLiteral("System default");
    const auto defaultEditor = desktopEditor(desktopFilePath(defaultEditorDesktopId()));
    const auto defaultName = defaultEditor.value(QStringLiteral("name")).toString();
    if (!defaultName.isEmpty())
        defaultLabel += QStringLiteral(" (%1)").arg(defaultName);
    editors.prepend(QVariantMap{{QStringLiteral("name"), defaultLabel},
                                {QStringLiteral("id"), QString()}});
    return editors;
}

bool openFolder(const QString &path)
{
    return !path.isEmpty() && QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

bool openEditor(const QString &desktopId, const QString &path, QString *error)
{
    if (path.isEmpty())
        return false;

    if (!desktopId.isEmpty()) {
        if (launchDesktopApplication(desktopId, path))
            return true;
        if (error) {
            *error = QStringLiteral(
                "The selected editor is unavailable. Choose another editor in Settings.");
        }
        return false;
    }

    if (launchDesktopApplication(defaultEditorDesktopId(), path))
        return true;

    const auto environment = QProcessEnvironment::systemEnvironment();
    QString editor = environment.value(QStringLiteral("VISUAL")).trimmed();
    if (editor.isEmpty())
        editor = environment.value(QStringLiteral("EDITOR")).trimmed();
    if (!editor.isEmpty()) {
        auto command = QProcess::splitCommand(editor);
        if (!command.isEmpty()) {
            const auto program = command.takeFirst();
            command.append(path);
            if (QProcess::startDetached(program, command, path))
                return true;
        }
    }

    const QStringList fallbacks = {
        QStringLiteral("code"), QStringLiteral("codium"), QStringLiteral("kate"),
        QStringLiteral("gnome-text-editor"), QStringLiteral("gedit")
    };
    for (const auto &fallback : fallbacks) {
        const auto executable = QStandardPaths::findExecutable(fallback);
        if (!executable.isEmpty() && QProcess::startDetached(executable, {path}, path))
            return true;
    }

    if (error) {
        *error = QStringLiteral(
            "Could not find an editor. Set the VISUAL or EDITOR environment variable.");
    }
    return false;
}

bool openTerminal(const QString &path, QString *error)
{
    if (path.isEmpty())
        return false;
    const auto executable = QStandardPaths::findExecutable(QStringLiteral("xdg-terminal-exec"));
    if (!executable.isEmpty() && QProcess::startDetached(executable, {}, path))
        return true;
    if (error)
        *error = QStringLiteral("Could not start a terminal.");
    return false;
}

}
