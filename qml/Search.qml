import QtQuick 2.15

Item {
    id: container
    //height: search.height+10
    property alias bound: bg
    property alias text: search.text
    property var current: ""
    property bool hasFocus: search.activeFocus
    signal focusReleased()
    signal searchInvalid()
    signal searchEmpty()
    signal searchSuccess()

    signal focusSuggestions()

    function gainFocus() {
        search.forceActiveFocus()
    }

    onTextChanged: {
        var s = text.slice(0, search.cursorPosition)
        var pos = s.search(new RegExp("[^\\[\\]-\\s]+$"));
        if(pos != -1) {
            current = s.slice(pos)
            return
        }
        current = ""
    }

    function tagStatus(tag) {
        var escaped = tag.replace(/[-[\]{}()*+?.,\\^$|#\s]/g, '\\$&');

        var pos = search.text.search(new RegExp("(^|[\\[\\s])" + escaped + "($|[\\]\\s])"));
        var neg = search.text.search(new RegExp("(^|[\\[\\s])-" + escaped + "($|[\\]\\s])"));
        if(neg != -1) {
            return 2
        } else if (pos != -1) {
            return 1
        }
        return 0
    }

    function tagToggle(tag) {
        var escaped = tag.replace(/[-[\]{}()*+?.,\\^$|#\s]/g, '\\$&');

        var s = new RegExp("(^|[\\[\\s])" + escaped + "($|[\\]\\s])");
        var i = search.text.search(s);
        if(i != -1) {
            if(search.text[i] !== tag[0])
                i++;
            search.text = search.text.slice(0, i) + "-" + search.text.slice(i);
            return
        }

        s = new RegExp("(^|[\\[\\s])-" + escaped + "($|[\\]\\s])");
        i = search.text.search(s);
        if(i != -1) {
            var l = tag.length + 1
            var ii = i;
            if(search.text[i] !== '-') {
                l++
                ii++;
            }

            search.text = search.text.substring(0, ii) + search.text.substring(i + l)

            if(search.text[0] === ' ') {
                search.text = search.text.slice(1)
            }

            return
        }

        if(search.text == "") {
            search.text = tag
        } else {
            if(search.text[search.text.length-1] !== " " && search.text[search.text.length-1] !== "[") {
                search.text += " "
            }
            search.text += tag
        }
    }

    function tagComplete(tag) {
        var s = search.text.slice(0, search.cursorPosition);
        var e = search.text.slice(search.cursorPosition);
        var r = tag.slice(current.length)

        var i = search.cursorPosition;

        search.text = s + tag.slice(current.length) + " " + e

        search.cursorPosition = i + r.length + 1
    }

    function clear() {
        search.text = ""
        doSearch(search.text)
    }



    Rectangle {
        id: bg
        anchors.fill: parent
        color: Constants.grey0
        clip: true

        SequentialAnimation on color {
            id: error
            running: false

            ColorAnimation {
                to: "#4f0000"
                duration: 100
            }
            ColorAnimation {
                to: Constants.grey0
                duration: 100
            }
        }

        SequentialAnimation on color {
            id: empty
            running: false

            ColorAnimation {
                to: "#4f4400"
                duration: 100
            }
            ColorAnimation {
                to: Constants.grey0
                duration: 100
            }
        }

        SequentialAnimation on color {
            id: success
            running: false

            ColorAnimation {
                to: "#004f08"
                duration: 100
            }
            ColorAnimation {
                to: Constants.grey0
                duration: 100
            }
        }

        Connections {
            target: container
            function onSearchInvalid() {
                error.start()
            }
            function onSearchSuccess() {
                success.start()
            }

            function onSearchEmpty() {
                empty.start()
            }
        }
    }

    TextInput {
        id: search
        color: "white"
        font.bold: true
        font.pointSize: 12
        selectByMouse: true
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        verticalAlignment: Text.AlignVCenter
        leftPadding: 8
        topPadding: 1
        onAccepted: {
            if(activeFocus) {
                doSearch(search.text)
                focusReleased()
            }
        }

        Keys.onPressed: {
            if(event.key === Qt.Key_Down && activeFocus) {
                focusSuggestions()
            }
            if(event.key === Qt.Key_Escape) {
                event.accepted = true
                focusReleased()
            }
        }
    }

    Keys.forwardTo: [search]
}
