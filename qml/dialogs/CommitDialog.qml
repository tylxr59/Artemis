import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Dialog {
    id: root
    required property var controller
    property bool featureMode: false
    title: featureMode ? "Commit to feature branch" : "Commit and push changes"
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 40 : 720, 720)
    height: Math.min(parent ? parent.height - 40 : 650, 650)
    standardButtons: Dialog.Cancel

    Connections {
        target: root.controller
        function onCommitDraftReady(message) {
            messageEdit.text = message
            if (root.featureMode)
                branchEdit.text = root.controller.suggestBranch(message)
            busy.running = false
        }
        function onCommitFinished(success, message) {
            busy.running = false
            resultLabel.text = message
            resultLabel.color = success ? Kirigami.Theme.positiveTextColor
                                        : Kirigami.Theme.negativeTextColor
            if (success) closeTimer.start()
        }
    }

    Timer {
        id: closeTimer
        interval: 1000
        onTriggered: root.close()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            Layout.fillWidth: true
            Label {
                Layout.fillWidth: true
                text: "Generated with the commit model configured in Settings"
                opacity: 0.65
            }
            Button {
                text: "Generate"
                onClicked: {
                    busy.running = true
                    resultLabel.text = ""
                    root.controller.generateCommitMessage()
                }
            }
            BusyIndicator { id: busy; running: false }
        }

        Label { text: "Commit message"; font.bold: true }
        TextArea {
            id: messageEdit
            Layout.fillWidth: true
            Layout.fillHeight: true
            placeholderText: "Generate or enter a commit message"
            wrapMode: TextEdit.Wrap
        }

        GridLayout {
            visible: root.featureMode
            columns: 2
            Layout.fillWidth: true
            Label { text: "Branch" }
            TextField {
                id: branchEdit
                Layout.fillWidth: true
                placeholderText: "feature/change-name"
            }
            Label { text: "Remote" }
            TextField {
                id: remoteEdit
                Layout.fillWidth: true
                text: "origin"
            }
        }

        Label {
            id: resultLabel
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Button {
                text: root.featureMode ? "Create, commit, and push" : "Commit and push all changes"
                enabled: messageEdit.text.trim().length > 0 && !busy.running
                onClicked: {
                    busy.running = true
                    resultLabel.text = ""
                    if (root.featureMode)
                        root.controller.commitFeatureBranch(messageEdit.text, branchEdit.text, remoteEdit.text)
                    else
                        root.controller.commitAllAndPush(messageEdit.text)
                }
            }
        }
    }

    onOpened: {
        resultLabel.text = ""
        messageEdit.text = ""
        branchEdit.text = ""
        busy.running = false
    }
}
