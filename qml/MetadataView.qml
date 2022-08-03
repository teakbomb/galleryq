import QtQuick 2.15
import QtGraphicalEffects 1.15
import gallery.q 1.0
import gallery.constants 1.0

Rectangle {
    color: Constants.bg1
    property var file

    ListView {
        anchors.fill: parent
        id: tagsView
        model: Sql {
            query: Constants.file_tags_query(file)
        }

        spacing: 3
        header: Item {
            height: 7
        }

        delegate: Item {
            x: 2
            width: tagsView.width-10
            height: 20

            property int tag_status: tagStatus(sql_tag)


            MouseArea {
                id: mouse
                hoverEnabled: true
                anchors.left: check.left
                anchors.right: tagText.right
                anchors.top: check.top
                anchors.bottom: check.bottom
                onClicked: {
                   tagToggle(sql_tag)
                }
            }


            Rectangle {
                x: countText.x - 10
                width: countText.width + 8
                anchors.top: countText.top
                anchors.bottom: countText.bottom
                radius: 5
                opacity: 0.5
                color: mouse.containsMouse ? Constants.grey1 : Constants.grey0
            }

            Rectangle {
                x: tagText.x
                width: tagText.contentWidth + 9
                anchors.top: tagText.top
                anchors.bottom: tagText.bottom
                radius: 5
                color: mouse.containsMouse ? Constants.grey1 : Constants.grey0
            }

            Item {
                id:  padding1
                width: 5
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
            }

            Rectangle {
                id: check
                anchors.left: padding1.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: height
                radius: 5
                color: mouse.containsMouse ? Constants.grey1 : Constants.grey0

                Image {
                    id: icon
                    source: tag_status == 0 ? "qrc:/icons/plus.svg" : (tag_status == 1 ? "qrc:/icons/tick.svg" : "qrc:/icons/cross.svg")
                    width: parent.width - 4
                    height: width
                    sourceSize: Qt.size(100, 100)
                    anchors.centerIn: parent
                }

                ColorOverlay {
                    anchors.fill: icon
                    source: icon
                    color: tag_status == 0 ? (mouse.containsMouse ? "white" : Constants.grey2) : (tag_status == 1 ? "green" : "#ba0000")
                }
            }
            Item {
                id:  padding2
                width: 5
                anchors.left: check.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
            }

            Text {
                id: tagText
                anchors.left: padding2.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width - 50
                elide: Text.ElideRight
                text: sql_tag
                padding: 5
                color: "white"
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                id: countText
                x: tagText.x + tagText.contentWidth + 10
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                text: sql_cnt
                font.pixelSize: 10
                padding: 5
                leftPadding: 2
                color: "white"
                verticalAlignment: Text.AlignVCenter
                opacity: 0.5
            }
        }
    }
}
