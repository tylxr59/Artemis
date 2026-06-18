import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Dialog {
    id: root
    required property var controller
    title: "Settings"
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 40 : 560, 560)
    standardButtons: Dialog.Cancel | Dialog.Apply

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
    }

    onOpened: loadValues()
    onApplied: {
        controller.codingModelId = codingModel.currentValue || ""
        controller.commitModelId = commitModel.currentValue || ""
        controller.titleModelId = titleModel.currentValue || ""
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.largeSpacing

        Label {
            text: "Models"
            font.bold: true
            font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: Kirigami.Units.largeSpacing
            rowSpacing: Kirigami.Units.smallSpacing

            Label { text: "Coding (new threads)" }
            ComboBox {
                id: codingModel
                Layout.fillWidth: true
                model: root.controller.models
                textRole: "name"
                valueRole: "id"
            }

            Label { text: "Commit messages" }
            ComboBox {
                id: commitModel
                Layout.fillWidth: true
                model: root.controller.models
                textRole: "name"
                valueRole: "id"
            }

            Label { text: "Thread titles" }
            ComboBox {
                id: titleModel
                Layout.fillWidth: true
                model: root.controller.models
                textRole: "name"
                valueRole: "id"
            }
        }

        Label {
            Layout.fillWidth: true
            text: "Commit messages and thread titles run in separate, read-only ephemeral threads."
            wrapMode: Text.Wrap
            opacity: 0.65
        }
    }
}
