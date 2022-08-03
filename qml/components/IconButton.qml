import QtQuick 2.15
import QtGraphicalEffects 1.15
import gallery.constants 1.0

Rectangle {
    id: button
    color: Constants.grey1
    property var icon
    property var iconColor: Constants.grey3
    property bool disabled: false
    property bool working: false
    property var inset: 10

    signal pressed();

    MouseArea {
        anchors.fill: parent
        id: mouse
        hoverEnabled: true

        onPressed: {
            if(!disabled)
                button.pressed()
        }
    }

    Image {
        id: img
        source: icon
        width: parent.width - inset
        height: width
        sourceSize: Qt.size(parent.width, parent.height)
        anchors.centerIn: parent
        RotationAnimator {
            target: img;
            loops: Animation.Infinite
            from: 0;
            to: 360;
            duration: 1000
            running: working
            onRunningChanged: {
                if(!running)
                    target.rotation = 0
            }
        }
    }

    ColorOverlay {
        id: color
        anchors.fill: img
        source: img
        color: disabled ? Qt.darker(iconColor) : (mouse.containsMouse ? "white" : iconColor)
        RotationAnimator {
            target: color;
            loops: Animation.Infinite
            from: 0;
            to: 360;
            duration: 1000
            running: working

            onRunningChanged: {
                if(!running)
                    target.rotation = 0
            }
        }
    }

}
