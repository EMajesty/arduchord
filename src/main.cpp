#include "Arduino.h"
#include "pins_arduino.h"
#include <USB-MIDI.h>
#include <Keypad.h>

USBMIDI_CREATE_DEFAULT_INSTANCE();

byte root = 60;

struct Chord {
    const char* name;
    byte notes[4];
};

struct Scale {
    const char* name;
    byte notes[7][2];
};

Chord recipes[] = {
    {"maj", {0, 4, 7}},         // 0
    {"maj6", {0, 4, 7, 9}},
    {"dom7", {0, 4, 7, 10}},
    {"maj7", {0, 4, 7, 11}},
    {"aug", {0, 4, 8}},
    {"aug7", {0, 4, 8, 10}},
    {"min", {0, 3, 7}},         // 6
    {"min6", {0, 3, 7, 9}},
    {"min7", {0, 3, 7, 10}},
    {"minmaj7", {0, 3, 7, 11}},
    {"dim", {0, 3, 6}},         // 10
    {"dim7", {0, 3, 6, 9}},
    {"hdim7", {0, 3, 6, 10}},
};

Scale scales[] = { // {semitones from root, chord type}
    {"major", {{0, 0}, {2, 6}, {4, 6}, {5, 0}, {7, 0}, {9, 6}, {11, 10}}},
    {"minor", {{0, 6}, {2, 10}, {3, 0}, {5, 6}, {7, 6}, {8, 0}, {10, 0}}},
};

byte currentScale = 1;

const byte ROWS = 1;
const byte COLS = 7;
char keys[ROWS][COLS] = {
    {0, 1, 2, 3, 4, 5, 6}
};
byte rowPins[ROWS] = {2};
byte colPins[COLS] = {3, 4, 5, 6, 7, 8, 9};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void keypadEvent(KeypadEvent);
void killAll();
void playChord(Chord, int);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    MIDI.begin(4);
    keypad.addEventListener(keypadEvent);
}

void loop() {
    keypad.getKey();
}

void keypadEvent(KeypadEvent key) {
    int keyNr = key - '0';
    switch (keypad.getState()) {
        case PRESSED:
            playChord(recipes[scales[currentScale].notes[key][1]], scales[currentScale].notes[key][0]);
            break;
        case RELEASED:
            break;
        case HOLD:
            break;
        case IDLE:
            break;
    }
}

void killAll() {
    for (int i = 0; i < 127; i++) {
        MIDI.sendNoteOff(i, 0, 1);
    }
}

void playChord(Chord chord, int offset) {
    killAll();
    for (uint i = 0; i < sizeof(chord.notes) / sizeof(chord.notes[0]); i++) {
        MIDI.sendNoteOn(root + offset + chord.notes[i], 127, 1);
    }
}
