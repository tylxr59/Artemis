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

    ColumnLayout {
        anchors.fill: parent
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
