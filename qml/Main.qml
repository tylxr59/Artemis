import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
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

    property bool reviewVisible: width >= 1100
    property bool navigationVisible: width >= 780
    readonly property bool hasProject: appController.selectedProjectPath.length > 0
    readonly property bool hasThread: appController.selectedThreadId.length > 0

    FileDialog {
        id: projectDialog
        title: "Add project folder"
        fileMode: FileDialog.OpenFile
        currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
        options: FileDialog.ShowDirsOnly
        onAccepted: appController.addProject(selectedFile)
    }

    FullAccessDialog {
        id: fullAccessDialog
        onAccepted: appController.acknowledgeFullAccess()
    }

    CommitDialog {
        id: commitDialog
        controller: appController
    }

    Connections {
        target: appController
        function onFullAccessRequired() { fullAccessDialog.open() }
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
                text: root.reviewVisible ? "Hide changes" : "Changes"
                visible: appController.selectedProjectIsGit
                onClicked: root.reviewVisible = !root.reviewVisible
            }
            Button {
                text: "Commit"
                visible: appController.selectedProjectIsGit
                enabled: appController.gitStatusText.length > 0
                onClicked: {
                    commitDialog.featureMode = false
                    commitDialog.open()
                    appController.generateCommitMessage(commitDialog.selectedModelId)
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
                        text: "Commit all changes"
                        onTriggered: {
                            commitDialog.featureMode = false
                            commitDialog.open()
                            appController.generateCommitMessage(commitDialog.selectedModelId)
                        }
                    }
                    MenuItem {
                        text: "Review and select changes"
                        onTriggered: {
                            root.reviewVisible = true
                            commitDialog.featureMode = false
                            commitDialog.open()
                        }
                    }
                    MenuItem {
                        text: "Commit to feature branch"
                        onTriggered: {
                            commitDialog.featureMode = true
                            commitDialog.open()
                            appController.generateCommitMessage(commitDialog.selectedModelId)
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

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Pane {
            id: navigationPane
            visible: root.navigationVisible || navigationDrawer.visible
            Layout.preferredWidth: 310
            Layout.fillHeight: true
            padding: Kirigami.Units.largeSpacing

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
                    Button {
                        text: "Add"
                        icon.name: "folder-new"
                        onClicked: projectDialog.open()
                        Accessible.name: "Add project folder"
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
                            Layout.fillWidth: true
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
                                    source: projectDelegate.isGit ? "vcs-normal" : "folder"
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
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            visible: projectDelegate.selected
                            spacing: 2

                            Repeater {
                                model: projectDelegate.selected ? appController.threads : []
                                delegate: ItemDelegate {
                                    required property int index
                                    required property var modelData
                                    Layout.fillWidth: true
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
                                      ? "No threads yet. Use + to start one."
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
                        onTriggered: appController.createThread(false, "")
                    }
                    MenuItem {
                        text: "New worktree thread"
                        enabled: appController.selectedProjectIsGit
                        onTriggered: appController.createThread(true, "")
                    }
                }
                Kirigami.Separator { Layout.fillWidth: true }
                RowLayout {
                    Layout.fillWidth: true
                    ToolButton {
                        text: "Terminal"
                        icon.name: "utilities-terminal"
                        enabled: root.hasProject
                        onClicked: appController.openTerminal()
                    }
                    Item { Layout.fillWidth: true }
                    ToolButton {
                        text: "Diagnostics"
                        icon.name: "tools-report-bug"
                        onClicked: diagnosticsDialog.open()
                    }
                }
            }
        }

        Rectangle {
            visible: root.navigationVisible
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: Kirigami.Theme.textColor
            opacity: 0.18
        }

        Pane {
            Layout.fillWidth: true
            Layout.fillHeight: true
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
                    leftMargin: Math.max(Kirigami.Units.largeSpacing, (width - 760) / 2)
                    rightMargin: leftMargin
                    topMargin: Kirigami.Units.largeSpacing
                    bottomMargin: Kirigami.Units.largeSpacing
                    delegate: ConversationDelegate { width: conversationList.width - conversationList.leftMargin - conversationList.rightMargin }
                    onCountChanged: positionViewAtEnd()

                    Kirigami.PlaceholderMessage {
                        anchors.centerIn: parent
                        width: Math.min(parent.width - 40, 480)
                        visible: conversationList.count === 0
                        text: !root.hasProject ? "Start with a project"
                              : !appController.providerReady ? "Codex is offline"
                              : root.hasThread ? "What would you like to build?"
                              : "Start a new thread"
                        explanation: !root.hasProject
                                     ? "Add a project folder to give Artemis a workspace."
                                     : !appController.providerReady
                                       ? "Open Diagnostics to inspect the Codex connection."
                                       : root.hasThread
                                         ? (appController.selectedProjectIsGit
                                            ? "Describe a task below. File changes will appear in the Changes pane."
                                            : "Describe a task below. Git review is unavailable for this folder.")
                                         : "Create a local thread, or use a worktree to isolate changes."
                    }

                    Button {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.verticalCenter
                        anchors.topMargin: Kirigami.Units.gridUnit * 4
                        visible: conversationList.count === 0
                                 && (!root.hasProject
                                     || (root.hasProject && appController.providerReady && !root.hasThread))
                        text: !root.hasProject ? "Add project folder" : "Create thread"
                        icon.name: !root.hasProject ? "folder-new" : "list-add"
                        onClicked: {
                            if (!root.hasProject)
                                projectDialog.open()
                            else
                                newThreadMenu.open()
                        }
                    }
                }

                Frame {
                    visible: root.hasThread
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
                            enabled: root.hasThread
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
                                enabled: !appController.turnRunning
                            }
                            Label {
                                text: "Full access"
                                color: Kirigami.Theme.negativeTextColor
                                ToolTip.text: "Codex can run commands and modify files without approval prompts."
                                ToolTip.visible: accessHover.hovered
                                HoverHandler { id: accessHover }
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
                                         && root.hasThread
                                onClicked: {
                                    appController.sendPrompt(composer.text)
                                    composer.clear()
                                }
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            visible: root.reviewVisible && appController.selectedProjectIsGit
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: Kirigami.Theme.textColor
            opacity: 0.18
        }

        ReviewPane {
            visible: root.reviewVisible && appController.selectedProjectIsGit
            Layout.preferredWidth: Math.max(390, root.width * 0.32)
            Layout.maximumWidth: 620
            Layout.fillHeight: true
            controller: appController
            onCommitRequested: {
                commitDialog.featureMode = false
                commitDialog.open()
                appController.generateCommitMessage(commitDialog.selectedModelId)
            }
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
