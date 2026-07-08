import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Frame {
    id: root
    property string iconName: "dialog-information"
    property string text: ""
    property color accentColor: Kirigami.Theme.textColor
    property bool useSmallFont: false

    Layout.fillWidth: true
    padding: Kirigami.Units.smallSpacing

    background: Rectangle {
        color: Qt.alpha(root.accentColor, 0.08)
        border.color: Qt.alpha(root.accentColor, 0.22)
        radius: Kirigami.Units.smallSpacing
    }

    contentItem: RowLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            source: root.iconName
            color: root.accentColor
            Layout.preferredWidth: Kirigami.Units.iconSizes.small
            Layout.preferredHeight: Kirigami.Units.iconSizes.small
        }

        Label {
            Layout.fillWidth: true
            text: root.text
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            color: root.accentColor
            font.pointSize: root.useSmallFont
                            ? Kirigami.Theme.smallFont.pointSize
                            : Kirigami.Theme.defaultFont.pointSize
        }
    }
}
