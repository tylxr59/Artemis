import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Dialog {
    id: root
    title: "Allow full system access?"
    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 40 : 580, 580)
    standardButtons: Dialog.Ok | Dialog.Cancel

    ColumnLayout {
        width: parent.width
        Kirigami.Icon {
            source: "security-low"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 48
            Layout.preferredHeight: 48
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: "Artemis is configured to run Codex with danger-full-access and no approval prompts. Codex can execute commands and modify files anywhere your user account can access, not only inside the selected project."
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            font.bold: true
            text: "Only continue if you understand and accept this access level."
        }
    }
}
