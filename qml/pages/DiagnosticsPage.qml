import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ScrollView {
    id: root
    required property var controller
    contentWidth: availableWidth

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

    function statusText() {
        if (controller.providerSetupRequired)
            return controller.providerSetupInstructions
        if (controller.mcpIssueText.length > 0)
            return controller.mcpIssueText
        if (controller.providerIssueText.length > 0)
            return controller.providerIssueText
        if (controller.statusText.length > 0)
            return controller.statusText
        return "No current issues."
    }

    ColumnLayout {
        width: root.availableWidth
        spacing: Kirigami.Units.largeSpacing

        Pane {
            Layout.fillWidth: true
            padding: Kirigami.Units.largeSpacing

            RowLayout {
                anchors.fill: parent
                spacing: Kirigami.Units.largeSpacing

                Kirigami.Icon {
                    source: controller.providerReady
                            ? "network-connect" : "network-disconnect"
                    color: root.connectionColor()
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    Label {
                        Layout.fillWidth: true
                        text: root.connectionText()
                        color: root.connectionColor()
                        font.bold: true
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize + 3
                    }

                    Label {
                        Layout.fillWidth: true
                        text: controller.providerVersion || "Codex not detected"
                        wrapMode: Text.WrapAnywhere
                        opacity: 0.68
                    }
                }

                Button {
                    text: "Refresh MCP"
                    icon.name: "view-refresh"
                    enabled: controller.providerReady && !controller.mcpBusy
                    onClicked: controller.refreshMcpServers()
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: width < 720 ? 1 : 2
            columnSpacing: Kirigami.Units.largeSpacing
            rowSpacing: Kirigami.Units.largeSpacing

            DiagnosticsSection {
                Layout.fillWidth: true
                title: "Runtime"

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: Kirigami.Units.largeSpacing
                    rowSpacing: Kirigami.Units.smallSpacing

                    Label { text: "Storage"; opacity: 0.62 }
                    Label {
                        Layout.fillWidth: true
                        text: controller.databasePath
                        wrapMode: Text.WrapAnywhere
                    }

                    Label { text: "MCP"; opacity: 0.62 }
                    Label {
                        Layout.fillWidth: true
                        text: controller.mcpBusy
                              ? "Refreshing servers"
                              : controller.mcpServers.length + " servers configured"
                        wrapMode: Text.Wrap
                    }
                }
            }

            DiagnosticsSection {
                Layout.fillWidth: true
                title: "Current Context"

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

        DiagnosticsSection {
            Layout.fillWidth: true
            title: "MCP Servers"

            Label {
                Layout.fillWidth: true
                visible: controller.mcpServers.length === 0
                text: "No MCP servers configured."
                opacity: 0.62
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                Repeater {
                    model: controller.mcpServers

                    delegate: ColumnLayout {
                        id: mcpRow
                        required property var modelData
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.topMargin: Kirigami.Units.smallSpacing
                            Layout.bottomMargin: Kirigami.Units.smallSpacing
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

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 0

                                Label {
                                    Layout.fillWidth: true
                                    text: mcpRow.modelData.title || mcpRow.modelData.name
                                    textFormat: Text.PlainText
                                    font.bold: true
                                    elide: Text.ElideRight
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: mcpRow.modelData.name
                                          + " · "
                                          + mcpRow.modelData.toolCount
                                          + " tools · "
                                          + mcpRow.modelData.resourceCount
                                          + " resources · "
                                          + mcpRow.modelData.authStatus
                                    textFormat: Text.PlainText
                                    opacity: 0.62
                                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                                    elide: Text.ElideRight
                                }
                            }
                        }

                        Separator {}
                    }
                }
            }
        }

        DiagnosticsSection {
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            title: "Latest Status"

            Label {
                Layout.fillWidth: true
                text: root.statusText()
                wrapMode: Text.Wrap
                color: controller.providerIssueText.length > 0
                       || controller.mcpIssueText.length > 0
                       ? Kirigami.Theme.negativeTextColor
                       : Kirigami.Theme.textColor
                opacity: 0.86
            }
        }
    }

    component DiagnosticsSection: ColumnLayout {
        property string title
        spacing: Kirigami.Units.smallSpacing

        Label {
            Layout.fillWidth: true
            text: title
            font.bold: true
        }

        Separator {}
    }

    component Separator: Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 1
        color: Kirigami.Theme.disabledTextColor
        opacity: 0.25
    }
}
