import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Frame {
    id: root
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
                text: root.title
                textFormat: Text.PlainText
                font.bold: true
                font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                elide: Text.ElideRight
            }

            Label {
                Layout.fillWidth: true
                visible: root.subtitle.length > 0
                text: root.subtitle
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
