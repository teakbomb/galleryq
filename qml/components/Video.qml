import QtQuick 2.0
import QtQuick.Controls 1.0

import gallery.mpv 1.0

MpvObject {
    id: video
    property string source
    property int maxWidth
    property int maxHeight
    property bool fill: false
    property bool loaded: implicitHeight != 0 && implicitWidth != 0
    property bool active: false
    property bool paused: false

    width: 100
    height: 100

    function sync() {
        var h = implicitHeight;
        var w = implicitWidth;

        if(h == 0 || w == 0)
            return;

        if(fill) {
            var wr = maxWidth / implicitWidth
            var hr = maxHeight / implicitHeight

            if(hr < wr) {
                h = maxHeight
                w = implicitWidth * hr
            } else {
                w = maxWidth
                h = implicitHeight * wr
            }
        } else {
            var r = 0;
            if(h > maxHeight) {
                r = maxHeight/h
                h *= r
                w *= r
            }
            if(w > maxWidth) {
                r = maxWidth/w
                h *= r
                w *= r
            }
        }

        height = parseInt(h)
        width = parseInt(w)
    }

    onSourceChanged: {
        if(active) {
            video.command(["loadfile", source])
            video.setProperty("pause", paused)
        }
    }

    onActiveChanged: {
        if(active) {
            video.command(["loadfile", source])
            video.setProperty("pause", paused)
        }
    }

    onPausedChanged: {
        if(active) {
            video.setProperty("pause", paused)
        }
    }

    onImplicitWidthChanged: {
        sync()
    }

    onImplicitHeightChanged: {
        sync()
    }

    onMaxWidthChanged: {
        sync()
    }

    onMaxHeightChanged: {
        sync()
    }

    Component.onCompleted: {
        update();
    }
}
