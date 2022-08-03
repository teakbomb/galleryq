import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.15
import gallery.q 1.0
import gallery.constants 1.0

Popup {
    id: popup
    parent: Overlay.overlay
    modal: true

    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    width: parent.width - 100
    height: parent.height - 100
    padding: 0

    background: Rectangle {
        color: Constants.grey1
    }

    property var header: headerBar
    property var title: "Title"
    property var titleIcon: "qrc:/icons/settings.svg"

    Rectangle {
        id: headerBar
        color: Constants.grey0
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        Image {
            id: img
            source: titleIcon
            width: parent.height - 4
            height: width
            sourceSize: Qt.size(100, 100)
            x: (title.width - title.contentWidth) / 2 - width - 5
            y: 2
            ColorOverlay {
                anchors.fill: parent
                source: parent
                color: Constants.grey3
            }
        }

        Image {
            id: close
            source: "qrc:/icons/cross.svg"
            width: 20
            height: width
            sourceSize: Qt.size(width, height)
            x: parent.width - width - (parent.height - height)/2
            anchors.verticalCenter: parent.verticalCenter
            ColorOverlay {
                anchors.fill: parent
                source: parent
                color: mouse.containsMouse ? "white" : Constants.grey3
            }
            MouseArea {
                id: mouse
                hoverEnabled: true
                anchors.fill: parent
                onPressed: {
                    popup.close()
                }
            }
        }


        Text {
            id: title
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            text: popup.title
            font.pixelSize: 16
            font.bold: true
            leftPadding: 4
            color: Constants.grey4
            elide: Text.ElideRight
            height: parent.height
            anchors.right: parent.right
            anchors.left: parent.left
        }
        height: 30
        width: parent.width
    }

    onOpened: {
        forceActiveFocus()
    }
}
