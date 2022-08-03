import QtQuick 2.15
import QtGraphicalEffects 1.15
import "components"

Item {
    id: thumb

    property string source
    property string type
    property var file
    property int count
    property bool selected

    signal select()
    signal open()
    signal contextMenu()

    RectangularGlow {
        anchors.fill: overlay
        visible: overlay.visible
        glowRadius:  selected ? 6 : 10
        opacity: 0.5
        spread: 0.2
        color: selected ? "white" : "black"
        cornerRadius: 10
    }

    LoadingSpinner {
        height: thumb.height/4
        width: thumb.width/4
        anchors.centerIn: thumb
        running: img.status !== Image.Ready
    }

    Image {
        id: img
        anchors.fill: thumb
        asynchronous: false
        source: "image://thumbnail/" + thumb.source
        fillMode: Image.PreserveAspectFit
        sourceSize.width: 512
        sourceSize.height: 512
        mipmap: true
    }



    Image {
        id: playIcon
        source: "qrc:/icons/play-circle.svg"
        height: thumb.height/4
        width: thumb.width/4
        sourceSize: Qt.size(100, 100)
        anchors.centerIn: thumb
        visible: thumb.type === "video"
        opacity: 0.5
    }

    Rectangle {
        id: overlay
        visible: img.status == Image.Ready
        x: img.x + Math.max(0, Math.floor((img.width - img.paintedWidth) / 2))
        y: img.y + Math.max(0, Math.floor((img.height - img.paintedHeight) / 2))
        width: Math.min(img.width, img.paintedWidth)
        height: Math.min(img.height, img.paintedHeight)
        color: thumbMouse.containsMouse ? "#50ffffff" : "#00000000"

        ThumbOverlay {
            anchors.fill: parent
            count: thumb.count
        }


        MouseArea {
            id: thumbMouse
            anchors.fill: parent
            hoverEnabled: true
            preventStealing: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onPressed: {
                thumb.select()
                if (mouse.button === Qt.RightButton) {
                    thumb.contextMenu()
                    return
                }
                if(mouse.flags === Qt.MouseEventCreatedDoubleClick && count > 0) {
                    thumb.open()
                }
            }
        }
    }
}
