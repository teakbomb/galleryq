import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.15
import gallery.q 1.0
import gallery.constants 1.0
import "components"

Item {

    Item {
        id: uh
        x: 20
        y: 20
        width: parent.width - 40
        height: 200
        ListView {
            id: sources
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: contentHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            model: Sql {
                id: sql
                query: "SELECT path, mode FROM sources;"
            }

            orientation: Qt.Vertical

            delegate: Rectangle {
                color: Constants.bg1

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

                    onFinished: {
                        sql.reload()
                    }
                }

                property bool modified: sql_mode !== mode.currentIndex + 1 || sql_path !== path.text


                IconButton {
                    id: reload
                    width: height
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    icon: "qrc:/icons/refresh.svg"
                    color: Constants.bg1
                    disabled: modified

                    onPressed: {
                        util.startLoad(sql_path)
                    }
                }

                TextInput {
                    id: path
                    verticalAlignment: Text.AlignVCenter
                    text: sql_path
                    font.pixelSize: 15
                    leftPadding: 4
                    color: "white"
                    selectByMouse: true
                    anchors.left: reload.right
                    anchors.right: mode.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                }

                Combo {
                    id: mode
                    font.pixelSize: 15
                    anchors.right: save.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 200
                    model: ["Collection", "Flat"]
                }

                IconButton {
                    id: save
                    width: height
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    icon: "qrc:/icons/save.svg"
                    disabled: !modified
                    color: Constants.bg1

                    onPressed: {
                        var status = util.updateSource(sql_path, path.text, mode.currentIndex + 1);
                        if(!status) {
                            error.start()
                        } else {
                            success.start()
                        }
                    }
                }

                width: uh.width
                height: 30
            }
        }
        IconButton {
            id: add
            y: sources.y + sources.height + 10
            height: 30
            width: height
            anchors.right: sources.right
            icon: "qrc:/icons/plus.svg"
            color: Constants.bg1

            onPressed: {
                util.newSource()
                sql.reload()
            }
        }

    }
}
