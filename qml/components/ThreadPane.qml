pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Pane {
    id: root
    required property var controller
    property string mode: "thread"
    signal closeRequested()
    padding: 0

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        ToolBar {
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Kirigami.Units.largeSpacing
                anchors.rightMargin: Kirigami.Units.smallSpacing

                Label {
                    text: root.mode === "diff" ? "Git diff" : "Thread"
                    font.bold: true
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                    Layout.fillWidth: true
                }
                ToolButton {
                    visible: root.mode === "diff"
                    icon.name: "view-refresh"
                    text: "Refresh"
                    onClicked: root.controller.refreshGit()
                }
                ToolButton {
                    icon.name: "window-close"
                    Accessible.name: "Close panel"
                    ToolTip.text: Accessible.name
                    ToolTip.visible: hovered
                    onClicked: root.closeRequested()
                }
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.mode === "diff" ? 1 : 0

            ScrollView {
                contentWidth: availableWidth

                ColumnLayout {
                    width: parent.width
                    spacing: Kirigami.Units.largeSpacing

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.margins: Kirigami.Units.largeSpacing
                        spacing: Kirigami.Units.smallSpacing

                        Label {
                            text: root.controller.selectedThreadTitle || "No thread selected"
                            font.bold: true
                            font.pointSize: Kirigami.Theme.defaultFont.pointSize + 2
                            Layout.fillWidth: true
                            wrapMode: Text.Wrap
                        }
                        Label {
                            visible: root.controller.selectedThreadId.length > 0
                            text: root.controller.turnRunning ? "In progress" : "Ready"
                            color: root.controller.turnRunning
                                   ? Kirigami.Theme.highlightColor
                                   : Kirigami.Theme.positiveTextColor
                        }
                    }

                    Kirigami.Separator { Layout.fillWidth: true }

                    GridLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.largeSpacing
                        Layout.rightMargin: Kirigami.Units.largeSpacing
                        columns: 2
                        columnSpacing: Kirigami.Units.largeSpacing
                        rowSpacing: Kirigami.Units.smallSpacing
                        visible: root.controller.selectedThreadId.length > 0

                        Label { text: "Model"; opacity: 0.6 }
                        Label {
                            text: root.controller.selectedThreadInfo.model || "Default"
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                        Label { text: "Workspace"; opacity: 0.6 }
                        Label {
                            text: "Project folder"
                            Layout.fillWidth: true
                        }
                        Label { text: "Path"; opacity: 0.6 }
                        Label {
                            id: workspacePathLabel
                            text: root.controller.selectedThreadInfo.cwd || ""
                            Layout.fillWidth: true
                            elide: Text.ElideMiddle
                            ToolTip.text: text
                            ToolTip.visible: workspacePathHover.hovered
                                             && workspacePathLabel.truncated
                            HoverHandler {
                                id: workspacePathHover
                            }
                        }
                    }

                    Kirigami.Separator {
                        Layout.fillWidth: true
                        visible: root.controller.selectedThreadId.length > 0
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.largeSpacing
                        Layout.rightMargin: Kirigami.Units.largeSpacing
                        Layout.bottomMargin: Kirigami.Units.largeSpacing
                        spacing: Kirigami.Units.smallSpacing

                        Label {
                            text: "Tasks"
                            font.bold: true
                            font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                        }
                        Label {
                            visible: root.controller.currentPlanExplanation.length > 0
                            text: root.controller.currentPlanExplanation
                            Layout.fillWidth: true
                            wrapMode: Text.Wrap
                            opacity: 0.7
                        }
                        TextArea {
                            Layout.fillWidth: true
                            visible: root.controller.currentTasks.length > 0
                            text: root.controller.currentTasks
                            textFormat: TextEdit.PlainText
                            readOnly: true
                            wrapMode: TextEdit.Wrap
                            selectByMouse: true
                            background: null
                        }
                        Repeater {
                            model: root.controller.currentPlan
                            delegate: Frame {
                                id: planStep
                                required property var modelData
                                Layout.fillWidth: true
                                padding: Kirigami.Units.smallSpacing

                                RowLayout {
                                    anchors.fill: parent
                                    spacing: Kirigami.Units.smallSpacing
                                    Kirigami.Icon {
                                        source: planStep.modelData.status === "completed"
                                                ? "task-complete"
                                                : planStep.modelData.status === "in_progress"
                                                  || planStep.modelData.status === "inProgress"
                                                  ? "media-playback-start"
                                                  : "media-playback-pause"
                                        Layout.preferredWidth: 18
                                        Layout.preferredHeight: 18
                                        color: planStep.modelData.status === "completed"
                                               ? Kirigami.Theme.positiveTextColor
                                               : planStep.modelData.status === "in_progress"
                                                 || planStep.modelData.status === "inProgress"
                                                 ? Kirigami.Theme.highlightColor
                                                 : Kirigami.Theme.textColor
                                        opacity: planStep.modelData.status === "pending" ? 0.45 : 1
                                    }
                                    Label {
                                        text: planStep.modelData.step || ""
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        font.strikeout: planStep.modelData.status === "completed"
                                        opacity: planStep.modelData.status === "pending" ? 0.62 : 1
                                    }
                                }
                            }
                        }
                        Kirigami.PlaceholderMessage {
                            Layout.fillWidth: true
                            visible: root.controller.currentTasks.length === 0
                                     && root.controller.currentPlan.length === 0
                            text: root.controller.selectedThreadId.length > 0
                                  ? "No tasks yet" : "No thread selected"
                            explanation: root.controller.selectedThreadId.length > 0
                                         ? "Generated tasks will appear here."
                                         : "Select or start a thread to see its tasks."
                        }
                    }
                }
            }

            Item {
                ScrollView {
                    anchors.fill: parent
                    visible: root.controller.diffText.length > 0
                    contentWidth: availableWidth

                    DiffView {
                        width: parent.width
                        diffText: root.controller.diffText
                    }
                }
                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: Math.min(parent.width - 40, 320)
                    visible: root.controller.diffText.length === 0
                    text: root.controller.selectedProjectIsGit
                          ? "Working tree is clean" : "Git unavailable"
                    explanation: root.controller.selectedProjectIsGit
                                 ? "Uncommitted changes will appear here."
                                 : "Select a Git repository to review changes."
                }
            }
        }
    }
}
