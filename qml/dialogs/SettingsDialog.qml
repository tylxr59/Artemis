import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import org.kde.kirigami as Kirigami

Window {
    id: root
    required property var controller
    property bool settingsApplied: false
    title: "Settings - Artemis"
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
            return "External context servers available to Codex."
        return "Desktop applications used for project actions."
    }

    function pageIcon(index) {
        if (index === 0)
            return "applications-development"
        if (index === 1)
            return "network-connect"
        return "applications-utilities"
    }

    function selectedCodingModel() {
        if (codingModel.currentIndex < 0
                || codingModel.currentIndex >= controller.models.length)
            return null
        return controller.models[codingModel.currentIndex]
    }

    function reasoningOptions() {
        const selected = selectedCodingModel()
        if (!selected)
            return []
        const defaultLabel = selected.defaultEffort
                ? "Default (" + selected.defaultEffort + ")" : "Model default"
        const options = [{ value: "", label: defaultLabel }]
        const efforts = Array.from(selected.efforts || [])
        for (let i = 0; i < efforts.length; ++i) {
            options.push({
                             value: efforts[i],
                             label: efforts[i].charAt(0).toUpperCase()
                                    + efforts[i].slice(1)
                         })
        }
        return options
    }

    function reasoningIndex(reasoningEffort) {
        const options = reasoningOptions()
        for (let i = 0; i < options.length; ++i) {
            if (options[i].value === reasoningEffort)
                return i
        }
        return 0
    }

    function reasoningSupported(reasoningEffort) {
        const options = reasoningOptions()
        for (let i = 0; i < options.length; ++i) {
            if (options[i].value === reasoningEffort)
                return true
        }
        return false
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

    function loadValues() {
        codingModel.currentIndex = modelIndex(controller.codingModelId)
        reasoningPicker.currentIndex = reasoningIndex(controller.codingReasoningEffort)
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
                Layout.preferredWidth: 210
                padding: Kirigami.Units.smallSpacing

                ColumnLayout {
                    anchors.fill: parent
                    spacing: Kirigami.Units.smallSpacing

                    Label {
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.smallSpacing
                        Layout.rightMargin: Kirigami.Units.smallSpacing
                        Layout.topMargin: Kirigami.Units.smallSpacing
                        text: "Settings"
                        font.bold: true
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize + 2
                        elide: Text.ElideRight
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.smallSpacing
                        Layout.rightMargin: Kirigami.Units.smallSpacing
                        text: "Configure Artemis"
                        opacity: 0.62
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                        elide: Text.ElideRight
                    }

                    Kirigami.Separator {
                        Layout.fillWidth: true
                        Layout.topMargin: Kirigami.Units.smallSpacing
                    }

                    ListView {
                        id: settingsNavigation
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        currentIndex: 0
                        spacing: 2
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
                            height: 54
                            highlighted: ListView.isCurrentItem
                            leftPadding: Kirigami.Units.smallSpacing
                            rightPadding: Kirigami.Units.smallSpacing
                            topPadding: 0
                            bottomPadding: 0
                            onClicked: settingsNavigation.currentIndex = index

                            background: Rectangle {
                                radius: Kirigami.Units.smallSpacing
                                color: navigationDelegate.highlighted
                                       ? Qt.alpha(Kirigami.Theme.highlightColor, 0.17)
                                       : navigationDelegate.hovered
                                         ? Qt.alpha(Kirigami.Theme.textColor, 0.05)
                                         : "transparent"
                            }

                            contentItem: RowLayout {
                                spacing: Kirigami.Units.smallSpacing

                                Kirigami.Icon {
                                    source: navigationDelegate.iconName
                                    color: navigationDelegate.highlighted
                                           ? Kirigami.Theme.highlightColor
                                           : Kirigami.Theme.textColor
                                    opacity: navigationDelegate.highlighted ? 1 : 0.72
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
                                        opacity: 0.62
                                        elide: Text.ElideRight
                                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                                    }
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
                    Layout.preferredHeight: 64

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: Kirigami.Units.largeSpacing
                        anchors.rightMargin: Kirigami.Units.smallSpacing
                        spacing: Kirigami.Units.smallSpacing

                        Kirigami.Icon {
                            source: root.pageIcon(settingsNavigation.currentIndex)
                            Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                            color: Kirigami.Theme.highlightColor
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 0

                            Label {
                                Layout.fillWidth: true
                                text: root.pageTitle(settingsNavigation.currentIndex)
                                font.bold: true
                                font.pointSize: Kirigami.Theme.defaultFont.pointSize + 2
                                elide: Text.ElideRight
                            }

                            Label {
                                Layout.fillWidth: true
                                text: root.pageDescription(settingsNavigation.currentIndex)
                                opacity: 0.64
                                elide: Text.ElideRight
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                            }
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
                            subtitle: "These choices are used when Artemis starts a new Codex task."

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
                                        if (!root.reasoningSupported(reasoningPicker.currentValue || ""))
                                            reasoningPicker.currentIndex = 0
                                    }
                                    Accessible.name: "Model for new threads"
                                }

                                ComboBox {
                                    id: reasoningPicker
                                    Kirigami.FormData.label: "Reasoning effort:"
                                    Layout.fillWidth: true
                                    model: root.reasoningOptions()
                                    textRole: "label"
                                    valueRole: "value"
                                    enabled: count > 1
                                    currentIndex: root.reasoningIndex(
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

                                            contentItem: RowLayout {
                                                spacing: Kirigami.Units.smallSpacing

                                                Kirigami.Icon {
                                                    source: mcpServerRow.modelData.authStatus === "notLoggedIn"
                                                            ? "emblem-warning" : "network-server"
                                                    color: root.authColor(mcpServerRow.modelData.authStatus)
                                                    Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                                                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                                                }

                                                ColumnLayout {
                                                    Layout.fillWidth: true
                                                    spacing: 1

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
                                                              + " - "
                                                              + mcpServerRow.modelData.toolCount
                                                              + " tools - "
                                                              + mcpServerRow.modelData.resourceCount
                                                              + " resources"
                                                        textFormat: Text.PlainText
                                                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                                                        opacity: 0.62
                                                        elide: Text.ElideRight
                                                    }
                                                }

                                                Kirigami.Icon {
                                                    source: mcpServerRow.modelData.authStatus === "notLoggedIn"
                                                            ? "dialog-warning" : "dialog-ok-apply"
                                                    color: root.authColor(mcpServerRow.modelData.authStatus)
                                                    Layout.preferredWidth: Kirigami.Units.iconSizes.small
                                                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                                                }

                                                Label {
                                                    text: root.authText(mcpServerRow.modelData.authStatus)
                                                    color: root.authColor(mcpServerRow.modelData.authStatus)
                                                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                                                    elide: Text.ElideRight
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

                                        Kirigami.Separator { Layout.fillWidth: true }
                                    }
                                }
                            }
                        }

                        SectionFrame {
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                            Layout.rightMargin: Kirigami.Units.largeSpacing
                            title: "Add server"
                            subtitle: "Register an HTTP endpoint or stdio command."

                            Kirigami.FormLayout {
                                Layout.fillWidth: true

                                TextField {
                                    id: mcpName
                                    Kirigami.FormData.label: "Name:"
                                    Layout.fillWidth: true
                                    placeholderText: "context7"
                                    Accessible.name: "MCP server name"
                                }

                                ComboBox {
                                    id: mcpTransport
                                    Kirigami.FormData.label: "Transport:"
                                    Layout.fillWidth: true
                                    model: [
                                        { value: "http", label: "HTTP URL" },
                                        { value: "stdio", label: "Stdio command" }
                                    ]
                                    textRole: "label"
                                    valueRole: "value"
                                    Accessible.name: "MCP server transport"
                                }

                                TextField {
                                    id: mcpTarget
                                    Kirigami.FormData.label: "Target:"
                                    Layout.fillWidth: true
                                    placeholderText: mcpTransport.currentValue === "http"
                                                     ? "https://example.com/mcp"
                                                     : "npx -y @upstash/context7-mcp"
                                    Accessible.name: "MCP server target"
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
                font.pointSize: Kirigami.Theme.smallFont.pointSize
            }
        }
    }
}
