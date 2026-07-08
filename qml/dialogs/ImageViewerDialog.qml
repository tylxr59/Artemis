pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import "../utils/AppHelpers.js" as AppHelpers

Dialog {
    id: root

    property string imagePath: ""
    property bool actualSize: false

    function showImage(path) {
        imagePath = path
        actualSize = false
        open()
    }

    modal: true
    width: Math.min(parent ? parent.width - 40 : 1400, 1400)
    height: Math.min(parent ? parent.height - 40 : 900, 900)
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
                text: root.actualSize ? "Fit" : "Actual size"
                onClicked: root.actualSize = !root.actualSize
            }
            ToolButton {
                icon.name: "window-close"
                Accessible.name: "Close image"
                ToolTip.text: Accessible.name
                ToolTip.visible: hovered
                onClicked: root.close()
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
                width: implicitWidth * (root.actualSize ? 1 : fitScale)
                height: implicitHeight * (root.actualSize ? 1 : fitScale)
                source: AppHelpers.localImageUrl(root.imagePath)
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
