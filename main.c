#define F_CPU 16000000
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <string.h>
#include "notes.h"
#define getName(var) #var
#define ANALOG_INPUT_PORT 0
#define SPEAKER_PORT PORTB
#define SPEAKER_DDR DDRB
#define SPEAKER_PIN 3
#define E 0
#define RS 1
#define LCDPORT PORTD
#define LCDCTRL PORTB
//#define LCDPORT PORTB
//#define LCDCTRL PORTC
int notesList2[90] = { 0, 31, 33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58, 62,
	65, 69, 73, 78, 82, 87, 93, 98, 104, 110, 117, 123, 131, 139, 147, 156,
	165, 175, 185, 196, 208, 220, 233, 247, 262, 277, 294., 311, 330, 349,
	370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784,
	831, 880, 932, 988, 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568,
	1661, 1760, 1865, 1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136,
3322, 3520, 3729, 3951, 4186, 4435, 4699, 4978 };
int myMelody[100];
char notesNames[90][4] = { "H", "B0", "C1", "CS1", "D1", "DS1", "E1", "F1",
	"FS1", "G1", "GS1", "A1", "AS1", "B1", "C2", "CS2", "D2", "DS2", "E2",
	"F2", "FS2", "G2", "GS2", "A2", "AS2", "B2", "C3", "CS3", "D3", "DS3",
	"E3", "F3", "FS3", "G3", "GS3", "A3", "AS3", "B3", "C4", "CS4", "D4",
	"DS4", "E4", "F4", "FS4", "G4", "GS4", "A4", "AS4", "B4", "C5", "CS5",
	"D5", "DS5", "E5", "F5", "FS5", "G5", "GS5", "A5", "AS5", "B5", "C6",
	"CS6", "D6", "DS6", "E6", "F6", "FS6", "G6", "GS6", "A6", "AS6", "B6",
	"C7", "CS7", "D7", "DS7", "E7", "F7", "FS7", "G7", "GS7", "A7", "AS7",
	"B7", "C8", "CS8", "D8", "DS8"
};
//Underworld melody
float underworld_melody[] = {
	NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
	NOTE_AS3, NOTE_AS4, 0, 0,
	NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
	NOTE_AS3, NOTE_AS4, 0, 0,
	NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
	NOTE_DS3, NOTE_DS4, 0, 0,
	NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
	NOTE_DS3, NOTE_DS4, 0, 0, NOTE_DS4, NOTE_CS4, NOTE_D4,
	NOTE_CS4, NOTE_DS4,
	NOTE_DS4, NOTE_GS3,
	NOTE_G3, NOTE_CS4,
	NOTE_C4, NOTE_FS4, NOTE_F4, NOTE_E3, NOTE_AS4, NOTE_A4,
	NOTE_GS4, NOTE_DS4, NOTE_B3,
NOTE_AS3, NOTE_A3, NOTE_GS3, 0, 0, 0 };
//Underwolrd tempo
float underworld_tempo[] = { 12, 12, 12, 12, 12, 12, 6, 3, 12, 12, 12, 12, 12,
	12, 6, 3, 12, 12, 12, 12, 12, 12, 6, 3, 12, 12, 12, 12, 12, 12, 6, 6,
	18, 18, 18, 6, 6, 6, 6, 6, 6, 18, 18, 18, 18, 18, 18, 10, 10, 10, 10,
10, 10, 3, 3, 3 };
// ------ FUNCTION DECLARATIONS ------
void delay(int ms);
void LCDinit(void);
void clear(void);
void shiftl(void);
void shiftr(void);
void send(unsigned char data);
void init(void);
void playNote(float duration, float frequency);
void playMelody();
//void playMelody(float *melody, float *tempo);
void syren();
void goToSecondLine(void);
void playMyMelody();
//-------- GLOBAL VARIABLES ---------------
volatile char mode = 0;
volatile char wasTriggered = 0;
ISR(ADC_vect) { // Executed every time an ADC conversion finishes
	// The next ADC conversion restarts immediately
	uint8_t read;
	read = ADCH; // reads first 8 bit, bits are left allined, so ADCH contains the 8MSB
	// last two bits are discard. read goes from 0 (0V) to 255 (5V)
	while (!ADIF) {
	}
	//read = ADCH;
	if (175 < read && read < 185) { // select
		mode = 1;
		wasTriggered = 1;
		} else if (115 < read && read < 125) { //left
		mode = 2;
		wasTriggered = 1;
		} else if (0 < read && read < 5) { //right
		mode = 3;
		wasTriggered = 1;
		} else if (71 < read && read < 81) { // down
		mode = 4;
		wasTriggered = 1;
		} else if (5 < read && read < 38) { // up
		mode = 5;
		wasTriggered = 1;
		} else if (240 < read) {
		mode = 0;
		wasTriggered = 0;
	}
}
void displayString(char *word) {
	clear();
	int textlen = strlen(word);
	//shiftr();
	for (int r = 0; r < textlen; r++) {
		send(word[r]);
	}
}
void displayStringLower(char *word) {
	goToSecondLine();
	int textlen = strlen(word);
	//shiftr();
	for (int r = 0; r < textlen; r++) {
		send(word[r]);
	}
}
int main() {
	init();
	displayString("Select mode:");
	displayStringLower("Composer|Player");
	int isPlayer = 0;
	int whichNote = 0;
	char buffer[10];
	int myMelodyNote = 0;
	int wantToPlay = 0;
	while (1) { // main loop
		while(isPlayer == 0){
			switch (mode) {
				case 2: //left
				isPlayer = 1;
				sprintf(buffer, "Selected: %s", "Composer");
				displayString(buffer);
				break;
				case 3: // right
				sprintf(buffer, "Selected: %s", "Player");
				displayString(buffer);
				isPlayer = 2;
				break;
			}
		}
		if(isPlayer <= 1){ //COMPOSER MODE
			_delay_ms(100);
			if (wasTriggered == 1) {
				switch (mode) {
					case 1:
					//displayString("Select");
					//displayString(notesNames[whichNote]);
					//playNote(500, notesList2[40]);
					//playMelody(underworld_melody, underworld_tempo);
					//playMelody();
					//syren();
					myMelody[myMelodyNote] = notesList2[whichNote];
					sprintf(buffer, "Selected: %s", notesNames[whichNote]);
					displayString(buffer);
					myMelodyNote++;
					wantToPlay = 0;
					break;
					case 2:
					sprintf(buffer, "Value: %s", "Left");
					displayString(buffer);
					wantToPlay++;
					break;
					case 3:
					sprintf(buffer, "%s", "Play Music?");
					clear();
					displayString(buffer);
					sprintf(buffer, "%s", "Left - accept");
					displayStringLower(buffer);
					_delay_ms(200);
					break;
					case 4:
					//displayString("Down");
					if (whichNote > 0)
					whichNote--;
					_delay_ms(200);
					wantToPlay = 0;
					playNote(20, notesList2[whichNote]);
					wasTriggered = 0;
					sprintf(buffer, "Value: %s", notesNames[whichNote]);
					displayString(buffer);
					break;
					case 5:
					//displayString("Up");
					if (whichNote < 90)
					whichNote++;
					_delay_ms(200);
					wantToPlay = 0;
					playNote(20, notesList2[whichNote]);
					wasTriggered = 0;
					sprintf(buffer, "Value: %s", notesNames[whichNote]);
					displayString(buffer);
					break;
					default:
					break;
				}
				if (wantToPlay > 0) {
					sprintf(buffer, "%s", "Playing");
					displayString(buffer);
					playMyMelody();
					wantToPlay = 0;
				}
			}
			} else if (isPlayer == 2) { // PLAYER MODE
			if (wasTriggered) {
				sprintf(buffer, "%s", "Player mode");
				displayString(buffer);
				sprintf(buffer, "%s", "Click Select");
				displayStringLower(buffer);
				switch (mode) {
					case 1:
					sprintf(buffer, "%s", "Underworld (up)");
					displayString(buffer);
					sprintf(buffer, "%s", "Siren (down)");
					displayStringLower(buffer);
					wasTriggered = 0;
					_delay_ms(200);
					break;
					case 2: // LEFT
					sprintf(buffer, "%s", "Going back");
					displayString(buffer);
					sprintf(buffer, "%s", "to main menu");
					displayStringLower(buffer);
					wasTriggered = 0;
					_delay_ms(200);
					isPlayer = 0;
					break;
					case 3:
					break;
					case 4: // UP
					syren();
					break;
					case 5: // DOWN
					playMelody();
					break;
				}
			}
		}
	}
}
// -------------- FUNCIONS ----------------
void syren() {
	int size = sizeof(notesList2) / sizeof(int);
	for (int thisNote = 0; thisNote < size; thisNote++) {
		playNote(20, notesList2[thisNote]);
	}
	for (int thisNote = size; thisNote > 1; thisNote--) {
		playNote(20, notesList2[thisNote]);
	}
}
void playMyMelody(){
	char buffer[10];
	int size = sizeof(myMelody) / sizeof(int);
	for (int thisNote = 0; thisNote < size; thisNote++) {
		playNote(20, myMelody[thisNote]);
		sprintf(buffer, "Note [Hz]: %i", myMelody[thisNote]);
		displayStringLower(buffer);
	}
}
void init(void) {
	cli();
	// clears interrupts
	// DDRC = 0xFF; // PCx all outputs
	// DDRB = 0xFF; // PBx all outputs
	//DDRB |= _BV(PINB4); // led is on pin 4
	DDRD = 0xFF;
	DDRB = 0xFF;
	// Inits ADC
	ADCSRA = 0; // Resets ADC
	ADCSRB &= ~(_BV(ADTS2) | _BV(ADTS1) | _BV(ADTS0)); // sets in free running mode
	ADCSRA |= _BV(ADEN) | _BV(ADATE); // Enable the ADC, enables auto triggering
	ADCSRA |= _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // Set prescaler to 128 (one ADC conversion will need about 104us)
	ADCSRA |= _BV(ADIE); // enables interrupts for ADC
	ADMUX |= _BV(REFS0) * 0 | _BV(REFS1) * 0; // 5V reference voltage
	ADMUX |= _BV(ADLAR); // Right justify the result
	ADMUX |= ANALOG_INPUT_PORT; // Sets ADC Port
	DIDR0 = 0; // disable input on ADC pins (leaving enabled the ADC muxer)
	sei();
	// sets interrupts
	ADCSRA |= _BV(ADSC); // starts conversion
	LCDinit();
	LCDCTRL |= (1 << RS);
}
void delay(int ms) {
	ms /= 100;
	char i;
	for (i = 0; i < ms; i++) {
		_delay_ms(100); // max is 262.14 ms / F_CPU in MHz
	}
}
void send(unsigned char data) {
	LCDCTRL |= (1 << E);
	LCDPORT = (data);
	_delay_loop_2(500);
	LCDCTRL &= ~(1 << E);
	_delay_loop_2(500);
	LCDCTRL |= (1 << E);
	LCDPORT = (data << 4);
	_delay_loop_2(500);
	LCDCTRL &= ~(1 << E);
	_delay_loop_2(500);
}
void LCDinit(void) {
	_delay_loop_2(1000);
	//_delay_loop_2(100000);
	LCDCTRL &= ~(1 << RS);
	send(0x33);
	send(0x32);
	send(0x28);
	send(0x06); //function set 2 lines N=1 F=0
	send(0x0F); //cursor on
	send(0x01); //clear
	_delay_loop_2(1000);
	//_delay_loop_2(100000);
	// send(0x31);//show 0
}
void clear(void) {
	LCDCTRL &= ~(1 << RS);
	send(0x01);
	_delay_loop_2(5000);
	//_delay_loop_2(500000);
	LCDCTRL |= (1 << RS);
}
void shiftl(void) {
	LCDCTRL &= ~(1 << RS);
	send(0x10);
	_delay_loop_2(5000);
	//_delay_loop_2(5000);
	LCDCTRL |= (1 << RS);
}
void shiftr(void) {
	LCDCTRL &= ~(1 << RS);
	send(0x14);
	_delay_loop_2(5000);
	//_delay_loop_2(5000);
	LCDCTRL |= (1 << RS);
}
void goToSecondLine(void) {
	LCDCTRL &= ~(1 << RS);
	send(0xC0);
	_delay_loop_2(5000);
	//_delay_loop_2(500000);
	LCDCTRL |= (1 << RS);
}
void playNote(float duration, float frequency) {
	// Physics variables
	long int i, cycles;
	float half_period;
	float wavelength;
	wavelength = (1 / frequency) * 1000;
	// Standard physics formula.
	cycles = duration / wavelength;
	// The number of cycles.
	half_period = wavelength / 2;
	// The time between each toggle.
	//Parameters calculation based of arduino code
	/* half_period = 1000000 / frequency / 2;
	cycles = frequency * duration / 1000;*/
	// Data direction register Pin 7
	// is set for output.
	SPEAKER_DDR |= (1 << SPEAKER_PIN);
	for (i = 0; i < cycles; i++)
	// The output pin 7 is toggled
	// for the 'cycles'number of times.
	// --------------------------------
	{
		_delay_ms(half_period);
		// Wait 1 half wavelength.
		SPEAKER_PORT |= (1 << SPEAKER_PIN);
		// Output 5 V to port Pin 7.
		_delay_ms(half_period);
		// Wait 1 half wavelength.
		SPEAKER_PORT &= ~(1 << SPEAKER_PIN);
		// 0 V at port pin 7.
	}
	return;
}
//void playMelody(float melody[], float tempo[]) {
void playMelody() {
	// if (tempo == 0)
	// tempo = 15;
	int size = sizeof(underworld_melody) / sizeof(float);
	for (int thisNote = 0; thisNote < size; thisNote++) {
		// to calculate the note duration, take one second
		// divided by the note type.
		//e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
		float noteDuration = 1000 / underworld_tempo[thisNote];
		//buzz(melodyPin, underworld_melody[thisNote], noteDuration);
		playNote(noteDuration * 0.5, underworld_melody[thisNote]);
		// to distinguish the notes, set a minimum time between them.
		// the note's duration + 30% seems to work well:
		float pauseBetweenNotes = noteDuration * 1.30;
		_delay_ms(pauseBetweenNotes);
		// stop the tone playing:
		//buzz(melodyPin, 0, noteDuration);
		playNote(noteDuration, 0);
	}
}