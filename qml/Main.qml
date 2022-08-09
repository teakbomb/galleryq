import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.15
import gallery.q 1.0
import gallery.constants 1.0
import "components"

Window {
    id: root
    title: 'GalleryQ'
    width: 1280
    height: 720
    visible: true
    color: Constants.bg1

    DropArea {
        id: dropArea;
        anchors.fill: parent
        onDropped: {
            for(var i = 0; i < drop.urls.length; i++) {
                util.onDrop(drop.urls[i])
            }
        }
    }


    property var node: 0
    property var stack: []
    property int stackLength: 0

    property var activeThumbNode: -1
    property var activeThumbTags: -1
    property var activeFullNode: -1

    function close() {
        if(search.text != "") {
            search.clear()
            return
        }

        if(stack.length > 0) {
            var last = stack.pop()
            stackLength--;
            root.node = last['node']
            thumbView.currentIndex = last['index']
            thumbView.contentY = last['position']
            return
        }
    }

    function open(node) {
        stack.push({'node': root.node, 'index':thumbView.currentIndex, 'position': thumbView.contentY, 'search': search.text})
        stackLength++;
        root.node = node
        thumbView.currentIndex = 0
    }

    Sql {
        id: model
        property string search: ""
        query: model.search == "" ? Constants.parent_query(root.node) : Constants.search_query(root.node, model.search)


        onCountChanged: {
            if(count > 0 && thumbView.currentIndex == -1) {
                thumbView.currentIndex = 0
            }
        }
    }

    property var active: model

    Item {
        id: thumbArea
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: thumbDivider.left
        width: Math.max(100, thumbDivider.x)

        ThumbView {
            id: thumbView
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: search.bottom
            anchors.bottom: bar.top
            clip: true
            model: active

            onOpen: {
                root.open(thumbView.currentItem.node)
            }

            onCurrentIndexChanged: {
                fullView.currentIndex = thumbView.currentIndex
            }

            onContextMenu: {
                var f = thumbView.currentItem
                contextMenu.node = f.node
                contextMenu.children = f.count
                contextMenu.source = f.source
                contextMenu.popup()
            }

            function activeChanged(node, tags) {
                activeThumbNode = node
                activeThumbTags = tags
            }
        }

        IconButton {
            id: back
            anchors.left: parent.left
            anchors.top: parent.top
            height: stackLength > 0 || model.search != "" ? 30 : 0
            width: height
            icon:  "qrc:/icons/back.svg"
            iconColor: Constants.grey4
            color: search.bound.color

            onPressed: {
                close()
            }
        }

        Search {
            id: search
            anchors.left: back.right
            anchors.right: parent.right
            anchors.top: parent.top
            height: 30

            function doSearch(query) {
                model.search = query

                if(search.text == "") {
                    return;
                }

                if(model.errored) {
                    search.searchInvalid()
                } else if (model.count == 0) {
                    search.searchEmpty()
                } else {
                    search.searchSuccess()
                    keyboardFocus.forceActiveFocus()
                }
            }

            onFocusReleased: {
                keyboardFocus.forceActiveFocus()
            }

            onUp: {
                suggestions.up()
            }

            onDown: {
                suggestions.down()
            }

            onAdd: {
                suggestions.add()
            }
        }

        Rectangle {
            color: "white"
            opacity: 0.05
            anchors.left: back.left
            anchors.right: search.right
            anchors.top: search.top
            anchors.bottom: search.bottom
            visible: search.hasFocus
        }

        RectangularGlow {
            visible: suggestions.visible && suggestions.height > 0
            anchors.fill: suggestions
            glowRadius: 5
            opacity: 0.5
            spread: 0.2
            color: "black"
            cornerRadius: 10
        }

        Suggestions {
            visible: (search.hasFocus && search.current !== "") || suggestions.activeFocus
            id: suggestions
            x: search.x+20
            y: search.y + search.height - 3
            width: search.width - 40
            model: Sql {
                query: Constants.tags_query(search.current, 20)
                onQueryChanged: {
                    suggestions.reset()
                    if(suggestions.currentIndex == -1)
                        suggestions.currentIndex = 0
                }
            }

            function tagSelected(tag) {
                search.tagComplete(tag)
            }

            Keys.forwardTo: [search]
        }



        Bar {
            id: bar
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 30

            settingsOpen: settings.visible

            onOpenSettings: {
                settings.open()
            }

            onOpenTrash: {
               util.startFullLoad();
            }
        }
    }


    Rectangle {
        z:10
        id: thumbDivider
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 5
        property int minX: 0
        property int maxX: parent.width-5
        color: Constants.grey1

        Component.onCompleted: {
            x = 300
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onPositionChanged: {
                if(pressedButtons) {
                    thumbDivider.x = Math.min(thumbDivider.maxX, Math.max(thumbDivider.minX, thumbDivider.x + mouseX))
                }
            }
        }

        onMaxXChanged: {
            thumbDivider.x = Math.min(thumbDivider.maxX, Math.max(thumbDivider.minX, thumbDivider.x))
        }
    }


    FullView {
        id: fullView
        anchors.left: thumbDivider.right
        width: Math.max(99, metadataDivider.x - (thumbDivider.x + thumbDivider.width))
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        clip: true
        model: active

        onCurrentIndexChanged: {
            thumbView.currentIndex = fullView.currentIndex
        }

        onContextMenu: {
            var f = getCurrent()
            contextMenu.node = f.node
            contextMenu.children = f.count
            contextMenu.source = f.source
            contextMenu.popup()
        }

        function activeChanged(node) {
            activeFullNode = node
        }
    }

    Rectangle {
        id: metadataDivider
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 5
        property int minX: 5
        property int maxX: Math.min(300, parent.width-thumbDivider.x)
        property int offset: 200
        x: root.width - offset
        color: Constants.grey1

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onPositionChanged: {
                if(pressedButtons) {
                    metadataDivider.offset = Math.min(metadataDivider.maxX, Math.max(metadataDivider.minX, root.width - (metadataDivider.x + mouseX)))
                }
            }
        }

        onMaxXChanged: {
            if(parent.width > 0 && thumbDivider.x > 0) //wait for the GUI to actually be constructed...
                metadataDivider.offset = Math.min(metadataDivider.maxX, Math.max(metadataDivider.minX, metadataDivider.offset))
        }

        Component.onCompleted: {
            offset = 200
        }
    }

    MetadataView {
        id: metadataView
        node: activeThumbTags == "" && activeFullNode != -1 ? activeFullNode : activeThumbNode

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: metadataDivider.right
        anchors.right: parent.right

        function tagToggle(tag) {
            search.tagToggle(tag)
        }
        function tagStatus(tag) {
            return search.tagStatus(tag)
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: !keyboardFocus.hasFocus ? Qt.LeftButton : 0
        propagateComposedEvents: true
        onPressed: {
            keyboardFocus.forceActiveFocus()
            mouse.accepted = false
        }
    }

    function deleteFile(node) {
        var y = thumbView.contentY
        util.deleteFile(node)
        root.node = root.node
        thumbView.contentY = y
    }

    ContextMenu {
        id: contextMenu
        property var node
        property var source
        property var children: 0

        onChildrenChanged: {
            if(children > 0) {
                del.text = "Delete " + children + " Item/s"
            } else {
                del.text = "Delete"
            }
        }

        Action {
            text: "Open Location"
            onTriggered: {
                util.openFolder(contextMenu.source)
            }
        }

        Action {
            id: del
            text: "Delete"
            onTriggered: {
                deleteFile(contextMenu.node)
            }
        }

        onClosed: {
            keyboardFocus.forceActiveFocus()
        }
    }

    Settings {
        id: settings

        parent: Overlay.overlay
        modal: true
        closePolicy: Popup.CloseOnEscape

        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        width: parent.width - 100
        height: parent.height - 100

        onOpened: {
            forceActiveFocus()
        }

        onClosed: {
            keyboardFocus.forceActiveFocus()
        }
    }

    Item {
        id: keyboardFocus
        focus: true
        anchors.fill: parent

        Keys.onPressed: {

            switch(event.key) {
            case Qt.Key_Escape:
                root.close()
                event.accepted = true
                break;
            case Qt.Key_Delete:
                deleteFile(fullView.getCurrent().file)
                break;
            case Qt.Key_Left:
                fullView.left(event.isAutoRepeat)
                event.accepted = true
                break;
            case Qt.Key_Right:
                fullView.right(event.isAutoRepeat)
                event.accepted = true
                break;
            case Qt.Key_Up:
                fullView.up(event.isAutoRepeat)
                event.accepted = true
                break;
            case Qt.Key_Down:
                fullView.down(event.isAutoRepeat)
                event.accepted = true
                break;
            case Qt.Key_Equal:
                if(thumbView.cellSize + 50 < parent.width) {
                    thumbView.cellSize += 50
                }
                event.accepted = true
                break;
            case Qt.Key_Minus:
                if(thumbView.cellSize - 50 >= 50) {
                    thumbView.cellSize -= 50
                }
                event.accepted = true
                break;
            case Qt.Key_F11:
                if(root.visibility == Window.FullScreen) {
                    root.visibility = Window.Windowed
                } else {
                    root.visibility = Window.FullScreen
                }
                break;
            case Qt.Key_Return:
                if(thumbView.currentItem.count > 0)
                    open(thumbView.currentItem.node)
                break;
            case Qt.Key_Space:
                fullView.paused = !fullView.paused
                break;
            case Qt.Key_Control:
                fullView.zoom = true;
                break;
            default:

            }
        }
        Keys.onReleased: {
            switch(event.key) {
            case Qt.Key_Control:
                fullView.zoom = false;
                break;
            }

            if(!fullView.fast)
               return;
            switch(event.key) {
            case Qt.Key_Left:
                fullView.fast = event.isAutoRepeat
                break;
            case Qt.Key_Right:
                fullView.fast = event.isAutoRepeat
                break;
            case Qt.Key_Up:
                fullView.fast = event.isAutoRepeat
                break;
            case Qt.Key_Down:
                fullView.fast = event.isAutoRepeat
                break;
            case Qt.Key_Control:
                fullView.zoom = false;
                break;
            }
        }
    }

    Component.onCompleted: {
        //settings.open()
        util.startFullLoad();
    }
}
