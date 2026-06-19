import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.artemis

Kirigami.ApplicationWindow {
    id: root
    width: 1500
    height: 900
    readonly property real conversationMinimumWidth: 760
    readonly property real navigationMinimumWidth: 220
    readonly property real threadPanelMinimumWidth: 300
    readonly property real splitHandleWidth: 5
    minimumWidth: conversationMinimumWidth
                  + (sidePanelVisible
                     ? threadPanelMinimumWidth + splitHandleWidth : 0)
    minimumHeight: 560
    visible: true
    title: appController.selectedThreadTitle.length > 0
           ? appController.selectedThreadTitle + " — Artemis" : "Artemis"

    property bool sidePanelVisible: false
    property string sidePanelMode: "thread"
    property bool navigationVisible: width >= conversationMinimumWidth
                                     + navigationMinimumWidth
                                     + splitHandleWidth
                                     + (sidePanelVisible
                                        ? threadPanelMinimumWidth
                                          + splitHandleWidth : 0)
    readonly property bool hasProject: appController.selectedProjectPath.length > 0
    readonly property bool hasThread: appController.selectedThreadId.length > 0
    property var composerImages: []

    function modelIndex(modelId) {
        const exact = modelPicker.indexOfValue(modelId)
        if (exact >= 0)
            return exact
        for (let i = 0; i < appController.models.length; ++i) {
            if (appController.models[i].isDefault)
                return i
        }
        return appController.models.length > 0 ? 0 : -1
    }

    function selectedModel() {
        if (modelPicker.currentIndex < 0
                || modelPicker.currentIndex >= appController.models.length)
            return null
        return appController.models[modelPicker.currentIndex]
    }

    function reasoningOptions() {
        const selected = selectedModel()
        if (!selected)
            return []
        const defaultLabel = selected.defaultEffort
                ? "Default (" + selected.defaultEffort + ")" : "Model default"
        const options = [{ value: "", label: defaultLabel }]
        const efforts = Array.from(selected.efforts || [])
        for (let i = 0; i < efforts.length; ++i)
            options.push({ value: efforts[i],
                           label: efforts[i].charAt(0).toUpperCase() + efforts[i].slice(1) })
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

    function localImageUrl(path) {
        return path.length > 0 ? "file://" + encodeURI(path) : ""
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

    CommitDialog {
        id: commitDialog
        controller: appController
    }

    SettingsDialog {
        id: settingsDialog
        controller: appController
    }

    Connections {
        target: appController
        function onPromptRestoreRequested(text, images) {
            if (composer.text.length === 0)
                composer.text = text
            const restored = Array.from(images || [])
            for (let i = 0; i < restored.length; ++i) {
                if (root.composerImages.indexOf(restored[i]) < 0)
                    root.composerImages = root.composerImages.concat([restored[i]])
            }
        }
        function onStatusMessage(text) {
            statusToast.showMessage(text)
        }
        function onTaskPanelRequested() {
            root.sidePanelMode = "thread"
            root.sidePanelVisible = true
        }
    }

    Popup {
        id: statusToast
        parent: Overlay.overlay
        property string message: ""

        function showMessage(text) {
            if (text.length === 0)
                return
            message = text
            open()
            dismissTimer.restart()
        }

        x: Overlay.overlay.width - width - Kirigami.Units.largeSpacing
        y: appHeader.height + Kirigami.Units.largeSpacing
        width: Math.min(360, Overlay.overlay.width
                             - Kirigami.Units.largeSpacing * 2)
        padding: Kirigami.Units.smallSpacing
        modal: false
        focus: false
        closePolicy: Popup.NoAutoClose

        background: Rectangle {
            radius: Kirigami.Units.smallSpacing
            color: Kirigami.Theme.backgroundColor
            border.color: Qt.alpha(Kirigami.Theme.textColor, 0.28)
        }

        contentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Label {
                text: statusToast.message
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                maximumLineCount: 4
                elide: Text.ElideRight
            }
            ToolButton {
                icon.name: "edit-copy"
                Accessible.name: "Copy notification"
                ToolTip.text: Accessible.name
                ToolTip.visible: hovered
                onClicked: appController.copyText(statusToast.message)
            }
        }

        Timer {
            id: dismissTimer
            interval: 5000
            onTriggered: statusToast.close()
        }

        Component.onCompleted: {
            if (appController.statusText.length > 0)
                showMessage(appController.statusText)
        }
    }

    header: ToolBar {
        id: appHeader
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Kirigami.Units.smallSpacing
            anchors.rightMargin: Kirigami.Units.smallSpacing

            ToolButton {
                visible: !root.navigationVisible
                text: "☰"
                Accessible.name: "Projects"
                onClicked: navigationDrawer.open()
            }
            Label {
                text: appController.selectedThreadTitle.length > 0
                      ? appController.selectedThreadTitle : "Artemis"
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
            Rectangle {
                Layout.preferredWidth: connectionRow.implicitWidth + Kirigami.Units.largeSpacing
                Layout.preferredHeight: connectionRow.implicitHeight + Kirigami.Units.smallSpacing
                radius: height / 2
                color: Qt.alpha(appController.providerReady
                                ? Kirigami.Theme.positiveTextColor
                                : Kirigami.Theme.negativeTextColor, 0.12)
                RowLayout {
                    id: connectionRow
                    anchors.centerIn: parent
                    spacing: Kirigami.Units.smallSpacing
                    Rectangle {
                        Layout.preferredWidth: 7
                        Layout.preferredHeight: 7
                        radius: 4
                        color: appController.providerReady
                               ? Kirigami.Theme.positiveTextColor
                               : Kirigami.Theme.negativeTextColor
                    }
                    Label {
                        text: appController.providerReady ? "Codex connected" : "Codex offline"
                        color: appController.providerReady
                               ? Kirigami.Theme.positiveTextColor
                               : Kirigami.Theme.negativeTextColor
                    }
                }
            }
            ToolButton {
                text: "Open folder"
                visible: root.hasProject
                onClicked: appController.openProjectFolder()
            }
            ToolButton {
                text: "Open editor"
                visible: root.hasProject
                onClicked: appController.openProjectEditor()
            }
            ToolButton {
                text: "Thread"
                visible: root.hasThread
                checkable: true
                checked: root.sidePanelVisible && root.sidePanelMode === "thread"
                onClicked: {
                    if (checked) {
                        root.sidePanelMode = "thread"
                        root.sidePanelVisible = true
                    } else {
                        root.sidePanelVisible = false
                    }
                }
            }
            ToolButton {
                text: "Diff"
                visible: appController.selectedProjectIsGit
                enabled: appController.hasGitChanges
                checkable: true
                checked: root.sidePanelVisible && root.sidePanelMode === "diff"
                onClicked: {
                    if (checked) {
                        root.sidePanelMode = "diff"
                        root.sidePanelVisible = true
                        appController.refreshGit()
                    } else {
                        root.sidePanelVisible = false
                    }
                }
            }
            Button {
                text: "Commit && Push"
                visible: appController.selectedProjectIsGit
                enabled: appController.hasGitChanges
                onClicked: {
                    commitDialog.featureMode = false
                    commitDialog.open()
                }
            }
            ToolButton {
                id: commitMenuButton
                text: "More"
                visible: appController.selectedProjectIsGit
                enabled: appController.hasGitChanges
                onClicked: commitMenu.open()
                Accessible.name: "More commit options"
                Menu {
                    id: commitMenu
                    y: commitMenuButton.height
                    MenuItem {
                        text: "Commit and push all changes"
                        onTriggered: {
                            commitDialog.featureMode = false
                            commitDialog.open()
                        }
                    }
                    MenuItem {
                        text: "Review and select changes"
                        onTriggered: {
                            root.sidePanelMode = "diff"
                            root.sidePanelVisible = true
                            commitDialog.featureMode = false
                            commitDialog.open()
                        }
                    }
                    MenuItem {
                        text: "Commit to feature branch"
                        onTriggered: {
                            commitDialog.featureMode = true
                            commitDialog.open()
                        }
                    }
                }
            }
        }
    }

    Drawer {
        id: navigationDrawer
        width: Math.min(root.width * 0.82, 340)
        height: root.height
        edge: Qt.LeftEdge
        modal: true
        contentItem: navigationPane
    }

    SplitView {
        id: workspaceSplit
        anchors.fill: parent
        orientation: Qt.Horizontal

        handle: Rectangle {
            implicitWidth: root.splitHandleWidth
            color: SplitHandle.pressed
                   ? Kirigami.Theme.highlightColor
                   : SplitHandle.hovered
                     ? Qt.alpha(Kirigami.Theme.highlightColor, 0.55)
                     : Qt.alpha(Kirigami.Theme.textColor, 0.18)

            Rectangle {
                anchors.centerIn: parent
                width: 1
                height: parent.height
                color: Kirigami.Theme.textColor
                opacity: 0.28
            }
        }

        Pane {
            id: navigationPane
            visible: root.navigationVisible || navigationDrawer.visible
            SplitView.preferredWidth: 310
            SplitView.minimumWidth: root.navigationVisible
                                    ? root.navigationMinimumWidth : 0
            SplitView.maximumWidth: root.navigationVisible ? 480 : 0
            padding: Kirigami.Units.largeSpacing
            clip: true

            ColumnLayout {
                anchors.fill: parent
                spacing: Kirigami.Units.largeSpacing

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: "Projects"
                        font.bold: true
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                        Layout.fillWidth: true
                    }
                    ToolButton {
                        icon.name: "folder-new"
                        onClicked: appController.chooseProjectFolder()
                        Accessible.name: "Add project folder"
                        ToolTip.text: Accessible.name
                        ToolTip.visible: hovered
                    }
                }
                TextField {
                    id: projectSearch
                    Layout.fillWidth: true
                    placeholderText: "Search projects"
                    visible: projectList.count > 4
                }
                ListView {
                    id: projectList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: appController.projects
                    spacing: Kirigami.Units.smallSpacing
                    delegate: ColumnLayout {
                        id: projectDelegate
                        required property int index
                        required property string name
                        required property string path
                        required property bool isGit
                        readonly property bool selected: index === appController.selectedProjectIndex
                        width: projectList.width
                        spacing: 2
                        visible: projectSearch.text.length === 0
                                 || name.toLowerCase().includes(projectSearch.text.toLowerCase())

                        ItemDelegate {
                            id: projectItem
                            Layout.fillWidth: true
                            Layout.maximumWidth: projectList.width
                            highlighted: projectDelegate.selected
                            leftPadding: Kirigami.Units.smallSpacing
                            rightPadding: Kirigami.Units.smallSpacing
                            contentItem: RowLayout {
                                spacing: Kirigami.Units.smallSpacing
                                Kirigami.Icon {
                                    source: projectDelegate.selected
                                            ? "arrow-down" : "arrow-right"
                                    Layout.preferredWidth: 14
                                    Layout.preferredHeight: 14
                                    opacity: 0.65
                                }
                                Kirigami.Icon {
                                    source: "folder"
                                    Layout.preferredWidth: 18
                                    Layout.preferredHeight: 18
                                }
                                Label {
                                    text: projectDelegate.name
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                    font.bold: projectDelegate.selected
                                }
                                ToolButton {
                                    text: "+"
                                    visible: projectDelegate.selected
                                    enabled: appController.providerReady
                                    Accessible.name: "New thread in " + projectDelegate.name
                                    ToolTip.text: Accessible.name
                                    ToolTip.visible: hovered
                                    onClicked: {
                                        if (!projectDelegate.selected)
                                            appController.selectProject(projectDelegate.index)
                                        appController.createThread(
                                            appController.codingModelId,
                                            appController.codingReasoningEffort,
                                            permissionPicker.currentValue)
                                    }
                                }
                            }
                            ToolTip.text: projectDelegate.path
                            ToolTip.visible: hovered
                            onClicked: appController.selectProject(projectDelegate.index)

                            TapHandler {
                                acceptedButtons: Qt.RightButton
                                onTapped: projectContextMenu.popup()
                            }
                            Menu {
                                id: projectContextMenu
                                MenuItem {
                                    text: "Remove project from Artemis"
                                    icon.name: "edit-delete"
                                    onTriggered: appController.removeProject(projectDelegate.index)
                                }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            visible: projectDelegate.selected
                            spacing: 2

                            Repeater {
                                model: projectDelegate.selected ? appController.threads : []
                                delegate: ItemDelegate {
                                    id: threadItem
                                    required property int index
                                    required property var modelData
                                    Layout.fillWidth: true
                                    Layout.maximumWidth: projectList.width
                                    leftPadding: Kirigami.Units.gridUnit * 2
                                    highlighted: appController.selectedThreadId === modelData.id
                                    contentItem: RowLayout {
                                        spacing: Kirigami.Units.smallSpacing
                                        Kirigami.Icon {
                                            source: "dialog-messages"
                                            Layout.preferredWidth: 16
                                            Layout.preferredHeight: 16
                                            opacity: 0.7
                                        }
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 1
                                            Label {
                                                text: modelData.title
                                                Layout.fillWidth: true
                                                elide: Text.ElideRight
                                            }
                                            Label {
                                                visible: modelData.external
                                                text: "External"
                                                font: Kirigami.Theme.smallFont
                                                opacity: 0.55
                                            }
                                        }
                                    }
                                    onClicked: {
                                        appController.selectThread(index)
                                        if (!root.navigationVisible)
                                            navigationDrawer.close()
                                    }

                                    TapHandler {
                                        acceptedButtons: Qt.RightButton
                                        onTapped: threadContextMenu.popup()
                                    }
                                    Menu {
                                        id: threadContextMenu
                                        MenuItem {
                                            text: "Remove thread from Artemis"
                                            icon.name: "edit-delete"
                                            onTriggered: appController.removeThread(index)
                                        }
                                    }
                                }
                            }

                            Label {
                                Layout.fillWidth: true
                                Layout.leftMargin: Kirigami.Units.gridUnit * 2
                                Layout.rightMargin: Kirigami.Units.smallSpacing
                                Layout.topMargin: Kirigami.Units.smallSpacing
                                Layout.bottomMargin: Kirigami.Units.smallSpacing
                                visible: appController.threads.length === 0
                                text: appController.providerReady
                                      ? "No chats yet. Send a message to start one."
                                      : "Codex is offline."
                                wrapMode: Text.Wrap
                                opacity: 0.55
                                font: Kirigami.Theme.smallFont
                            }
                        }
                    }
                    ColumnLayout {
                        anchors.centerIn: parent
                        width: parent.width
                        visible: projectList.count === 0
                        spacing: Kirigami.Units.smallSpacing
                        Label {
                            text: "No projects added"
                            Layout.alignment: Qt.AlignHCenter
                            font.bold: true
                        }
                        Label {
                            text: "Add a folder to start working with Codex."
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.Wrap
                            opacity: 0.65
                        }
                    }
                }
                Kirigami.Separator { Layout.fillWidth: true }
                RowLayout {
                    Layout.fillWidth: true
                    ToolButton {
                        icon.name: "utilities-terminal"
                        enabled: root.hasProject
                        onClicked: appController.openTerminal()
                        Accessible.name: "Open terminal"
                        ToolTip.text: Accessible.name
                        ToolTip.visible: hovered
                    }
                    Item { Layout.fillWidth: true }
                    ToolButton {
                        icon.name: "settings-configure"
                        onClicked: settingsDialog.open()
                        Accessible.name: "Settings"
                        ToolTip.text: Accessible.name
                        ToolTip.visible: hovered
                    }
                    ToolButton {
                        icon.name: "tools-report-bug"
                        onClicked: diagnosticsDialog.open()
                        Accessible.name: "Diagnostics"
                        ToolTip.text: Accessible.name
                        ToolTip.visible: hovered
                    }
                }
            }
        }

        Pane {
            id: conversationPane
            SplitView.fillWidth: true
            SplitView.minimumWidth: root.conversationMinimumWidth
            padding: 0

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                ListView {
                    id: conversationList
                    property bool followTail: true
                    property bool tailScrollPending: false
                    readonly property real contentViewportWidth: Math.max(
                        0, width - (conversationScrollBar.visible
                                    ? conversationScrollBar.width
                                      + Kirigami.Units.smallSpacing : 0))

                    function updateFollowTail() {
                        const distanceFromEnd = contentHeight - height - contentY
                        followTail = distanceFromEnd <= Kirigami.Units.gridUnit * 2
                    }

                    function scrollToEndIfFollowing() {
                        if (!followTail || conversationScrollBar.pressed
                                || tailScrollPending)
                            return
                        tailScrollPending = true
                        Qt.callLater(function() {
                            conversationList.tailScrollPending = false
                            if (conversationList.followTail
                                    && !conversationScrollBar.pressed)
                                conversationList.positionViewAtEnd()
                        })
                    }

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: appController.conversation
                    clip: true
                    ScrollBar.vertical: ScrollBar {
                        id: conversationScrollBar
                        policy: ScrollBar.AsNeeded

                        onPressedChanged: {
                            if (pressed)
                                conversationList.followTail = false
                            else
                                conversationList.updateFollowTail()
                        }
                    }
                    spacing: Kirigami.Units.largeSpacing
                    topMargin: Kirigami.Units.largeSpacing
                    bottomMargin: Kirigami.Units.largeSpacing
                    footer: Item {
                        width: conversationList.contentViewportWidth
                        implicitHeight: workingStatus.visible
                                        ? workingStatus.implicitHeight : 0

                        Label {
                            id: workingStatus
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: Math.min(840, parent.width)
                            visible: appController.turnRunning
                            horizontalAlignment: Text.AlignHCenter
                            text: "Working · " + appController.turnElapsedText + " elapsed"
                            font: Kirigami.Theme.smallFont
                            opacity: 0.45
                            topPadding: Kirigami.Units.smallSpacing
                            bottomPadding: Kirigami.Units.smallSpacing
                        }
                    }
                    delegate: Item {
                        id: conversationRow
                        required property int index
                        required property string eventType
                        required property string title
                        required property string content
                        required property var metadata

                        readonly property real horizontalGutter: Kirigami.Units.largeSpacing
                        width: conversationList.contentViewportWidth
                        implicitHeight: conversationDelegate.implicitHeight
                        onImplicitHeightChanged: {
                            if (index === conversationList.count - 1)
                                conversationList.scrollToEndIfFollowing()
                        }

                        ConversationDelegate {
                            id: conversationDelegate
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: Math.min(840, parent.width
                                                 - conversationRow.horizontalGutter * 2)
                            eventType: conversationRow.eventType
                            title: conversationRow.title
                            content: conversationRow.content
                            metadata: conversationRow.metadata
                            onImageOpenRequested: path => imageViewer.showImage(path)
                        }
                    }
                    onCountChanged: scrollToEndIfFollowing()
                    onDraggingChanged: {
                        if (dragging)
                            followTail = false
                    }
                    onMovementEnded: updateFollowTail()
                    onWidthChanged: forceLayout()

                    WheelHandler {
                        target: null
                        onWheel: function(event) {
                            conversationList.followTail = false
                        }
                    }

                    Connections {
                        target: appController
                        function onTurnRunningChanged() {
                            if (appController.turnRunning) {
                                conversationList.followTail = true
                                conversationList.scrollToEndIfFollowing()
                            }
                        }
                    }

                    Kirigami.PlaceholderMessage {
                        anchors.centerIn: parent
                        width: Math.min(parent.width - 40, 480)
                        visible: conversationList.count === 0
                        text: !root.hasProject ? "Start with a project"
                              : !appController.providerReady ? "Codex is offline"
                              : "What would you like to build?"
                        explanation: !root.hasProject
                                     ? "Add a project folder to give Artemis a workspace."
                                     : !appController.providerReady
                                       ? "Open Diagnostics to inspect the Codex connection."
                                       : appController.selectedProjectIsGit
                                         ? "Describe a task below. Use Diff to review file changes."
                                         : "Describe a task below. Git review is unavailable for this folder."
                    }

                    Button {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.verticalCenter
                        anchors.topMargin: Kirigami.Units.gridUnit * 4
                        visible: conversationList.count === 0 && !root.hasProject
                        text: "Add project folder"
                        icon.name: "folder-new"
                        onClicked: appController.chooseProjectFolder()
                    }
                }

                Kirigami.Separator {
                    visible: root.hasProject && appController.providerReady
                    Layout.fillWidth: true
                }

                Frame {
                    id: composerFrame
                    visible: root.hasProject && appController.providerReady
                    Layout.fillWidth: true
                    Layout.maximumWidth: 840
                    Layout.alignment: Qt.AlignHCenter
                    Layout.margins: Kirigami.Units.largeSpacing
                    padding: Kirigami.Units.smallSpacing

                    ColumnLayout {
                        anchors.fill: parent
                        Flickable {
                            Layout.fillWidth: true
                            Layout.preferredHeight: root.composerImages.length > 0 ? 92 : 0
                            visible: root.composerImages.length > 0
                            contentWidth: attachmentRow.implicitWidth
                            contentHeight: height
                            clip: true

                            Row {
                                id: attachmentRow
                                height: parent.height
                                spacing: Kirigami.Units.smallSpacing

                                Repeater {
                                    model: root.composerImages
                                    delegate: Frame {
                                        required property string modelData
                                        required property int index
                                        width: 112
                                        height: 88
                                        padding: 3

                                        Image {
                                            anchors.fill: parent
                                            source: root.localImageUrl(modelData)
                                            fillMode: Image.PreserveAspectCrop
                                            asynchronous: true
                                            cache: false
                                        }
                                        RoundButton {
                                            anchors.top: parent.top
                                            anchors.right: parent.right
                                            anchors.margins: 3
                                            width: 26
                                            height: 26
                                            text: "×"
                                            Accessible.name: "Remove image"
                                            onClicked: {
                                                const next = root.composerImages.slice()
                                                next.splice(index, 1)
                                                root.composerImages = next
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        TextArea {
                            id: composer
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.max(90, implicitHeight)
                            Layout.maximumHeight: 260
                            placeholderText: appController.turnRunning
                                             ? "Add guidance while Artemis is working…"
                                             : "Describe a task, ask a question, or paste an error…"
                            wrapMode: TextEdit.Wrap
                            enabled: root.hasProject && appController.providerReady
                            Keys.onPressed: event => {
                                if (event.matches(StandardKey.Paste)) {
                                    const imagePath = appController.pasteClipboardImage()
                                    if (imagePath.length > 0) {
                                        root.composerImages =
                                                root.composerImages.concat([imagePath])
                                        event.accepted = true
                                        return
                                    }
                                }
                                if (!(event.modifiers & Qt.ShiftModifier)
                                        && (event.key === Qt.Key_Return || event.key === Qt.Key_Enter)) {
                                    sendButton.clicked()
                                    event.accepted = true
                                }
                            }
                        }
                        GridLayout {
                            id: composerControls
                            readonly property bool compact: conversationPane.width < 800
                            Layout.fillWidth: true
                            Layout.minimumWidth: 0
                            columns: compact ? 2 : 7

                            ComboBox {
                                id: modelPicker
                                Layout.row: 0
                                Layout.column: 0
                                Layout.fillWidth: composerControls.compact
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: composerControls.compact ? 0 : 180
                                model: appController.models
                                textRole: "name"
                                valueRole: "id"
                                currentIndex: root.modelIndex(appController.codingModelId)
                                enabled: !appController.turnRunning
                                onActivated: {
                                    appController.codingModelId = currentValue || ""
                                    const options = root.reasoningOptions()
                                    let supported = false
                                    for (let i = 0; i < options.length; ++i) {
                                        if (options[i].value
                                                === appController.codingReasoningEffort) {
                                            supported = true
                                            break
                                        }
                                    }
                                    if (!supported)
                                        appController.codingReasoningEffort = ""
                                }
                            }
                            ComboBox {
                                id: reasoningPicker
                                Layout.row: 0
                                Layout.column: 1
                                Layout.fillWidth: composerControls.compact
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: composerControls.compact ? 0 : 150
                                model: root.reasoningOptions()
                                textRole: "label"
                                valueRole: "value"
                                currentIndex: root.reasoningIndex(
                                                  appController.codingReasoningEffort)
                                enabled: !appController.turnRunning && count > 1
                                onActivated: appController.codingReasoningEffort =
                                             currentValue || ""
                                ToolTip.text: "Reasoning effort for new threads"
                                ToolTip.visible: hovered
                            }
                            ComboBox {
                                id: permissionPicker
                                Layout.row: composerControls.compact ? 1 : 0
                                Layout.column: composerControls.compact ? 0 : 2
                                Layout.fillWidth: composerControls.compact
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: composerControls.compact ? 0 : 170
                                model: [
                                    { value: "approval-required", label: "Supervised" },
                                    { value: "auto-accept-edits", label: "Auto-accept edits" },
                                    { value: "full-access", label: "Full access" }
                                ]
                                textRole: "label"
                                valueRole: "value"
                                currentIndex: 2
                                enabled: !appController.turnRunning
                                ToolTip.text: currentValue === "approval-required"
                                              ? "Ask before commands and file changes."
                                              : currentValue === "auto-accept-edits"
                                                ? "Auto-approve edits, ask before other actions."
                                                : "Allow commands and edits without prompts."
                                ToolTip.visible: hovered
                            }
                            Button {
                                id: collaborationModeButton
                                Layout.row: composerControls.compact ? 1 : 0
                                Layout.column: composerControls.compact ? 1 : 3
                                Layout.fillWidth: composerControls.compact
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: composerControls.compact ? 0 : 92
                                text: checked ? "Plan" : "Build"
                                checkable: true
                                enabled: !appController.turnRunning
                                Accessible.name: checked ? "Plan mode" : "Build mode"
                                ToolTip.text: checked
                                              ? "Plan the work without making changes."
                                              : "Work on the task and make changes."
                                ToolTip.visible: hovered
                            }
                            Item {
                                visible: !composerControls.compact
                                Layout.row: 0
                                Layout.column: 4
                                Layout.fillWidth: true
                            }
                            Item {
                                id: contextUsageIndicator
                                Layout.row: composerControls.compact ? 2 : 0
                                Layout.column: composerControls.compact ? 0 : 5
                                Layout.preferredWidth: 36
                                Layout.preferredHeight: 36
                                Layout.alignment: composerControls.compact
                                                  ? Qt.AlignLeft : Qt.AlignCenter
                                visible: appController.hasTokenUsage

                                Canvas {
                                    id: contextUsageRing
                                    anchors.fill: parent
                                    anchors.margins: 3
                                    onPaint: {
                                        const context = getContext("2d")
                                        const center = width / 2
                                        const radius = Math.min(width, height) / 2 - 2
                                        if (radius <= 0)
                                            return
                                        const start = -Math.PI / 2
                                        const progress = Math.min(
                                            1, appController.contextUsagePercent / 100)
                                        context.clearRect(0, 0, width, height)
                                        context.lineWidth = 3
                                        context.lineCap = "round"
                                        context.strokeStyle = Qt.alpha(
                                            Kirigami.Theme.textColor, 0.18)
                                        context.beginPath()
                                        context.arc(center, center, radius, 0, Math.PI * 2)
                                        context.stroke()
                                        context.strokeStyle =
                                            appController.contextUsagePercent >= 90
                                            ? Kirigami.Theme.negativeTextColor
                                            : appController.contextUsagePercent >= 75
                                              ? Kirigami.Theme.neutralTextColor
                                              : Kirigami.Theme.highlightColor
                                        context.beginPath()
                                        context.arc(center, center, radius, start,
                                                    start + Math.PI * 2 * progress)
                                        context.stroke()
                                    }

                                    Connections {
                                        target: appController
                                        function onTokenUsageChanged() {
                                            contextUsageRing.requestPaint()
                                        }
                                    }
                                }
                                Label {
                                    anchors.centerIn: parent
                                    text: appController.contextUsagePercent
                                    font.pixelSize: 9
                                    font.bold: true
                                }
                                HoverHandler {
                                    id: contextUsageHover
                                }
                                ToolTip {
                                    id: contextUsagePopup
                                    visible: contextUsageHover.hovered
                                    delay: 300
                                    timeout: -1
                                    x: contextUsageIndicator.width - width
                                    y: -height - Kirigami.Units.smallSpacing
                                    padding: Kirigami.Units.largeSpacing

                                    contentItem: ColumnLayout {
                                        spacing: Kirigami.Units.smallSpacing
                                        Label {
                                            text: "CONTEXT WINDOW"
                                            opacity: 0.65
                                            font.pixelSize: 10
                                            font.letterSpacing: 1
                                        }
                                        Label {
                                            text: appController.contextUsagePercent + "% · "
                                                  + root.compactTokenCount(
                                                      appController.contextTokens)
                                                  + "/"
                                                  + root.compactTokenCount(
                                                      appController.modelContextWindow)
                                                  + " context used"
                                            font.bold: true
                                        }
                                        Label {
                                            text: "Total processed: "
                                                  + root.compactTokenCount(
                                                      appController.totalProcessedTokens)
                                                  + " tokens"
                                            opacity: 0.65
                                        }
                                        Label {
                                            text: "Automatically compacts its context when needed."
                                            opacity: 0.65
                                        }
                                    }
                                }
                            }
                            RoundButton {
                                id: sendButton
                                Layout.row: composerControls.compact ? 2 : 0
                                Layout.column: composerControls.compact ? 1 : 6
                                Layout.alignment: composerControls.compact
                                                  ? Qt.AlignRight : Qt.AlignCenter
                                text: appController.turnRunning ? "■" : "↑"
                                ToolTip.text: appController.turnRunning
                                              ? "Stop" : "Send (Enter)"
                                ToolTip.visible: hovered
                                enabled: appController.turnRunning
                                         || ((composer.text.trim().length > 0
                                              || root.composerImages.length > 0)
                                             && root.hasProject
                                             && appController.providerReady)
                                onClicked: {
                                    if (appController.turnRunning) {
                                        appController.interruptTurn()
                                        return
                                    }
                                    if (appController.sendPrompt(
                                                composer.text,
                                                root.composerImages,
                                                modelPicker.currentValue
                                                    || appController.codingModelId,
                                                appController.codingReasoningEffort,
                                                permissionPicker.currentValue,
                                                collaborationModeButton.checked
                                                    ? "plan" : "default")) {
                                        composer.clear()
                                        root.composerImages = []
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        ThreadPane {
            visible: root.sidePanelVisible
            SplitView.preferredWidth: Math.min(620, Math.max(390, root.width * 0.32))
            SplitView.minimumWidth: visible ? root.threadPanelMinimumWidth : 0
            SplitView.maximumWidth: visible ? 700 : 0
            controller: appController
            mode: root.sidePanelMode
            onCloseRequested: root.sidePanelVisible = false
        }
    }

    Dialog {
        id: imageViewer
        property string imagePath: ""
        property bool actualSize: false

        function showImage(path) {
            imagePath = path
            actualSize = false
            open()
        }

        modal: true
        width: Math.min(root.width - 40, 1400)
        height: Math.min(root.height - 40, 900)
        anchors.centerIn: parent
        padding: 0

        header: ToolBar {
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Kirigami.Units.largeSpacing
                anchors.rightMargin: Kirigami.Units.smallSpacing

                Label {
                    text: "Image"
                    font.bold: true
                    Layout.fillWidth: true
                }
                ToolButton {
                    text: imageViewer.actualSize ? "Fit" : "Actual size"
                    onClicked: imageViewer.actualSize = !imageViewer.actualSize
                }
                ToolButton {
                    icon.name: "window-close"
                    Accessible.name: "Close image"
                    ToolTip.text: Accessible.name
                    ToolTip.visible: hovered
                    onClicked: imageViewer.close()
                }
            }
        }

        contentItem: Rectangle {
            color: "#111111"

            Flickable {
                id: imageViewport
                anchors.fill: parent
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                contentWidth: Math.max(width, fullImage.width)
                contentHeight: Math.max(height, fullImage.height)

                Image {
                    id: fullImage
                    readonly property real fitScale: status === Image.Ready
                                                     ? Math.min(1,
                                                                imageViewport.width / implicitWidth,
                                                                imageViewport.height / implicitHeight)
                                                     : 1
                    x: Math.max(0, (imageViewport.width - width) / 2)
                    y: Math.max(0, (imageViewport.height - height) / 2)
                    width: implicitWidth * (imageViewer.actualSize ? 1 : fitScale)
                    height: implicitHeight * (imageViewer.actualSize ? 1 : fitScale)
                    source: root.localImageUrl(imageViewer.imagePath)
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    cache: false
                }

                ScrollBar.horizontal: ScrollBar {}
                ScrollBar.vertical: ScrollBar {}
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: fullImage.status === Image.Loading
                visible: running
            }
        }
    }

    Dialog {
        id: diagnosticsDialog
        title: "Diagnostics"
        modal: true
        width: Math.min(root.width - 40, 680)
        height: Math.min(root.height - 40, 560)
        anchors.centerIn: parent
        contentItem: DiagnosticsPage { controller: appController }
        standardButtons: Dialog.Close
    }
}
