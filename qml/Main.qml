import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.artemis

Kirigami.ApplicationWindow {
    id: root
    width: 1500
    height: 900
    minimumWidth: 760
    minimumHeight: 560
    visible: true
    title: appController.selectedThreadTitle.length > 0
           ? appController.selectedThreadTitle + " — Artemis" : "Artemis"

    property bool sidePanelVisible: false
    property string sidePanelMode: "thread"
    property bool navigationVisible: width >= 780
    readonly property bool hasProject: appController.selectedProjectPath.length > 0
    readonly property bool hasThread: appController.selectedThreadId.length > 0

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

    CommitDialog {
        id: commitDialog
        controller: appController
    }

    SettingsDialog {
        id: settingsDialog
        controller: appController
    }

    Connections {
        target: appController
        function onPromptRestoreRequested(text) {
            if (composer.text.length === 0)
                composer.text = text
        }
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Kirigami.Units.smallSpacing
            anchors.rightMargin: Kirigami.Units.smallSpacing

            ToolButton {
                visible: !root.navigationVisible
                text: "☰"
                Accessible.name: "Projects"
                onClicked: navigationDrawer.open()
            }
            Label {
                text: appController.selectedThreadTitle.length > 0
                      ? appController.selectedThreadTitle : "Artemis"
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
            Rectangle {
                Layout.preferredWidth: connectionRow.implicitWidth + Kirigami.Units.largeSpacing
                Layout.preferredHeight: connectionRow.implicitHeight + Kirigami.Units.smallSpacing
                radius: height / 2
                color: Qt.alpha(appController.providerReady
                                ? Kirigami.Theme.positiveTextColor
                                : Kirigami.Theme.negativeTextColor, 0.12)
                RowLayout {
                    id: connectionRow
                    anchors.centerIn: parent
                    spacing: Kirigami.Units.smallSpacing
                    Rectangle {
                        Layout.preferredWidth: 7
                        Layout.preferredHeight: 7
                        radius: 4
                        color: appController.providerReady
                               ? Kirigami.Theme.positiveTextColor
                               : Kirigami.Theme.negativeTextColor
                    }
                    Label {
                        text: appController.providerReady ? "Codex connected" : "Codex offline"
                        color: appController.providerReady
                               ? Kirigami.Theme.positiveTextColor
                               : Kirigami.Theme.negativeTextColor
                    }
                }
            }
            ToolButton {
                text: "Open folder"
                visible: root.hasProject
                onClicked: appController.openProjectFolder()
            }
            ToolButton {
                text: "Thread"
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
                visible: appController.selectedProjectIsGit
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
                text: "Commit & push"
                visible: appController.selectedProjectIsGit
                enabled: appController.gitStatusText.length > 0
                onClicked: {
                    commitDialog.featureMode = false
                    commitDialog.open()
                    appController.generateCommitMessage()
                }
            }
            ToolButton {
                id: commitMenuButton
                text: "More"
                visible: appController.selectedProjectIsGit
                enabled: appController.gitStatusText.length > 0
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
                            appController.generateCommitMessage()
                        }
                    }
                    MenuItem {
                        text: "Review and select changes"
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
                            appController.generateCommitMessage()
                        }
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
        contentItem: navigationPane
    }

    SplitView {
        id: workspaceSplit
        anchors.fill: parent
        orientation: Qt.Horizontal

        handle: Rectangle {
            implicitWidth: 5
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

        Pane {
            id: navigationPane
            visible: root.navigationVisible || navigationDrawer.visible
            SplitView.preferredWidth: 310
            SplitView.minimumWidth: root.navigationVisible ? 220 : 0
            SplitView.maximumWidth: root.navigationVisible ? 480 : 0
            padding: Kirigami.Units.largeSpacing
            clip: true

            ColumnLayout {
                anchors.fill: parent
                spacing: Kirigami.Units.largeSpacing

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: "Projects"
                        font.bold: true
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                        Layout.fillWidth: true
                    }
                    ToolButton {
                        icon.name: "folder-new"
                        onClicked: appController.chooseProjectFolder()
                        Accessible.name: "Add project folder"
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
                        width: projectList.width
                        spacing: 2
                        visible: projectSearch.text.length === 0
                                 || name.toLowerCase().includes(projectSearch.text.toLowerCase())

                        ItemDelegate {
                            id: projectItem
                            Layout.fillWidth: true
                            Layout.maximumWidth: projectList.width
                            highlighted: projectDelegate.selected
                            leftPadding: Kirigami.Units.smallSpacing
                            rightPadding: Kirigami.Units.smallSpacing
                            contentItem: RowLayout {
                                spacing: Kirigami.Units.smallSpacing
                                Kirigami.Icon {
                                    source: projectDelegate.selected
                                            ? "arrow-down" : "arrow-right"
                                    Layout.preferredWidth: 14
                                    Layout.preferredHeight: 14
                                    opacity: 0.65
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
                                    text: "+"
                                    visible: projectDelegate.selected
                                    enabled: appController.providerReady
                                    Accessible.name: "New thread in " + projectDelegate.name
                                    ToolTip.text: Accessible.name
                                    ToolTip.visible: hovered
                                    onClicked: {
                                        if (!projectDelegate.selected)
                                            appController.selectProject(projectDelegate.index)
                                        newThreadMenu.open()
                                    }
                                }
                            }
                            ToolTip.text: projectDelegate.path
                            ToolTip.visible: hovered
                            onClicked: appController.selectProject(projectDelegate.index)

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
                            visible: projectDelegate.selected
                            spacing: 2

                            Repeater {
                                model: projectDelegate.selected ? appController.threads : []
                                delegate: ItemDelegate {
                                    id: threadItem
                                    required property int index
                                    required property var modelData
                                    Layout.fillWidth: true
                                    Layout.maximumWidth: projectList.width
                                    leftPadding: Kirigami.Units.gridUnit * 2
                                    highlighted: appController.selectedThreadId === modelData.id
                                    contentItem: RowLayout {
                                        spacing: Kirigami.Units.smallSpacing
                                        Kirigami.Icon {
                                            source: "dialog-messages"
                                            Layout.preferredWidth: 16
                                            Layout.preferredHeight: 16
                                            opacity: 0.7
                                        }
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 1
                                            Label {
                                                text: modelData.title
                                                Layout.fillWidth: true
                                                elide: Text.ElideRight
                                            }
                                            Label {
                                                text: (modelData.location === "worktree"
                                                       ? "Worktree" : "Local")
                                                      + (modelData.external ? " · External" : "")
                                                font: Kirigami.Theme.smallFont
                                                opacity: 0.55
                                            }
                                        }
                                    }
                                    onClicked: {
                                        appController.selectThread(index)
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
                                            onTriggered: appController.removeThread(index)
                                        }
                                    }
                                }
                            }

                            Label {
                                Layout.fillWidth: true
                                Layout.leftMargin: Kirigami.Units.gridUnit * 2
                                Layout.rightMargin: Kirigami.Units.smallSpacing
                                Layout.topMargin: Kirigami.Units.smallSpacing
                                Layout.bottomMargin: Kirigami.Units.smallSpacing
                                visible: appController.threads.length === 0
                                text: appController.providerReady
                                      ? "No chats yet. Send a message to start one."
                                      : "Codex is offline."
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
                Menu {
                    id: newThreadMenu
                    MenuItem {
                        text: "New local thread"
                        onTriggered: appController.createThread(
                                         false, appController.codingModelId,
                                         permissionPicker.currentValue)
                    }
                    MenuItem {
                        text: "New worktree thread"
                        enabled: appController.selectedProjectIsGit
                        onTriggered: appController.createThread(
                                         true, appController.codingModelId,
                                         permissionPicker.currentValue)
                    }
                }
                Kirigami.Separator { Layout.fillWidth: true }
                RowLayout {
                    Layout.fillWidth: true
                    ToolButton {
                        icon.name: "utilities-terminal"
                        enabled: root.hasProject
                        onClicked: appController.openTerminal()
                        Accessible.name: "Open terminal"
                        ToolTip.text: Accessible.name
                        ToolTip.visible: hovered
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

        Pane {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 360
            padding: 0

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                ListView {
                    id: conversationList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: appController.conversation
                    clip: true
                    spacing: Kirigami.Units.largeSpacing
                    topMargin: Kirigami.Units.largeSpacing
                    bottomMargin: Kirigami.Units.largeSpacing
                    delegate: Item {
                        id: conversationRow
                        required property string eventType
                        required property string title
                        required property string content
                        required property var metadata

                        readonly property real horizontalGutter: Kirigami.Units.largeSpacing
                        width: conversationList.width
                        implicitHeight: conversationDelegate.implicitHeight

                        ConversationDelegate {
                            id: conversationDelegate
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: Math.min(840, parent.width
                                                 - conversationRow.horizontalGutter * 2)
                            eventType: conversationRow.eventType
                            title: conversationRow.title
                            content: conversationRow.content
                            metadata: conversationRow.metadata
                        }
                    }
                    onCountChanged: positionViewAtEnd()
                    onWidthChanged: forceLayout()

                    Kirigami.PlaceholderMessage {
                        anchors.centerIn: parent
                        width: Math.min(parent.width - 40, 480)
                        visible: conversationList.count === 0
                        text: !root.hasProject ? "Start with a project"
                              : !appController.providerReady ? "Codex is offline"
                              : "What would you like to build?"
                        explanation: !root.hasProject
                                     ? "Add a project folder to give Artemis a workspace."
                                     : !appController.providerReady
                                       ? "Open Diagnostics to inspect the Codex connection."
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

                Frame {
                    visible: root.hasProject && appController.providerReady
                    Layout.fillWidth: true
                    Layout.maximumWidth: 840
                    Layout.alignment: Qt.AlignHCenter
                    Layout.margins: Kirigami.Units.largeSpacing
                    padding: Kirigami.Units.smallSpacing

                    ColumnLayout {
                        anchors.fill: parent
                        TextArea {
                            id: composer
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.max(90, implicitHeight)
                            Layout.maximumHeight: 260
                            placeholderText: appController.turnRunning
                                             ? "Add guidance while Artemis is working…"
                                             : "Describe a task, ask a question, or paste an error…"
                            wrapMode: TextEdit.Wrap
                            enabled: root.hasProject && appController.providerReady
                            Keys.onPressed: event => {
                                if ((event.modifiers & Qt.ControlModifier)
                                        && (event.key === Qt.Key_Return || event.key === Qt.Key_Enter)) {
                                    sendButton.clicked()
                                    event.accepted = true
                                }
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            ComboBox {
                                id: modelPicker
                                Layout.preferredWidth: 180
                                model: appController.models
                                textRole: "name"
                                valueRole: "id"
                                currentIndex: root.modelIndex(appController.codingModelId)
                                enabled: !appController.turnRunning
                                onActivated: appController.codingModelId = currentValue || ""
                            }
                            ComboBox {
                                id: permissionPicker
                                Layout.preferredWidth: 170
                                model: [
                                    { value: "approval-required", label: "Supervised" },
                                    { value: "auto-accept-edits", label: "Auto-accept edits" },
                                    { value: "full-access", label: "Full access" }
                                ]
                                textRole: "label"
                                valueRole: "value"
                                currentIndex: 2
                                enabled: !appController.turnRunning
                                ToolTip.text: currentValue === "approval-required"
                                              ? "Ask before commands and file changes."
                                              : currentValue === "auto-accept-edits"
                                                ? "Auto-approve edits, ask before other actions."
                                                : "Allow commands and edits without prompts."
                                ToolTip.visible: hovered
                            }
                            Label {
                                text: appController.statusText
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignRight
                                elide: Text.ElideRight
                                opacity: 0.7
                            }
                            Button {
                                visible: appController.turnRunning
                                text: "Stop"
                                onClicked: appController.interruptTurn()
                            }
                            RoundButton {
                                id: sendButton
                                text: appController.turnRunning ? "↪" : "↑"
                                ToolTip.text: appController.turnRunning
                                              ? "Send guidance" : "Send (Ctrl+Enter)"
                                ToolTip.visible: hovered
                                enabled: composer.text.trim().length > 0
                                         && root.hasProject
                                         && appController.providerReady
                                onClicked: {
                                    appController.sendPrompt(composer.text,
                                                             appController.codingModelId,
                                                             permissionPicker.currentValue)
                                    composer.clear()
                                }
                            }
                        }
                    }
                }
            }
        }

        ThreadPane {
            visible: root.sidePanelVisible
            SplitView.preferredWidth: Math.min(620, Math.max(390, root.width * 0.32))
            SplitView.minimumWidth: visible ? 300 : 0
            SplitView.maximumWidth: visible ? 700 : 0
            controller: appController
            mode: root.sidePanelMode
            onCloseRequested: root.sidePanelVisible = false
        }
    }

    Dialog {
        id: diagnosticsDialog
        title: "Diagnostics"
        modal: true
        width: Math.min(root.width - 40, 680)
        height: Math.min(root.height - 40, 560)
        anchors.centerIn: parent
        contentItem: DiagnosticsPage { controller: appController }
        standardButtons: Dialog.Close
    }
}
