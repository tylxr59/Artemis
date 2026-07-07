/****************************************************************************
** Meta object code from reading C++ file 'AppController.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../artemis-git/src/app/AppController.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AppController.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN7Artemis13AppControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto Artemis::AppController::qt_create_metaobjectdata<qt_meta_tag_ZN7Artemis13AppControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Artemis::AppController",
        "threadsChanged",
        "",
        "projectThreadsLoaded",
        "projectPath",
        "QVariantList",
        "threads",
        "projectThreadRemoved",
        "threadId",
        "modelsChanged",
        "settingsChanged",
        "selectedProjectChanged",
        "selectedThreadChanged",
        "currentTasksChanged",
        "currentPlanChanged",
        "taskPanelRequested",
        "turnRunningChanged",
        "workingThreadsChanged",
        "completedThreadsChanged",
        "pendingUserInputChanged",
        "turnElapsedChanged",
        "providerReadyChanged",
        "providerSetupChanged",
        "providerIssueChanged",
        "mcpServersChanged",
        "statusTextChanged",
        "statusMessage",
        "text",
        "tokenUsageChanged",
        "diffChanged",
        "gitRepositoryUrlChanged",
        "commitDraftFinished",
        "success",
        "message",
        "commitFinished",
        "commitLockBlocked",
        "promptRestoreRequested",
        "images",
        "chooseProjectFolder",
        "addProject",
        "input",
        "removeSelectedProject",
        "removeProject",
        "index",
        "removeThread",
        "removeProjectThread",
        "projectIndex",
        "selectProject",
        "selectProjectThread",
        "selectThread",
        "createThread",
        "modelId",
        "reasoningEffort",
        "permissionMode",
        "sendPrompt",
        "imageValues",
        "collaborationMode",
        "copyText",
        "pasteClipboardImage",
        "interruptTurn",
        "answerPendingUserInput",
        "answer",
        "refreshGit",
        "generateCommitMessage",
        "commitAllAndPush",
        "subject",
        "body",
        "commitFeatureBranch",
        "branch",
        "remote",
        "retryLockedCommit",
        "removeCommitLockAndRetry",
        "cancelLockedCommit",
        "suggestBranch",
        "openProjectFolder",
        "openProjectEditor",
        "openTerminal",
        "refreshMcpServers",
        "reloadMcpServers",
        "loginMcpServer",
        "name",
        "addMcpServer",
        "transport",
        "target",
        "removeMcpServer",
        "projects",
        "ProjectTreeModel*",
        "conversation",
        "ConversationModel*",
        "models",
        "codingModelId",
        "codingReasoningEffort",
        "commitModelId",
        "titleModelId",
        "editorOptions",
        "selectedEditorId",
        "terminalOptions",
        "selectedTerminalId",
        "selectedProjectIndex",
        "selectedProjectPath",
        "selectedProjectName",
        "selectedProjectIsGit",
        "selectedThreadId",
        "selectedThreadTitle",
        "selectedThreadInfo",
        "QVariantMap",
        "currentTasks",
        "currentPlan",
        "currentPlanExplanation",
        "turnRunning",
        "workingThreadIds",
        "completedThreadIds",
        "hasPendingUserInput",
        "pendingUserInputQuestion",
        "pendingUserInputQuestionNumber",
        "pendingUserInputQuestionCount",
        "turnElapsedText",
        "providerReady",
        "providerSetupRequired",
        "providerSetupInstructions",
        "providerIssueText",
        "providerVersion",
        "mcpServers",
        "mcpBusy",
        "mcpIssueText",
        "mcpLoginUrl",
        "statusText",
        "contextTokens",
        "totalProcessedTokens",
        "modelContextWindow",
        "contextUsagePercent",
        "hasTokenUsage",
        "diffText",
        "gitStatusText",
        "gitRepositoryUrl",
        "hasGitChanges",
        "databasePath"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'threadsChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'projectThreadsLoaded'
        QtMocHelpers::SignalData<void(const QString &, const QVariantList &)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 4 }, { 0x80000000 | 5, 6 },
        }}),
        // Signal 'projectThreadRemoved'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 4 }, { QMetaType::QString, 8 },
        }}),
        // Signal 'modelsChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'settingsChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'selectedProjectChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'selectedThreadChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'currentTasksChanged'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'currentPlanChanged'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'taskPanelRequested'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'turnRunningChanged'
        QtMocHelpers::SignalData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'workingThreadsChanged'
        QtMocHelpers::SignalData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'completedThreadsChanged'
        QtMocHelpers::SignalData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'pendingUserInputChanged'
        QtMocHelpers::SignalData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'turnElapsedChanged'
        QtMocHelpers::SignalData<void()>(20, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'providerReadyChanged'
        QtMocHelpers::SignalData<void()>(21, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'providerSetupChanged'
        QtMocHelpers::SignalData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'providerIssueChanged'
        QtMocHelpers::SignalData<void()>(23, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'mcpServersChanged'
        QtMocHelpers::SignalData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'statusTextChanged'
        QtMocHelpers::SignalData<void()>(25, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'statusMessage'
        QtMocHelpers::SignalData<void(const QString &)>(26, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 27 },
        }}),
        // Signal 'tokenUsageChanged'
        QtMocHelpers::SignalData<void()>(28, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'diffChanged'
        QtMocHelpers::SignalData<void()>(29, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'gitRepositoryUrlChanged'
        QtMocHelpers::SignalData<void()>(30, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'commitDraftFinished'
        QtMocHelpers::SignalData<void(bool, const QString &)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 32 }, { QMetaType::QString, 33 },
        }}),
        // Signal 'commitFinished'
        QtMocHelpers::SignalData<void(bool, const QString &)>(34, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 32 }, { QMetaType::QString, 33 },
        }}),
        // Signal 'commitLockBlocked'
        QtMocHelpers::SignalData<void(const QString &)>(35, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 33 },
        }}),
        // Signal 'promptRestoreRequested'
        QtMocHelpers::SignalData<void(const QString &, const QVariantList &)>(36, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 27 }, { 0x80000000 | 5, 37 },
        }}),
        // Method 'chooseProjectFolder'
        QtMocHelpers::MethodData<void()>(38, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'addProject'
        QtMocHelpers::MethodData<void(const QString &)>(39, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 40 },
        }}),
        // Method 'removeSelectedProject'
        QtMocHelpers::MethodData<void()>(41, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'removeProject'
        QtMocHelpers::MethodData<void(int)>(42, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 43 },
        }}),
        // Method 'removeThread'
        QtMocHelpers::MethodData<void(int)>(44, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 43 },
        }}),
        // Method 'removeProjectThread'
        QtMocHelpers::MethodData<void(int, const QString &)>(45, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 46 }, { QMetaType::QString, 8 },
        }}),
        // Method 'selectProject'
        QtMocHelpers::MethodData<void(int)>(47, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 43 },
        }}),
        // Method 'selectProjectThread'
        QtMocHelpers::MethodData<void(int, const QString &)>(48, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 46 }, { QMetaType::QString, 8 },
        }}),
        // Method 'selectThread'
        QtMocHelpers::MethodData<void(int)>(49, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 43 },
        }}),
        // Method 'createThread'
        QtMocHelpers::MethodData<void(const QString &, const QString &, const QString &)>(50, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 51 }, { QMetaType::QString, 52 }, { QMetaType::QString, 53 },
        }}),
        // Method 'sendPrompt'
        QtMocHelpers::MethodData<bool(const QString &, const QVariantList &, const QString &, const QString &, const QString &, const QString &)>(54, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 27 }, { 0x80000000 | 5, 55 }, { QMetaType::QString, 51 }, { QMetaType::QString, 52 },
            { QMetaType::QString, 53 }, { QMetaType::QString, 56 },
        }}),
        // Method 'copyText'
        QtMocHelpers::MethodData<void(const QString &)>(57, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 27 },
        }}),
        // Method 'pasteClipboardImage'
        QtMocHelpers::MethodData<QString()>(58, 2, QMC::AccessPublic, QMetaType::QString),
        // Method 'interruptTurn'
        QtMocHelpers::MethodData<void()>(59, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'answerPendingUserInput'
        QtMocHelpers::MethodData<bool(const QString &)>(60, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 61 },
        }}),
        // Method 'refreshGit'
        QtMocHelpers::MethodData<void()>(62, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'generateCommitMessage'
        QtMocHelpers::MethodData<void()>(63, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'commitAllAndPush'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(64, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 65 }, { QMetaType::QString, 66 },
        }}),
        // Method 'commitFeatureBranch'
        QtMocHelpers::MethodData<void(const QString &, const QString &, const QString &, const QString &)>(67, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 65 }, { QMetaType::QString, 66 }, { QMetaType::QString, 68 }, { QMetaType::QString, 69 },
        }}),
        // Method 'retryLockedCommit'
        QtMocHelpers::MethodData<void()>(70, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'removeCommitLockAndRetry'
        QtMocHelpers::MethodData<void()>(71, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'cancelLockedCommit'
        QtMocHelpers::MethodData<void()>(72, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'suggestBranch'
        QtMocHelpers::MethodData<QString(const QString &) const>(73, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 33 },
        }}),
        // Method 'openProjectFolder'
        QtMocHelpers::MethodData<void()>(74, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'openProjectEditor'
        QtMocHelpers::MethodData<void()>(75, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'openTerminal'
        QtMocHelpers::MethodData<void()>(76, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'refreshMcpServers'
        QtMocHelpers::MethodData<void()>(77, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'reloadMcpServers'
        QtMocHelpers::MethodData<void()>(78, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'loginMcpServer'
        QtMocHelpers::MethodData<void(const QString &)>(79, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 80 },
        }}),
        // Method 'addMcpServer'
        QtMocHelpers::MethodData<void(const QString &, const QString &, const QString &)>(81, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 80 }, { QMetaType::QString, 82 }, { QMetaType::QString, 83 },
        }}),
        // Method 'removeMcpServer'
        QtMocHelpers::MethodData<void(const QString &)>(84, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 80 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'projects'
        QtMocHelpers::PropertyData<ProjectTreeModel*>(85, 0x80000000 | 86, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'conversation'
        QtMocHelpers::PropertyData<ConversationModel*>(87, 0x80000000 | 88, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'threads'
        QtMocHelpers::PropertyData<QVariantList>(6, 0x80000000 | 5, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'models'
        QtMocHelpers::PropertyData<QVariantList>(89, 0x80000000 | 5, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 3),
        // property 'codingModelId'
        QtMocHelpers::PropertyData<QString>(90, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'codingReasoningEffort'
        QtMocHelpers::PropertyData<QString>(91, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'commitModelId'
        QtMocHelpers::PropertyData<QString>(92, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'titleModelId'
        QtMocHelpers::PropertyData<QString>(93, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'editorOptions'
        QtMocHelpers::PropertyData<QVariantList>(94, 0x80000000 | 5, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'selectedEditorId'
        QtMocHelpers::PropertyData<QString>(95, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'terminalOptions'
        QtMocHelpers::PropertyData<QVariantList>(96, 0x80000000 | 5, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'selectedTerminalId'
        QtMocHelpers::PropertyData<QString>(97, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'selectedProjectIndex'
        QtMocHelpers::PropertyData<int>(98, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable, 5),
        // property 'selectedProjectPath'
        QtMocHelpers::PropertyData<QString>(99, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'selectedProjectName'
        QtMocHelpers::PropertyData<QString>(100, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'selectedProjectIsGit'
        QtMocHelpers::PropertyData<bool>(101, QMetaType::Bool, QMC::DefaultPropertyFlags, 5),
        // property 'selectedThreadId'
        QtMocHelpers::PropertyData<QString>(102, QMetaType::QString, QMC::DefaultPropertyFlags, 6),
        // property 'selectedThreadTitle'
        QtMocHelpers::PropertyData<QString>(103, QMetaType::QString, QMC::DefaultPropertyFlags, 6),
        // property 'selectedThreadInfo'
        QtMocHelpers::PropertyData<QVariantMap>(104, 0x80000000 | 105, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 6),
        // property 'currentTasks'
        QtMocHelpers::PropertyData<QString>(106, QMetaType::QString, QMC::DefaultPropertyFlags, 7),
        // property 'currentPlan'
        QtMocHelpers::PropertyData<QVariantList>(107, 0x80000000 | 5, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 8),
        // property 'currentPlanExplanation'
        QtMocHelpers::PropertyData<QString>(108, QMetaType::QString, QMC::DefaultPropertyFlags, 8),
        // property 'turnRunning'
        QtMocHelpers::PropertyData<bool>(109, QMetaType::Bool, QMC::DefaultPropertyFlags, 10),
        // property 'workingThreadIds'
        QtMocHelpers::PropertyData<QStringList>(110, QMetaType::QStringList, QMC::DefaultPropertyFlags, 11),
        // property 'completedThreadIds'
        QtMocHelpers::PropertyData<QStringList>(111, QMetaType::QStringList, QMC::DefaultPropertyFlags, 12),
        // property 'hasPendingUserInput'
        QtMocHelpers::PropertyData<bool>(112, QMetaType::Bool, QMC::DefaultPropertyFlags, 13),
        // property 'pendingUserInputQuestion'
        QtMocHelpers::PropertyData<QVariantMap>(113, 0x80000000 | 105, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 13),
        // property 'pendingUserInputQuestionNumber'
        QtMocHelpers::PropertyData<int>(114, QMetaType::Int, QMC::DefaultPropertyFlags, 13),
        // property 'pendingUserInputQuestionCount'
        QtMocHelpers::PropertyData<int>(115, QMetaType::Int, QMC::DefaultPropertyFlags, 13),
        // property 'turnElapsedText'
        QtMocHelpers::PropertyData<QString>(116, QMetaType::QString, QMC::DefaultPropertyFlags, 14),
        // property 'providerReady'
        QtMocHelpers::PropertyData<bool>(117, QMetaType::Bool, QMC::DefaultPropertyFlags, 15),
        // property 'providerSetupRequired'
        QtMocHelpers::PropertyData<bool>(118, QMetaType::Bool, QMC::DefaultPropertyFlags, 16),
        // property 'providerSetupInstructions'
        QtMocHelpers::PropertyData<QString>(119, QMetaType::QString, QMC::DefaultPropertyFlags, 16),
        // property 'providerIssueText'
        QtMocHelpers::PropertyData<QString>(120, QMetaType::QString, QMC::DefaultPropertyFlags, 17),
        // property 'providerVersion'
        QtMocHelpers::PropertyData<QString>(121, QMetaType::QString, QMC::DefaultPropertyFlags, 15),
        // property 'mcpServers'
        QtMocHelpers::PropertyData<QVariantList>(122, 0x80000000 | 5, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 18),
        // property 'mcpBusy'
        QtMocHelpers::PropertyData<bool>(123, QMetaType::Bool, QMC::DefaultPropertyFlags, 18),
        // property 'mcpIssueText'
        QtMocHelpers::PropertyData<QString>(124, QMetaType::QString, QMC::DefaultPropertyFlags, 18),
        // property 'mcpLoginUrl'
        QtMocHelpers::PropertyData<QString>(125, QMetaType::QString, QMC::DefaultPropertyFlags, 18),
        // property 'statusText'
        QtMocHelpers::PropertyData<QString>(126, QMetaType::QString, QMC::DefaultPropertyFlags, 19),
        // property 'contextTokens'
        QtMocHelpers::PropertyData<qint64>(127, QMetaType::LongLong, QMC::DefaultPropertyFlags, 21),
        // property 'totalProcessedTokens'
        QtMocHelpers::PropertyData<qint64>(128, QMetaType::LongLong, QMC::DefaultPropertyFlags, 21),
        // property 'modelContextWindow'
        QtMocHelpers::PropertyData<qint64>(129, QMetaType::LongLong, QMC::DefaultPropertyFlags, 21),
        // property 'contextUsagePercent'
        QtMocHelpers::PropertyData<int>(130, QMetaType::Int, QMC::DefaultPropertyFlags, 21),
        // property 'hasTokenUsage'
        QtMocHelpers::PropertyData<bool>(131, QMetaType::Bool, QMC::DefaultPropertyFlags, 21),
        // property 'diffText'
        QtMocHelpers::PropertyData<QString>(132, QMetaType::QString, QMC::DefaultPropertyFlags, 22),
        // property 'gitStatusText'
        QtMocHelpers::PropertyData<QString>(133, QMetaType::QString, QMC::DefaultPropertyFlags, 22),
        // property 'gitRepositoryUrl'
        QtMocHelpers::PropertyData<QString>(134, QMetaType::QString, QMC::DefaultPropertyFlags, 23),
        // property 'hasGitChanges'
        QtMocHelpers::PropertyData<bool>(135, QMetaType::Bool, QMC::DefaultPropertyFlags, 22),
        // property 'databasePath'
        QtMocHelpers::PropertyData<QString>(136, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AppController, qt_meta_tag_ZN7Artemis13AppControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Artemis::AppController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Artemis13AppControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Artemis13AppControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Artemis13AppControllerE_t>.metaTypes,
    nullptr
} };

void Artemis::AppController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AppController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->threadsChanged(); break;
        case 1: _t->projectThreadsLoaded((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[2]))); break;
        case 2: _t->projectThreadRemoved((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 3: _t->modelsChanged(); break;
        case 4: _t->settingsChanged(); break;
        case 5: _t->selectedProjectChanged(); break;
        case 6: _t->selectedThreadChanged(); break;
        case 7: _t->currentTasksChanged(); break;
        case 8: _t->currentPlanChanged(); break;
        case 9: _t->taskPanelRequested(); break;
        case 10: _t->turnRunningChanged(); break;
        case 11: _t->workingThreadsChanged(); break;
        case 12: _t->completedThreadsChanged(); break;
        case 13: _t->pendingUserInputChanged(); break;
        case 14: _t->turnElapsedChanged(); break;
        case 15: _t->providerReadyChanged(); break;
        case 16: _t->providerSetupChanged(); break;
        case 17: _t->providerIssueChanged(); break;
        case 18: _t->mcpServersChanged(); break;
        case 19: _t->statusTextChanged(); break;
        case 20: _t->statusMessage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 21: _t->tokenUsageChanged(); break;
        case 22: _t->diffChanged(); break;
        case 23: _t->gitRepositoryUrlChanged(); break;
        case 24: _t->commitDraftFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 25: _t->commitFinished((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 26: _t->commitLockBlocked((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 27: _t->promptRestoreRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[2]))); break;
        case 28: _t->chooseProjectFolder(); break;
        case 29: _t->addProject((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 30: _t->removeSelectedProject(); break;
        case 31: _t->removeProject((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 32: _t->removeThread((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 33: _t->removeProjectThread((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 34: _t->selectProject((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 35: _t->selectProjectThread((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 36: _t->selectThread((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 37: _t->createThread((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 38: { bool _r = _t->sendPrompt((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[5])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[6])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 39: _t->copyText((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 40: { QString _r = _t->pasteClipboardImage();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 41: _t->interruptTurn(); break;
        case 42: { bool _r = _t->answerPendingUserInput((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 43: _t->refreshGit(); break;
        case 44: _t->generateCommitMessage(); break;
        case 45: _t->commitAllAndPush((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 46: _t->commitFeatureBranch((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4]))); break;
        case 47: _t->retryLockedCommit(); break;
        case 48: _t->removeCommitLockAndRetry(); break;
        case 49: _t->cancelLockedCommit(); break;
        case 50: { QString _r = _t->suggestBranch((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 51: _t->openProjectFolder(); break;
        case 52: _t->openProjectEditor(); break;
        case 53: _t->openTerminal(); break;
        case 54: _t->refreshMcpServers(); break;
        case 55: _t->reloadMcpServers(); break;
        case 56: _t->loginMcpServer((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 57: _t->addMcpServer((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 58: _t->removeMcpServer((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::threadsChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)(const QString & , const QVariantList & )>(_a, &AppController::projectThreadsLoaded, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)(const QString & , const QString & )>(_a, &AppController::projectThreadRemoved, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::modelsChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::settingsChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::selectedProjectChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::selectedThreadChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::currentTasksChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::currentPlanChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::taskPanelRequested, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::turnRunningChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::workingThreadsChanged, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::completedThreadsChanged, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::pendingUserInputChanged, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::turnElapsedChanged, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::providerReadyChanged, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::providerSetupChanged, 16))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::providerIssueChanged, 17))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::mcpServersChanged, 18))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::statusTextChanged, 19))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)(const QString & )>(_a, &AppController::statusMessage, 20))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::tokenUsageChanged, 21))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::diffChanged, 22))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)()>(_a, &AppController::gitRepositoryUrlChanged, 23))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)(bool , const QString & )>(_a, &AppController::commitDraftFinished, 24))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)(bool , const QString & )>(_a, &AppController::commitFinished, 25))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)(const QString & )>(_a, &AppController::commitLockBlocked, 26))
            return;
        if (QtMocHelpers::indexOfMethod<void (AppController::*)(const QString & , const QVariantList & )>(_a, &AppController::promptRestoreRequested, 27))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ConversationModel* >(); break;
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ProjectTreeModel* >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<ProjectTreeModel**>(_v) = _t->projects(); break;
        case 1: *reinterpret_cast<ConversationModel**>(_v) = _t->conversation(); break;
        case 2: *reinterpret_cast<QVariantList*>(_v) = _t->threads(); break;
        case 3: *reinterpret_cast<QVariantList*>(_v) = _t->models(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->codingModelId(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->codingReasoningEffort(); break;
        case 6: *reinterpret_cast<QString*>(_v) = _t->commitModelId(); break;
        case 7: *reinterpret_cast<QString*>(_v) = _t->titleModelId(); break;
        case 8: *reinterpret_cast<QVariantList*>(_v) = _t->editorOptions(); break;
        case 9: *reinterpret_cast<QString*>(_v) = _t->selectedEditorId(); break;
        case 10: *reinterpret_cast<QVariantList*>(_v) = _t->terminalOptions(); break;
        case 11: *reinterpret_cast<QString*>(_v) = _t->selectedTerminalId(); break;
        case 12: *reinterpret_cast<int*>(_v) = _t->selectedProjectIndex(); break;
        case 13: *reinterpret_cast<QString*>(_v) = _t->selectedProjectPath(); break;
        case 14: *reinterpret_cast<QString*>(_v) = _t->selectedProjectName(); break;
        case 15: *reinterpret_cast<bool*>(_v) = _t->selectedProjectIsGit(); break;
        case 16: *reinterpret_cast<QString*>(_v) = _t->selectedThreadId(); break;
        case 17: *reinterpret_cast<QString*>(_v) = _t->selectedThreadTitle(); break;
        case 18: *reinterpret_cast<QVariantMap*>(_v) = _t->selectedThreadInfo(); break;
        case 19: *reinterpret_cast<QString*>(_v) = _t->currentTasks(); break;
        case 20: *reinterpret_cast<QVariantList*>(_v) = _t->currentPlan(); break;
        case 21: *reinterpret_cast<QString*>(_v) = _t->currentPlanExplanation(); break;
        case 22: *reinterpret_cast<bool*>(_v) = _t->turnRunning(); break;
        case 23: *reinterpret_cast<QStringList*>(_v) = _t->workingThreadIds(); break;
        case 24: *reinterpret_cast<QStringList*>(_v) = _t->completedThreadIds(); break;
        case 25: *reinterpret_cast<bool*>(_v) = _t->hasPendingUserInput(); break;
        case 26: *reinterpret_cast<QVariantMap*>(_v) = _t->pendingUserInputQuestion(); break;
        case 27: *reinterpret_cast<int*>(_v) = _t->pendingUserInputQuestionNumber(); break;
        case 28: *reinterpret_cast<int*>(_v) = _t->pendingUserInputQuestionCount(); break;
        case 29: *reinterpret_cast<QString*>(_v) = _t->turnElapsedText(); break;
        case 30: *reinterpret_cast<bool*>(_v) = _t->providerReady(); break;
        case 31: *reinterpret_cast<bool*>(_v) = _t->providerSetupRequired(); break;
        case 32: *reinterpret_cast<QString*>(_v) = _t->providerSetupInstructions(); break;
        case 33: *reinterpret_cast<QString*>(_v) = _t->providerIssueText(); break;
        case 34: *reinterpret_cast<QString*>(_v) = _t->providerVersion(); break;
        case 35: *reinterpret_cast<QVariantList*>(_v) = _t->mcpServers(); break;
        case 36: *reinterpret_cast<bool*>(_v) = _t->mcpBusy(); break;
        case 37: *reinterpret_cast<QString*>(_v) = _t->mcpIssueText(); break;
        case 38: *reinterpret_cast<QString*>(_v) = _t->mcpLoginUrl(); break;
        case 39: *reinterpret_cast<QString*>(_v) = _t->statusText(); break;
        case 40: *reinterpret_cast<qint64*>(_v) = _t->contextTokens(); break;
        case 41: *reinterpret_cast<qint64*>(_v) = _t->totalProcessedTokens(); break;
        case 42: *reinterpret_cast<qint64*>(_v) = _t->modelContextWindow(); break;
        case 43: *reinterpret_cast<int*>(_v) = _t->contextUsagePercent(); break;
        case 44: *reinterpret_cast<bool*>(_v) = _t->hasTokenUsage(); break;
        case 45: *reinterpret_cast<QString*>(_v) = _t->diffText(); break;
        case 46: *reinterpret_cast<QString*>(_v) = _t->gitStatusText(); break;
        case 47: *reinterpret_cast<QString*>(_v) = _t->gitRepositoryUrl(); break;
        case 48: *reinterpret_cast<bool*>(_v) = _t->hasGitChanges(); break;
        case 49: *reinterpret_cast<QString*>(_v) = _t->databasePath(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 4: _t->setCodingModelId(*reinterpret_cast<QString*>(_v)); break;
        case 5: _t->setCodingReasoningEffort(*reinterpret_cast<QString*>(_v)); break;
        case 6: _t->setCommitModelId(*reinterpret_cast<QString*>(_v)); break;
        case 7: _t->setTitleModelId(*reinterpret_cast<QString*>(_v)); break;
        case 9: _t->setSelectedEditorId(*reinterpret_cast<QString*>(_v)); break;
        case 11: _t->setSelectedTerminalId(*reinterpret_cast<QString*>(_v)); break;
        case 12: _t->selectProject(*reinterpret_cast<int*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *Artemis::AppController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Artemis::AppController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Artemis13AppControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Artemis::AppController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 59)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 59;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 59)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 59;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 50;
    }
    return _id;
}

// SIGNAL 0
void Artemis::AppController::threadsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Artemis::AppController::projectThreadsLoaded(const QString & _t1, const QVariantList & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void Artemis::AppController::projectThreadRemoved(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void Artemis::AppController::modelsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void Artemis::AppController::settingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void Artemis::AppController::selectedProjectChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void Artemis::AppController::selectedThreadChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void Artemis::AppController::currentTasksChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void Artemis::AppController::currentPlanChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void Artemis::AppController::taskPanelRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void Artemis::AppController::turnRunningChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void Artemis::AppController::workingThreadsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void Artemis::AppController::completedThreadsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void Artemis::AppController::pendingUserInputChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void Artemis::AppController::turnElapsedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}

// SIGNAL 15
void Artemis::AppController::providerReadyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 15, nullptr);
}

// SIGNAL 16
void Artemis::AppController::providerSetupChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 16, nullptr);
}

// SIGNAL 17
void Artemis::AppController::providerIssueChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 17, nullptr);
}

// SIGNAL 18
void Artemis::AppController::mcpServersChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 18, nullptr);
}

// SIGNAL 19
void Artemis::AppController::statusTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 19, nullptr);
}

// SIGNAL 20
void Artemis::AppController::statusMessage(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 20, nullptr, _t1);
}

// SIGNAL 21
void Artemis::AppController::tokenUsageChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 21, nullptr);
}

// SIGNAL 22
void Artemis::AppController::diffChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 22, nullptr);
}

// SIGNAL 23
void Artemis::AppController::gitRepositoryUrlChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 23, nullptr);
}

// SIGNAL 24
void Artemis::AppController::commitDraftFinished(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 24, nullptr, _t1, _t2);
}

// SIGNAL 25
void Artemis::AppController::commitFinished(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 25, nullptr, _t1, _t2);
}

// SIGNAL 26
void Artemis::AppController::commitLockBlocked(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 26, nullptr, _t1);
}

// SIGNAL 27
void Artemis::AppController::promptRestoreRequested(const QString & _t1, const QVariantList & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 27, nullptr, _t1, _t2);
}
QT_WARNING_POP
