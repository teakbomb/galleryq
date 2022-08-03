import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.15
import gallery.constants 1.0

Menu {
    readonly property real menuItemSize: 20
    topPadding: 2
    bottomPadding: 2

    delegate: MenuItem {
        id: menuItem
        implicitWidth: 150
        implicitHeight: menuItemSize
        hoverEnabled: true
        //font.bold: true
        font.pixelSize: 14

        contentItem: Text {
            text: menuItem.text
            font: menuItem.font


            color: "white"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            implicitWidth: 150
            implicitHeight: menuItemSize
            color: menuItem.hovered ? Constants.grey2 : "transparent"
        }
    }



    background: RectangularGlow {
        implicitWidth: 150
        implicitHeight: menuItemSize
        glowRadius: 5
        //spread: 0.2
        color: "black"
        cornerRadius: 10

        Rectangle {
            anchors.fill: parent
            color: Constants.grey1
        }

    }
}
