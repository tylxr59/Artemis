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

    readonly property bool isTool: eventType === "command" || eventType === "file"
    readonly property bool isReasoning: eventType === "reasoning"
    readonly property bool isStatus: eventType === "status"
    readonly property bool isRunning: metadata && metadata.lifecycle === "started"
    readonly property string compactContent: content.replace(/\s+/g, " ").trim()

    implicitHeight: card.implicitHeight

    Rectangle {
        id: card
        width: root.eventType === "user" ? Math.min(parent.width * 0.76, 640) : parent.width
        anchors.right: root.eventType === "user" ? parent.right : undefined
        radius: Kirigami.Units.mediumSpacing
        color: root.eventType === "user"
               ? Kirigami.Theme.alternateBackgroundColor
               : root.eventType === "error"
                 ? Qt.alpha(Kirigami.Theme.negativeTextColor, 0.12)
                 : root.isTool || root.isReasoning
                   ? Qt.alpha(Kirigami.Theme.alternateBackgroundColor, 0.46)
                   : "transparent"
        border.width: root.isTool || root.isReasoning ? 1 : 0
        border.color: Qt.alpha(Kirigami.Theme.textColor, 0.12)
        implicitHeight: contentLoader.implicitHeight

        Loader {
            id: contentLoader
            anchors.left: parent.left
            anchors.right: parent.right
            sourceComponent: root.isTool ? toolComponent
                             : root.isReasoning ? reasoningComponent
                             : root.isStatus ? statusComponent : messageComponent
        }
    }

    Component {
        id: toolComponent
        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            implicitHeight: Math.max(38, toolText.implicitHeight + Kirigami.Units.largeSpacing)

            Kirigami.Icon {
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.preferredWidth: 16
                Layout.preferredHeight: 16
                source: root.eventType === "file" ? "document-edit" : "utilities-terminal"
                opacity: 0.62
            }
            Label {
                text: root.isRunning ? "Running" : (root.eventType === "file" ? "Changed files" : "Ran command")
                font: Kirigami.Theme.smallFont
                opacity: 0.58
            }
            Label {
                id: toolText
                Layout.fillWidth: true
                Layout.rightMargin: Kirigami.Units.largeSpacing
                text: "—  " + root.compactContent
                font.family: root.eventType === "command" ? "monospace" : Kirigami.Theme.defaultFont.family
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                elide: Text.ElideRight
                opacity: root.isRunning ? 0.55 : 0.72
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
            font: Kirigami.Theme.smallFont
            opacity: 0.45
            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing
        }
    }

    Component {
        id: messageComponent
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            implicitHeight: messageText.implicitHeight + Kirigami.Units.largeSpacing * 2

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.topMargin: Kirigami.Units.largeSpacing
                text: root.title
                font.bold: true
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                visible: root.eventType === "user" || root.eventType === "error"
                opacity: root.eventType === "error" ? 1 : 0.62
                color: root.eventType === "error"
                       ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
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
                readOnly: true
                wrapMode: TextEdit.Wrap
                selectByMouse: true
                background: null
                font.family: root.eventType === "diff" ? "monospace"
                                                       : Kirigami.Theme.defaultFont.family
            }
        }
    }
}
