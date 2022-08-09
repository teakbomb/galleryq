import QtQuick 2.15
import QtQuick.Controls 2.15
import gallery.q 1.0
import gallery.constants 1.0
import "components"

GridView {
    id: hView
    cellWidth: width
    cellHeight: height
    flow: GridView.FlowTopToBottom
    highlightFollowsCurrentItem: true
    snapMode: GridView.SnapOneRow
    highlightRangeMode: GridView.ApplyRange
    interactive: false
    clip: true
    property bool paused: false
    property bool zoom: false

    cacheBuffer: 1.5*height

    signal contextMenu()

    function getCurrent() {
        return currentItem.currentItem
    }

    property bool fast: false

    function left(auto) {
        var start = hView.currentIndex
        hView.moveCurrentIndexLeft()
        if(auto && start !== hView.currentIndex) {
            fast = true
        } else if(start === hView.currentIndex) {
            fast = false
        }
    }

    function right(auto) {
        var start = hView.currentIndex
        hView.moveCurrentIndexRight()
        if(auto && start !== hView.currentIndex) {
            fast = true
        } else if(start === hView.currentIndex) {
            fast = false
        }
    }

    function down(auto) {
        var start = hView.currentItem.currentIndex
        hView.currentItem.moveCurrentIndexDown()
        if(auto && start !== hView.currentItem.currentIndex) {
            fast = true
        } else if(start === hView.currentItem.currentIndex) {
            fast = false
        }
    }

    function up(auto) {
        var start = hView.currentItem.currentIndex
        hView.currentItem.moveCurrentIndexUp()
        if(auto && start !== hView.currentItem.currentIndex) {
            fast = true
        } else if(start === hView.currentItem.currentIndex) {
            fast = false
        }
    }

    function align() {
        hView.positionViewAtIndex(hView.currentIndex, GridView.Contain)
    }

    onCurrentIndexChanged: {
        align()
        align()
    }
    onContentWidthChanged: align()
    onContentHeightChanged: align()

    MouseArea {
        id: mouseControls
        anchors.fill: parent
        acceptedButtons: Qt.BackButton | Qt.ForwardButton
        property var direction: false
        onPressed: {
            if(mouse.button === Qt.BackButton) {
                hView.right(false)
                direction = false
            }
            if(mouse.button === Qt.ForwardButton) {
                hView.left(false)
                direction = true
            }
        }

        onPressAndHold: {
            autoclick.running = true
        }

        onReleased: {
            autoclick.running = false
        }

        Timer {
            id: autoclick
            interval: 10
            running: false
            repeat: true
            onTriggered: {
                if(!mouseControls.direction) {
                    hView.right(false)
                } else {
                    hView.left(false)
                }
            }
        }

        onWheel: {
            if(zoom) {
                wheel.accepted = false
                return
            }
            if(wheel.angleDelta.y < 0) {
                hView.down(false)
            } else {
                hView.up(false)
            }
        }
    }

    delegate: GridView {
        id: vView
        width: hView.width
        height: hView.height

        model: Sql {
            query: Constants.children_query(sql_node, sql_children)
        }

        cellWidth: width
        cellHeight: height
        snapMode: GridView.SnapToRow
        highlightRangeMode: GridView.ApplyRange
        interactive: false
        ScrollBar.vertical: ScrollBar {
            id: scroll
            onPressedChanged: {
                if(!pressed) {
                    vView.returnToBounds()
                    vView.currentIndex = vView.indexAt(vView.width/2, vView.contentY + vView.height/2)
                    align()
                }
            }
        }

        function align() {
            vView.positionViewAtIndex(vView.currentIndex, GridView.Contain)
        }
        onCurrentIndexChanged: {
            if(!ScrollBar.vertical.active)
                align()
        }
        onContentWidthChanged: align()
        onContentHeightChanged: align()

        property int hIndex: index

        delegate: Rectangle {
            id: item
            width: vView.width
            height: vView.height

            property int vIndex: index
            property var node: sql_node
            property var source: sql_path
            property var count: sql_children
            property bool selected: hView.currentIndex === hIndex && vView.currentIndex === vIndex

            color: Constants.bg0

            Media {
                id: media
                anchors.fill: parent
                source: sql_path
                type: sql_type
                thumbnail: hView.width < 100
                fast: hView.fast || scroll.pressed || autoclick.running
                active: selected
                paused: hView.paused

                //Drag.active: mouse.drag.active
                //MouseArea {
                //    id: mouse
                //    propagateComposedEvents: true
                //    anchors.fill: media.bounds
                //    drag.target: media
                //}

                onContextMenu: {
                    hView.contextMenu()
                }

            }

            onNodeChanged: {
                if(selected && hIndex !== -1 && vIndex !== -1) {
                    activeChanged(item.node)
                }
            }

            onSelectedChanged: {
                if(selected && hIndex !== -1 && vIndex !== -1) {
                    activeChanged(item.node)
                }
            }

            Component.onCompleted: {
                var a = hView.currentIndex === hIndex && vView.currentIndex === vIndex;
                var b = hIndex === 0 && vIndex === 0 && hView.currentIndex === -1 && vView.currentIndex === -1;

                if(a || b) {
                    activeChanged(item.node)
                }
            }
        }
    }
}
