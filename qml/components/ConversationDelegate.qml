import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: root
    required property string eventType
    required property string title
    required property string content
    required property var metadata
    implicitHeight: card.implicitHeight

    Rectangle {
        id: card
        width: root.eventType === "user" ? Math.min(parent.width * 0.72, 600) : parent.width
        anchors.right: root.eventType === "user" ? parent.right : undefined
        radius: Kirigami.Units.mediumSpacing
        color: root.eventType === "user"
               ? Kirigami.Theme.alternateBackgroundColor
               : root.eventType === "error"
                 ? Qt.alpha(Kirigami.Theme.negativeTextColor, 0.12)
                 : root.eventType === "command" || root.eventType === "file"
                   ? Kirigami.Theme.alternateBackgroundColor : "transparent"
        border.width: root.eventType === "command" || root.eventType === "file" ? 1 : 0
        border.color: Kirigami.Theme.separatorColor
        implicitHeight: contentColumn.implicitHeight + Kirigami.Units.largeSpacing * 2

        ColumnLayout {
            id: contentColumn
            anchors.fill: parent
            anchors.margins: Kirigami.Units.largeSpacing
            spacing: Kirigami.Units.smallSpacing
            Label {
                text: root.title
                font.bold: true
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                visible: root.title.length > 0
                opacity: root.eventType === "error" ? 1 : 0.7
                color: root.eventType === "error"
                       ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            }
            TextArea {
                Layout.fillWidth: true
                text: root.content
                readOnly: true
                wrapMode: TextEdit.Wrap
                selectByMouse: true
                background: null
                font.family: root.eventType === "command" || root.eventType === "diff"
                             ? "monospace" : Kirigami.Theme.defaultFont.family
            }
        }
    }
}
