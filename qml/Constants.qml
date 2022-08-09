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

    function parent_query(file) {
        return "SELECT * FROM nodes INNER JOIN files ON nodes.file = files.file WHERE parent = " + file + " ORDER BY node;"
    }

    function search_query(node, query) {
        query = util.processQuery(query)

        if(query === "")
            query = "probablywontmatchrightsurelythiswillnevermatch"

        query = "SELECT nodes.*, files.* FROM (SELECT rowid FROM search WHERE tags MATCH '" + query + "') JOIN nodes ON rowid = nodes.node JOIN files ON nodes.file = files.file";

        if(node !== 0) {
            query += " WHERE parent = " + node
        }

        return query + ";"
    }

    function children_query(node, children) {
        if(children === 0) {
            return "SELECT * FROM nodes INNER JOIN files ON nodes.file = files.file WHERE node = " + node + " ORDER BY node;"
        } else {
            return "SELECT * FROM nodes INNER JOIN files ON nodes.file = files.file WHERE parent = " + node + " ORDER BY node;"
        }
    }

    function node_tags_query(node) {
        return "SELECT * FROM (SELECT tag FROM tags WHERE node = " + node + ") as 'tags' INNER JOIN tag_count ON tags.tag = tag_count.tag;"
    }

    function tags_query(match, limit) {
        return "SELECT * FROM tag_count WHERE tag LIKE '" + match + "%' ORDER BY cnt DESC LIMIT " + limit + ";"
    }
}
