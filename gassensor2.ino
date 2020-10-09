#include <Adafruit_DotStar.h>

#define NUMPIXELS 1 // Number of LEDs in strip
 
// Here's how to control the LEDs from any two pins:
#define DATAPIN    7
#define CLOCKPIN   8
Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/Org_01.h>

const int pwmPin = 1;
unsigned long startTime = millis();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
  strip.begin();
  strip.show();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  display.setFont(&Org_01);
  pinMode(pwmPin, INPUT_PULLUP);
  Serial.println("MHZ test");

}

void loop() {
  Serial.println("------------------------------");
  Serial.print("Time from start: ");
  Serial.print((millis() - startTime) / 1000);
  Serial.println(" s");
  int ppm_uart = readCO2UART();
  int ppmPWM = readPPMPWM();
  Serial.print(ppmPWM); 
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("CO2 PPM:");
  display.setTextSize(5);
  display.setCursor(0, 50);
  display.println(ppmPWM);
  display.display();      // Show initial text
  delay(100);
  //calibrate();
  //delay(7000);
  //Serial.print("Done Calibration");
}

int readCO2UART(){
  byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
  byte response[9]; // for answer

  Serial.println("Sending CO2 request...");
  Serial1.write(cmd, 9); //request PPM CO2

  // clear the buffer
  memset(response, 0, 9);
  int i = 0;
  while (Serial1.available() == 0) {
//    Serial.print("Waiting for response ");
//    Serial.print(i);
//    Serial.println(" s");
    delay(1000);
    i++;
  }
  if (Serial1.available() > 0) {
      Serial1.readBytes(response, 9);
  }
  // print out the response in hexa
  for (int i = 0; i < 9; i++) {
    Serial.print(String(response[i], HEX));
    Serial.print("   ");
  }
  Serial.println("");

  // checksum
  //byte check = getCheckSum(response);
  //if (response[8] != check) {
  //  Serial.println("Checksum not OK!");
  //  Serial.print("Received: ");
  //  Serial.println(response[8]);
  //  Serial.print("Should be: ");
  //  Serial.println(check);
  //}
 
  // ppm
  int ppm_uart = 256 * int(response[2]) + int(response[3]);
  Serial.print("PPM UART: ");
  Serial.println(ppm_uart);

  // temp
  byte temp = response[4] - 40;
  Serial.print("Temperature? ");
  Serial.println(temp);

  // status
  byte status = response[5];
  Serial.print("Status? ");
  Serial.println(status);
  if (status == 0x40) {
    Serial.println("Status OK");
  }
 
  return ppm_uart;
}

void calibrate()
{
  byte cmdCal[9] = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78};  // calibrate command
  Serial.write(cmdCal, 9);
  delay(3000);
}

int readPPMPWM() {
  while (digitalRead(pwmPin) == LOW) {}; // wait for pulse to go high
  long t0 = millis();
  while (digitalRead(pwmPin) == HIGH) {}; // wait for pulse to go low
  long t1 = millis();
  while (digitalRead(pwmPin) == LOW) {}; // wait for pulse to go high again
  long t2 = millis();
  long th = t1-t0;
  long tl = t2-t1;
  long ppm = 5000L * (th - 2) / (th + tl - 4);
  while (digitalRead(pwmPin) == HIGH) {}; // wait for pulse to go low
  delay(10); // allow output to settle.
  return int(ppm);
}
