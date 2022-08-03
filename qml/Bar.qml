import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.15
import gallery.constants 1.0
import "components"
Rectangle {
    id: bar
    color: Constants.grey0

    property bool settingsOpen


    signal openSettings()
    signal openTrash()

    Row {
        IconButton {
            height: bar.height
            width: height
            icon: "qrc:/icons/settings.svg"
            onPressed: bar.openSettings()
            working: settingsOpen
            color: Constants.grey0
        }

        IconButton {
            height: bar.height
            width: height
            icon: "qrc:/icons/trash.svg"
            color: Constants.grey0

            onPressed: bar.openTrash()
        }
    }
}
