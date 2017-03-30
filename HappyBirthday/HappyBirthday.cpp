//
// Created by Andri Yadi on 3/29/17.
//

#include <Arduino.h>
#include <ESPectro.h>

ESPectro_Button button;

#define SPEAKER_GPIO_NO 15
#define MIC_IN_GPIO_NO  5 //2 --> 2 is used by Neopixel, so I choose GPIO 5 for mic interrupt pin

int melody[]= {196,196,220,196,262,247,196,196,220,196,294,262,196,196,392,330,262,247,220,349,349,330,262,294,262};
int noteDurations[] = {8,8,4,4,4,2,8,8,4,4,4,2,8,8,4,4,4,4,3,8,8,4,4,4,2};

#define MELODY_COUNT sizeof(melody)/sizeof(int)

volatile bool blowed = false;
volatile bool musicStopped = false;

int currentNoteIdx = 0;
uint32_t startPlay = 0;

ESPectro_Neopixel_Default neopixel;

void turnOffNeo() {
    for(uint16_t i = 0; i < neopixel.PixelCount(); i++) {
        neopixel.turnOff(i);
    }
}

/*
void playTone(int _pin, int _frequency, int _length){
    analogWriteFreq(_frequency);
    analogWrite(_pin, 512);
    delay(_length);
    analogWrite(_pin, 0);
}
*/

void playMusic(int thisNote) {

    int noteDuration = 1000/noteDurations[thisNote];
    tone(SPEAKER_GPIO_NO, melody[thisNote], noteDuration * 1.5);
    //playTone(SPEAKER_GPIO_NO, melody[thisNote], noteDuration * 1.5);
    int pauseBetweenNotes = noteDuration * 2;//1.60;
    delay(pauseBetweenNotes);
    noTone(SPEAKER_GPIO_NO);

}

void stopMusic() {
    // Ignore any stopping attempt before 3 seconds, optional though,
    if (millis() - startPlay < 3000) {
        return;
    }

    musicStopped = true;
    turnOffNeo();

    digitalWrite(SPEAKER_GPIO_NO, LOW);
}

void onButtonUp() {
    //Stop music when button is pressed
    stopMusic();
}

void interrupted() {
    blowed = true;
}

void setup(){
    Serial.begin(115200);

    pinMode(SPEAKER_GPIO_NO, OUTPUT);
    digitalWrite(SPEAKER_GPIO_NO, LOW);

    //Attach interrupt to detect when mic is blowed
    pinMode(MIC_IN_GPIO_NO, INPUT);
    attachInterrupt(MIC_IN_GPIO_NO, interrupted, RISING);

    button.begin();

    //Register event handler for "button up" event to stop music manually
    button.onButtonUp(onButtonUp);

    neopixel.Begin();

    //Gradient color
    for(uint16_t idx = 0; idx < neopixel.PixelCount(); idx++) {
        float hue = 360.0f - ((idx * 1.0f / neopixel.PixelCount()) * 90);
        HslColor color = HslColor(hue / 360.0f, 1.0f, 0.5f);
        neopixel.SetPixelColor(idx, color);
    }

    neopixel.Show();

    startPlay = millis();
}

void loop(){

    button.run();

    if (blowed) {
        blowed = false;
        stopMusic();
    }

    if (!musicStopped) {
        playMusic(currentNoteIdx);
        currentNoteIdx++;

        if (currentNoteIdx >= MELODY_COUNT) {
            delay(1000);
            currentNoteIdx = 0; //delay 1s and replay
        }
    }
}