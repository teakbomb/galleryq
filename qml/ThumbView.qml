import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

GridView {
    id: thumbView

    property int cellSize: 250
    property int padding: 10

    signal open()
    signal contextMenu()

    function align() {
        thumbView.positionViewAtIndex(thumbView.currentIndex, GridView.Contain)
    }

    onWidthChanged: align()

    cellWidth: (thumbView.width-padding)/Math.max(Math.ceil(thumbView.width/cellSize), 1)
    cellHeight: cellWidth
    topMargin: padding
    leftMargin: padding

    ScrollBar.vertical: ScrollBar {    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        onWheel: {
            if(wheel.angleDelta.y < 0) {
                thumbView.moveCurrentIndexRight()
            } else {
                thumbView.moveCurrentIndexLeft()
            }
        }
    }

    delegate: Thumb {
        id: thumb
        width: cellWidth-padding
        height: cellHeight-padding

        source: sql_path
        type: sql_type
        node: sql_node
        count: sql_children
        property var tags: sql_tags
        property var tIndex: index

        selected: thumbView.currentIndex === tIndex

        onSelect: {
            thumbView.currentIndex = index
        }

        onContextMenu: {
            thumbView.contextMenu()
        }

        onOpen: {
            thumbView.open()
        }

        onNodeChanged: {
            if(selected && tIndex !== -1) {
                activeChanged(thumb.node, thumb.tags)
            }
        }

        onSelectedChanged: {
            if(selected && tIndex !== -1) {
                activeChanged(thumb.node, thumb.tags)
            }
        }

        Component.onCompleted: {
            if(thumbView.currentIndex === tIndex) {
                activeChanged(thumb.node, thumb.tags)
            }
        }

    }
}
