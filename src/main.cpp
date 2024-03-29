#include "Arduino.h"
#include "pins_arduino.h"
#include <USB-MIDI.h>
#include <Keypad.h>
#include <Wire.h>
// #include <LiquidCrystal_I2C.h>
//#include <cstddef>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

USBMIDI_CREATE_DEFAULT_INSTANCE();

hd44780_I2Cexp lcd;

const int LCD_COLS = 20;
const int LCD_ROWS = 4;

byte root = 50;

struct Chord {
    const char* name;
    byte notes[4];
};

struct Scale {
    const char* name;
    byte notes[7][2];
};

Chord recipes[] = {
    {"major", {0, 4, 7}},           // 0
    {"major 6", {0, 4, 7, 9}},      // 1
    {"dominant 7", {0, 4, 7, 10}},  // 2
    {"major 7", {0, 4, 7, 11}},     // 3
    {"augmented", {0, 4, 8}},       // 4
    {"augmented 7", {0, 4, 8, 10}}, // 5
    {"minor", {0, 3, 7}},           // 6
    {"minor 6", {0, 3, 7, 9}},      // 7
    {"minor 7", {0, 3, 7, 10}},     // 8
    {"minormajor 7", {0, 3, 7, 11}},// 9
    {"diminished", {0, 3, 6}},      // 10
    {"diminished 7", {0, 3, 6, 9}}, // 11
    {"half diminished 7", {0, 3, 6, 10}},   // 12
};

Scale scales[] = { // {semitones from root, chord type}
    {"major", {{0, 0}, {2, 6}, {4, 6}, {5, 0}, {7, 0}, {9, 6}, {11, 10}}},
    {"minor", {{0, 6}, {2, 10}, {3, 0}, {5, 6}, {7, 6}, {8, 0}, {10, 0}}},
    {"minor 7", {{0, 8}, {2, 12}, {3, 3}, {5, 8}, {7, 2}, {8, 3}, {10, 2}}},
};

byte currentScale = 2;
Chord* currentChord;

String noteNames[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

const byte ROWS = 1;
const byte COLS = 7;
char keys[ROWS][COLS] = {
    {0, 1, 2, 3, 4, 5, 6}
};
byte rowPins[ROWS] = {4};
byte colPins[COLS] = {5, 6, 7, 8, 9, 10, 16};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void keypadEvent(KeypadEvent);
void killAll();
void playChord(Chord*, int);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    MIDI.begin(4);
    keypad.addEventListener(keypadEvent);

    int status;
    status = lcd.begin(LCD_COLS, LCD_ROWS);
    if (status) {
        hd44780::fatalError(status);
    }
    lcd.print("HELLO");
}

void loop() {
    keypad.getKey();
}

void keypadEvent(KeypadEvent key) {
    switch (keypad.getState()) {
        case PRESSED:
            if (key < 7) {
                playChord(&recipes[scales[currentScale].notes[key][1]], scales[currentScale].notes[key][0]);
            }
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

void playChord(Chord *chord, int offset) {
    // for (int i = 0; i < sizeof(currentChord->notes) / sizeof(currentChord->notes[0]); i++) {
    //     MIDI.sendNoteOff(currentChord->notes[i], 0, 1);
    // }
    // 
    // currentChord = chord;
    killAll();
    for (uint i = 0; i < sizeof(chord->notes) / sizeof(chord->notes[0]); i++) {
        MIDI.sendNoteOn(root + offset + chord->notes[i], 127, 1);
    }

    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print(noteNames[(root) % (sizeof(noteNames) / sizeof(noteNames[0]))]);
    lcd.print(" ");
    lcd.print(scales[currentScale].name);
    lcd.setCursor(4, 1);
    lcd.print(noteNames[(root + offset) % (sizeof(noteNames) / sizeof(noteNames[0]))]);
    lcd.print(" ");
    lcd.print(chord->name);
}
