#include <Arduino.h>

/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 by ThingPulse, Daniel Eichhorn
 * Copyright (c) 2018 by Fabrice Weinberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ThingPulse invests considerable time and money to develop these open source libraries.
 * Please support us by buying our products (and not the clones) from
 * https://thingpulse.com
 *
 */

#include "configuration.h"

 // Include the correct display library
 // For a connection via I2C using Wire include
 #include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
 #include <SSD1306Wire.h> // legacy include: `#include "SSD1306.h"`
 // or #include "SH1106Wire.h", legacy include: `#include "SH1106.h"`
 // For a connection via I2C using brzo_i2c (must be installed) include
 // #include <brzo_i2c.h> // Only needed for Arduino 1.6.5 and earlier
 // #include "SSD1306Brzo.h"
 // #include "SH1106Brzo.h"
 // For a connection via SPI include
 // #include <SPI.h> // Only needed for Arduino 1.6.5 and earlier
 // #include "SSD1306Spi.h"
 // #include "SH1106SPi.h"

//#include <OLEDDisplay.h>

// Include the UI lib
#include <OLEDDisplayUi.h>

#include <OneButton.h>

// Include custom images

#include "images.h"

// Initialize the OLED display using Wire library

//#define MY_OLED_SDA (4)
//#define MY_OLED_SCL (15)
//#define MY_OLED_RST (16)


SSD1306Wire  display(0x3c, I2C_SDA, I2C_SCL);
//SSD1306Wire  display(0x3c, MY_OLED_SDA, MY_OLED_SCL);

//SH1106Wire display(0x3c, D3, D5);

OLEDDisplayUi ui     ( &display );

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 0, String(millis()));
}

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y

  display->drawXbm(x + 34, y + 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Demonstrates the 3 included default sizes. The fonts come from SSD1306Fonts.h file
  // Besides the default fonts there will be a program to convert TrueType fonts into this format
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 10 + y, "Arial 10");

  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 20 + y, "Arial 16");

  display->setFont(ArialMT_Plain_24);
  display->drawString(0 + x, 34 + y, "Arial 24");
}

void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Text alignment demo
  display->setFont(ArialMT_Plain_10);

  // The coordinates define the left starting point of the text
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 11 + y, "Left aligned (0,10)");

  // The coordinates define the center of the text
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 22 + y, "Center aligned (64,22)");

  // The coordinates define the right end of the text
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128 + x, 33 + y, "Right aligned (128,33)");
}

void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Demo for drawStringMaxWidth:
  // with the third parameter you can define the width after which words will be wrapped.
  // Currently only spaces and "-" are allowed for wrapping
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, "Lorem ipsum\n dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore.");
}

void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    display->setFont(ArialMT_Plain_10);

  // The coordinates define the left starting point of the text
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 11 + y, "Merry XMAS June");

  // The coordinates define the center of the text
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 22 + y, "Love ya.....");


}


class ButtonThread
{
// Prepare for button presses
    OneButton userButton;

    static bool shutdown_on_long_stop;

  public:
    static uint32_t longPressTime;

    // callback returns the period for the next callback invocation (or 0 if we should no longer be called)
    ButtonThread()
    {
        userButton = OneButton(BUTTON_PIN, true, true);

        userButton.attachClick(userButtonPressed);
        userButton.attachDuringLongPress(userButtonPressedLong);
        userButton.attachDoubleClick(userButtonDoublePressed);
        userButton.attachLongPressStart(userButtonPressedLongStart);
        userButton.attachLongPressStop(userButtonPressedLongStop);
    }

    void Tick(){
      userButton.tick();
    }

  protected:
    /// If the button is pressed we suppress CPU sleep until release
    int32_t runOnce()
    {
        userButton.tick();
        return 5;
    }

  private:
    static void userButtonPressed()
    {
      Serial.println("press!\n");
      ui.nextFrame();
    }

    static void userButtonPressedLong()
    {
        Serial.println("Long press!\n");

        // If user button is held down for 5 seconds, shutdown the device.
        if (millis() - longPressTime > 5 * 1000) {
          Serial.println("Long press > 5 secs!\n");
        } else {
            Serial.printf("Long press %u\n", (uint32_t)(millis() - longPressTime));
        }
    }

    static void userButtonDoublePressed()
    {
      Serial.println("Double press\n");
      ui.previousFrame();
    }

    static void userButtonPressedLongStart()
    {
        Serial.println("Long press start!\n");
        longPressTime = millis();
    }

    static void userButtonPressedLongStop()
    {
        Serial.println("Long press stop!\n");
        longPressTime = 0;
    }
};




// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = { drawFrame1, drawFrame2, drawFrame3, drawFrame4, drawFrame5 };

// how many frames are there?
int frameCount = 5;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;


static ButtonThread  *buttonThread;
uint32_t ButtonThread::longPressTime = 0;

void setup() {
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

  Serial.println();

  buttonThread = new ButtonThread();  // create a new button to move thru screens


  #ifdef RESET_OLED
      pinMode(RESET_OLED, OUTPUT);
      digitalWrite(RESET_OLED, 1);
  #endif


	// The ESP is capable of rendering 60fps in 80Mhz mode
	// but that won't give you much time for anything else
	// run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(10);

	// Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Initialising the UI will init the display too.

  ui.init();

  display.flipScreenVertically();

      // Require presses to switch between frames.
  ui.disableAutoTransition();

}


void loop() {
  int remainingTimeBudget = ui.update();
  buttonThread->Tick();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }
}
