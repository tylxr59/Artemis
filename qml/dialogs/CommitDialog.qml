import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Dialog {
    id: root
    required property var controller
    property bool featureMode: false
    property bool busy: false
    property bool generating: false
    property bool lockBlocked: false
    title: featureMode ? "Commit to feature branch" : "Commit and push changes"
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 40 : 720, 720)
    height: Math.min(parent ? parent.height - 40 : 650, 650)

    footer: DialogButtonBox {
        standardButtons: DialogButtonBox.Cancel

        Label {
            text: "Generated with the commit model configured in Settings"
            opacity: 0.65
            DialogButtonBox.buttonRole: DialogButtonBox.HelpRole
        }

        Button {
            text: root.featureMode ? "Create, commit, and push" : "Commit and push all changes"
            enabled: subjectEdit.text.trim().length > 0 && !root.busy
            DialogButtonBox.buttonRole: DialogButtonBox.ActionRole
            onClicked: {
                root.busy = true
                root.generating = false
                resultLabel.text = ""
                if (root.featureMode)
                    root.controller.commitFeatureBranch(
                        subjectEdit.text, bodyEdit.text, branchEdit.text, remoteEdit.text)
                else
                    root.controller.commitAllAndPush(subjectEdit.text, bodyEdit.text)
            }
        }
    }

    Connections {
        target: root.controller
        function onCommitDraftFinished(success, message) {
            if (!success) {
                root.busy = false
                root.generating = false
                resultLabel.text = message
                resultLabel.color = Kirigami.Theme.negativeTextColor
                return
            }
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
            root.busy = false
            root.generating = false
        }
        function onCommitFinished(success, message) {
            root.busy = false
            root.lockBlocked = false
            resultLabel.text = message
            resultLabel.color = success ? Kirigami.Theme.positiveTextColor
                                        : Kirigami.Theme.negativeTextColor
            if (success) closeTimer.start()
        }
        function onCommitLockBlocked(message) {
            root.busy = false
            root.lockBlocked = true
            resultLabel.text = message
            resultLabel.color = Kirigami.Theme.negativeTextColor
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
            Item { Layout.fillWidth: true }
            Label {
                text: "Generating..."
                opacity: root.generating ? 1 : 0
            }
        }

        Label { text: "Repository"; font.bold: true }
        TextField {
            Layout.fillWidth: true
            text: root.controller.selectedWorkspacePath
            readOnly: true
            selectByMouse: true
            Accessible.name: "Git repository"
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
            visible: root.lockBlocked
            Layout.fillWidth: true

            Button {
                text: "Retry"
                onClicked: {
                    root.lockBlocked = false
                    root.busy = true
                    root.controller.retryLockedCommit()
                }
            }
            Button {
                text: "Remove stale lock…"
                onClicked: removeLockDialog.open()
            }
            Button {
                text: "Cancel operation"
                onClicked: root.controller.cancelLockedCommit()
            }
            Item { Layout.fillWidth: true }
        }

    }

    Dialog {
        id: removeLockDialog
        title: "Remove Git index lock?"
        modal: true
        anchors.centerIn: parent

        contentItem: Label {
            width: 420
            wrapMode: Text.Wrap
            text: "Only continue if no other Git process or Git client is using this repository. "
                  + "Removing an active lock can damage the Git index."
        }

        footer: DialogButtonBox {
            standardButtons: DialogButtonBox.Cancel
            Button {
                text: "Remove lock and retry"
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                onClicked: {
                    removeLockDialog.close()
                    root.lockBlocked = false
                    root.busy = true
                    root.controller.removeCommitLockAndRetry()
                }
            }
        }
    }

    onOpened: {
        resultLabel.text = ""
        subjectEdit.text = ""
        bodyEdit.text = ""
        branchEdit.text = ""
        root.lockBlocked = false
        root.busy = true
        root.generating = true
        root.controller.generateCommitMessage()
    }

    onRejected: {
        if (root.lockBlocked)
            root.controller.cancelLockedCommit()
    }
}
