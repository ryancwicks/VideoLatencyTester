
/**
 * Video Latency Tester
 * 
 * This tool measures video latency from camera to screen.
 * 
 * Wiring:
 * 
 * I2C CLK - Pin A5
 * I2C SDA - Pin A4
 * 
 * LEDOUT - Pin 3
 * Photodiode in - Pin 2
 * 
 * You will need to install the RingBuffer, AdaFruit GFX and AdaFruit SSD1306 libraries.
 * 
 * Since I bought a KeyYees Arduino Nano V3, I need to set the board to Arduino Nano and the bootloader to the ATMege 328 (Old Bootloader) to get it to program.
 * 
 * The cheap OLED's I bought from Amazon use 7 bit I2C addressing, so the I2C address 0s 0x3C, rather than 0x3D.
 * 
 * The photodiode I used uses negative logic.
 */

#include <RingBuf.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const int SCREEN_WIDTH=128; // OLED display width, in pixels
const int SCREEN_HEIGHT=64; // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C 

const int LED_OUT = 3;
const int LED_OUT_2 = 13;
const int PD_IN = 2;

volatile unsigned long start_time;
volatile unsigned long last_time;
volatile bool triggered;
volatile RingBuf<unsigned long, 10> average_times;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void stop_timer () {
  if (triggered) {
    unsigned long now = micros();
    if (now < start_time) {
      last_time = 0; //Too short a measurement, or rollover has happened.
    }
    last_time = now - start_time - 10000;
  
    if (average_times.isFull()) {
      unsigned long temp;
      average_times.pop(temp);
    }
    average_times.push(last_time);
    //noInterrupts();
    triggered = false;
  }
}

void update_display() {
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font


  display.println("    Latency Tester");
  display.println("");
  display.println("Latency (ms): ");
  display.print("   ");
  display.println(static_cast<float>(last_time)/1000., DEC);
  unsigned long ave_latency = 0;
  for (int i = 0; i < average_times.size(); ++i) {
    ave_latency += average_times[i];
  }
  if (!average_times.isEmpty()) {
    ave_latency /= average_times.size();
  }
  display.println("Mean Latency (ms): ");
  display.print("   ");
  display.println(static_cast<float>(ave_latency)/1000., DEC);

  display.display();
}

void setup() {

  average_times.clear();
  last_time = 0;
  // put your setup code here, to run once:
  Serial.begin (115200);
  
  pinMode (LED_OUT, OUTPUT);
  digitalWrite(LED_OUT, LOW);
  pinMode (PD_IN, INPUT_PULLUP);
  pinMode (LED_OUT_2, OUTPUT);
  digitalWrite(LED_OUT_2, LOW);


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.display();
  triggered = false;
  attachInterrupt(digitalPinToInterrupt (PD_IN), stop_timer, FALLING);
}

void loop() {
  update_display();
  
  start_time = micros();
  triggered = true;
  //interrupts();
  delay(10);
  digitalWrite(LED_OUT_2, HIGH);
  digitalWrite(LED_OUT, HIGH); 
  delay(100);
  digitalWrite(LED_OUT, LOW);
  digitalWrite(LED_OUT_2, LOW);
  delay(900);
  Serial.println(last_time, DEC);
}
