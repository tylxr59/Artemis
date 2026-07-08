import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import "../utils/AppHelpers.js" as AppHelpers

RowLayout {
    id: root
    required property var server
    default property alias trailingContent: trailingArea.data

    Layout.fillWidth: true
    Layout.topMargin: Kirigami.Units.smallSpacing
    Layout.bottomMargin: Kirigami.Units.smallSpacing
    spacing: Kirigami.Units.smallSpacing

    Kirigami.Icon {
        source: root.server.authStatus === "notLoggedIn"
                ? "emblem-warning" : "network-server"
        color: AppHelpers.authColor(root.server.authStatus,
                                    Kirigami.Theme.neutralTextColor,
                                    Kirigami.Theme.positiveTextColor)
        Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
        Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 1

        Label {
            Layout.fillWidth: true
            text: root.server.title || root.server.name
            textFormat: Text.PlainText
            font.bold: true
            elide: Text.ElideRight
        }

        Label {
            Layout.fillWidth: true
            text: root.server.name
                  + " - "
                  + root.server.toolCount
                  + " tools - "
                  + root.server.resourceCount
                  + " resources"
            textFormat: Text.PlainText
            opacity: 0.62
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            elide: Text.ElideRight
        }
    }

    RowLayout {
        id: trailingArea
        spacing: Kirigami.Units.smallSpacing
    }
}
