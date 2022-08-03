import QtQuick 2.15
import QtQuick.Layouts 1.15
import gallery.q 1.0

Item {
    property int count

    Rectangle {
        color: "#80000000"
        anchors.fill: info
        visible: info.visible
    }

    RowLayout {
        visible: count > 0
        id: info
        anchors.left: parent.left
        anchors.top: parent.top
        height: 20
        spacing: 0

        Item {
            width: 20
            height: 20
            Image {
                id: typeIcon
                source: "qrc:/icons/pages.svg"
                anchors.fill: parent
                sourceSize: Qt.size(50, 50)
                antialiasing: true
            }
        }
        Text {
            height: 20
            id: infoText
            verticalAlignment: Text.AlignVCenter
            color: "white"
            text: count
            leftPadding: 0
            rightPadding: 4
            bottomPadding: 1
        }
    }
}
