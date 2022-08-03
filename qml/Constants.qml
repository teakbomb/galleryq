pragma Singleton
import QtQuick 2.15
import gallery.q 1.0

QtObject {
    readonly property var bg0: "#1d1d1d"
    readonly property var bg1: "#242424"
    readonly property var bg2: "#2a2a2a"
    readonly property var grey0: "#303030"
    readonly property var grey1: "#404040"
    readonly property var grey2: "#505050"
    readonly property var grey3: "#606060"
    readonly property var grey4: "#707070"
    readonly property var red: "#ba0000"
    readonly property var green: "green"

    function file_query(file) {
        return "SELECT * FROM files WHERE parent = " + file + " ORDER BY file;"
    }

    function file_search_query(file, query) {
        query = util.processQuery(query)

        var file_query = "SELECT * FROM files"

        if(file !== 0) {
            file_query += " WHERE parent = " + file
        }

        if(query === "")
            query = "probablywontmatchrightsurelythiswillnevermatch"

        var tags_query = "SELECT rowid FROM search WHERE tags MATCH '" + query + "'"

        return "SELECT file, parent, path, type, tags, children FROM (" + tags_query + ") as 't' JOIN (" + file_query + ") as 'f' ON t.rowid == f.file ORDER BY file;"
    }

    function file_children_query(file, children) {
        if(children === 0) {
            return "SELECT * FROM files as a WHERE file = " + file + " ORDER BY file;"
        } else {
            return "SELECT * FROM files as a WHERE parent = " + file + " ORDER BY file;"
        }
    }

    function file_tags_query(file) {
        return "SELECT * FROM (SELECT tag FROM tags WHERE file = " + file + ") as 'tags' INNER JOIN tag_count ON tags.tag = tag_count.tag;"
    }

    function tags_query(match, limit) {
        return "SELECT * FROM tag_count WHERE tag LIKE '" + match + "%' ORDER BY cnt DESC LIMIT " + limit + ";"
    }
}
