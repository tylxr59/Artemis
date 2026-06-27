import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Dialog {
    id: root
    required property var controller
    property bool settingsApplied: false
    title: "Settings"
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 40 : 620, 620)
    height: Math.min(parent ? parent.height - 40 : 760, 760)

    function modelIndex(modelId) {
        const exact = codingModel.indexOfValue(modelId)
        if (exact >= 0)
            return exact
        for (let i = 0; i < controller.models.length; ++i) {
            if (controller.models[i].isDefault)
                return i
        }
        return controller.models.length > 0 ? 0 : -1
    }

    function loadValues() {
        codingModel.currentIndex = modelIndex(controller.codingModelId)
        commitModel.currentIndex = modelIndex(controller.commitModelId)
        titleModel.currentIndex = modelIndex(controller.titleModelId)
        const editorIndex = editorPicker.indexOfValue(controller.selectedEditorId)
        editorPicker.currentIndex = editorIndex >= 0 ? editorIndex : 0
        const terminalIndex = terminalPicker.indexOfValue(controller.selectedTerminalId)
        terminalPicker.currentIndex = terminalIndex >= 0 ? terminalIndex : 0
        settingsApplied = false
        confirmationTimer.stop()
        controller.refreshMcpServers()
    }

    function clearConfirmation() {
        settingsApplied = false
        confirmationTimer.stop()
    }

    function applyValues() {
        const codingModelId = codingModel.currentValue || ""
        const commitModelId = commitModel.currentValue || ""
        const titleModelId = titleModel.currentValue || ""
        const editorId = editorPicker.currentValue || ""
        const terminalId = terminalPicker.currentValue || ""

        controller.codingModelId = codingModelId
        controller.commitModelId = commitModelId
        controller.titleModelId = titleModelId
        controller.selectedEditorId = editorId
        controller.selectedTerminalId = terminalId

        settingsApplied = controller.codingModelId === codingModelId
                && controller.commitModelId === commitModelId
                && controller.titleModelId === titleModelId
                && controller.selectedEditorId === editorId
                && controller.selectedTerminalId === terminalId
        if (settingsApplied)
            confirmationTimer.restart()
    }

    onOpened: loadValues()
    onApplied: applyValues()

    Timer {
        id: confirmationTimer
        interval: 3500
        onTriggered: root.settingsApplied = false
    }

    footer: DialogButtonBox {
        standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Apply

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            opacity: root.settingsApplied ? 1 : 0
            DialogButtonBox.buttonRole: DialogButtonBox.HelpRole

            Behavior on opacity {
                NumberAnimation { duration: Kirigami.Units.shortDuration }
            }

            Kirigami.Icon {
                source: "dialog-ok-apply"
                color: Kirigami.Theme.positiveTextColor
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
            }

            Label {
                text: "Settings applied"
                color: Kirigami.Theme.positiveTextColor
                font.pointSize: Kirigami.Theme.smallFont.pointSize
            }
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.availableWidth
            spacing: Kirigami.Units.largeSpacing

            Frame {
                Layout.fillWidth: true
                padding: Kirigami.Units.largeSpacing

                ColumnLayout {
                    anchors.fill: parent
                    spacing: Kirigami.Units.largeSpacing

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        Kirigami.Icon {
                            source: "applications-development"
                            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 0

                            Label {
                                text: "Models"
                                font.bold: true
                                font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                            }
                            Label {
                                text: "Choose which model Artemis uses for each task."
                                opacity: 0.65
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                            }
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        columnSpacing: Kirigami.Units.largeSpacing
                        rowSpacing: Kirigami.Units.smallSpacing

                        Label { text: "New threads" }
                        ComboBox {
                            id: codingModel
                            Layout.fillWidth: true
                            model: root.controller.models
                            textRole: "name"
                            valueRole: "id"
                            onActivated: root.clearConfirmation()
                            Accessible.name: "Model for new threads"
                        }

                        Label { text: "Commit messages" }
                        ComboBox {
                            id: commitModel
                            Layout.fillWidth: true
                            model: root.controller.models
                            textRole: "name"
                            valueRole: "id"
                            onActivated: root.clearConfirmation()
                            Accessible.name: "Model for commit messages"
                        }

                        Label { text: "Thread titles" }
                        ComboBox {
                            id: titleModel
                            Layout.fillWidth: true
                            model: root.controller.models
                            textRole: "name"
                            valueRole: "id"
                            onActivated: root.clearConfirmation()
                            Accessible.name: "Model for thread titles"
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: "Commit messages and thread titles run in separate, read-only ephemeral threads."
                        wrapMode: Text.Wrap
                        opacity: 0.65
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                    }
                }
            }

            Frame {
                Layout.fillWidth: true
                padding: Kirigami.Units.largeSpacing

                ColumnLayout {
                    anchors.fill: parent
                    spacing: Kirigami.Units.largeSpacing

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        Kirigami.Icon {
                            source: "network-connect"
                            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 0

                            Label {
                                text: "MCP"
                                font.bold: true
                                font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                            }
                            Label {
                                text: "Manage Codex Model Context Protocol servers."
                                opacity: 0.65
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                            }
                        }

                        ToolButton {
                            icon.name: "view-refresh"
                            enabled: !root.controller.mcpBusy
                            Accessible.name: "Refresh MCP servers"
                            ToolTip.text: Accessible.name
                            ToolTip.visible: hovered
                            onClicked: root.controller.refreshMcpServers()
                        }

                        ToolButton {
                            icon.name: "system-reboot"
                            enabled: !root.controller.mcpBusy
                            Accessible.name: "Reload MCP configuration"
                            ToolTip.text: Accessible.name
                            ToolTip.visible: hovered
                            onClicked: root.controller.reloadMcpServers()
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: root.controller.mcpIssueText.length > 0
                        text: root.controller.mcpIssueText
                        color: Kirigami.Theme.negativeTextColor
                        wrapMode: Text.Wrap
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.controller.mcpLoginUrl.length > 0
                        spacing: Kirigami.Units.smallSpacing

                        Label {
                            Layout.fillWidth: true
                            text: root.controller.mcpLoginUrl
                            elide: Text.ElideMiddle
                        }
                        Button {
                            text: "Open login"
                            icon.name: "internet-web-browser"
                            onClicked: Qt.openUrlExternally(root.controller.mcpLoginUrl)
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 1

                        Repeater {
                            model: root.controller.mcpServers

                            delegate: ItemDelegate {
                                id: mcpServerRow
                                required property var modelData
                                Layout.fillWidth: true
                                leftPadding: Kirigami.Units.smallSpacing
                                rightPadding: Kirigami.Units.smallSpacing

                                contentItem: RowLayout {
                                    spacing: Kirigami.Units.smallSpacing

                                    Kirigami.Icon {
                                        source: mcpServerRow.modelData.authStatus === "notLoggedIn"
                                                ? "emblem-warning" : "network-server"
                                        Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                                        Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                                        color: mcpServerRow.modelData.authStatus === "notLoggedIn"
                                               ? Kirigami.Theme.neutralTextColor
                                               : Kirigami.Theme.textColor
                                        opacity: 0.8
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 0

                                        Label {
                                            Layout.fillWidth: true
                                            text: mcpServerRow.modelData.title
                                                  || mcpServerRow.modelData.name
                                            textFormat: Text.PlainText
                                            font.bold: true
                                            elide: Text.ElideRight
                                        }
                                        Label {
                                            Layout.fillWidth: true
                                            text: mcpServerRow.modelData.name
                                                  + " · "
                                                  + mcpServerRow.modelData.toolCount
                                                  + " tools · "
                                                  + mcpServerRow.modelData.resourceCount
                                                  + " resources · "
                                                  + mcpServerRow.modelData.authStatus
                                            textFormat: Text.PlainText
                                            font.pointSize: Kirigami.Theme.smallFont.pointSize
                                            opacity: 0.62
                                            elide: Text.ElideRight
                                        }
                                    }

                                    ToolButton {
                                        icon.name: "dialog-password"
                                        enabled: !root.controller.mcpBusy
                                        Accessible.name: "Log in to MCP server"
                                        ToolTip.text: Accessible.name
                                        ToolTip.visible: hovered
                                        onClicked: root.controller.loginMcpServer(
                                                       mcpServerRow.modelData.name)
                                    }
                                    ToolButton {
                                        icon.name: "edit-delete-remove"
                                        enabled: !root.controller.mcpBusy
                                        Accessible.name: "Remove MCP server"
                                        ToolTip.text: Accessible.name
                                        ToolTip.visible: hovered
                                        onClicked: root.controller.removeMcpServer(
                                                       mcpServerRow.modelData.name)
                                    }
                                }
                            }
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        columnSpacing: Kirigami.Units.largeSpacing
                        rowSpacing: Kirigami.Units.smallSpacing

                        Label { text: "Name" }
                        TextField {
                            id: mcpName
                            Layout.fillWidth: true
                            placeholderText: "context7"
                        }

                        Label { text: "Transport" }
                        ComboBox {
                            id: mcpTransport
                            Layout.fillWidth: true
                            model: [
                                { value: "http", label: "HTTP URL" },
                                { value: "stdio", label: "Stdio command" }
                            ]
                            textRole: "label"
                            valueRole: "value"
                        }

                        Label { text: "Target" }
                        TextField {
                            id: mcpTarget
                            Layout.fillWidth: true
                            placeholderText: mcpTransport.currentValue === "http"
                                             ? "https://example.com/mcp"
                                             : "npx -y @upstash/context7-mcp"
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Item { Layout.fillWidth: true }
                        Button {
                            text: "Add server"
                            icon.name: "list-add"
                            enabled: !root.controller.mcpBusy
                                     && mcpName.text.trim().length > 0
                                     && mcpTarget.text.trim().length > 0
                            onClicked: {
                                root.controller.addMcpServer(
                                            mcpName.text,
                                            mcpTransport.currentValue,
                                            mcpTarget.text)
                                mcpName.clear()
                                mcpTarget.clear()
                            }
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

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        Kirigami.Icon {
                            source: "applications-utilities"
                            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 0

                            Label {
                                text: "Applications"
                                font.bold: true
                                font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                            }
                            Label {
                                text: "Set the desktop applications used to open project resources."
                                opacity: 0.65
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                            }
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        columnSpacing: Kirigami.Units.largeSpacing
                        rowSpacing: Kirigami.Units.smallSpacing

                        Label { text: "Repositories" }
                        ComboBox {
                            id: editorPicker
                            Layout.fillWidth: true
                            model: root.controller.editorOptions
                            textRole: "name"
                            valueRole: "id"
                            onActivated: root.clearConfirmation()
                            Accessible.name: "Application for repositories"
                        }

                        Label { text: "Terminals" }
                        ComboBox {
                            id: terminalPicker
                            Layout.fillWidth: true
                            model: root.controller.terminalOptions
                            textRole: "name"
                            valueRole: "id"
                            onActivated: root.clearConfirmation()
                            Accessible.name: "Application for terminals"
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: "Available editors, IDEs, and terminal emulators are detected from installed desktop applications."
                        wrapMode: Text.Wrap
                        opacity: 0.65
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                    }
                }
            }
        }
    }
}
