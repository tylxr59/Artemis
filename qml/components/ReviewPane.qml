import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Pane {
    id: root
    required property var controller
    signal commitRequested()
    padding: 0

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        ToolBar {
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Kirigami.Units.largeSpacing
                anchors.rightMargin: Kirigami.Units.largeSpacing
                Label {
                    text: "Changes"
                    font.bold: true
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                    Layout.fillWidth: true
                }
                ToolButton {
                    text: "Refresh"
                    icon.name: "view-refresh"
                    onClicked: root.controller.refreshGit()
                }
                Button {
                    text: "Commit"
                    enabled: root.controller.selectedProjectIsGit
                             && root.controller.gitStatusText.length > 0
                    onClicked: root.commitRequested()
                }
            }
        }

        TabBar {
            id: tabs
            Layout.fillWidth: true
            TabButton { text: "Diff" }
            TabButton { text: "Status" }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabs.currentIndex

            Item {
                ScrollView {
                    anchors.fill: parent
                    visible: root.controller.diffText.length > 0
                    TextArea {
                        text: root.controller.diffText
                        readOnly: true
                        selectByMouse: true
                        wrapMode: TextEdit.NoWrap
                        font.family: "monospace"
                        color: Kirigami.Theme.textColor
                    }
                }
                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: Math.min(parent.width - 40, 320)
                    visible: root.controller.diffText.length === 0
                    text: root.controller.selectedProjectIsGit
                          ? "Working tree is clean" : "Git unavailable"
                    explanation: root.controller.selectedProjectIsGit
                                 ? "Changes made by Artemis will appear here."
                                 : "Select a Git repository to review changes."
                }
            }
            Item {
                ScrollView {
                    anchors.fill: parent
                    visible: root.controller.gitStatusText.length > 0
                    TextArea {
                        text: root.controller.gitStatusText
                        readOnly: true
                        selectByMouse: true
                        wrapMode: TextEdit.NoWrap
                        font.family: "monospace"
                    }
                }
                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: Math.min(parent.width - 40, 320)
                    visible: root.controller.gitStatusText.length === 0
                    text: "No changed files"
                    explanation: "Git status details will appear here."
                }
            }
        }
    }
}
