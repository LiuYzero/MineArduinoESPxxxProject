#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Wire.h>

TFT_eSPI TFT = TFT_eSPI();
TFT_eSprite eSprite = TFT_eSprite(&TFT);
void setup() {
    TFT.init();
    TFT.setRotation(1);
    TFT.fillScreen(TFT_BLACK);
    TFT.initDMA();
    eSprite.setTextFont(1);
    eSprite.setTextColor(TFT_WHITE);
    eSprite.createSprite(240, 135);
}

void loop() {
    eSprite.setTextFont(1);
    eSprite.setCursor(20, 20);
    eSprite.println("Hello World\n");
    eSprite.pushSprite(0, 0);
}