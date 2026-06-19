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
            const separator = message.indexOf("\n\n")
            if (separator < 0) {
                subjectEdit.text = message.trim()
                bodyEdit.text = ""
            } else {
                subjectEdit.text = message.substring(0, separator).trim()
                bodyEdit.text = message.substring(separator + 2).trim()
            }
            if (root.featureMode)
                branchEdit.text = root.controller.suggestBranch(subjectEdit.text)
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

        Label { text: "Subject"; font.bold: true }
        TextField {
            id: subjectEdit
            Layout.fillWidth: true
            placeholderText: "Concise summary in imperative mood"
            maximumLength: 120
            onTextEdited: {
                if (root.featureMode && branchEdit.text.length === 0)
                    branchEdit.text = root.controller.suggestBranch(text)
            }
        }

        Label { text: "Body"; font.bold: true }
        TextArea {
            id: bodyEdit
            Layout.fillWidth: true
            Layout.fillHeight: true
            placeholderText: "Explain what changed and why (optional)"
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
                enabled: subjectEdit.text.trim().length > 0 && !busy.running
                onClicked: {
                    busy.running = true
                    resultLabel.text = ""
                    if (root.featureMode)
                        root.controller.commitFeatureBranch(
                            subjectEdit.text, bodyEdit.text, branchEdit.text, remoteEdit.text)
                    else
                        root.controller.commitAllAndPush(subjectEdit.text, bodyEdit.text)
                }
            }
        }
    }

    onOpened: {
        resultLabel.text = ""
        subjectEdit.text = ""
        bodyEdit.text = ""
        branchEdit.text = ""
        busy.running = false
    }
}
