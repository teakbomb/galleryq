import QtQuick 2.0

Rectangle {
    id: divider
    width: 5
    property int minX: 0
    property int maxX: 100

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onPositionChanged: {
            if(pressedButtons) {
                divider.x = Math.min(maxX, Math.max(minX, divider.x + mouseX))
            }
        }
    }

    onMinXChanged: {
        divider.x = Math.min(maxX, Math.max(minX, divider.x))
    }

    onMaxXChanged: {
        divider.x = Math.min(maxX, Math.max(minX, divider.x))
    }
}
