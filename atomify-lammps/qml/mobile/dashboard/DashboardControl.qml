import QtQuick 2.5
import QtQuick.Layouts 1.1

import "qrc:/visualization"

Item {
    id: controlRoot
    signal clicked

    property list<Item> fixes
    property Item fullControl
    property Item miniControl
    property real itemSize: 10
}

