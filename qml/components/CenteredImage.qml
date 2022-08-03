import QtQuick 2.15

Image {
    id: img

    property int maxWidth
    property int maxHeight
    property bool fill: false

    //width: 100
    //height: 100

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

    asynchronous: true

    onStatusChanged: {
        sync()
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
}
