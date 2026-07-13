#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "BluetoothSerial.h"
#include <ESP32Servo.h>

//be confirm that bluetooth module is working
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled!
#endif

// for display
#define screen_width 128
#define screen_height 64
#define oled_reset -1
#define screen_address 0x3C

// display initialization
Adafruit_SSD1306 display(screen_width, screen_height, &Wire, oled_reset);

// gpio pins
#define push_button_pin 4
#define light_pin 2
#define green_led_pin 5
#define red_led_pin 18
#define buzzer_pin 26
#define servo_motor_pin 15

// bluetooth pins
const String door_lock_pass = "9";   // door PIN 
const String light_command  = "L";   // command for light


bool lightState = false;
bool doorOpen = false;
bool lastButtonState = HIGH;
bool lastBTState = false;
bool waitingForPin = false;

String displayMessage = "";
String btMessage = "";

bool btMessageActive = false;
unsigned long btMessageTimer = 0;
const unsigned long BT_MESSAGE_DURATION = 1000;

unsigned long currentMillis = 0;

bool greenLedActive = false;
unsigned long greenLedTimer = 0;
const unsigned long GREEN_LED_DURATION = 1000;

bool alarmActive = false;
unsigned long alarmTimer = 0;
const unsigned long ALARM_DURATION = 3000;

unsigned long messageTimer = 0;
const unsigned long MESSAGE_DURATION = 3000;

// bluetooth and servo initialization
BluetoothSerial SerialBT;
Servo doorServo;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_Smart_Hub");

  lastBTState = SerialBT.hasClient();

  // pin modes
  pinMode(push_button_pin, INPUT_PULLUP);
  pinMode(light_pin, OUTPUT);
  pinMode(green_led_pin, OUTPUT);
  pinMode(red_led_pin, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);

  // make everything off initially
  digitalWrite(light_pin, LOW);
  digitalWrite(green_led_pin, LOW);
  digitalWrite(red_led_pin, LOW);
  digitalWrite(buzzer_pin, LOW);

  // do the servo Setup
  doorServo.setPeriodHertz(50);
  doorServo.attach(servo_motor_pin, 500, 2400);
  doorServo.write(0); // locked position

  // ensure the oled displaynis okay
  if (!display.begin(SSD1306_SWITCHCAPVCC, screen_address)) {
    Serial.println(F("OLED failed"));
    for (;;);
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  currentMillis = millis();

  // Bluetooth Connection Status, if connected or not
  bool currentBTState = SerialBT.hasClient();
  if (currentBTState != lastBTState) {
    lastBTState = currentBTState;
    btMessageActive = true;
    btMessageTimer = currentMillis;

    if (currentBTState) {
      btMessage = "BT\nCONNECTED";
    } else {
      btMessage = "BT\nDISCONNECTED";
    }
  }

  // Push Button logic for physically on off
  bool currentButtonState = digitalRead(push_button_pin);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    lightState = !lightState;
    digitalWrite(light_pin, lightState ? HIGH : LOW);
    delay(50); // simple debounce
  }
  lastButtonState = currentButtonState;

  // bluetooth
  while (SerialBT.available()) {
    char c = SerialBT.read();

    Serial.print("Received: ");
    Serial.println((int)c);

    // Ignore line endings
    if (c == '\n' || c == '\r') {
      continue;
    }

    if (waitingForPin) {
      waitingForPin = false;

      if (c == '9') {
        // correct door pass
        doorOpen = !doorOpen;
        doorServo.write(doorOpen ? 90 : 0);

        greenLedActive = true;
        greenLedTimer = currentMillis;
        digitalWrite(green_led_pin, HIGH);

        displayMessage = "PIN CORRECT\nDOOR MOVED";
        messageTimer = currentMillis;
      } else {
        // wrong PIN
        alarmActive = true;
        alarmTimer = currentMillis;
        digitalWrite(buzzer_pin, HIGH);

        displayMessage = "WRONG PIN!";
        messageTimer = currentMillis;
      }

      continue;
    }

    // Normal mode
    if (c == 'L' || c == 'l') {
      // Light turning on or off
      lightState = !lightState;
      digitalWrite(light_pin, lightState ? HIGH : LOW);

      displayMessage = lightState ? "LIGHT ON" : "LIGHT OFF";
      messageTimer = currentMillis;
    }
    else if (c == '1') {
      // pin mode activated
      waitingForPin = true;
      displayMessage = "INPUT PIN";
      // no timer; stay on screen until next command
    }
    else {
      // Unknown command
      alarmActive = true;
      alarmTimer = currentMillis;
      digitalWrite(buzzer_pin, HIGH);

      displayMessage = "WRONG COMMAND";
      messageTimer = currentMillis;
    }
  }


  // Green LED timeout
  if (greenLedActive && (currentMillis - greenLedTimer >= GREEN_LED_DURATION)) {
    greenLedActive = false;
    digitalWrite(green_led_pin, LOW);
  }


  // Alarm timeout and red LED blinking
  if (alarmActive) {
    if (currentMillis - alarmTimer >= ALARM_DURATION) {
      alarmActive = false;
      digitalWrite(buzzer_pin, LOW);
      digitalWrite(red_led_pin, LOW);
    } else {
      if ((currentMillis / 250) % 2 == 0) {
        digitalWrite(red_led_pin, HIGH);
      } else {
        digitalWrite(red_led_pin, LOW);
      }
    }
  }

  // Clearing expired messages 
  if (btMessageActive && (currentMillis - btMessageTimer >= BT_MESSAGE_DURATION)) {
    btMessageActive = false;
    btMessage = "";
  }

  if (!waitingForPin && displayMessage != "INPUT PIN" &&
      displayMessage != "" &&
      (currentMillis - messageTimer >= MESSAGE_DURATION)) {
    displayMessage = "";
  }




  // Display Logic

  display.clearDisplay();


  if (btMessageActive) {
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 15);
    display.println(btMessage);
  }

  else if (waitingForPin) {
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 15);
    display.println("INPUT PIN");
  }

  else if (displayMessage != "" && displayMessage != "INPUT PIN") {
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 15);
    display.println(displayMessage);
  }



  // how the Default screen look like
  else {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("-- HOME AUTOMATION --");

    display.setCursor(0, 20);
    display.setTextSize(1.75);
    if (doorOpen) {
      display.println("DOOR: OPEN");
    } else {
      display.println("DOOR: LOCKED");
    }

    display.setCursor(0, 45);
    if (lightState) {
      display.println("LIGHT: ON");
    } else {
      display.println("LIGHT: OFF");
    }
  }

  display.display();
}