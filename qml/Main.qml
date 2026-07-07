pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import org.kde.kirigami as Kirigami
import org.artemis

Kirigami.ApplicationWindow {
    id: root
    width: 1500
    height: 900
    readonly property real conversationMinimumWidth: 760
    readonly property real navigationMinimumWidth: 220
    readonly property real threadPanelMinimumWidth: 300
    property real threadPanelWidth: 350
    readonly property real splitHandleWidth: 5
    minimumWidth: conversationMinimumWidth
                  + (sidePanelVisible
                     ? threadPanelWidth + splitHandleWidth : 0)
    minimumHeight: 560
    visible: true
    title: appController.selectedThreadTitle.length > 0
           ? appController.selectedThreadTitle + " — Artemis" : "Artemis"

    property bool sidePanelVisible: false
    property string sidePanelMode: "thread"
    property bool navigationVisible: width >= conversationMinimumWidth
                                     + navigationMinimumWidth
                                     + splitHandleWidth
                                     + (sidePanelVisible
                                        ? threadPanelWidth
                                          + splitHandleWidth : 0)
    readonly property bool hasProject: appController.selectedProjectPath.length > 0
    readonly property bool hasThread: appController.selectedThreadId.length > 0
    readonly property bool pendingQuestionVisible:
        appController.hasPendingUserInput
        && appController.pendingUserInputQuestion.threadId
           === appController.selectedThreadId
    property string selectedQuestionOption: ""
    property var composerImages: []
    property var projectThreadCache: ({})
    property var expandedProjects: ({})

    function cachedProjectThreads(path) {
        return projectThreadCache[path] || []
    }

    function cacheProjectThreads(path, threads) {
        if (path.length === 0)
            return
        const updated = Object.assign({}, projectThreadCache)
        updated[path] = Array.from(threads || [])
        projectThreadCache = updated
    }

    function removeCachedProjectThread(path, threadId) {
        const threads = cachedProjectThreads(path)
        cacheProjectThreads(path, threads.filter(thread => thread.id !== threadId))
    }

    function projectExpanded(path) {
        return expandedProjects[path] === true
    }

    function setProjectExpanded(path, expanded) {
        const updated = Object.assign({}, expandedProjects)
        updated[path] = expanded
        expandedProjects = updated
    }

    function modelIndex(modelId) {
        const exact = modelPicker.indexOfValue(modelId)
        if (exact >= 0)
            return exact
        for (let i = 0; i < appController.models.length; ++i) {
            if (appController.models[i].isDefault)
                return i
        }
        return appController.models.length > 0 ? 0 : -1
    }

    function selectedModel() {
        if (modelPicker.currentIndex < 0
                || modelPicker.currentIndex >= appController.models.length)
            return null
        return appController.models[modelPicker.currentIndex]
    }

    function reasoningOptions() {
        const selected = selectedModel()
        if (!selected)
            return []
        const defaultLabel = selected.defaultEffort
                ? "Default (" + selected.defaultEffort + ")" : "Model default"
        const options = [{ value: "", label: defaultLabel }]
        const efforts = Array.from(selected.efforts || [])
        for (let i = 0; i < efforts.length; ++i)
            options.push({ value: efforts[i],
                           label: efforts[i].charAt(0).toUpperCase() + efforts[i].slice(1) })
        return options
    }

    function reasoningIndex(reasoningEffort) {
        const options = reasoningOptions()
        for (let i = 0; i < options.length; ++i) {
            if (options[i].value === reasoningEffort)
                return i
        }
        return 0
    }

    function localImageUrl(path) {
        return path.length > 0 ? "file://" + encodeURI(path) : ""
    }

    function compactTokenCount(value) {
        if (value >= 1000000) {
            const millions = value / 1000000
            return (millions >= 10 ? millions.toFixed(0) : millions.toFixed(1))
                    .replace(/\.0$/, "") + "m"
        }
        if (value >= 1000)
            return Math.round(value / 1000) + "k"
        return value.toString()
    }

    function submitPendingQuestion() {
        const typedAnswer = composer.text.trim()
        const answer = typedAnswer.length > 0 ? typedAnswer : selectedQuestionOption
        if (!appController.answerPendingUserInput(answer))
            return false
        selectedQuestionOption = ""
        composer.clear()
        composer.forceActiveFocus()
        return true
    }

    CommitDialog {
        id: commitDialog
        controller: appController
    }

    SettingsDialog {
        id: settingsDialog
        controller: appController
        transientParent: root
    }

    Connections {
        target: appController
        function onPromptRestoreRequested(text, images) {
            if (composer.text.length === 0)
                composer.text = text
            const restored = Array.from(images || [])
            for (let i = 0; i < restored.length; ++i) {
                if (root.composerImages.indexOf(restored[i]) < 0)
                    root.composerImages = root.composerImages.concat([restored[i]])
            }
        }
        function onStatusMessage(text) {
            statusToast.showMessage(text)
        }
        function onThreadsChanged() {
            if (appController.threads.length > 0)
                root.cacheProjectThreads(appController.selectedProjectPath,
                                         appController.threads)
        }
        function onProjectThreadsLoaded(projectPath, threads) {
            root.cacheProjectThreads(projectPath, threads)
        }
        function onProjectThreadRemoved(projectPath, threadId) {
            root.removeCachedProjectThread(projectPath, threadId)
        }
        function onTaskPanelRequested() {
            root.sidePanelMode = "thread"
            root.sidePanelVisible = true
        }
        function onPendingUserInputChanged() {
            root.selectedQuestionOption = ""
        }
    }

    Popup {
        id: statusToast
        parent: Overlay.overlay
        property string message: ""

        function showMessage(text) {
            if (text.length === 0)
                return
            message = text
            open()
            dismissTimer.restart()
        }

        x: Overlay.overlay.width - width - Kirigami.Units.largeSpacing
        y: appHeader.height + Kirigami.Units.largeSpacing
        width: Math.min(360, Overlay.overlay.width
                             - Kirigami.Units.largeSpacing * 2)
        padding: Kirigami.Units.smallSpacing
        modal: false
        focus: false
        closePolicy: Popup.NoAutoClose

        background: Rectangle {
            radius: Kirigami.Units.smallSpacing
            color: Kirigami.Theme.backgroundColor
            border.color: Qt.alpha(Kirigami.Theme.textColor, 0.28)
        }

        contentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Label {
                text: statusToast.message
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                maximumLineCount: 4
                elide: Text.ElideRight
            }
            ToolButton {
                icon.name: "edit-copy"
                Accessible.name: "Copy notification"
                ToolTip.text: Accessible.name
                ToolTip.visible: hovered
                onClicked: appController.copyText(statusToast.message)
            }
        }

        Timer {
            id: dismissTimer
            interval: 5000
            onTriggered: statusToast.close()
        }

        Component.onCompleted: {
            if (appController.statusText.length > 0)
                showMessage(appController.statusText)
        }
    }

    header: ToolBar {
        id: appHeader
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Kirigami.Units.smallSpacing
            anchors.rightMargin: Kirigami.Units.smallSpacing

            ToolButton {
                visible: !root.navigationVisible
                text: "☰"
                Accessible.name: "Projects"
                onClicked: navigationDrawer.visible ? navigationDrawer.close()
                                                    : navigationDrawer.open()
            }
            Label {
                text: appController.selectedThreadTitle.length > 0
                      ? appController.selectedThreadTitle : "Artemis"
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
            ToolButton {
                text: "Open folder"
                icon.name: "folder-open"
                display: AbstractButton.IconOnly
                ToolTip.text: text
                ToolTip.visible: hovered
                visible: root.hasProject
                onClicked: appController.openProjectFolder()
            }
            ToolButton {
                text: "Open editor"
                icon.name: "document-edit"
                display: AbstractButton.IconOnly
                ToolTip.text: text
                ToolTip.visible: hovered
                visible: root.hasProject
                onClicked: appController.openProjectEditor()
            }
            ToolButton {
                text: "Open terminal"
                icon.name: "utilities-terminal-symbolic"
                display: AbstractButton.IconOnly
                ToolTip.text: text
                ToolTip.visible: hovered
                visible: root.hasProject
                onClicked: appController.openTerminal()
            }
            ToolButton {
                text: "Thread"
                icon.name: "help-about"
                display: AbstractButton.IconOnly
                ToolTip.text: text
                ToolTip.visible: hovered
                visible: root.hasThread
                checkable: true
                checked: root.sidePanelVisible && root.sidePanelMode === "thread"
                onClicked: {
                    if (checked) {
                        root.sidePanelMode = "thread"
                        root.sidePanelVisible = true
                    } else {
                        root.sidePanelVisible = false
                    }
                }
            }
            ToolButton {
                text: "Diff"
                icon.name: "vcs-diff"
                display: AbstractButton.IconOnly
                ToolTip.text: text
                ToolTip.visible: hovered
                visible: appController.selectedProjectIsGit
                enabled: appController.hasGitChanges
                checkable: true
                checked: root.sidePanelVisible && root.sidePanelMode === "diff"
                onClicked: {
                    if (checked) {
                        root.sidePanelMode = "diff"
                        root.sidePanelVisible = true
                        appController.refreshGit()
                    } else {
                        root.sidePanelVisible = false
                    }
                }
            }
            Button {
                text: "Commit && Push"
                visible: appController.selectedProjectIsGit
                enabled: appController.hasGitChanges
                onClicked: {
                    commitDialog.featureMode = false
                    commitDialog.open()
                }
            }
            ToolButton {
                id: commitMenuButton
                text: "More"
                visible: appController.selectedProjectIsGit
                enabled: appController.hasGitChanges
                onClicked: commitMenu.open()
                Accessible.name: "More commit options"
                Menu {
                    id: commitMenu
                    y: commitMenuButton.height
                    MenuItem {
                        text: "Commit and push all changes"
                        onTriggered: {
                            commitDialog.featureMode = false
                            commitDialog.open()
                        }
                    }
                    MenuItem {
                        text: "Review changes before committing"
                        onTriggered: {
                            root.sidePanelMode = "diff"
                            root.sidePanelVisible = true
                            commitDialog.featureMode = false
                            commitDialog.open()
                        }
                    }
                    MenuItem {
                        text: "Commit to feature branch"
                        onTriggered: {
                            commitDialog.featureMode = true
                            commitDialog.open()
                        }
                    }
                }
            }
        }
    }

    Component {
        id: navigationPaneComponent
        Pane {
            id: navigationPane
            readonly property real projectRowHeight: 36
            readonly property real threadRowHeight: 34
            readonly property real threadMetadataRowHeight: 44
            readonly property real horizontalInset: Kirigami.Units.smallSpacing
            readonly property real threadIndent: Kirigami.Units.gridUnit * 2
            readonly property real rowRadius: 5
            padding: Kirigami.Units.largeSpacing
            clip: true

            ColumnLayout {
                anchors.fill: parent
                spacing: Kirigami.Units.smallSpacing

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: navigationPane.projectRowHeight
                    spacing: Kirigami.Units.smallSpacing
                    Label {
                        text: "Projects"
                        font.bold: true
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                        Layout.fillWidth: true
                    }
                    ToolButton {
                        flat: true
                        icon.name: "folder-new"
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        onClicked: appController.chooseProjectFolder()
                        Accessible.name: "Add project folder"
                        ToolTip.text: Accessible.name
                        ToolTip.visible: hovered
                    }
                    ToolButton {
                        flat: true
                        visible: !root.navigationVisible
                        icon.name: "window-close"
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        onClicked: navigationDrawer.close()
                        Accessible.name: "Close projects"
                        ToolTip.text: Accessible.name
                        ToolTip.visible: hovered
                    }
                }
                TextField {
                    id: projectSearch
                    Layout.fillWidth: true
                    placeholderText: "Search projects"
                    visible: projectList.count > 4
                }
                ListView {
                    id: projectList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: appController.projects
                    spacing: Kirigami.Units.smallSpacing
                    delegate: ColumnLayout {
                        id: projectDelegate
                        required property int index
                        required property string name
                        required property string path
                        required property bool isGit
                        readonly property bool selected: index === appController.selectedProjectIndex
                        readonly property bool expanded: root.projectExpanded(path)
                        property bool showAllThreads: false
                        readonly property var projectThreads: root.cachedProjectThreads(path)
                        width: projectList.width
                        spacing: Kirigami.Units.smallSpacing
                        visible: projectSearch.text.length === 0
                                 || name.toLowerCase().includes(projectSearch.text.toLowerCase())

                        Component.onCompleted: {
                            if (selected)
                                root.setProjectExpanded(path, true)
                        }

                        ItemDelegate {
                            id: projectItem
                            Layout.fillWidth: true
                            Layout.maximumWidth: projectList.width
                            Layout.preferredHeight: navigationPane.projectRowHeight
                            leftPadding: navigationPane.horizontalInset
                            rightPadding: navigationPane.horizontalInset
                            topPadding: 0
                            bottomPadding: 0
                            background: Rectangle {
                                radius: navigationPane.rowRadius
                                color: projectDelegate.selected
                                       ? Qt.alpha(Kirigami.Theme.highlightColor, 0.22)
                                       : projectItem.hovered
                                         ? Qt.alpha(Kirigami.Theme.textColor, 0.07)
                                         : "transparent"
                            }
                            contentItem: RowLayout {
                                spacing: Kirigami.Units.smallSpacing
                                ToolButton {
                                    flat: true
                                    padding: 0
                                    icon.name: projectDelegate.expanded
                                               ? "arrow-down" : "arrow-right"
                                    Layout.preferredWidth: 24
                                    Layout.preferredHeight: 24
                                    Accessible.name: (projectDelegate.expanded
                                                      ? "Collapse " : "Expand ")
                                                     + projectDelegate.name
                                    onClicked: {
                                        const expanding = !projectDelegate.expanded
                                        root.setProjectExpanded(projectDelegate.path, expanding)
                                        if (expanding
                                                && !projectDelegate.selected)
                                            appController.selectProject(projectDelegate.index)
                                    }
                                }
                                Kirigami.Icon {
                                    source: "folder"
                                    Layout.preferredWidth: 18
                                    Layout.preferredHeight: 18
                                }
                                Label {
                                    text: projectDelegate.name
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                    font.bold: projectDelegate.selected
                                }
                                ToolButton {
                                    flat: true
                                    padding: 0
                                    text: "+"
                                    visible: projectDelegate.expanded
                                    enabled: appController.providerReady
                                    Layout.preferredWidth: 24
                                    Layout.preferredHeight: 24
                                    Accessible.name: "New thread in " + projectDelegate.name
                                    ToolTip.text: Accessible.name
                                    ToolTip.visible: hovered
                                    onClicked: {
                                        if (!projectDelegate.selected)
                                            appController.selectProject(projectDelegate.index)
                                        appController.createThread(
                                            appController.codingModelId,
                                            appController.codingReasoningEffort,
                                            permissionPicker.currentValue)
                                    }
                                }
                            }
                            ToolTip.text: projectDelegate.path
                            ToolTip.visible: hovered
                            onClicked: {
                                root.setProjectExpanded(projectDelegate.path, true)
                                appController.selectProject(projectDelegate.index)
                            }

                            TapHandler {
                                acceptedButtons: Qt.RightButton
                                onTapped: projectContextMenu.popup()
                            }
                            Menu {
                                id: projectContextMenu
                                MenuItem {
                                    text: "Remove project from Artemis"
                                    icon.name: "edit-delete"
                                    onTriggered: appController.removeProject(projectDelegate.index)
                                }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            visible: projectDelegate.expanded
                            spacing: 0

                            Repeater {
                                model: projectDelegate.showAllThreads
                                       ? projectDelegate.projectThreads
                                       : projectDelegate.projectThreads.slice(0, 5)
                                delegate: ItemDelegate {
                                    id: threadItem
                                    required property int index
                                    required property var modelData
                                    Layout.fillWidth: true
                                    Layout.maximumWidth: projectList.width
                                    Layout.preferredHeight: modelData.external
                                                            ? navigationPane.threadMetadataRowHeight
                                                            : navigationPane.threadRowHeight
                                    leftPadding: navigationPane.threadIndent
                                    rightPadding: navigationPane.horizontalInset
                                    topPadding: Kirigami.Units.smallSpacing
                                    bottomPadding: Kirigami.Units.smallSpacing
                                    background: Rectangle {
                                        radius: navigationPane.rowRadius
                                        color: appController.selectedThreadId
                                               === threadItem.modelData.id
                                               ? Qt.alpha(Kirigami.Theme.highlightColor, 0.16)
                                               : threadItem.hovered
                                                 ? Qt.alpha(Kirigami.Theme.textColor, 0.06)
                                                 : "transparent"
                                    }
                                    contentItem: RowLayout {
                                        spacing: Kirigami.Units.smallSpacing
                                        Item {
                                            id: threadStatus
                                            readonly property bool working:
                                                appController.workingThreadIds.indexOf(
                                                    threadItem.modelData.id) >= 0
                                            readonly property bool completed:
                                                appController.completedThreadIds.indexOf(
                                                    threadItem.modelData.id) >= 0
                                            readonly property bool pending:
                                                threadItem.modelData.pending === true
                                            Layout.preferredWidth: 16
                                            Layout.preferredHeight: 16
                                            Accessible.name: pending ? "Starting"
                                                                      : working ? "Working"
                                                                      : completed ? "Completed"
                                                                                  : "Thread"

                                            Kirigami.Icon {
                                                anchors.fill: parent
                                                visible: !threadStatus.working
                                                         && !threadStatus.pending
                                                source: threadStatus.completed
                                                        ? "dialog-ok-apply"
                                                        : "dialog-messages"
                                                opacity: 0.7
                                            }

                                            Kirigami.Icon {
                                                id: workingThreadIcon
                                                anchors.fill: parent
                                                visible: threadStatus.working
                                                         || threadStatus.pending
                                                source: "view-refresh"
                                                opacity: 0.7
                                                NumberAnimation on rotation {
                                                    running: workingThreadIcon.visible
                                                    from: 0
                                                    to: 360
                                                    duration: 1200
                                                    loops: Animation.Infinite
                                                }
                                            }
                                        }
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 0
                                            Label {
                                                text: threadItem.modelData.title
                                                Layout.fillWidth: true
                                                elide: Text.ElideRight
                                            }
                                            Label {
                                                visible: threadItem.modelData.external
                                                         || threadItem.modelData.pending
                                                text: threadItem.modelData.pending
                                                      ? "Starting" : "External"
                                                font: Kirigami.Theme.smallFont
                                                opacity: 0.55
                                            }
                                        }
                                    }
                                    onClicked: {
                                        appController.selectProjectThread(
                                            projectDelegate.index,
                                            threadItem.modelData.id)
                                        if (!root.navigationVisible)
                                            navigationDrawer.close()
                                    }

                                    TapHandler {
                                        acceptedButtons: Qt.RightButton
                                        onTapped: threadContextMenu.popup()
                                    }
                                    Menu {
                                        id: threadContextMenu
                                        MenuItem {
                                            text: "Remove thread from Artemis"
                                            icon.name: "edit-delete"
                                            enabled: !threadItem.modelData.pending
                                            onTriggered: appController.removeProjectThread(
                                                projectDelegate.index,
                                                threadItem.modelData.id)
                                        }
                                    }
                                }
                            }

                            ToolButton {
                                Layout.leftMargin: navigationPane.threadIndent
                                                   + 16
                                                   + Kirigami.Units.smallSpacing
                                Layout.preferredHeight: 28
                                visible: projectDelegate.projectThreads.length > 5
                                flat: true
                                padding: 0
                                text: projectDelegate.showAllThreads
                                      ? "Show less"
                                      : "Show "
                                        + (projectDelegate.projectThreads.length - 5)
                                        + " more"
                                font: Kirigami.Theme.smallFont
                                opacity: hovered ? 0.9 : 0.55
                                onClicked: projectDelegate.showAllThreads =
                                           !projectDelegate.showAllThreads
                            }

                            Label {
                                Layout.fillWidth: true
                                Layout.leftMargin: navigationPane.threadIndent
                                                   + 16
                                                   + Kirigami.Units.smallSpacing
                                Layout.rightMargin: navigationPane.horizontalInset
                                Layout.topMargin: Kirigami.Units.smallSpacing
                                Layout.bottomMargin: Kirigami.Units.smallSpacing
                                visible: projectDelegate.projectThreads.length === 0
                                         && projectDelegate.selected
                                text: appController.providerReady
                                      ? "No chats yet. Send a message to start one."
                                      : appController.providerSetupRequired
                                        ? "Codex setup is required."
                                        : "Codex is unavailable."
                                wrapMode: Text.Wrap
                                opacity: 0.55
                                font: Kirigami.Theme.smallFont
                            }
                        }
                    }
                    ColumnLayout {
                        anchors.centerIn: parent
                        width: parent.width
                        visible: projectList.count === 0
                        spacing: Kirigami.Units.smallSpacing
                        Label {
                            text: "No projects added"
                            Layout.alignment: Qt.AlignHCenter
                            font.bold: true
                        }
                        Label {
                            text: "Add a folder to start working with Codex."
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.Wrap
                            opacity: 0.65
                        }
                    }
                }
                Kirigami.Separator { Layout.fillWidth: true }
                RowLayout {
                    Layout.fillWidth: true
                    Rectangle {
                        id: connectionBadge
                        visible: !appController.providerReady
                        Layout.preferredWidth: connectionRow.implicitWidth
                                               + Kirigami.Units.largeSpacing
                        Layout.preferredHeight: Math.max(
                            28, connectionLabel.implicitHeight
                                + Kirigami.Units.smallSpacing * 2)
                        radius: height / 2
                        color: Qt.alpha(Kirigami.Theme.negativeTextColor, 0.12)
                        RowLayout {
                            id: connectionRow
                            anchors.fill: parent
                            anchors.leftMargin: Kirigami.Units.largeSpacing / 2
                            anchors.rightMargin: Kirigami.Units.largeSpacing / 2
                            anchors.bottomMargin: 2
                            spacing: Kirigami.Units.smallSpacing
                            Rectangle {
                                Layout.preferredWidth: 7
                                Layout.preferredHeight: 7
                                radius: 4
                                color: Kirigami.Theme.negativeTextColor
                            }
                            Label {
                                id: connectionLabel
                                Layout.alignment: Qt.AlignVCenter
                                text: appController.providerSetupRequired
                                      ? "Codex setup required"
                                      : "Codex unavailable"
                                color: Kirigami.Theme.negativeTextColor
                            }
                        }
                        ToolTip.text: appController.providerSetupRequired
                                      ? appController.providerSetupInstructions
                                      : appController.providerIssueText
                        ToolTip.visible: badgeHover.hovered
                        HoverHandler { id: badgeHover }
                    }
                    Item { Layout.fillWidth: true }
                    ToolButton {
                        icon.name: "settings-configure"
                        onClicked: settingsDialog.open()
                        Accessible.name: "Settings"
                        ToolTip.text: Accessible.name
                        ToolTip.visible: hovered
                    }
                    ToolButton {
                        icon.name: "tools-report-bug"
                        onClicked: diagnosticsDialog.open()
                        Accessible.name: "Diagnostics"
                        ToolTip.text: Accessible.name
                        ToolTip.visible: hovered
                    }
                }
            }
        }
    }

    Drawer {
        id: navigationDrawer
        width: Math.min(root.width * 0.82, 340)
        height: root.height
        edge: Qt.LeftEdge
        modal: true
        contentItem: Loader {
            active: navigationDrawer.visible
            sourceComponent: navigationPaneComponent
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        SplitView {
            id: workspaceSplit
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal

            handle: Rectangle {
                implicitWidth: root.splitHandleWidth
                color: SplitHandle.pressed
                       ? Kirigami.Theme.highlightColor
                       : SplitHandle.hovered
                         ? Qt.alpha(Kirigami.Theme.highlightColor, 0.55)
                         : Qt.alpha(Kirigami.Theme.textColor, 0.18)

                Rectangle {
                    anchors.centerIn: parent
                    width: 1
                    height: parent.height
                    color: Kirigami.Theme.textColor
                    opacity: 0.28
                }
            }

            Loader {
                id: navigationPaneHost
                visible: root.navigationVisible
                active: root.navigationVisible
                sourceComponent: navigationPaneComponent
                SplitView.preferredWidth: 310
                SplitView.minimumWidth: root.navigationVisible
                                        ? root.navigationMinimumWidth : 0
                SplitView.maximumWidth: root.navigationVisible ? 480 : 0
            }

            Pane {
            id: conversationPane
            SplitView.fillWidth: true
            SplitView.minimumWidth: root.conversationMinimumWidth
            padding: 0

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ListView {
                        id: conversationList
                        property bool followTail: true
                        property bool tailScrollPending: false
                        property bool followUpdatePending: false
                        property bool layoutRefreshPending: false
                        readonly property real tailThreshold:
                            Kirigami.Units.gridUnit * 2
                        readonly property real distanceFromEnd: Math.max(
                            0, contentHeight - height - contentY)
                        readonly property bool awayFromTail:
                            distanceFromEnd > tailThreshold
                        readonly property real contentViewportWidth: Math.max(
                            0, width - (conversationScrollBar.visible
                                        ? conversationScrollBar.width
                                          + Kirigami.Units.smallSpacing : 0))

                        function updateFollowTail() {
                            followTail = !awayFromTail
                        }

                        function updateFollowTailLater() {
                            if (followUpdatePending)
                                return
                            followUpdatePending = true
                            Qt.callLater(function() {
                                conversationList.followUpdatePending = false
                                conversationList.updateFollowTail()
                            })
                        }

                        function scrollToEndIfFollowing() {
                            if (!followTail || conversationScrollBar.pressed
                                    || tailScrollPending)
                                return
                            tailScrollPending = true
                            Qt.callLater(function() {
                                conversationList.tailScrollPending = false
                                if (conversationList.followTail
                                        && !conversationScrollBar.pressed)
                                    conversationList.positionViewAtEnd()
                            })
                        }

                        function resumeFollowing() {
                            followTail = true
                            positionViewAtEnd()
                            scrollToEndIfFollowing()
                        }

                        function refreshLayoutLater() {
                            if (layoutRefreshPending)
                                return
                            layoutRefreshPending = true
                            Qt.callLater(function() {
                                conversationList.layoutRefreshPending = false
                                conversationList.forceLayout()
                                conversationList.scrollToEndIfFollowing()
                            })
                        }

                        anchors.fill: parent
                        model: appController.conversation
                        clip: true
                        cacheBuffer: Math.max(height * 8, 12000)
                        reuseItems: true
                        boundsBehavior: Flickable.StopAtBounds
                        ScrollBar.vertical: ScrollBar {
                            id: conversationScrollBar
                            policy: ScrollBar.AsNeeded

                            onPressedChanged: {
                                if (pressed)
                                    conversationList.followTail = false
                                else
                                    conversationList.updateFollowTail()
                            }
                        }
                        spacing: Kirigami.Units.largeSpacing
                        topMargin: Kirigami.Units.largeSpacing
                        bottomMargin: Kirigami.Units.largeSpacing
                        footer: Item {
                            width: conversationList.contentViewportWidth
                            implicitHeight: workingStatus.visible
                                            ? workingStatus.implicitHeight : 0

                            Label {
                                id: workingStatus
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: Math.min(840, parent.width)
                                visible: appController.turnRunning
                                horizontalAlignment: Text.AlignHCenter
                                text: "Working · " + appController.turnElapsedText + " elapsed"
                                font: Kirigami.Theme.smallFont
                                opacity: 0.45
                                topPadding: Kirigami.Units.smallSpacing
                                bottomPadding: Kirigami.Units.smallSpacing
                            }
                        }
                        delegate: Item {
                            id: conversationRow
                            required property int index
                            required property string eventType
                            required property string title
                            required property string content
                            required property var metadata

                            readonly property real horizontalGutter: Kirigami.Units.largeSpacing
                            width: conversationList.contentViewportWidth
                            implicitHeight: conversationDelegate.implicitHeight
                            onImplicitHeightChanged: {
                                if (index === conversationList.count - 1)
                                    conversationList.scrollToEndIfFollowing()
                            }

                            ConversationDelegate {
                                id: conversationDelegate
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: Math.min(840, parent.width
                                                     - conversationRow.horizontalGutter * 2)
                                eventType: conversationRow.eventType
                                title: conversationRow.title
                                content: conversationRow.content
                                metadata: conversationRow.metadata
                                onImageOpenRequested: path => imageViewer.showImage(path)
                                onContentLayoutChanged: conversationList.refreshLayoutLater()
                            }
                        }
                        onCountChanged: scrollToEndIfFollowing()
                        onDraggingChanged: {
                            if (dragging)
                                followTail = false
                        }
                        onMovementEnded: updateFollowTail()
                        onWidthChanged: forceLayout()

                        WheelHandler {
                            target: null
                            blocking: false
                            onWheel: function(event) {
                                conversationList.followTail = false
                                conversationList.updateFollowTailLater()
                            }
                        }

                        Connections {
                            target: appController
                            function onTurnRunningChanged() {
                                if (appController.turnRunning) {
                                    conversationList.followTail = true
                                    conversationList.scrollToEndIfFollowing()
                                }
                            }
                        }

                        Kirigami.PlaceholderMessage {
                            anchors.centerIn: parent
                            width: Math.min(parent.width - 40, 480)
                            visible: conversationList.count === 0
                            text: !root.hasProject ? "Start with a project"
                                  : appController.providerSetupRequired
                                    ? "Set up Codex"
                                  : !appController.providerReady ? "Codex is unavailable"
                                  : "What would you like to build?"
                            explanation: !root.hasProject
                                         ? "Add a project folder to give Artemis a workspace."
                                         : appController.providerSetupRequired
                                           ? appController.providerSetupInstructions
                                         : !appController.providerReady
                                           ? (appController.providerIssueText
                                              || "Open Diagnostics to inspect the Codex connection.")
                                           : appController.selectedProjectIsGit
                                             ? "Describe a task below. Use Diff to review file changes."
                                             : "Describe a task below. Git review is unavailable for this folder."
                        }

                        Button {
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.verticalCenter
                            anchors.topMargin: Kirigami.Units.gridUnit * 4
                            visible: conversationList.count === 0 && !root.hasProject
                            text: "Add project folder"
                            icon.name: "folder-new"
                            onClicked: appController.chooseProjectFolder()
                        }
                    }

                    Button {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: Kirigami.Units.largeSpacing
                        z: 1
                        visible: !conversationList.followTail
                                 && conversationList.awayFromTail
                        text: "Scroll to bottom"
                        icon.name: "go-bottom"
                        onClicked: conversationList.resumeFollowing()
                        Accessible.name: text
                    }
                }

                Kirigami.Separator {
                    visible: root.hasProject && appController.providerReady
                    Layout.fillWidth: true
                }

                Frame {
                    id: composerFrame
                    visible: root.hasProject && appController.providerReady
                    Layout.fillWidth: true
                    Layout.maximumWidth: 840
                    Layout.alignment: Qt.AlignHCenter
                    Layout.margins: Kirigami.Units.largeSpacing
                    padding: root.pendingQuestionVisible
                             ? Kirigami.Units.largeSpacing
                             : Kirigami.Units.smallSpacing

                    ColumnLayout {
                        anchors.fill: parent
                        ColumnLayout {
                            id: userInputFrame
                            visible: root.pendingQuestionVisible
                            Layout.fillWidth: true
                            spacing: Kirigami.Units.smallSpacing

                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    Layout.fillWidth: true
                                    text: (appController.pendingUserInputQuestion.header
                                           || "Question").toUpperCase()
                                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                                    font.letterSpacing: 1
                                    opacity: 0.52
                                }
                                Label {
                                    visible: appController.pendingUserInputQuestionCount > 1
                                    text: appController.pendingUserInputQuestionNumber
                                          + " of "
                                          + appController.pendingUserInputQuestionCount
                                    font: Kirigami.Theme.smallFont
                                    opacity: 0.52
                                }
                            }
                            Label {
                                Layout.fillWidth: true
                                Layout.bottomMargin: Kirigami.Units.smallSpacing
                                text: appController.pendingUserInputQuestion.question || ""
                                wrapMode: Text.Wrap
                                font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                            }
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 1

                                Repeater {
                                    model: appController.pendingUserInputQuestion.options || []
                                    delegate: ItemDelegate {
                                        id: answerOption
                                        required property var modelData
                                        required property int index
                                        readonly property bool selected:
                                            root.selectedQuestionOption === modelData.label

                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 52
                                        leftPadding: Kirigami.Units.largeSpacing
                                        rightPadding: Kirigami.Units.largeSpacing
                                        checkable: true
                                        checked: selected
                                        highlighted: selected
                                        Accessible.name: modelData.label
                                        Accessible.description: modelData.description || ""
                                        onClicked: root.selectedQuestionOption = modelData.label

                                        contentItem: RowLayout {
                                            spacing: Kirigami.Units.largeSpacing

                                            Frame {
                                                Layout.preferredWidth: 24
                                                Layout.preferredHeight: 24
                                                padding: 0
                                                Label {
                                                    anchors.centerIn: parent
                                                    text: answerOption.index + 1
                                                    font: Kirigami.Theme.smallFont
                                                    opacity: 0.7
                                                }
                                            }
                                            Label {
                                                text: answerOption.modelData.label
                                                font.bold: answerOption.selected
                                            }
                                            Label {
                                                Layout.fillWidth: true
                                                text: answerOption.modelData.description || ""
                                                elide: Text.ElideRight
                                                opacity: 0.48
                                            }
                                            Kirigami.Icon {
                                                Layout.preferredWidth: 18
                                                Layout.preferredHeight: 18
                                                source: "checkmark"
                                                visible: answerOption.selected
                                                color: Kirigami.Theme.textColor
                                            }
                                        }
                                    }
                                }
                            }
                            Kirigami.Separator {
                                Layout.fillWidth: true
                                Layout.topMargin: Kirigami.Units.smallSpacing
                                Layout.bottomMargin: Kirigami.Units.smallSpacing
                            }
                        }
                        Flickable {
                            Layout.fillWidth: true
                            Layout.preferredHeight: root.composerImages.length > 0 ? 92 : 0
                            visible: root.composerImages.length > 0
                                     && !root.pendingQuestionVisible
                            contentWidth: attachmentRow.implicitWidth
                            contentHeight: height
                            clip: true

                            Row {
                                id: attachmentRow
                                height: parent.height
                                spacing: Kirigami.Units.smallSpacing

                                Repeater {
                                    model: root.composerImages
                                    delegate: Frame {
                                        required property string modelData
                                        required property int index
                                        width: 112
                                        height: 88
                                        padding: 3

                                        Image {
                                            anchors.fill: parent
                                            source: root.localImageUrl(modelData)
                                            fillMode: Image.PreserveAspectCrop
                                            asynchronous: true
                                            cache: false
                                        }
                                        RoundButton {
                                            anchors.top: parent.top
                                            anchors.right: parent.right
                                            anchors.margins: 3
                                            width: 26
                                            height: 26
                                            text: "×"
                                            Accessible.name: "Remove image"
                                            onClicked: {
                                                const next = root.composerImages.slice()
                                                next.splice(index, 1)
                                                root.composerImages = next
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        ScrollView {
                            id: composerScrollView
                            Layout.fillWidth: true
                            Layout.preferredHeight: root.pendingQuestionVisible
                                                    ? 72
                                                    : Math.min(
                                                          260,
                                                          Math.max(
                                                              90,
                                                              composer.implicitHeight))
                            Layout.maximumHeight: 260
                            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                            TextArea {
                                id: composer
                                width: composerScrollView.availableWidth
                                placeholderText: root.pendingQuestionVisible
                                                 ? "Type your own answer, or leave this blank to use the selected option"
                                                 : appController.threadCreationPending
                                                 ? "Starting thread..."
                                                 : appController.turnRunning
                                                 ? "Add guidance while Artemis is working…"
                                                 : "Describe a task, ask a question, or paste an error…"
                                wrapMode: TextEdit.Wrap
                                enabled: root.hasProject && appController.providerReady
                                Keys.onPressed: event => {
                                    if (!root.pendingQuestionVisible
                                            && event.matches(StandardKey.Paste)) {
                                        const imagePath = appController.pasteClipboardImage()
                                        if (imagePath.length > 0) {
                                            root.composerImages =
                                                    root.composerImages.concat([imagePath])
                                            event.accepted = true
                                            return
                                        }
                                    }
                                    if (!(event.modifiers & Qt.ShiftModifier)
                                            && (event.key === Qt.Key_Return
                                                || event.key === Qt.Key_Enter)) {
                                        if (root.pendingQuestionVisible)
                                            sendButton.clicked()
                                        else
                                            sendPromptButton.clicked()
                                        event.accepted = true
                                    }
                                }
                            }
                        }
                        GridLayout {
                            id: composerControls
                            readonly property bool compact: conversationPane.width < 800
                            Layout.fillWidth: true
                            Layout.minimumWidth: 0
                            columns: compact ? 2 : 7

                            ComboBox {
                                id: modelPicker
                                Layout.row: 0
                                Layout.column: 0
                                Layout.fillWidth: composerControls.compact
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: composerControls.compact ? 0 : 180
                                model: appController.models
                                textRole: "name"
                                valueRole: "id"
                                currentIndex: root.modelIndex(appController.codingModelId)
                                enabled: !appController.turnRunning
                                         && !appController.threadCreationPending
                                onActivated: {
                                    appController.codingModelId = currentValue || ""
                                    const options = root.reasoningOptions()
                                    let supported = false
                                    for (let i = 0; i < options.length; ++i) {
                                        if (options[i].value
                                                === appController.codingReasoningEffort) {
                                            supported = true
                                            break
                                        }
                                    }
                                    if (!supported)
                                        appController.codingReasoningEffort = ""
                                }
                            }
                            ComboBox {
                                id: reasoningPicker
                                Layout.row: 0
                                Layout.column: 1
                                Layout.fillWidth: composerControls.compact
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: composerControls.compact ? 0 : 150
                                model: root.reasoningOptions()
                                textRole: "label"
                                valueRole: "value"
                                currentIndex: root.reasoningIndex(
                                                  appController.codingReasoningEffort)
                                enabled: !appController.turnRunning
                                         && !appController.threadCreationPending
                                         && count > 1
                                onActivated: appController.codingReasoningEffort =
                                             currentValue || ""
                                ToolTip.text: "Reasoning effort for new threads"
                                ToolTip.visible: hovered
                            }
                            ComboBox {
                                id: permissionPicker
                                Layout.row: composerControls.compact ? 1 : 0
                                Layout.column: composerControls.compact ? 0 : 2
                                Layout.fillWidth: composerControls.compact
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: composerControls.compact ? 0 : 170
                                model: [
                                    { value: "approval-required", label: "Supervised" },
                                    { value: "auto-accept-edits", label: "Auto-accept edits" },
                                    { value: "full-access", label: "Full access" }
                                ]
                                textRole: "label"
                                valueRole: "value"
                                currentIndex: 2
                                enabled: !appController.turnRunning
                                         && !appController.threadCreationPending
                                ToolTip.text: currentValue === "approval-required"
                                              ? "Ask before commands and file changes."
                                              : currentValue === "auto-accept-edits"
                                                ? "Auto-approve edits, ask before other actions."
                                                : "Allow commands and edits without prompts."
                                ToolTip.visible: hovered
                            }
                            Button {
                                id: collaborationModeButton
                                Layout.row: composerControls.compact ? 1 : 0
                                Layout.column: composerControls.compact ? 1 : 3
                                Layout.fillWidth: composerControls.compact
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: composerControls.compact ? 0 : 92
                                text: checked ? "Plan" : "Build"
                                checkable: true
                                enabled: !appController.turnRunning
                                         && !appController.threadCreationPending
                                Accessible.name: checked ? "Plan mode" : "Build mode"
                                ToolTip.text: checked
                                              ? "Plan the work without making changes."
                                              : "Work on the task and make changes."
                                ToolTip.visible: hovered
                            }
                            Item {
                                visible: !composerControls.compact
                                Layout.row: 0
                                Layout.column: 4
                                Layout.fillWidth: true
                            }
                            Item {
                                id: contextUsageIndicator
                                Layout.row: composerControls.compact ? 2 : 0
                                Layout.column: composerControls.compact ? 0 : 5
                                Layout.preferredWidth: 36
                                Layout.preferredHeight: 36
                                Layout.alignment: composerControls.compact
                                                  ? Qt.AlignLeft : Qt.AlignCenter
                                visible: appController.hasTokenUsage

                                Canvas {
                                    id: contextUsageRing
                                    anchors.fill: parent
                                    anchors.margins: 3
                                    onPaint: {
                                        const context = getContext("2d")
                                        const center = width / 2
                                        const radius = Math.min(width, height) / 2 - 2
                                        if (radius <= 0)
                                            return
                                        const start = -Math.PI / 2
                                        const progress = Math.min(
                                            1, appController.contextUsagePercent / 100)
                                        context.clearRect(0, 0, width, height)
                                        context.lineWidth = 3
                                        context.lineCap = "round"
                                        context.strokeStyle = Qt.alpha(
                                            Kirigami.Theme.textColor, 0.18)
                                        context.beginPath()
                                        context.arc(center, center, radius, 0, Math.PI * 2)
                                        context.stroke()
                                        context.strokeStyle =
                                            appController.contextUsagePercent >= 90
                                            ? Kirigami.Theme.negativeTextColor
                                            : appController.contextUsagePercent >= 75
                                              ? Kirigami.Theme.neutralTextColor
                                              : Kirigami.Theme.highlightColor
                                        context.beginPath()
                                        context.arc(center, center, radius, start,
                                                    start + Math.PI * 2 * progress)
                                        context.stroke()
                                    }

                                    Connections {
                                        target: appController
                                        function onTokenUsageChanged() {
                                            contextUsageRing.requestPaint()
                                        }
                                    }
                                }
                                Label {
                                    anchors.centerIn: parent
                                    text: appController.contextUsagePercent
                                    font.pixelSize: 9
                                    font.bold: true
                                }
                                HoverHandler {
                                    id: contextUsageHover
                                }
                                ToolTip {
                                    id: contextUsagePopup
                                    visible: contextUsageHover.hovered
                                    delay: 300
                                    timeout: -1
                                    x: contextUsageIndicator.width - width
                                    y: -height - Kirigami.Units.smallSpacing
                                    padding: Kirigami.Units.largeSpacing

                                    contentItem: ColumnLayout {
                                        spacing: Kirigami.Units.smallSpacing
                                        Label {
                                            text: "CONTEXT WINDOW"
                                            opacity: 0.65
                                            font.pixelSize: 10
                                            font.letterSpacing: 1
                                        }
                                        Label {
                                            text: appController.contextUsagePercent + "% · "
                                                  + root.compactTokenCount(
                                                      appController.contextTokens)
                                                  + "/"
                                                  + root.compactTokenCount(
                                                      appController.modelContextWindow)
                                                  + " context used"
                                            font.bold: true
                                        }
                                        Label {
                                            text: "Total processed: "
                                                  + root.compactTokenCount(
                                                      appController.totalProcessedTokens)
                                                  + " tokens"
                                            opacity: 0.65
                                        }
                                        Label {
                                            text: "Automatically compacts its context when needed."
                                            opacity: 0.65
                                        }
                                    }
                                }
                            }
                            RowLayout {
                                Layout.row: composerControls.compact ? 2 : 0
                                Layout.column: composerControls.compact ? 1 : 6
                                Layout.alignment: composerControls.compact
                                                  ? Qt.AlignRight : Qt.AlignCenter

                                RoundButton {
                                    visible: appController.turnRunning
                                    text: "■"
                                    Accessible.name: "Stop"
                                    ToolTip.text: "Stop"
                                    ToolTip.visible: hovered
                                    onClicked: {
                                        appController.interruptTurn()
                                    }
                                }

                                Button {
                                    id: sendButton
                                    visible: root.pendingQuestionVisible
                                    text: "Submit answer"
                                    Accessible.name: text
                                    ToolTip.text: "Submit selected or typed answer (Enter)"
                                    ToolTip.visible: hovered
                                    enabled: (composer.text.trim().length > 0
                                              || root.selectedQuestionOption.length > 0)
                                             && root.hasProject
                                             && appController.providerReady
                                    onClicked: root.submitPendingQuestion()
                                }

                                RoundButton {
                                    id: sendPromptButton
                                    visible: !root.pendingQuestionVisible
                                    text: appController.threadCreationPending ? "…" : "↑"
                                    Accessible.name: appController.turnRunning
                                                     || appController.threadCreationPending
                                                     ? "Add message" : "Send"
                                    ToolTip.text: appController.turnRunning
                                                  ? "Add message to current turn (Enter)"
                                                  : appController.threadCreationPending
                                                  ? "Starting thread"
                                                  : "Send (Enter)"
                                    ToolTip.visible: hovered
                                    enabled: (composer.text.trim().length > 0
                                             || root.composerImages.length > 0)
                                             && root.hasProject
                                             && appController.providerReady
                                             && !appController.threadCreationPending
                                    onClicked: {
                                        if (appController.sendPrompt(
                                                    composer.text,
                                                    root.composerImages,
                                                    modelPicker.currentValue
                                                        || appController.codingModelId,
                                                    appController.codingReasoningEffort,
                                                    permissionPicker.currentValue,
                                                    collaborationModeButton.checked
                                                        ? "plan" : "default")) {
                                            composer.clear()
                                            root.composerImages = []
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        }

        Rectangle {
            id: threadPanelHandle
            visible: root.sidePanelVisible
            Layout.fillHeight: true
            Layout.preferredWidth: visible ? root.splitHandleWidth : 0
            color: threadPanelDrag.pressed
                   ? Kirigami.Theme.highlightColor
                   : threadPanelDrag.containsMouse
                     ? Qt.alpha(Kirigami.Theme.highlightColor, 0.55)
                     : Qt.alpha(Kirigami.Theme.textColor, 0.18)

            Rectangle {
                anchors.centerIn: parent
                width: 1
                height: parent.height
                color: Kirigami.Theme.textColor
                opacity: 0.28
            }

            MouseArea {
                id: threadPanelDrag
                property real pressGlobalX: 0
                property real pressWidth: 0
                anchors.fill: parent
                anchors.leftMargin: -3
                anchors.rightMargin: -3
                hoverEnabled: true
                cursorShape: Qt.SplitHCursor
                onPressed: mouse => {
                    pressGlobalX = mapToGlobal(mouse.x, mouse.y).x
                    pressWidth = root.threadPanelWidth
                }
                onPositionChanged: mouse => {
                    if (!pressed)
                        return
                    const globalX = mapToGlobal(mouse.x, mouse.y).x
                    root.threadPanelWidth = Math.max(
                        root.threadPanelMinimumWidth,
                        Math.min(700, pressWidth + pressGlobalX - globalX))
                }
            }
        }

        ThreadPane {
            visible: root.sidePanelVisible
            Layout.fillHeight: true
            Layout.minimumWidth: visible ? root.threadPanelWidth : 0
            Layout.preferredWidth: visible ? root.threadPanelWidth : 0
            Layout.maximumWidth: visible ? root.threadPanelWidth : 0
            controller: appController
            mode: root.sidePanelMode
            onCloseRequested: root.sidePanelVisible = false
        }
    }

    ImageViewerDialog {
        id: imageViewer
    }

    Window {
        id: diagnosticsDialog
        title: "Diagnostics - Artemis"
        visible: false
        modality: Qt.NonModal
        transientParent: root
        width: 900
        height: 640
        minimumWidth: 720
        minimumHeight: 520
        color: Kirigami.Theme.backgroundColor

        function centerOnMainWindow() {
            x = root.x + Math.round((root.width - width) / 2)
            y = root.y + Math.round((root.height - height) / 2)
        }

        function open() {
            if (!visible) {
                centerOnMainWindow()
                visible = true
            }
            raise()
            requestActivate()
        }

        Shortcut {
            sequence: "Esc"
            onActivated: diagnosticsDialog.close()
        }

        DiagnosticsPage {
            anchors.fill: parent
            controller: appController
        }
    }
}
