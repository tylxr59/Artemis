pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: root

    required property string diffText
    property bool compact: false
    property int compactFileLimit: 5
    property int compactLineLimit: 10
    property bool expandedByDefault: true

    readonly property var files: parseDiff(diffText)
    readonly property var visibleFiles: compact ? files.slice(0, compactFileLimit) : files
    readonly property int additions: totalFor("additions")
    readonly property int deletions: totalFor("deletions")

    implicitHeight: contentColumn.implicitHeight

    function displayPath(path) {
        if (path.startsWith("a/") || path.startsWith("b/"))
            return path.substring(2)
        return path
    }

    function parseRange(value) {
        const parts = value.split(",")
        return Number(parts[0])
    }

    function parseDiff(text) {
        if (!text)
            return []

        const result = []
        const sourceLines = text.replace(/\r\n/g, "\n").split("\n")
        let file = null
        let oldLine = 0
        let newLine = 0

        function finishFile() {
            if (!file)
                return
            if (!file.path)
                file.path = file.newPath || file.oldPath || "Changes"
            result.push(file)
            file = null
        }

        function startFile(oldPath, newPath) {
            finishFile()
            file = {
                path: displayPath(newPath !== "/dev/null" ? newPath : oldPath),
                oldPath: displayPath(oldPath),
                newPath: displayPath(newPath),
                additions: 0,
                deletions: 0,
                lines: []
            }
        }

        for (let index = 0; index < sourceLines.length; ++index) {
            const line = sourceLines[index]
            if (line.startsWith("diff --git ")) {
                const match = /^diff --git a\/(.+) b\/(.+)$/.exec(line)
                startFile(match ? match[1] : "", match ? match[2] : "")
                continue
            }
            if (!file)
                startFile("", "")

            if (line.startsWith("--- ")) {
                file.oldPath = displayPath(line.substring(4).split("\t")[0])
                continue
            }
            if (line.startsWith("+++ ")) {
                file.newPath = displayPath(line.substring(4).split("\t")[0])
                file.path = file.newPath !== "/dev/null" ? file.newPath : file.oldPath
                continue
            }
            if (line.startsWith("@@")) {
                const match = /^@@ -([0-9]+(?:,[0-9]+)?) \+([0-9]+(?:,[0-9]+)?) @@(.*)$/.exec(line)
                if (match) {
                    oldLine = parseRange(match[1])
                    newLine = parseRange(match[2])
                }
                file.lines.push({
                    type: "hunk",
                    text: match && match[3].trim().length > 0 ? match[3].trim() : line,
                    oldNumber: "",
                    newNumber: ""
                })
                continue
            }
            if (line.startsWith("+")) {
                file.additions++
                file.lines.push({
                    type: "addition",
                    text: line.substring(1),
                    oldNumber: "",
                    newNumber: newLine++
                })
                continue
            }
            if (line.startsWith("-")) {
                file.deletions++
                file.lines.push({
                    type: "deletion",
                    text: line.substring(1),
                    oldNumber: oldLine++,
                    newNumber: ""
                })
                continue
            }
            if (line.startsWith(" ")) {
                file.lines.push({
                    type: "context",
                    text: line.substring(1),
                    oldNumber: oldLine++,
                    newNumber: newLine++
                })
                continue
            }
            if (line === "\\ No newline at end of file") {
                file.lines.push({
                    type: "note",
                    text: line,
                    oldNumber: "",
                    newNumber: ""
                })
                continue
            }
            if (line.startsWith("index ") || line.startsWith("new file mode ")
                    || line.startsWith("deleted file mode ")
                    || line.startsWith("similarity index ")
                    || line.startsWith("rename from ") || line.startsWith("rename to "))
                continue

            if (line.length > 0) {
                file.lines.push({
                    type: "note",
                    text: line,
                    oldNumber: "",
                    newNumber: ""
                })
            }
        }
        finishFile()
        return result
    }

    function totalFor(key) {
        let total = 0
        for (let index = 0; index < files.length; ++index)
            total += files[index][key]
        return total
    }

    ColumnLayout {
        id: contentColumn
        width: parent.width
        spacing: root.compact ? Kirigami.Units.smallSpacing : Kirigami.Units.largeSpacing

        RowLayout {
            Layout.fillWidth: true
            visible: root.compact && root.files.length > 0
            spacing: Kirigami.Units.smallSpacing

            Label {
                Layout.alignment: Qt.AlignVCenter
                text: "Changed files (" + root.files.length + ")"
                font.bold: true
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                opacity: 0.72
            }
            Item { Layout.fillWidth: true }
            Label {
                Layout.alignment: Qt.AlignVCenter
                text: "+" + root.additions
                color: Kirigami.Theme.positiveTextColor
                font.family: "monospace"
                font.bold: true
            }
            Label {
                Layout.alignment: Qt.AlignVCenter
                text: "−" + root.deletions
                color: Kirigami.Theme.negativeTextColor
                font.family: "monospace"
                font.bold: true
            }
        }

        Repeater {
            model: root.visibleFiles

            delegate: Rectangle {
                id: fileCard
                required property var modelData
                property bool expanded: root.expandedByDefault
                readonly property var visibleLines: root.compact
                                                    ? modelData.lines.slice(0, root.compactLineLimit)
                                                    : modelData.lines

                Layout.fillWidth: true
                implicitHeight: fileLayout.implicitHeight
                radius: Kirigami.Units.smallSpacing
                color: Qt.alpha(Kirigami.Theme.alternateBackgroundColor, 0.34)
                border.width: 1
                border.color: Qt.alpha(Kirigami.Theme.textColor, 0.14)
                clip: true

                ColumnLayout {
                    id: fileLayout
                    width: parent.width
                    spacing: 0

                    ItemDelegate {
                        id: fileHeader
                        Layout.fillWidth: true
                        Layout.preferredHeight: root.compact ? 38 : 42
                        leftInset: 0
                        rightInset: 0
                        topInset: 0
                        bottomInset: 0
                        leftPadding: Kirigami.Units.smallSpacing
                        rightPadding: Kirigami.Units.largeSpacing
                        topPadding: 0
                        bottomPadding: 2
                        onClicked: fileCard.expanded = !fileCard.expanded

                        contentItem: RowLayout {
                            spacing: Kirigami.Units.smallSpacing

                            Kirigami.Icon {
                                source: fileCard.expanded ? "arrow-down" : "arrow-right"
                                Layout.preferredWidth: 14
                                Layout.preferredHeight: 14
                                opacity: 0.62
                            }
                            Kirigami.Icon {
                                source: "text-x-generic"
                                Layout.preferredWidth: 17
                                Layout.preferredHeight: 17
                                opacity: 0.72
                            }
                            Label {
                                id: filePathLabel
                                text: fileCard.modelData.path
                                textFormat: Text.PlainText
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignVCenter
                                elide: Text.ElideMiddle
                                font.bold: true
                                font.pointSize: Kirigami.Theme.smallFont.pointSize
                                ToolTip.text: text
                                ToolTip.visible: fileHeader.hovered && filePathLabel.truncated
                            }
                            Label {
                                Layout.alignment: Qt.AlignVCenter
                                text: "+" + fileCard.modelData.additions
                                color: Kirigami.Theme.positiveTextColor
                                font.family: "monospace"
                                font.bold: true
                            }
                            Label {
                                Layout.alignment: Qt.AlignVCenter
                                text: "−" + fileCard.modelData.deletions
                                color: Kirigami.Theme.negativeTextColor
                                font.family: "monospace"
                                font.bold: true
                            }
                        }
                    }

                    Kirigami.Separator {
                        Layout.fillWidth: true
                        visible: fileCard.expanded
                    }

                    Column {
                        Layout.fillWidth: true
                        visible: fileCard.expanded

                        Repeater {
                            model: fileCard.visibleLines

                            delegate: Rectangle {
                                id: diffLine
                                required property var modelData
                                width: fileCard.width
                                height: modelData.type === "hunk"
                                        ? (root.compact ? 30 : 34)
                                        : (root.compact ? 22 : 24)
                                color: modelData.type === "addition"
                                       ? Qt.alpha(Kirigami.Theme.positiveTextColor, 0.12)
                                       : modelData.type === "deletion"
                                         ? Qt.alpha(Kirigami.Theme.negativeTextColor, 0.12)
                                         : modelData.type === "hunk"
                                           ? Qt.alpha(Kirigami.Theme.highlightColor, 0.08)
                                           : "transparent"

                                HoverHandler {
                                    id: lineHover
                                }

                                Rectangle {
                                    width: 3
                                    height: parent.height
                                    color: diffLine.modelData.type === "addition"
                                           ? Kirigami.Theme.positiveTextColor
                                           : diffLine.modelData.type === "deletion"
                                             ? Kirigami.Theme.negativeTextColor
                                             : "transparent"
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    spacing: 0

                                    Label {
                                        Layout.preferredWidth: root.compact ? 34 : 40
                                        text: diffLine.modelData.oldNumber
                                        horizontalAlignment: Text.AlignRight
                                        rightPadding: Kirigami.Units.smallSpacing
                                        font.family: "monospace"
                                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                                        color: diffLine.modelData.type === "deletion"
                                               ? Kirigami.Theme.negativeTextColor
                                               : Kirigami.Theme.textColor
                                        opacity: text.length > 0 ? 0.56 : 0
                                    }
                                    Label {
                                        Layout.preferredWidth: root.compact ? 34 : 40
                                        text: diffLine.modelData.newNumber
                                        horizontalAlignment: Text.AlignRight
                                        rightPadding: Kirigami.Units.smallSpacing
                                        font.family: "monospace"
                                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                                        color: diffLine.modelData.type === "addition"
                                               ? Kirigami.Theme.positiveTextColor
                                               : Kirigami.Theme.textColor
                                        opacity: text.length > 0 ? 0.56 : 0
                                    }
                                    Rectangle {
                                        Layout.preferredWidth: 1
                                        Layout.fillHeight: true
                                        color: Qt.alpha(Kirigami.Theme.textColor, 0.1)
                                    }
                                    Label {
                                        Layout.preferredWidth: root.compact ? 18 : 20
                                        text: diffLine.modelData.type === "addition" ? "+"
                                              : diffLine.modelData.type === "deletion" ? "−" : ""
                                        horizontalAlignment: Text.AlignHCenter
                                        font.family: "monospace"
                                        font.bold: true
                                        color: diffLine.modelData.type === "addition"
                                               ? Kirigami.Theme.positiveTextColor
                                               : Kirigami.Theme.negativeTextColor
                                    }
                                    Label {
                                        id: codeLabel
                                        Layout.fillWidth: true
                                        Layout.rightMargin: Kirigami.Units.smallSpacing
                                        text: diffLine.modelData.text.length > 0
                                              ? diffLine.modelData.text : " "
                                        textFormat: Text.PlainText
                                        elide: Text.ElideRight
                                        font.family: "monospace"
                                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                                        color: diffLine.modelData.type === "hunk"
                                               ? Kirigami.Theme.highlightColor
                                               : diffLine.modelData.type === "note"
                                                 ? Qt.alpha(Kirigami.Theme.textColor, 0.58)
                                                 : Kirigami.Theme.textColor
                                        ToolTip.text: diffLine.modelData.text
                                        ToolTip.visible: lineHover.hovered && codeLabel.truncated
                                    }
                                }
                            }
                        }

                        Label {
                            width: fileCard.width
                            visible: root.compact
                                     && fileCard.modelData.lines.length > fileCard.visibleLines.length
                            text: "… " + (fileCard.modelData.lines.length
                                           - fileCard.visibleLines.length) + " more lines"
                            leftPadding: Kirigami.Units.largeSpacing
                            topPadding: Kirigami.Units.smallSpacing
                            bottomPadding: Kirigami.Units.smallSpacing
                            font: Kirigami.Theme.smallFont
                            opacity: 0.55
                        }
                    }
                }
            }
        }

        Label {
            Layout.fillWidth: true
            visible: root.compact && root.files.length > root.visibleFiles.length
            text: "… " + (root.files.length - root.visibleFiles.length) + " more changed files"
            horizontalAlignment: Text.AlignHCenter
            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing
            font: Kirigami.Theme.smallFont
            opacity: 0.55
        }
    }
}
