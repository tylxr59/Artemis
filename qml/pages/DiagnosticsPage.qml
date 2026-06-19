import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ScrollView {
    id: root
    required property var controller

    ColumnLayout {
        width: root.availableWidth
        spacing: Kirigami.Units.largeSpacing
        Label { text: "Artemis diagnostics"; font.bold: true; font.pointSize: 16 }
        GridLayout {
            columns: 2
            Layout.fillWidth: true
            Label { text: "Codex" }
            Label { text: root.controller.providerVersion || "Not detected" }
            Label { text: "Connection" }
            Label { text: root.controller.providerReady ? "Connected" : "Disconnected" }
            Label { text: "Database" }
            Label { text: root.controller.databasePath; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
            Label { text: "Project" }
            Label { text: root.controller.selectedProjectPath || "None"; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
            Label { text: "Status" }
            Label { text: root.controller.statusText; wrapMode: Text.Wrap; Layout.fillWidth: true }
        }
    }
}
