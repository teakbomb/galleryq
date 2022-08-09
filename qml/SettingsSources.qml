import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.15
import gallery.q 1.0
import gallery.constants 1.0
import "components"

Item {

    Item {
        id: container
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
                query: "SELECT * FROM sources;"
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
                required property int sql_status
                required property int sql_mode
                required property var sql_path
                required property int sql_source

                onSql_sourceChanged: {
                    star.saved = util.isSourceSaved(sql_path)
                }

                onSql_pathChanged: {
                    star.saved = util.isSourceSaved(sql_path)
                }

                IconButton {
                    id: reload
                    width: height
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    icon: "qrc:/icons/refresh.svg"
                    color: "transparent"
                    disabled: modified
                    working: sql_status === 2

                    onPressed: {
                        util.startLoad(sql_source)
                    }

                    tooltip: "Load source"
                }

                IconButton {
                    id: star
                    width: height
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.left: reload.right
                    property var saved: util.isSourceSaved(sql_path)
                    icon: saved && !modified ? "qrc:/icons/star.svg": "qrc:/icons/star-outline.svg"
                    color: "transparent"
                    disabled: reload.working || modified

                    onPressed: {
                        util.toggleSourceSaved(sql_path)
                    }

                    Connections {
                        target: util
                        function onConfigChanged() {
                            star.saved = util.isSourceSaved(sql_path)
                        }
                    }

                    tooltip: saved ? "Unfavourite source" : "Favourite source"
                }

                TextInput {
                    id: path
                    verticalAlignment: Text.AlignVCenter
                    text: sql_path
                    font.pixelSize: 15
                    leftPadding: 4
                    color: "white"
                    selectByMouse: true
                    anchors.left: star.right
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
                    currentIndex: sql_mode - 1
                    width: 200
                    model: ["Collection", "Flat"]
                }

                IconButton {
                    id: save
                    width: height
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.right: trash.left
                    icon: "qrc:/icons/save.svg"
                    disabled: reload.working || !modified
                    color: Constants.bg1

                    onPressed: {
                        var status = util.updateSource(sql_source, path.text, mode.currentIndex + 1);
                        if(!status) {
                            error.start()
                        } else {
                            success.start()
                        }
                    }
                }

                IconButton {
                    id: trash
                    width: height
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    icon: check ? "qrc:/icons/tick.svg" : "qrc:/icons/trash.svg"
                    iconHoverColor: check ? "#4f0000" : "white"
                    color: "transparent"
                    disabled: reload.working
                    property bool check: false

                    onPressed: {
                        if(!check) {
                            check = true
                            return
                        }
                        util.deleteSource(sql_source);
                    }

                    onLeave: {
                        check = false
                    }

                    tooltip: check ? "Really delete source?" : "Delete source"
                }

                width: container.width
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

            tooltip: "New source"
        }

    }
}
