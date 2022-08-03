import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: view
    property string source
    property string type
    property bool thumbnail: true
    property bool fast: false
    property bool active: false
    property bool paused: false

    property MouseArea mouse: mouse    
    property Item bounds: bg

    signal contextMenu()

    color: "#00000000"
    clip: true

    Rectangle {
        id: bg
        anchors.fill: thumb
        color: "#000000"
    }

    CenteredImage {
        id: thumb
        anchors.centerIn: parent
        source: "image://thumbnail/"  + view.source
        sourceSize.width: 512
        sourceSize.height: 512
        maxWidth: full.visible ? full.width : (video.visible ? video.width : view.width)
        maxHeight: full.visible ? full.height : (video.visible ? video.height :view.height)
        fill: true
    }

    CenteredImage {
        id: full
        asynchronous: true
        anchors.centerIn: parent
        source: fast || view.type != "image" || view.thumbnail || thumb.status != Image.Ready ? "" : "file:///"  + view.source
        visible: view.type == "image" && !view.thumbnail && full.status == Image.Ready && thumb.status == Image.Ready
        smooth: true
        maxWidth: view.width
        maxHeight: view.height
        sourceSize: {Qt.size(view.width, view.height)}
        fill: true
    }

    Video {
        id: video
        anchors.centerIn: parent
        source: fast || view.type != "video" || view.thumbnail ? "" : view.source
        visible: view.type == "video" && !view.thumbnail && thumb.status == Image.Ready && !fast
        maxWidth: view.width
        maxHeight: view.height
        active: view.active
        fill:true
        paused: view.paused
    }

    MouseArea {
        id: mouse
        anchors.fill: thumb
        hoverEnabled: video.visible

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        drag.target: thumb

        onClicked: {
            if (mouse.button === Qt.RightButton) {
                contextMenu()
            } else if(video.visible) {
                video.command(["keydown", "MBTN_LEFT"])
                video.command(["keyup", "MBTN_LEFT"])
            }
        }

        onMouseXChanged: {
            if(video.visible) {
                video.command(["mouse", "%1".arg(parseInt(mouseX)), "%1".arg(parseInt(mouseY))])
            }
        }

        onMouseYChanged: {
            if(video.visible) {
                video.command(["mouse", "%1".arg(parseInt(mouseX)), "%1".arg(parseInt(mouseY))])
            }
        }
    }

    Drag.hotSpot.x: x
    Drag.hotSpot.y: y
    Drag.dragType: Drag.Automatic
    Drag.mimeData: { "text/uri-list": "file:///" + source }
    Drag.active: mouse.drag.active
}
