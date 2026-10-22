// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

// Splash screen: a sunken track with three square blocks marching across it, classic style.
// Everything is aligned to whole pixels and moves in whole-pixel steps, so the edges stay crisp.
// Colors come from the color scheme's complementary set (the classic desktop blue in Beveled
// Classic); bevel shades are derived from it.

import QtQuick
import org.kde.kirigami as Kirigami

Rectangle {
    id: root

    // Set by the splash engine as startup progresses; at stage 5 the desktop is about ready.
    property int stage

    Kirigami.Theme.colorSet: Kirigami.Theme.Complementary
    Kirigami.Theme.inherit: false
    color: Kirigami.Theme.backgroundColor

    onStageChanged: {
        if (stage == 2) {
            fade.from = 0;
            fade.to = 1;
            fade.running = true;
        } else if (stage == 5) {
            fade.from = 1;
            fade.to = 0;
            fade.running = true;
        }
    }

    Item {
        id: content
        anchors.fill: parent
        opacity: 0

        // Sunken track: two rings, dark on top/left, light on bottom/right (bottom/right own the
        // corners, as everywhere in Beveled).
        Item {
            id: track
            readonly property int blockSize: Math.round(Kirigami.Units.gridUnit * 1.1)
            readonly property int gap: 2
            readonly property int bevel: 2
            readonly property int innerWidth: 10 * (blockSize + gap)

            width: innerWidth + 2 * bevel
            height: blockSize + 2 * gap + 2 * bevel
            x: Math.round((parent.width - width) / 2)
            y: Math.round(parent.height * 0.6)

            readonly property color face: Kirigami.Theme.backgroundColor
            readonly property color dark: Qt.darker(face, 1.6)
            readonly property color shadow: Qt.darker(face, 3.2)
            readonly property color light: Qt.lighter(face, 1.8)

            Rectangle { color: Kirigami.Theme.alternateBackgroundColor; x: track.bevel; y: track.bevel; width: track.innerWidth; height: track.height - 2 * track.bevel }
            Rectangle { color: track.dark; x: 0; y: 0; width: track.width - 1; height: 1 }
            Rectangle { color: track.dark; x: 0; y: 0; width: 1; height: track.height - 1 }
            Rectangle { color: track.shadow; x: 1; y: 1; width: track.width - 3; height: 1 }
            Rectangle { color: track.shadow; x: 1; y: 1; width: 1; height: track.height - 3 }
            Rectangle { color: track.light; x: 0; y: track.height - 1; width: track.width; height: 1 }
            Rectangle { color: track.light; x: track.width - 1; y: 0; width: 1; height: track.height }
            Rectangle { color: track.face; x: 1; y: track.height - 2; width: track.width - 2; height: 1 }
            Rectangle { color: track.face; x: track.width - 2; y: 1; width: 1; height: track.height - 2 }

            // The marching blocks, clipped to the inside of the track.
            Item {
                id: lane
                x: track.bevel + track.gap
                y: track.bevel + track.gap
                width: track.innerWidth - 2 * track.gap
                height: track.blockSize
                clip: true

                property int offset: -3 * (track.blockSize + track.gap)
                // Whole pixels per frame, so the blocks never land between pixels.
                readonly property int step: 3

                Repeater {
                    model: 3
                    Rectangle {
                        color: Kirigami.Theme.highlightColor
                        width: track.blockSize
                        height: track.blockSize
                        x: lane.offset + index * (track.blockSize + track.gap)
                    }
                }

                Timer {
                    interval: 30
                    repeat: true
                    // No motion when the user disabled animations.
                    running: Kirigami.Units.longDuration > 1
                    onTriggered: {
                        lane.offset += lane.step;
                        if (lane.offset > lane.width) {
                            lane.offset = -3 * (track.blockSize + track.gap);
                        }
                    }
                }
            }
        }
    }

    OpacityAnimator {
        id: fade
        target: content
        duration: Kirigami.Units.longDuration
        running: false
    }
}
