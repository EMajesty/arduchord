#include "Arduino.h"
#include "pins_arduino.h"
#include <USB-MIDI.h>
#include <Keypad.h>
#include <Wire.h>
#include <avr/pgmspace.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <EncoderButton.h>

USBMIDI_CREATE_DEFAULT_INSTANCE();

hd44780_I2Cexp lcd;

const int LCD_COLS = 20;
const int LCD_ROWS = 4;

byte root = 60;

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

byte currentScale = 2;
Chord* currentChord = &recipes[0];
bool mode = true;
byte currentOffset;

const String noteNames[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

const byte ROWS = 5;
const byte COLS = 4;

char keys[ROWS][COLS] = { // this shit is all the way fucked
    {'m', 'o', 'q', 's'},
    {'l', 'n', 'p', 'r'},
    {'a', 'k', 'j', 'h'},
    {'b', 'c', 'i', 'g'},
    {'t', 'd', 'e', 'f'},
};

/*
 *  ( m )                   ( a )       ( t )
 *      ( n )           ( l )   ( b )
 *  ( o )           ( k )           ( c )
 *      ( p )       ( j )           ( d )
 *  ( q )           ( i )           ( e )
 *      ( r )           ( h )   ( f )
 *  ( s )                   ( g )
 */

byte colPins[COLS] = {16, 14, 15, 18};
byte rowPins[ROWS] = {10, 9, 8, 7, 6};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// EncoderButton eb(4, 5);

void keypadEvent(KeypadEvent);
// void encoderEvent(EncoderButton& eb);
void killAll();
void playChord(byte);
void drawScreen();

void setup() {
    // pinMode(LED_BUILTIN, OUTPUT);
    MIDI.begin(4);
    keypad.addEventListener(keypadEvent);
    // eb.setEncoderHandler(encoderEvent);

    lcd.begin(LCD_COLS, LCD_ROWS);
    drawScreen();
}

void loop() {
    keypad.getKey();
    // eb.update();
}

void keypadEvent(KeypadEvent key) {
    switch (keypad.getState()) {
        case PRESSED:
            switch (key) {
                case 'h': // c
                    playChord(0);
                    break;
                case 'g': // g
                    playChord(7);
                    break;
                case 'i': // d
                    playChord(2);
                    break;
                case 'e': // a
                    playChord(9);
                    break;
                case 'd': // e
                    playChord(4);
                    break;
                case 't': // b
                    playChord(11);
                    break;
                case 'b': // gb
                    playChord(6);
                    break;
                case 'a': // db
                    playChord(1);
                    break;
                case 'c': // ab
                    playChord(8);
                    break;
                case 'k': // eb
                    playChord(3);
                    break;
                case 'j': // bb
                    playChord(10);
                    break;
                case 'r': // f
                    playChord(5);
                    break;

                
                case 'f':
                    //encoder
                    break;
            }
            // if (key < 12) {
            //     // currentOffset = scales[currentScale].notes[key][0];
            //     // playChord(&recipes[scales[currentScale].notes[key][1]]);
            //     playChord(key);
            //     drawScreen();
            // } 
            drawScreen();
            lcd.setCursor(0, 0);
            lcd.print(key);
            break;
        case RELEASED:
            break;
        case HOLD:
            break;
        case IDLE:
            break;
    }
}

// void encoderEvent(EncoderButton& eb) {
//     if (mode) {
//         currentScale += eb.increment();
//         if (currentScale >= 2) {
//             currentScale = 2;
//         } else if (currentScale <= 0) {
//             currentScale = 0;
//         }
//     } else {
//         root += eb.increment();
//         if (root >= 100) {
//             root = 100;
//         } else if (root <= 0) {
//             root = 0;
//         }
//     }
//     drawScreen();
// }

void killAll() {
    for (int i = 0; i < 127; i++) {
        MIDI.sendNoteOff(i, 0, 1);
    }
}

void playChord(byte offset) {
    killAll();
    for (uint i = 0; i < sizeof(currentChord->notes) / sizeof(currentChord->notes[0]); i++) {
        MIDI.sendNoteOn(root + offset + currentChord->notes[i], 127, 1);
    }
}

void drawScreen() {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print(noteNames[(root) % (sizeof(noteNames) / sizeof(noteNames[0]))]);
    lcd.print(" ");
    // lcd.print(scales[currentScale].name);
    lcd.setCursor(1, 1);
    // lcd.print(noteNames[(root + currentOffset) % (sizeof(noteNames) / sizeof(noteNames[0]))]);
    lcd.print(" ");
    lcd.print(currentChord->name);
    lcd.setCursor(1, 2);
    lcd.print(root);
    lcd.setCursor(15, 0);
    if (mode) {
        lcd.print("S");
    } else {
        lcd.print("C");
    }
}
