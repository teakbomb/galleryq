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

    property var scale: 1.0
    property var max: false

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

    property var current: full.visible ? full : (video.loaded ? video : view)

    CenteredImage {
        id: thumb
        source: "image://thumbnail/"  + view.source
        sourceSize.width: 512
        sourceSize.height: 512
        anchors.centerIn: current
        maxWidth: current.width
        maxHeight: current.height
        fill: true
    }

    CenteredImage {
        id: full
        anchors.centerIn: thumb
        asynchronous: true
        source: fast || view.type != "image" || view.thumbnail || thumb.status != Image.Ready ? "" : "file:///"  + view.source
        visible: view.type == "image" && !view.thumbnail && full.status == Image.Ready && thumb.status == Image.Ready
        smooth: implicitWidth*2 < width && implicitHeight*2 < height ? false : true
        maxWidth: view.width
        maxHeight: view.height
        sourceSize.width: view.width
        sourceSize.height: view.height
        fill: true
    }

    CenteredImage {
        id: fullMax
        asynchronous: true
        source: !max || fast || view.type != "image" || view.thumbnail || thumb.status != Image.Ready ? "" : "file:///"  + view.source
        visible: view.type == "image" && !view.thumbnail && full.status == Image.Ready && thumb.status == Image.Ready

        smooth: implicitWidth*2 < width && implicitHeight*2 < height ? false : true
        maxWidth: full.maxWidth
        maxHeight: full.maxHeight
        mipmap: true
        fill: true

        anchors.centerIn: full
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.MiddleButton

        property var startX: 0
        property var startY: 0
        property var posX: 0
        property var posY: 0
        property var dragging: false

        onPressed: {
            posX = current.x
            posY = current.y
            startX = mouseX
            startY = mouseY
            dragging = true
        }

        onReleased: {
            dragging = false
        }

        onPositionChanged: {
            if(dragging) {
                current.anchors.centerIn = undefined

                current.x = posX + (mouseX - startX)
                current.y = posY + (mouseY - startY)

                bound()
            }
        }

        function bound() {
            var dx = (current.maxWidth - current.width)/2
            var dy = (current.maxHeight - current.height)/2

            var x = current.x + dx
            var y = current.y + dy
            var w = current.width
            var h = current.height

            if(x + w - dx < view.width/2)
                x = view.width/2 - w + dx
            if(y + h - dy < view.height/2)
                y = view.height/2 - h + dy

            if(x > view.width/2 + dx)
                x = view.width/2 + dx

            if(y > view.height/2 + dy)
                y = view.height/2 + dy

            current.x = x - dx
            current.y = y - dy
        }

        function scale(cx, cy, d) {
            current.anchors.centerIn = undefined

            d = view.scale * d

            var f = ((view.scale + d)/view.scale) -1

            if(view.scale + d < 0.1) {
                return
            }

            view.scale += d

            current.maxWidth = view.scale * view.width
            current.maxHeight = view.scale * view.height

            var dx = f*(cx - current.x)
            var dy = f*(cy - current.y)

            current.x -= dx
            current.y -= dy
            posX -= dx
            posY -= dy

            bound()

            view.max = true
        }

        onWheel: {
            if(wheel.angleDelta.y < 0) {
                wheel.accepted = true
                scale(wheel.x, wheel.y, -0.1)
            } else {
                wheel.accepted = true
                scale(wheel.x, wheel.y, 0.1)
            }
        }
    }

    Video {
        id: video
        anchors.centerIn: scale == 1.0 ? view : null
        source: fast || view.type != "video" || view.thumbnail ? "" : view.source
        visible: loaded && view.type == "video" && !view.thumbnail && thumb.status == Image.Ready && !fast
        maxWidth: view.width
        maxHeight: view.height
        active: view.active
        fill:true
        paused: view.paused
    }

    MouseArea {
        id: mouse
        anchors.fill: current
        hoverEnabled: video.visible

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        drag.target: thumb

        onClicked: {
            switch(mouse.button) {
            case Qt.RightButton:
                contextMenu()
                break;
            case Qt.LeftButton:
                if(video.visible) {
                    video.command(["keydown", "MBTN_LEFT"])
                    video.command(["keyup", "MBTN_LEFT"])
                }
                break;
            default:
                mouse.accepted = false
                break;
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
