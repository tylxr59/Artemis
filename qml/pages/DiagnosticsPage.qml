import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ScrollView {
    id: root
    required property var controller
    contentWidth: availableWidth
    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
    }

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

    function connectionIcon() {
        if (controller.providerReady)
            return "network-connect"
        if (controller.providerSetupRequired)
            return "dialog-warning"
        return "network-disconnect"
    }

    function connectionDetail() {
        if (controller.providerSetupRequired)
            return controller.providerSetupInstructions
        if (controller.providerIssueText.length > 0)
            return controller.providerIssueText
        return controller.providerVersion || "Codex not detected"
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

    function statusColor() {
        if (controller.providerIssueText.length > 0
                || controller.mcpIssueText.length > 0)
            return Kirigami.Theme.negativeTextColor
        if (controller.providerSetupRequired)
            return Kirigami.Theme.neutralTextColor
        return Kirigami.Theme.textColor
    }

    function statusIcon() {
        if (controller.providerIssueText.length > 0
                || controller.mcpIssueText.length > 0)
            return "dialog-error"
        if (controller.providerSetupRequired)
            return "dialog-warning"
        return "dialog-information"
    }

    function authText(status) {
        const value = status || ""
        if (value === "notLoggedIn")
            return "Not logged in"
        if (value.length === 0)
            return "Unknown"
        return value.charAt(0).toUpperCase() + value.slice(1)
    }

    function authColor(status) {
        return status === "notLoggedIn"
                ? Kirigami.Theme.neutralTextColor
                : Kirigami.Theme.positiveTextColor
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

    ColumnLayout {
        width: root.availableWidth
        spacing: Kirigami.Units.largeSpacing

        Item { Layout.preferredHeight: Kirigami.Units.smallSpacing }

        Frame {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            padding: Kirigami.Units.largeSpacing

            background: Rectangle {
                color: Qt.alpha(root.connectionColor(), 0.07)
                border.color: Qt.alpha(root.connectionColor(), 0.24)
                radius: Kirigami.Units.smallSpacing
            }

            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing

                Kirigami.Icon {
                    source: root.connectionIcon()
                    color: root.connectionColor()
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        Layout.fillWidth: true
                        text: "Codex connection"
                        opacity: 0.68
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                        elide: Text.ElideRight
                    }

                    Label {
                        Layout.fillWidth: true
                        text: root.connectionText()
                        color: root.connectionColor()
                        font.bold: true
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize + 5
                        elide: Text.ElideRight
                    }

                    Label {
                        Layout.fillWidth: true
                        text: root.connectionDetail()
                        textFormat: Text.PlainText
                        wrapMode: Text.Wrap
                        opacity: 0.76
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
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            columns: root.availableWidth < 820 ? 1 : 2
            columnSpacing: Kirigami.Units.largeSpacing
            rowSpacing: Kirigami.Units.largeSpacing

            SectionFrame {
                title: "Runtime"
                subtitle: "Local paths and provider state."

                Kirigami.FormLayout {
                    Layout.fillWidth: true

                    Label {
                        Kirigami.FormData.label: "Storage:"
                        Layout.fillWidth: true
                        text: controller.databasePath
                        textFormat: Text.PlainText
                        wrapMode: Text.WrapAnywhere
                    }

                    Label {
                        Kirigami.FormData.label: "Codex:"
                        Layout.fillWidth: true
                        text: controller.providerVersion || "Codex not detected"
                        textFormat: Text.PlainText
                        wrapMode: Text.WrapAnywhere
                    }

                    Label {
                        Kirigami.FormData.label: "MCP:"
                        Layout.fillWidth: true
                        text: controller.mcpBusy
                              ? "Refreshing servers"
                              : controller.mcpServers.length + " servers configured"
                        textFormat: Text.PlainText
                        wrapMode: Text.Wrap
                    }
                }
            }

            SectionFrame {
                title: "Current context"
                subtitle: controller.turnRunning
                          ? "Working - " + controller.turnElapsedText + " elapsed"
                          : "Ready"

                Kirigami.FormLayout {
                    Layout.fillWidth: true

                    Label {
                        Kirigami.FormData.label: "Project:"
                        Layout.fillWidth: true
                        text: controller.selectedProjectPath || "None"
                        textFormat: Text.PlainText
                        wrapMode: Text.WrapAnywhere
                    }

                    Label {
                        Kirigami.FormData.label: "Thread:"
                        Layout.fillWidth: true
                        text: controller.selectedThreadTitle || "None"
                        textFormat: Text.PlainText
                        wrapMode: Text.Wrap
                    }

                    Label {
                        Kirigami.FormData.label: "Git:"
                        Layout.fillWidth: true
                        text: controller.selectedProjectIsGit
                              ? "Repository detected" : "Not a repository"
                        textFormat: Text.PlainText
                        wrapMode: Text.Wrap
                    }

                    Label {
                        Kirigami.FormData.label: "Remote:"
                        Layout.fillWidth: true
                        visible: controller.gitRepositoryUrl.length > 0
                        text: controller.gitRepositoryUrl
                        textFormat: Text.PlainText
                        wrapMode: Text.WrapAnywhere
                    }
                }
            }

            SectionFrame {
                title: "Context window"
                subtitle: controller.hasTokenUsage
                          ? controller.contextUsagePercent + "% in use"
                          : "No token usage reported"

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

                    ProgressBar {
                        Layout.fillWidth: true
                        visible: controller.hasTokenUsage
                        from: 0
                        to: 100
                        value: controller.contextUsagePercent
                    }

                    Kirigami.FormLayout {
                        Layout.fillWidth: true

                        Label {
                            Kirigami.FormData.label: "Context:"
                            Layout.fillWidth: true
                            text: controller.hasTokenUsage
                                  ? root.compactTokenCount(controller.contextTokens)
                                    + " / "
                                    + root.compactTokenCount(controller.modelContextWindow)
                                  : "Unavailable"
                            textFormat: Text.PlainText
                        }

                        Label {
                            Kirigami.FormData.label: "Processed:"
                            Layout.fillWidth: true
                            text: controller.hasTokenUsage
                                  ? root.compactTokenCount(controller.totalProcessedTokens)
                                    + " tokens"
                                  : "Unavailable"
                            textFormat: Text.PlainText
                        }
                    }
                }
            }

            SectionFrame {
                title: "Git state"
                subtitle: controller.hasGitChanges
                          ? "Working tree has changes"
                          : "No tracked changes reported"

                Kirigami.FormLayout {
                    Layout.fillWidth: true

                    Label {
                        Kirigami.FormData.label: "Repository:"
                        Layout.fillWidth: true
                        text: controller.selectedProjectIsGit
                              ? "Available" : "Unavailable"
                        textFormat: Text.PlainText
                    }

                    Label {
                        Kirigami.FormData.label: "Changes:"
                        Layout.fillWidth: true
                        text: controller.hasGitChanges
                              ? "Changes detected" : "No changes"
                        textFormat: Text.PlainText
                    }
                }
            }
        }

        SectionFrame {
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            title: "MCP servers"
            subtitle: controller.mcpBusy
                      ? "Refreshing server status"
                      : controller.mcpServers.length + " configured"

            Label {
                Layout.fillWidth: true
                visible: controller.mcpServers.length === 0
                text: "No MCP servers configured."
                opacity: 0.62
            }

            ColumnLayout {
                Layout.fillWidth: true
                visible: controller.mcpServers.length > 0
                spacing: 0

                Repeater {
                    model: controller.mcpServers

                    delegate: ColumnLayout {
                        id: mcpRow
                        required property var modelData
                        Layout.fillWidth: true
                        spacing: 0

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.topMargin: Kirigami.Units.smallSpacing
                            Layout.bottomMargin: Kirigami.Units.smallSpacing
                            spacing: Kirigami.Units.smallSpacing

                            Kirigami.Icon {
                                source: mcpRow.modelData.authStatus === "notLoggedIn"
                                        ? "emblem-warning" : "dialog-ok-apply"
                                color: root.authColor(mcpRow.modelData.authStatus)
                                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 1

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
                                          + " - "
                                          + mcpRow.modelData.toolCount
                                          + " tools - "
                                          + mcpRow.modelData.resourceCount
                                          + " resources"
                                    textFormat: Text.PlainText
                                    opacity: 0.62
                                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                                    elide: Text.ElideRight
                                }
                            }

                            StatusPill {
                                text: root.authText(mcpRow.modelData.authStatus)
                                accentColor: root.authColor(mcpRow.modelData.authStatus)
                            }
                        }

                        Kirigami.Separator { Layout.fillWidth: true }
                    }
                }
            }
        }

        SectionFrame {
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            title: "Latest status"
            subtitle: "Most recent setup, provider, MCP, or application message."

            MessageStrip {
                iconName: root.statusIcon()
                text: root.statusText()
                accentColor: root.statusColor()
            }
        }
    }

    component SectionFrame: Frame {
        id: sectionFrame
        property string title
        property string subtitle: ""
        default property alias content: contentArea.data
        Layout.fillWidth: true
        padding: 0

        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
            border.color: Qt.alpha(Kirigami.Theme.textColor, 0.16)
            radius: Kirigami.Units.smallSpacing
        }

        contentItem: ColumnLayout {
            spacing: 0

            ColumnLayout {
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.largeSpacing
                spacing: 2

                Label {
                    Layout.fillWidth: true
                    text: sectionFrame.title
                    textFormat: Text.PlainText
                    font.bold: true
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    visible: sectionFrame.subtitle.length > 0
                    text: sectionFrame.subtitle
                    textFormat: Text.PlainText
                    opacity: 0.64
                    wrapMode: Text.Wrap
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                }
            }

            Kirigami.Separator { Layout.fillWidth: true }

            ColumnLayout {
                id: contentArea
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.largeSpacing
                spacing: Kirigami.Units.largeSpacing
            }
        }
    }

    component StatusPill: Rectangle {
        id: statusPill
        property string text: ""
        property color accentColor: Kirigami.Theme.textColor
        Layout.preferredWidth: pillLabel.implicitWidth + Kirigami.Units.largeSpacing
        Layout.preferredHeight: Math.max(24, pillLabel.implicitHeight
                                         + Kirigami.Units.smallSpacing)
        radius: height / 2
        color: Qt.alpha(accentColor, 0.10)
        border.color: Qt.alpha(accentColor, 0.24)

        Label {
            id: pillLabel
            anchors.centerIn: parent
            text: statusPill.text
            color: statusPill.accentColor
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            font.bold: true
        }
    }

    component MessageStrip: Frame {
        id: messageStrip
        property string iconName: "dialog-information"
        property string text: ""
        property color accentColor: Kirigami.Theme.textColor
        Layout.fillWidth: true
        padding: Kirigami.Units.smallSpacing

        background: Rectangle {
            color: Qt.alpha(messageStrip.accentColor, 0.08)
            border.color: Qt.alpha(messageStrip.accentColor, 0.22)
            radius: Kirigami.Units.smallSpacing
        }

        contentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                source: messageStrip.iconName
                color: messageStrip.accentColor
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
            }

            Label {
                Layout.fillWidth: true
                text: messageStrip.text
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
                color: messageStrip.accentColor
            }
        }
    }
}
