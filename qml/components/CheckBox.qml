import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import gallery.constants 1.0

Rectangle {
    id: button
    color: Constants.grey1
    property var iconColor: Constants.grey3
    property var iconHoverColor: "white"
    property bool disabled: false
    property var tooltip: ""

    property bool checked: false

    signal toggled();
    signal enter();
    signal leave();

    MouseArea {
        anchors.fill: parent
        id: mouse
        hoverEnabled: true

        onPressed: {
            if(!disabled)
                button.pressed()
        }

        onHoveredChanged: {
            if(containsMouse) {
                button.enter()
            } else {
                button.leave()
            }
        }
    }

    InfoToolTip {
        id: infoToolTip
        visible: !working && !disabled && tooltip != "" && mouse.containsMouse
        delay: 100
        text: tooltip
    }

    Image {
        id: img
        source: checked ? ""
        width: parent.width - inset
        height: width
        sourceSize: Qt.size(parent.width, parent.height)
        anchors.centerIn: parent
        RotationAnimator {
            id: imgAnimation
            target: img;
            loops: Animation.Infinite
            from: 0;
            to: 360;
            duration: 1000

            onStopped: {
                target.rotation = 0
            }
        }
    }

    ColorOverlay {
        id: color
        anchors.fill: img
        source: img
        color: disabled ? Qt.darker(iconColor) : (mouse.containsMouse ? iconHoverColor : iconColor)
        RotationAnimator {
            id: colorAnimation
            target: color;
            loops: Animation.Infinite
            from: 0;
            to: 360;
            duration: 1000

            onStopped: {
                target.rotation = 0
            }
        }
    }

    function sync() {
        if(working) {
            imgAnimation.restart()
            colorAnimation.restart()
        } else {
            imgAnimation.stop()
            colorAnimation.stop()
        }
    }

    onWorkingChanged: sync()
    Component.onCompleted: sync()
    onVisibleChanged: sync()

}
