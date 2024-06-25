#include "Arduino.h"
#include "pins_arduino.h"
#include <USB-MIDI.h>
#include <Keypad.h>
#include <Wire.h>
#include <avr/pgmspace.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <EncoderButton.h>

#define IDNAME(name) #name

USBMIDI_CREATE_DEFAULT_INSTANCE();

bool summoningSickness = true;

hd44780_I2Cexp lcd;

const int LCD_COLS = 16;
const int LCD_ROWS = 2;

enum EncoderMode {
    SCALE,
    NOTE,
    OCTAVE
};

int root = 60;
byte offset = 0;

// enum Inversion {
//     ROOT,
//     FIRST,
//     SECOND,
//     THIRD
// };

struct Chord {
    const char* name;
    byte notes[4];
};

// struct Scale {
//     const char* name;
//     byte notes[7][2];
// };

struct Scale {
    const char* name;
    byte notes[7];
};

// Chord recipes[] = {
//     {"major", {0, 4, 7}},           // 0
//     {"major 6", {0, 4, 7, 9}},      // 1
//     {"dominant 7", {0, 4, 7, 10}},  // 2
//     {"major 7", {0, 4, 7, 11}},     // 3
//     {"augmented", {0, 4, 8}},       // 4
//     {"augmented 7", {0, 4, 8, 10}}, // 5
//     {"minor", {0, 3, 7}},           // 6
//     {"minor 6", {0, 3, 7, 9}},      // 7
//     {"minor 7", {0, 3, 7, 10}},     // 8
//     {"minormajor 7", {0, 3, 7, 11}},// 9
//     {"diminished", {0, 3, 6}},      // 10
//     {"diminished 7", {0, 3, 6, 9}}, // 11
//     {"half diminished 7", {0, 3, 6, 10}},   // 12
// };

Chord chords[] = {
    {"third", {0, 2, 4}},
    {"fourth", {0, 3, 6}},
    {"seventh", {0, 2, 6}},
    {"ninth", {0, 2, 8}},
    {"eleventh", {0, 2, 10}},
    {"thirteenth", {0, 2, 12}},
};

// Scale scales[] = { // {semitones from root, chord type}
//     {"major", {{0, 0}, {2, 6}, {4, 6}, {5, 0}, {7, 0}, {9, 6}, {11, 10}}},
//     {"minor", {{0, 6}, {2, 10}, {3, 0}, {5, 6}, {7, 6}, {8, 0}, {10, 0}}},
//     {"minor 7", {{0, 8}, {2, 12}, {3, 3}, {5, 8}, {7, 2}, {8, 3}, {10, 2}}},
// };

Scale scales[] = { // {semitones from root}
    {"major", {0, 2, 4, 5, 7, 9, 11}},
    {"mixolydian", {0, 2, 4, 5, 7, 9, 10}},
    {"lydian", {0, 2, 4, 6, 7, 9, 11}},
    {"lydiandom", {0, 2, 4, 6, 7, 9, 10}},
    {"phrygiandom", {0, 1, 4, 5, 7, 8, 10}},
    {"natminor", {0, 2, 3, 5, 7, 8, 10}},
    {"dorian", {0, 2, 3, 5, 7, 9, 10}},
    {"harminor", {0, 2, 3, 5, 7, 8, 11}},
    {"melminor", {0, 2, 3, 5, 7, 9, 11}},
    {"phrygian", {0, 1, 3, 5, 7, 8, 10}},
    {"locrian", {0, 1, 3, 5, 6, 8, 10}},
};

byte currentScaleIndex = 0;
byte scalesCount = sizeof(scales) / sizeof scales[0];
// Chord* currentChord = &recipes[0];
// byte currentOffset;
bool playing = false;

