pragma ComponentBehavior: Bound

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
    signal imageOpenRequested(string path)
    signal contentLayoutChanged()

    readonly property bool isTool: eventType === "command" || eventType === "file"
                                  || eventType === "mcp"
    readonly property bool isReasoning: eventType === "reasoning"
    readonly property bool isStatus: eventType === "status"
    readonly property bool isDiff: eventType === "diff"
    readonly property bool isRunning: metadata && metadata.lifecycle === "started"
    readonly property string compactContent: content.replace(/\s+/g, " ").trim()
    readonly property var images: metadata && metadata.images ? metadata.images : []

    function localImageUrl(path) {
        return path.length > 0 ? "file://" + encodeURI(path) : ""
    }

    implicitHeight: card.implicitHeight
    onImplicitHeightChanged: contentLayoutChanged()
    onContentChanged: contentLayoutChanged()

    Rectangle {
        id: card
        width: root.eventType === "user" ? Math.min(parent.width * 0.76, 640) : parent.width
        anchors.right: root.eventType === "user" ? parent.right : undefined
        radius: Kirigami.Units.mediumSpacing
        color: root.eventType === "user"
               ? Kirigami.Theme.alternateBackgroundColor
               : root.eventType === "error"
                 ? Qt.alpha(Kirigami.Theme.negativeTextColor, 0.12)
                 : root.isTool || root.isReasoning || root.isDiff
                   ? Qt.alpha(Kirigami.Theme.alternateBackgroundColor, 0.46)
                   : "transparent"
        border.width: root.isTool || root.isReasoning || root.isDiff ? 1 : 0
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.12)
        implicitHeight: contentLoader.implicitHeight
        clip: root.isDiff

        Loader {
            id: contentLoader
            anchors.left: parent.left
            anchors.right: parent.right
            sourceComponent: root.isTool ? toolComponent
                             : root.isReasoning ? reasoningComponent
                             : root.isStatus ? statusComponent
                             : root.isDiff ? diffComponent : messageComponent
        }
    }

    Component {
        id: toolComponent
        ColumnLayout {
            spacing: 0

            AbstractButton {
                id: toolToggle
                Layout.fillWidth: true
                implicitHeight: Math.max(44, contentItem.implicitHeight
                                             + topPadding + bottomPadding)
                leftPadding: Kirigami.Units.largeSpacing
                rightPadding: Kirigami.Units.largeSpacing
                topPadding: Math.max(0, Kirigami.Units.smallSpacing - 1)
                bottomPadding: Kirigami.Units.smallSpacing + 1
                checkable: root.eventType === "command" || root.eventType === "mcp"
                hoverEnabled: checkable
                Accessible.name: root.eventType === "command"
                                 ? (checked ? "Hide full command" : "Show full command")
                                 : root.title
                ToolTip.text: Accessible.name
                ToolTip.visible: hovered && checkable

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Item {
                        Layout.preferredWidth: 14
                        Layout.preferredHeight: 14

                        Kirigami.Icon {
                            anchors.fill: parent
                            visible: root.eventType === "command" || root.eventType === "mcp"
                            source: toolToggle.checked ? "arrow-down" : "arrow-right"
                            opacity: 0.5
                        }
                    }
                    Kirigami.Icon {
                        Layout.preferredWidth: 17
                        Layout.preferredHeight: 17
                        source: root.eventType === "file"
                                ? "document-edit"
                                : root.eventType === "mcp"
                                  ? "network-connect" : "utilities-terminal"
                        color: root.isRunning ? Kirigami.Theme.highlightColor
                                              : Kirigami.Theme.textColor
                        opacity: root.isRunning ? 0.9 : 0.62
                    }
                    Label {
                        Layout.alignment: Qt.AlignVCenter
                        text: root.isRunning
                              ? "Running"
                              : (root.eventType === "file"
                                 ? "Changed files"
                                 : root.eventType === "mcp"
                                   ? root.title : "Ran command")
                        font.family: Kirigami.Theme.smallFont.family
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                        font.bold: true
                        opacity: root.isRunning ? 0.8 : 0.62
                    }
                    Rectangle {
                        Layout.preferredWidth: 1
                        Layout.preferredHeight: 14
                        color: Qt.alpha(Kirigami.Theme.textColor, 0.16)
                    }
                    Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        text: root.compactContent
                        textFormat: Text.PlainText
                        font.family: root.eventType === "command"
                                     ? "monospace"
                                     : Kirigami.Theme.defaultFont.family
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                        elide: Text.ElideRight
                        opacity: root.isRunning ? 0.55 : 0.72
                    }
                }

                background: Rectangle {
                    radius: card.radius
                    color: toolToggle.hovered
                           ? Qt.alpha(Kirigami.Theme.textColor, 0.05)
                           : "transparent"
                }

                HoverHandler {
                    enabled: toolToggle.checkable
                    cursorShape: Qt.PointingHandCursor
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.bottomMargin: Kirigami.Units.largeSpacing
                implicitHeight: fullCommandText.implicitHeight
                                + Kirigami.Units.largeSpacing * 2
                visible: toolToggle.checked
                radius: Kirigami.Units.smallSpacing
                color: Qt.alpha(Kirigami.Theme.backgroundColor, 0.64)
                border.width: 1
                border.color: Qt.alpha(Kirigami.Theme.textColor, 0.1)

                TextEdit {
                    id: fullCommandText
                    anchors.fill: parent
                    anchors.margins: Kirigami.Units.largeSpacing
                    text: root.content
                    textFormat: TextEdit.PlainText
                    readOnly: true
                    wrapMode: TextEdit.Wrap
                    selectByMouse: true
                    color: Kirigami.Theme.textColor
                    selectionColor: Kirigami.Theme.highlightColor
                    selectedTextColor: Kirigami.Theme.highlightedTextColor
                    font.family: "monospace"
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    opacity: 0.86
                }
            }
        }
    }

    Component {
        id: reasoningComponent
        ColumnLayout {
            spacing: 0
            ToolButton {
                id: reasoningToggle
                Layout.fillWidth: true
                checkable: true
                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Kirigami.Icon {
                        source: reasoningToggle.checked ? "arrow-down" : "arrow-right"
                        Layout.preferredWidth: 14
                        Layout.preferredHeight: 14
                        opacity: 0.55
                    }
                    Label {
                        text: "Reasoning"
                        Layout.fillWidth: true
                        font: Kirigami.Theme.smallFont
                        opacity: 0.58
                    }
                }
            }
            TextArea {
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.bottomMargin: Kirigami.Units.largeSpacing
                visible: reasoningToggle.checked
                text: root.content
                textFormat: TextEdit.PlainText
                readOnly: true
                wrapMode: TextEdit.Wrap
                selectByMouse: true
                background: null
                opacity: 0.72
            }
        }
    }

    Component {
        id: statusComponent
        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: root.content
            textFormat: Text.PlainText
            font: Kirigami.Theme.smallFont
            opacity: 0.45
            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing
        }
    }

    Component {
        id: diffComponent
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            DiffView {
                Layout.fillWidth: true
                diffText: root.content
                compact: true
                embedded: true
                expandedByDefault: false
            }
        }
    }

    Component {
        id: messageComponent
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.topMargin: Kirigami.Units.largeSpacing
                text: root.title
                textFormat: Text.PlainText
                font.bold: true
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                visible: root.eventType === "user" || root.eventType === "error"
                opacity: root.eventType === "error" ? 1 : 0.62
                color: root.eventType === "error"
                       ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            }
            ListView {
                Layout.fillWidth: true
                Layout.preferredHeight: root.images.length > 0 ? 150 : 0
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                visible: root.images.length > 0
                orientation: ListView.Horizontal
                spacing: Kirigami.Units.smallSpacing
                clip: true
                model: root.images

                delegate: Item {
                    id: imageThumbnail
                    required property string modelData
                    width: Math.min(220, implicitWidth)
                    height: 150
                    implicitWidth: thumbnail.implicitWidth

                    Image {
                        id: thumbnail
                        anchors.fill: parent
                        source: root.localImageUrl(imageThumbnail.modelData)
                        fillMode: Image.PreserveAspectFit
                        asynchronous: true
                        cache: false
                    }
                    HoverHandler {
                        id: thumbnailHover
                        cursorShape: Qt.PointingHandCursor
                    }
                    TapHandler {
                        onTapped: root.imageOpenRequested(imageThumbnail.modelData)
                    }
                    ToolTip {
                        visible: thumbnailHover.hovered
                        text: "Open full-size image"
                    }
                }
            }
            TextArea {
                id: messageText
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.bottomMargin: Kirigami.Units.largeSpacing
                Layout.topMargin: root.eventType === "user" || root.eventType === "error"
                                  ? 0 : Kirigami.Units.largeSpacing
                text: root.content
                textFormat: root.eventType === "assistant"
                            ? TextEdit.MarkdownText : TextEdit.PlainText
                visible: root.content.length > 0
                readOnly: true
                wrapMode: TextEdit.Wrap
                selectByMouse: true
                background: null
                font.family: Kirigami.Theme.defaultFont.family
                onContentHeightChanged: root.contentLayoutChanged()
                onImplicitHeightChanged: root.contentLayoutChanged()
                onLinkActivated: function(link) {
                    Qt.openUrlExternally(link)
                }

                HoverHandler {
                    cursorShape: messageText.hoveredLink.length > 0
                                 ? Qt.PointingHandCursor : Qt.IBeamCursor
                }
            }
        }
    }
}
