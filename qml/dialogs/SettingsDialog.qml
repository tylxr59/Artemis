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
    width: Math.min(parent ? parent.width - 40 : 760, 760)
    height: Math.min(parent ? parent.height - 40 : 620, 620)

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

    contentItem: RowLayout {
        spacing: 0

        Pane {
            Layout.fillHeight: true
            Layout.preferredWidth: 154
            padding: Kirigami.Units.smallSpacing

            ColumnLayout {
                anchors.fill: parent
                spacing: Kirigami.Units.smallSpacing

                Label {
                    Layout.fillWidth: true
                    Layout.leftMargin: Kirigami.Units.smallSpacing
                    Layout.rightMargin: Kirigami.Units.smallSpacing
                    text: "Categories"
                    opacity: 0.62
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    font.capitalization: Font.AllUppercase
                }

                ListView {
                    id: settingsNavigation
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    currentIndex: 0
                    model: ListModel {
                        ListElement {
                            title: "Models"
                            subtitle: "Defaults"
                            iconName: "applications-development"
                        }
                        ListElement {
                            title: "MCP"
                            subtitle: "Servers"
                            iconName: "network-connect"
                        }
                        ListElement {
                            title: "Applications"
                            subtitle: "Open with"
                            iconName: "applications-utilities"
                        }
                    }

                    delegate: ItemDelegate {
                        id: navigationDelegate
                        required property int index
                        required property string title
                        required property string subtitle
                        required property string iconName
                        width: ListView.view.width
                        height: 48
                        highlighted: ListView.isCurrentItem
                        onClicked: settingsNavigation.currentIndex = index

                        contentItem: RowLayout {
                            spacing: Kirigami.Units.smallSpacing

                            Kirigami.Icon {
                                source: navigationDelegate.iconName
                                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 0

                                Label {
                                    Layout.fillWidth: true
                                    text: navigationDelegate.title
                                    font.bold: navigationDelegate.highlighted
                                    elide: Text.ElideRight
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: navigationDelegate.subtitle
                                    opacity: 0.58
                                    elide: Text.ElideRight
                                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                                }
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: Kirigami.Theme.disabledTextColor
            opacity: 0.28
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 58
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                spacing: Kirigami.Units.smallSpacing

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    Label {
                        Layout.fillWidth: true
                        text: settingsNavigation.currentIndex === 0 ? "Models"
                              : settingsNavigation.currentIndex === 1 ? "MCP Servers"
                              : "Applications"
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize + 4
                    }

                    Label {
                        Layout.fillWidth: true
                        text: settingsNavigation.currentIndex === 0
                              ? "Model defaults for interactive and background tasks."
                              : settingsNavigation.currentIndex === 1
                                ? "External context servers available to Codex."
                                : "Desktop applications used for project actions."
                        opacity: 0.62
                        elide: Text.ElideRight
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                    }
                }

                RowLayout {
                    visible: settingsNavigation.currentIndex === 1
                    spacing: Kirigami.Units.smallSpacing

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
            }

            Separator {}

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: settingsNavigation.currentIndex

                ScrollView {
                    contentWidth: availableWidth

                    ColumnLayout {
                        width: parent.availableWidth
                        spacing: Kirigami.Units.largeSpacing

                        SettingsSectionHeader {
                            title: "Default Models"
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
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
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
                            text: "Commit messages and thread titles run in separate, read-only ephemeral threads."
                            wrapMode: Text.Wrap
                            opacity: 0.65
                            font.pointSize: Kirigami.Theme.smallFont.pointSize
                        }
                    }
                }

                ScrollView {
                    contentWidth: availableWidth

                    ColumnLayout {
                        width: parent.availableWidth
                        spacing: Kirigami.Units.largeSpacing

                        Label {
                            Layout.fillWidth: true
                            Layout.topMargin: Kirigami.Units.largeSpacing
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
                            visible: root.controller.mcpIssueText.length > 0
                            text: root.controller.mcpIssueText
                            color: Kirigami.Theme.negativeTextColor
                            wrapMode: Text.Wrap
                            font.pointSize: Kirigami.Theme.smallFont.pointSize
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
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

                        SettingsSectionHeader {
                            title: "Configured Servers"
                        }

                        Label {
                            Layout.fillWidth: true
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
                            visible: root.controller.mcpServers.length === 0
                            text: "No MCP servers configured."
                            opacity: 0.62
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
                            spacing: 0

                            Repeater {
                                model: root.controller.mcpServers

                                delegate: ColumnLayout {
                                    id: mcpServerRow
                                    required property var modelData
                                    Layout.fillWidth: true
                                    spacing: Kirigami.Units.smallSpacing

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Layout.topMargin: Kirigami.Units.smallSpacing
                                        Layout.bottomMargin: Kirigami.Units.smallSpacing
                                        spacing: Kirigami.Units.smallSpacing

                                        Kirigami.Icon {
                                            source: mcpServerRow.modelData.authStatus === "notLoggedIn"
                                                    ? "emblem-warning" : "network-server"
                                            Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                                            Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                                            color: mcpServerRow.modelData.authStatus === "notLoggedIn"
                                                   ? Kirigami.Theme.neutralTextColor
                                                   : Kirigami.Theme.textColor
                                            opacity: 0.85
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

                                    Separator {}
                                }
                            }
                        }

                        SettingsSectionHeader {
                            title: "Add Server"
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
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
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
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

                ScrollView {
                    contentWidth: availableWidth

                    ColumnLayout {
                        width: parent.availableWidth
                        spacing: Kirigami.Units.largeSpacing

                        SettingsSectionHeader {
                            title: "Open Project Resources"
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
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
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
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

    component SettingsSectionHeader: ColumnLayout {
        property string title
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.leftMargin: Kirigami.Units.largeSpacing
        Layout.rightMargin: Kirigami.Units.largeSpacing
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