const String noteNames[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

const byte ROWS = 2;
const byte COLS = 6;

char keys[ROWS][COLS] = {
    {'a', 'b', '2', '4', '6', 'e'},
    {'c', 'd', '1', '3', '5', '7'},
};

byte colPins[COLS] = {16, 10, 9, 8, 7, 6};
byte rowPins[ROWS] = {15, 14};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

EncoderButton encoder(4, 5);

enum EncoderMode encoderMode = NOTE;

void keypadEvent(KeypadEvent);
void encoderEvent(EncoderButton& eb);
void killAll();
void playChord(byte);
void drawScreen();

void setup() {
    // pinMode(LED_BUILTIN, OUTPUT);
    MIDI.begin(4);
    keypad.addEventListener(keypadEvent);
    encoder.setEncoderHandler(encoderEvent);

    lcd.begin(LCD_COLS, LCD_ROWS);
    lcd.setCursor(0, 0);
    lcd.print("   HELL WORLD   ");
    lcd.setCursor(0, 1);
    lcd.print("ELECTRIC MAJESTY");
}

void loop() {
    keypad.getKey();
    encoder.update();
    
    if (summoningSickness) { 
        if (millis() > 3000) {
            lcd.setCursor(0, 0);
            lcd.print("   ARDUCHORD!   ");
            lcd.setCursor(0, 1);
            lcd.print("ELECTRIC MAJESTY");
        }

        if (millis() > 6000) {
            drawScreen();
            summoningSickness = false;
        }
    }
}

void keypadEvent(KeypadEvent key) {
    switch (keypad.getState()) {
        case PRESSED:
            switch (key) {
                case '1':
                    playChord(0);
                    break;
                case '2':
                    break;
                case '3':
                    break;
                case '4':
                    break;
                case '5':
                    break;
                case '6':
                    break;
                case '7':
                    break;

                case 'a':
                    encoderMode = SCALE;
                    // 7th chord
                    break;
                case 'b':
                    encoderMode = OCTAVE;
                    // 9th chord
                    break;
                case 'c':
                    // 11th chord
                    break;
                case 'd':
                    // 13th chord
                    break;

                case 'e':
                    killAll();
                    break;
            }
            break;

        case RELEASED:
            switch (key) {
                case 'a':
                    encoderMode = NOTE;
                    break;
                case 'b':
                    encoderMode = NOTE;
                    break;
                case 'c':
                    encoderMode = NOTE;
                    break;
                case 'd':
                    encoderMode = NOTE;
                    break;
            }
            break;

        case HOLD:
            break;

        case IDLE:
            break;
    }

    drawScreen();
}

void encoderEvent(EncoderButton& _encoder) {
    switch (encoderMode) {
        case SCALE:
            currentScaleIndex += _encoder.increment();
            if (currentScaleIndex >= scalesCount) {
                currentScaleIndex = scalesCount;
            } else if (currentScaleIndex <= 0) {
                currentScaleIndex = 0;
            }
            break;

        case NOTE:
            root += _encoder.increment();
            if (root > 100) {
                root = 100;
            } else if (root < 0) {
                root = 0;
            }
            break;

        case OCTAVE:
            root += _encoder.increment() * 12;
            if (root > 100) {
                root = 100;
            } else if (root < 0) {
                root = 0;
            }
            break;
    }
    drawScreen();
}

void killAll() {
    for (int i = 0; i < 127; i++) {
        MIDI.sendNoteOff(i, 0, 1);
    }
    playing = false;
}

void playChord(byte offset) {
    killAll();
    // for (uint i = 0; i < sizeof(currentChord->notes) / sizeof(currentChord->notes[0]); i++) {
    //     MIDI.sendNoteOn(root + offset + currentChord->notes[i], 127, 1);
    // }
    playing = true;
    drawScreen();
}

void drawScreen() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(root);
    lcd.print(" ");
    lcd.print(noteNames[(root) % (sizeof(noteNames) / sizeof(noteNames[0]))]);

    lcd.setCursor(15, 0);
    switch (encoderMode) {
        case SCALE:
            lcd.print("S");
            break;
        case NOTE:
            lcd.print("N");
            break;
        case OCTAVE:
            lcd.print("O");
            break;
    }

    lcd.setCursor(0, 1);
    if (playing) {
        lcd.print(noteNames[(root + offset) % (sizeof(noteNames) / sizeof(noteNames[0]))]);
        lcd.print(" ");
    } else {
        lcd.print("                ");
    }

    // lcd.print(" ");
    // // lcd.print(scales[currentScale].name);
    // lcd.setCursor(1, 1);
    // // lcd.print(noteNames[(root + currentOffset) % (sizeof(noteNames) / sizeof(noteNames[0]))]);
    // lcd.print(" ");
    // lcd.print(currentChord->name);
}
