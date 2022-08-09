import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.15
import gallery.q 1.0
import gallery.constants 1.0
import "components"

PopupWindow {
    id: settings
    title: "Settings"
    titleIcon: "qrc:/icons/settings.svg"

    Rectangle {
        color: Constants.bg1
        anchors.left: parent.left
        anchors.top: settings.header.bottom
        anchors.bottom: parent.bottom
        width: 150

        ListView {
            id: tabList
            anchors.fill: parent
            clip:true
            snapMode: ListView.SnapToItem
            boundsBehavior: Flickable.StopAtBounds
            interactive: false

            model: [{"name": "GUI", "icon": "qrc:/icons/cube-outline.svg"}, {"name": "Sources", "icon": "qrc:/icons/bookshelf.svg"}]

            delegate: Rectangle {
                id: del
                property var i: index
                color: i === tabList.currentIndex ? Constants.bg2 : (mouse.containsMouse ? Constants.bg1 : Constants.bg1)

                Image {
                    id: icon
                    source: modelData.icon
                    width: 20
                    height: width
                    sourceSize: Qt.size(width, height)
                    x: (parent.height - height)/2
                    anchors.verticalCenter: parent.verticalCenter
                    ColorOverlay {
                        anchors.fill: parent
                        source: parent
                        color: i === tabList.currentIndex ? "white" : Constants.grey4
                    }
                }
                Text {
                    verticalAlignment: Text.AlignVCenter
                    text: modelData.name
                    font.pixelSize: 15
                    leftPadding: 4
                    font.bold: true
                    color: i === tabList.currentIndex ? "white" : Constants.grey4
                    elide: Text.ElideRight
                    height: parent.height
                    anchors.right: parent.right
                    anchors.left: icon.right
                }

                MouseArea {
                    id: mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    property bool added: false
                    onPressed: {
                        tabList.currentIndex = i
                    }
                }
                height: 30
                width: parent.width
            }
            orientation: Qt.Vertical
        }

    }
    Rectangle {
        color: Constants.bg0
        anchors.right: parent.right
        anchors.top: settings.header.bottom
        anchors.bottom: parent.bottom
        width: parent.width - 150 - 5

        StackLayout {
            anchors.fill: parent
            currentIndex: tabList.currentIndex
            SettingsGui {

            }
            SettingsSources {

            }
        }
    }

    onOpened: {
        forceActiveFocus()
    }

    onClosed: {
        keyboardFocus.forceActiveFocus()
    }
}
