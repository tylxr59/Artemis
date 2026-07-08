import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import org.kde.kirigami as Kirigami
import "../components"
import "../utils/AppHelpers.js" as AppHelpers

Window {
    id: root
    required property var controller
    property bool settingsApplied: false
    title: "Configure - Artemis"
    visible: false
    modality: Qt.NonModal
    width: 920
    height: 560
    minimumWidth: 760
    minimumHeight: 500
    color: Kirigami.Theme.backgroundColor

    function centerOnTransientParent() {
        if (!transientParent)
            return
        x = transientParent.x + Math.round((transientParent.width - width) / 2)
        y = transientParent.y + Math.round((transientParent.height - height) / 2)
    }

    function open() {
        if (!visible) {
            loadValues()
            centerOnTransientParent()
            visible = true
        }
        raise()
        requestActivate()
    }

    function pageTitle(index) {
        if (index === 0)
            return "Models"
        if (index === 1)
            return "MCP Servers"
        return "Applications"
    }

    function pageDescription(index) {
        if (index === 0)
            return "Model defaults for new work, commits, and generated titles."
        if (index === 1)
            return "External context servers available to the active provider."
        return "Desktop applications used for project actions."
    }

    function pageIcon(index) {
        if (index === 0)
            return "applications-development"
        if (index === 1)
            return "network-connect"
        return "applications-utilities"
    }

    function loadValues() {
        codingModel.currentIndex = AppHelpers.modelIndex(
                    codingModel, controller.models, controller.codingModelId)
        reasoningPicker.currentIndex = AppHelpers.reasoningIndex(
                    AppHelpers.selectedModel(controller.models,
                                             codingModel.currentIndex),
                    controller.codingReasoningEffort)
        commitModel.currentIndex = AppHelpers.modelIndex(
                    commitModel, controller.models, controller.commitModelId)
        titleModel.currentIndex = AppHelpers.modelIndex(
                    titleModel, controller.models, controller.titleModelId)
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
        const codingReasoningEffort = reasoningPicker.currentValue || ""
        const commitModelId = commitModel.currentValue || ""
        const titleModelId = titleModel.currentValue || ""
        const editorId = editorPicker.currentValue || ""
        const terminalId = terminalPicker.currentValue || ""

        controller.codingModelId = codingModelId
        controller.codingReasoningEffort = codingReasoningEffort
        controller.commitModelId = commitModelId
        controller.titleModelId = titleModelId
        controller.selectedEditorId = editorId
        controller.selectedTerminalId = terminalId

        settingsApplied = controller.codingModelId === codingModelId
                && controller.codingReasoningEffort === codingReasoningEffort
                && controller.commitModelId === commitModelId
                && controller.titleModelId === titleModelId
                && controller.selectedEditorId === editorId
                && controller.selectedTerminalId === terminalId
        if (settingsApplied)
            confirmationTimer.restart()
    }

    Timer {
        id: confirmationTimer
        interval: 3500
        onTriggered: root.settingsApplied = false
    }

    Shortcut {
        sequence: "Esc"
        onActivated: root.close()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Pane {
                Layout.fillHeight: true
                Layout.preferredWidth: 132
                padding: Kirigami.Units.smallSpacing

                background: Rectangle {
                    color: Kirigami.Theme.alternateBackgroundColor
                }

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    ListView {
                        id: settingsNavigation
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.topMargin: Kirigami.Units.smallSpacing
                        clip: true
                        currentIndex: 0
                        spacing: Kirigami.Units.smallSpacing
                        model: ListModel {
                            ListElement {
                                title: "Models"
                                subtitle: "Default choices"
                                iconName: "applications-development"
                            }
                            ListElement {
                                title: "MCP Servers"
                                subtitle: "Tools and resources"
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
                            height: 72
                            highlighted: ListView.isCurrentItem
                            leftPadding: Kirigami.Units.smallSpacing
                            rightPadding: Kirigami.Units.smallSpacing
                            topPadding: Kirigami.Units.smallSpacing
                            bottomPadding: Kirigami.Units.smallSpacing
                            onClicked: settingsNavigation.currentIndex = index
                            ToolTip.text: navigationDelegate.subtitle
                            ToolTip.visible: hovered && navigationDelegate.subtitle.length > 0

                            background: Rectangle {
                                radius: Kirigami.Units.smallSpacing
                                color: navigationDelegate.highlighted
                                       ? Qt.alpha(Kirigami.Theme.highlightColor, 0.17)
                                       : navigationDelegate.hovered
                                         ? Qt.alpha(Kirigami.Theme.textColor, 0.05)
                                         : "transparent"
                            }

                            contentItem: ColumnLayout {
                                spacing: 2

                                Kirigami.Icon {
                                    Layout.alignment: Qt.AlignHCenter
                                    source: navigationDelegate.iconName
                                    color: navigationDelegate.highlighted
                                           ? Kirigami.Theme.highlightColor
                                           : Kirigami.Theme.textColor
                                    opacity: navigationDelegate.highlighted ? 1 : 0.72
                                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: navigationDelegate.title
                                    horizontalAlignment: Text.AlignHCenter
                                    font.bold: navigationDelegate.highlighted
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                }
            }

            Kirigami.Separator {
                Layout.preferredWidth: 1
                Layout.fillHeight: true
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                ToolBar {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: Kirigami.Units.largeSpacing
                        anchors.rightMargin: Kirigami.Units.smallSpacing
                        spacing: Kirigami.Units.smallSpacing

                        Label {
                            Layout.fillWidth: true
                            text: root.pageTitle(settingsNavigation.currentIndex)
                            font.pointSize: Kirigami.Theme.defaultFont.pointSize + 4
                            elide: Text.ElideRight
                        }

                        BusyIndicator {
                            visible: settingsNavigation.currentIndex === 1
                                     && root.controller.mcpBusy
                            running: visible
                            Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                        }

                        ToolButton {
                            visible: settingsNavigation.currentIndex === 1
                            icon.name: "view-refresh"
                            enabled: !root.controller.mcpBusy
                            Accessible.name: "Refresh MCP servers"
                            ToolTip.text: Accessible.name
                            ToolTip.visible: hovered
                            onClicked: root.controller.refreshMcpServers()
                        }

                        ToolButton {
                            visible: settingsNavigation.currentIndex === 1
                            icon.name: "system-reboot"
                            enabled: !root.controller.mcpBusy
                            Accessible.name: "Reload MCP configuration"
                            ToolTip.text: Accessible.name
                            ToolTip.visible: hovered
                            onClicked: root.controller.reloadMcpServers()
                        }
                    }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: settingsNavigation.currentIndex

                ScrollView {
                    id: modelsPage
                    contentWidth: availableWidth
                    background: Rectangle {
                        color: Kirigami.Theme.backgroundColor
                    }

                    ColumnLayout {
                        width: modelsPage.availableWidth
                        spacing: Kirigami.Units.largeSpacing

                        Item { Layout.preferredHeight: Kirigami.Units.smallSpacing }

                        SectionFrame {
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
                            title: "Default models"
                            subtitle: "These choices are used when Artemis starts a new provider task."

                            Kirigami.FormLayout {
                                Layout.fillWidth: true

                                ComboBox {
                                    id: codingModel
                                    Kirigami.FormData.label: "New threads:"
                                    Layout.fillWidth: true
                                    model: root.controller.models
                                    textRole: "name"
                                    valueRole: "id"
                                    onActivated: {
                                        root.clearConfirmation()
                                        if (!AppHelpers.reasoningSupported(
                                                    AppHelpers.selectedModel(
                                                        root.controller.models,
                                                        codingModel.currentIndex),
                                                    reasoningPicker.currentValue || ""))
                                            reasoningPicker.currentIndex = 0
                                    }
                                    Accessible.name: "Model for new threads"
                                }

                                ComboBox {
                                    id: reasoningPicker
                                    Kirigami.FormData.label: "Reasoning effort:"
                                    Layout.fillWidth: true
                                    model: AppHelpers.reasoningOptions(
                                               AppHelpers.selectedModel(
                                                   root.controller.models,
                                                   codingModel.currentIndex))
                                    textRole: "label"
                                    valueRole: "value"
                                    enabled: count > 1
                                    currentIndex: AppHelpers.reasoningIndex(
                                                      AppHelpers.selectedModel(
                                                          root.controller.models,
                                                          codingModel.currentIndex),
                                                      root.controller.codingReasoningEffort)
                                    onActivated: root.clearConfirmation()
                                    Accessible.name: "Reasoning effort for new threads"
                                }

                                ComboBox {
                                    id: commitModel
                                    Kirigami.FormData.label: "Commit messages:"
                                    Layout.fillWidth: true
                                    model: root.controller.models
                                    textRole: "name"
                                    valueRole: "id"
                                    onActivated: root.clearConfirmation()
                                    Accessible.name: "Model for commit messages"
                                }

                                ComboBox {
                                    id: titleModel
                                    Kirigami.FormData.label: "Thread titles:"
                                    Layout.fillWidth: true
                                    model: root.controller.models
                                    textRole: "name"
                                    valueRole: "id"
                                    onActivated: root.clearConfirmation()
                                    Accessible.name: "Model for thread titles"
                                }
                            }

                            MessageStrip {
                                iconName: "dialog-information"
                                text: "Commit messages and thread titles run in separate, read-only ephemeral threads."
                                accentColor: Kirigami.Theme.highlightColor
                                useSmallFont: true
                            }
                        }

                        Item {
                            Layout.fillHeight: true
                            Layout.minimumHeight: Kirigami.Units.largeSpacing
                        }
                    }
                }

                ScrollView {
                    id: mcpPage
                    contentWidth: availableWidth
                    background: Rectangle {
                        color: Kirigami.Theme.backgroundColor
                    }

                    ColumnLayout {
                        width: mcpPage.availableWidth
                        spacing: Kirigami.Units.largeSpacing

                        Item { Layout.preferredHeight: Kirigami.Units.smallSpacing }

                        MessageStrip {
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
                            visible: root.controller.mcpIssueText.length > 0
                            iconName: "dialog-error"
                            text: root.controller.mcpIssueText
                            accentColor: Kirigami.Theme.negativeTextColor
                            useSmallFont: true
                        }

                        SectionFrame {
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
                            visible: root.controller.mcpLoginUrl.length > 0
                            title: "Login requested"
                            subtitle: "Continue authentication in the browser."

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: Kirigami.Units.smallSpacing

                                Label {
                                    Layout.fillWidth: true
                                    text: root.controller.mcpLoginUrl
                                    elide: Text.ElideMiddle
                                    textFormat: Text.PlainText
                                }

                                Button {
                                    text: "Open login"
                                    icon.name: "internet-web-browser"
                                    onClicked: Qt.openUrlExternally(root.controller.mcpLoginUrl)
                                }
                            }
                        }

                        SectionFrame {
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
                            title: "Configured servers"
                            subtitle: root.controller.mcpBusy
                                      ? "Refreshing server status"
                                      : root.controller.mcpServers.length + " configured"

                            Label {
                                Layout.fillWidth: true
                                visible: root.controller.mcpServers.length === 0
                                text: "No MCP servers configured."
                                opacity: 0.62
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                visible: root.controller.mcpServers.length > 0
                                spacing: 0

                                Repeater {
                                    model: root.controller.mcpServers

                                    delegate: ColumnLayout {
                                        id: mcpServerRow
                                        required property var modelData
                                        Layout.fillWidth: true
                                        spacing: 0

                                        ItemDelegate {
                                            id: mcpServerDelegate
                                            Layout.fillWidth: true
                                            leftPadding: 0
                                            rightPadding: 0
                                            topPadding: Kirigami.Units.smallSpacing
                                            bottomPadding: Kirigami.Units.smallSpacing

                                            background: Rectangle {
                                                color: mcpServerDelegate.hovered
                                                       ? Qt.alpha(Kirigami.Theme.textColor, 0.05)
                                                       : "transparent"
                                            }

                                            contentItem: McpServerRow {
                                                server: mcpServerRow.modelData

                                                Kirigami.Icon {
                                                    source: mcpServerRow.modelData.authStatus === "notLoggedIn"
                                                            ? "dialog-warning" : "dialog-ok-apply"
                                                    color: AppHelpers.authColor(
                                                               mcpServerRow.modelData.authStatus,
                                                               Kirigami.Theme.neutralTextColor,
                                                               Kirigami.Theme.positiveTextColor)
                                                    Layout.preferredWidth: Kirigami.Units.iconSizes.small
                                                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                                                }

                                                Label {
                                                    text: AppHelpers.authText(
                                                              mcpServerRow.modelData.authStatus)
                                                    color: AppHelpers.authColor(
                                                               mcpServerRow.modelData.authStatus,
                                                               Kirigami.Theme.neutralTextColor,
                                                               Kirigami.Theme.positiveTextColor)
                                                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                                                    elide: Text.ElideRight
                                                }

                                                ToolButton {
                                                    icon.name: "dialog-password"
                                                    visible: mcpServerRow.modelData.authStatus === "notLoggedIn"
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

                                        Kirigami.Separator { Layout.fillWidth: true }
                                    }
                                }
                            }
                        }

                        SectionFrame {
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
                            title: "Add server"
                            subtitle: "Register an MCP server for the active provider."

                            Kirigami.FormLayout {
                                Layout.fillWidth: true

                                TextField {
                                    id: mcpName
                                    Kirigami.FormData.label: "Name:"
                                    Layout.fillWidth: true
                                    placeholderText: "my-mcp-server"
                                    Accessible.name: "MCP server name"
                                }

                                ComboBox {
                                    id: mcpTransport
                                    Kirigami.FormData.label: "Transport:"
                                    Layout.fillWidth: true
                                    model: [
                                        { value: "http", label: "HTTP URL" },
                                        { value: "stdio", label: "Command" }
                                    ]
                                    textRole: "label"
                                    valueRole: "value"
                                    Accessible.name: "MCP server transport"
                                }

                                TextField {
                                    id: mcpTarget
                                    Kirigami.FormData.label: mcpTransport.currentValue === "http"
                                                            ? "URL:" : "Command:"
                                    Layout.fillWidth: true
                                    placeholderText: mcpTransport.currentValue === "http"
                                                     ? "https://mcp.example.com/mcp"
                                                     : "npx -y example-mcp-server"
                                    Accessible.name: mcpTransport.currentValue === "http"
                                                     ? "MCP server URL"
                                                     : "MCP server command"
                                }

                                TextField {
                                    id: mcpBearerToken
                                    Kirigami.FormData.label: "Bearer token:"
                                    Layout.fillWidth: true
                                    visible: mcpTransport.currentValue === "http"
                                    enabled: visible
                                    echoMode: TextInput.Password
                                    passwordMaskDelay: 0
                                    placeholderText: "Optional API key"
                                    Accessible.name: "MCP bearer token"
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
                                                    mcpTarget.text,
                                                    mcpTransport.currentValue === "http"
                                                    ? mcpBearerToken.text : "")
                                        mcpName.clear()
                                        mcpTarget.clear()
                                        mcpBearerToken.clear()
                                    }
                                }
                            }
                        }

                        Item {
                            Layout.fillHeight: true
                            Layout.minimumHeight: Kirigami.Units.largeSpacing
                        }
                    }
                }

                ScrollView {
                    id: applicationsPage
                    contentWidth: availableWidth
                    background: Rectangle {
                        color: Kirigami.Theme.backgroundColor
                    }

                    ColumnLayout {
                        width: applicationsPage.availableWidth
                        spacing: Kirigami.Units.largeSpacing

                        Item { Layout.preferredHeight: Kirigami.Units.smallSpacing }

                        SectionFrame {
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
                            title: "Open project resources"
                            subtitle: "Detected editors, IDEs, and terminal emulators."

                            Kirigami.FormLayout {
                                Layout.fillWidth: true

                                ComboBox {
                                    id: editorPicker
                                    Kirigami.FormData.label: "Repositories:"
                                    Layout.fillWidth: true
                                    model: root.controller.editorOptions
                                    textRole: "name"
                                    valueRole: "id"
                                    onActivated: root.clearConfirmation()
                                    Accessible.name: "Application for repositories"
                                }

                                ComboBox {
                                    id: terminalPicker
                                    Kirigami.FormData.label: "Terminals:"
                                    Layout.fillWidth: true
                                    model: root.controller.terminalOptions
                                    textRole: "name"
                                    valueRole: "id"
                                    onActivated: root.clearConfirmation()
                                    Accessible.name: "Application for terminals"
                                }
                            }

                            MessageStrip {
                                iconName: "dialog-information"
                                text: "Available editors, IDEs, and terminal emulators are detected from installed desktop applications."
                                accentColor: Kirigami.Theme.highlightColor
                                useSmallFont: true
                            }
                        }

                        Item {
                            Layout.fillHeight: true
                            Layout.minimumHeight: Kirigami.Units.largeSpacing
                        }
                    }
                }
                }
            }
        }

        Kirigami.Separator { Layout.fillWidth: true }

        Pane {
            Layout.fillWidth: true
            padding: Kirigami.Units.smallSpacing

            RowLayout {
                anchors.fill: parent
                spacing: Kirigami.Units.smallSpacing

                RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    opacity: root.settingsApplied ? 1 : 0

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

                Item { Layout.fillWidth: true }

                Button {
                    text: "OK"
                    icon.name: "dialog-ok"
                    onClicked: {
                        root.applyValues()
                        if (root.settingsApplied)
                            root.close()
                    }
                }

                Button {
                    text: "Apply"
                    icon.name: "dialog-ok-apply"
                    onClicked: root.applyValues()
                }

                Button {
                    text: "Cancel"
                    icon.name: "dialog-cancel"
                    onClicked: root.close()
                }
            }
        }
    }

}
