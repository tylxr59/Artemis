import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ScrollView {
    id: root
    required property var controller

    function connectionText() {
        if (controller.providerSetupRequired)
            return "Setup required"
        return controller.providerReady ? "Connected" : "Disconnected"
    }

    function connectionColor() {
        if (controller.providerReady)
            return Kirigami.Theme.positiveTextColor
        if (controller.providerSetupRequired)
            return Kirigami.Theme.neutralTextColor
        return Kirigami.Theme.negativeTextColor
    }

    ColumnLayout {
        width: root.availableWidth
        spacing: Kirigami.Units.largeSpacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                Label {
                    text: "Diagnostics"
                    font.bold: true
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize + 8
                }
                Label {
                    Layout.fillWidth: true
                    text: "Runtime, provider, project, and integration state."
                    opacity: 0.62
                    wrapMode: Text.Wrap
                }
            }

            Button {
                text: "Refresh MCP"
                icon.name: "view-refresh"
                enabled: controller.providerReady && !controller.mcpBusy
                onClicked: controller.refreshMcpServers()
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: width < 760 ? 1 : 3
            columnSpacing: Kirigami.Units.largeSpacing
            rowSpacing: Kirigami.Units.largeSpacing

            Frame {
                Layout.fillWidth: true
                padding: Kirigami.Units.largeSpacing

                ColumnLayout {
                    anchors.fill: parent
                    spacing: Kirigami.Units.smallSpacing

                    RowLayout {
                        Layout.fillWidth: true
                        Kirigami.Icon {
                            source: controller.providerReady
                                    ? "network-connect" : "network-disconnect"
                            color: root.connectionColor()
                            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                        }
                        Label {
                            text: root.connectionText()
                            color: root.connectionColor()
                            font.bold: true
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: controller.providerVersion || "Codex not detected"
                        wrapMode: Text.WrapAnywhere
                        opacity: 0.74
                    }
                }
            }

            Frame {
                Layout.fillWidth: true
                padding: Kirigami.Units.largeSpacing

                ColumnLayout {
                    anchors.fill: parent
                    spacing: Kirigami.Units.smallSpacing

                    RowLayout {
                        Layout.fillWidth: true
                        Kirigami.Icon {
                            source: "database"
                            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                            opacity: 0.75
                        }
                        Label {
                            text: "Storage"
                            font.bold: true
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: controller.databasePath
                        wrapMode: Text.WrapAnywhere
                        opacity: 0.74
                    }
                }
            }

            Frame {
                Layout.fillWidth: true
                padding: Kirigami.Units.largeSpacing

                ColumnLayout {
                    anchors.fill: parent
                    spacing: Kirigami.Units.smallSpacing

                    RowLayout {
                        Layout.fillWidth: true
                        Kirigami.Icon {
                            source: "network-server"
                            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                            opacity: 0.75
                        }
                        Label {
                            text: "MCP"
                            font.bold: true
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: controller.mcpBusy
                              ? "Refreshing servers"
                              : controller.mcpServers.length + " servers configured"
                        opacity: 0.74
                    }
                }
            }
        }

        Frame {
            Layout.fillWidth: true
            padding: Kirigami.Units.largeSpacing

            ColumnLayout {
                anchors.fill: parent
                spacing: Kirigami.Units.largeSpacing

                Label {
                    text: "Current Context"
                    font.bold: true
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: Kirigami.Units.largeSpacing
                    rowSpacing: Kirigami.Units.smallSpacing

                    Label { text: "Project"; opacity: 0.62 }
                    Label {
                        Layout.fillWidth: true
                        text: controller.selectedProjectPath || "None"
                        wrapMode: Text.WrapAnywhere
                    }

                    Label { text: "Thread"; opacity: 0.62 }
                    Label {
                        Layout.fillWidth: true
                        text: controller.selectedThreadTitle || "None"
                        wrapMode: Text.Wrap
                    }

                    Label { text: "Git"; opacity: 0.62 }
                    Label {
                        Layout.fillWidth: true
                        text: controller.selectedProjectIsGit
                              ? "Repository detected" : "Not a repository"
                        wrapMode: Text.Wrap
                    }
                }
            }
        }

        Frame {
            Layout.fillWidth: true
            padding: Kirigami.Units.largeSpacing
            visible: controller.mcpServers.length > 0

            ColumnLayout {
                anchors.fill: parent
                spacing: Kirigami.Units.smallSpacing

                Label {
                    text: "MCP Servers"
                    font.bold: true
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                }

                Repeater {
                    model: controller.mcpServers

                    delegate: RowLayout {
                        id: mcpRow
                        required property var modelData
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        Kirigami.Icon {
                            source: mcpRow.modelData.authStatus === "notLoggedIn"
                                    ? "emblem-warning" : "dialog-ok-apply"
                            color: mcpRow.modelData.authStatus === "notLoggedIn"
                                   ? Kirigami.Theme.neutralTextColor
                                   : Kirigami.Theme.positiveTextColor
                            Layout.preferredWidth: Kirigami.Units.iconSizes.small
                            Layout.preferredHeight: Kirigami.Units.iconSizes.small
                        }
                        Label {
                            Layout.fillWidth: true
                            text: (mcpRow.modelData.title || mcpRow.modelData.name)
                                  + " · "
                                  + mcpRow.modelData.toolCount
                                  + " tools · "
                                  + mcpRow.modelData.authStatus
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }

        Frame {
            Layout.fillWidth: true
            padding: Kirigami.Units.largeSpacing
            visible: controller.providerSetupRequired
                     || controller.providerIssueText.length > 0
                     || controller.mcpIssueText.length > 0
                     || controller.statusText.length > 0

            ColumnLayout {
                anchors.fill: parent
                spacing: Kirigami.Units.smallSpacing

                Label {
                    text: "Latest Status"
                    font.bold: true
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                }
                Label {
                    Layout.fillWidth: true
                    text: controller.providerSetupRequired
                          ? controller.providerSetupInstructions
                          : controller.mcpIssueText.length > 0
                            ? controller.mcpIssueText
                            : controller.providerIssueText.length > 0
                              ? controller.providerIssueText
                              : controller.statusText
                    wrapMode: Text.Wrap
                    color: controller.providerIssueText.length > 0
                           || controller.mcpIssueText.length > 0
                           ? Kirigami.Theme.negativeTextColor
                           : Kirigami.Theme.textColor
                    opacity: 0.86
                }
            }
        }
    }
}
