import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.15
import gallery.q 1.0
import gallery.constants 1.0
import "components"

Item {
    Item {
        x: 20
        y: 20
        width: parent.width - 40
        height: 200

        Item {
            id: row
            height: 30
            width: parent.width

            property bool modified: util != null ? (util.thumbnailPath !== path.text) : false

            Text {
                id: label
                text: "Thumbnail Folder"
                font.pixelSize: 15
                leftPadding: 4
                rightPadding: 16
                font.bold: true
                color: "white"
                verticalAlignment: Text.AlignVCenter
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
            }

            Rectangle {
                id: bg
                color: Constants.bg1
                anchors.left: path.left
                anchors.right: clear.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                SequentialAnimation on color {
                    id: error
                    running: false

                    ColorAnimation {
                        to: "#4f0000"
                        duration: 100
                    }
                    ColorAnimation {
                        to: Constants.bg1
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
                        to: Constants.bg1
                        duration: 100
                    }
                }

                Rectangle {
                    id: progress
                    color: "#004f08"
                    x: 0
                    y: 0
                    width: 0
                    height: parent.height

                    Connections {
                        target: util
                        function onProgressChanged() {
                            progress.width = bg.width * util.thumbnailProgress
                        }
                    }
                }
            }

            TextInput {
                text: ""
                id: path
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 15
                leftPadding: 4
                color: "white"
                selectByMouse: true
                anchors.left: label.right
                anchors.right: save.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                clip: true

                Component.onCompleted: {
                    text = util.thumbnailPath;
                }
            }

            IconButton {
                id: save
                width: height
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: reload.left
                icon: "qrc:/icons/save.svg"
                disabled: !row.modified || reload.working
                color: "transparent"

                onPressed: {
                    var status = util.setThumbnailPath(path.text)
                    if(!status) {
                        error.start()
                    } else {
                        path.text = util.thumbnailPath
                        success.start()
                    }
                }
            }

            IconButton {
                id: reload
                width: height
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: clear.left
                icon: "qrc:/icons/refresh.svg"
                color: "transparent"
                disabled: row.modified
                working: util != null ? (util.thumbnailProgress != 0.0) : false

                onPressed: {
                    util.generateThumbnails();
                }

                tooltip: "Generate all thumbnails"
            }
            IconButton {
                id: clear
                width: height
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                icon: check ? "qrc:/icons/tick.svg" : "qrc:/icons/trash.svg"
                iconHoverColor: check ? "#4f0000" : "white"
                color: "transparent"
                disabled: row.modified || reload.working
                property bool check: false

                onPressed: {
                    if(!check) {
                        check = true
                         return
                    }
                    util.deleteThumbnails();
                }

                onLeave: {
                    check = false
                }

                tooltip: check ? "Really delete all thumbnails?" : "Delete all thumbnails"
            }
        }
    }
}
