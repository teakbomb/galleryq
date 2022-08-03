import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

import gallery.constants 1.0


ListView {
    id: suggestions

    signal focusText();

    property int mice: 0

    function reset() {
        mice = 0
        currentIndex = 0
    }

    clip:false
    snapMode: ListView.SnapToItem
    boundsBehavior: Flickable.StopAtBounds
    interactive: false

    onCurrentIndexChanged: {
        suggestions.positionViewAtIndex(suggestions.currentIndex, ListView.Contain)
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: 0.3
    }

    delegate: Rectangle {
        id: del
        property var i: index
        property var tag: sql_tag
        color: i === currentIndex ? (suggestions.activeFocus || mice != 0 ? Constants.grey3 : Constants.grey2) : (mouse.containsMouse ? Constants.grey2 : Constants.grey1)
        Text {
            verticalAlignment: Text.AlignVCenter
            text: sql_tag
            font.pixelSize: 15
            leftPadding: 4
            color: "white"
            elide: Text.ElideRight
            height: parent.height
            anchors.right: count.left
            anchors.left: parent.left
        }
        Text {
            id: count
            anchors.right: parent.right
            verticalAlignment: Text.AlignVCenter
            text: sql_cnt
            font.pixelSize: 12
            rightPadding: 4
            color: "white"
            height: parent.height
        }

        MouseArea {
            id:mouse
            anchors.fill: parent
            hoverEnabled: true
            property bool added: false
            onPressed: {
                suggestions.tagSelected(tag)
            }
            onHoveredChanged: {
                if(containsMouse) {
                    mice++
                    added = true
                } else {
                    mice--
                    added = false
                }
            }
            Component.onDestruction: {
                if(added) {
                    mice--
                }
            }
            onWheel: {
                if(wheel.angleDelta.y < 0) {
                    if(suggestions.contentY + suggestions.height + 20 <= suggestions.contentHeight)
                        suggestions.contentY += 20
                } else {
                    if(suggestions.contentY - 20 >= 0)
                        suggestions.contentY -= 20
                }
            }
        }
        height: 20
        width: suggestions.width
    }
    orientation: Qt.Vertical
    height: Math.min(99, contentHeight)

    Keys.onPressed: {
        switch(event.key) {
        case Qt.Key_Up:
            event.accepted = true
            if(currentIndex == 0)
                focusText()
            else
                decrementCurrentIndex()
            break;
        case Qt.Key_Down:
            event.accepted = true
            incrementCurrentIndex()
            break;
        case Qt.Key_Return:
            event.accepted = true
            tagSelected(currentItem.tag)
            break;

        }
    }
}
