#include <Arduino.h>
#include <WiFi.h> // Connect to WiFi network
#include <EEPROM.h> // read and write from flash memory
// #include <WebServer.h> /.. Used AsyncWebServer instead
#include <ESP32Time.h> // Real Time library
#include <ESPAsyncWebServer.h> // Asynchronous Web Server Library
#include <SPIFFS.h> // File system library for internal ESP32 memory
#include <AsyncTCP.h> // Asynchronous TCP Library
#include <Adafruit_GFX.h> // Core graphics library
#include <Adafruit_SSD1306.h> // Hardware-specific library for SSD1306
#include <Wire.h> // I2C library
#include <ESP32Servo.h>


ESP32Time rtc(0); // Create a Real Time Clock object to be used throughout the code

#define i1 11
#define i2 9
#define BUTTON_PIN       21  // GIOP21 pin connected to button
#define LEDN_PIN          5
#define BUZZER_PIN       7  // GIOP22 pin connected to buzzer
#define LEVEL_PIN        13  // GIOP23 pin connected to level sensor

// EEPROM functions
void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0';
  return String(data);
}

void writeIntIntoEEPROM(int address, int number)
{ 
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}

int readIntFromEEPROM(int address)
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}
///////////////////////////////////////////////

void SetTime(String timeString){
  const char *time_details = timeString.c_str();
  struct tm tim;
  strptime(time_details, "%H:%M:%S", &tim);
  rtc.setTime(30, 24, 15, 17, 1, 2021);
  // Serial.println('Time set to: ');
  // Serial.println(tim.tm_hour);
}
/// ///////////////////////////////////////
void buzz(){
  for (int i = 0; i < 5; i++)
  {
    tone(BUZZER_PIN, 4100, 1000);
    delay(200);
  }
  
}

String processor(const String& var){
  Serial.println(var);
  if(var == "TIME"){ 
    return rtc.getTime();
  }
  return String();
}

// void display(){
//   // OLED display declaration
//   #define SCREEN_WIDTH 128 // OLED display width, in pixels
//   #define SCREEN_HEIGHT 64 // OLED display height, in pixels
//   #define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
//   Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//   // Initialize the OLED display
//   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
//     Serial.println(F("SSD1306 allocation failed"));
//     for(;;); // Don't proceed, loop forever
//   }
//   // Clear the buffer
//   display.clearDisplay();
//   // Draw a single pixel in white
//   display.drawPixel(10, 10, SSD1306_WHITE);
//   // Show the display buffer on the screen. You MUST call display() after
//   // drawing commands to make them visible on screen!
//   display.display();
//   delay(2000);
//   // Clear the buffer
//   display.clearDisplay();
//   // Draw a single pixel in white
//   display.drawPixel(10, 10, SSD1306_WHITE);
//   // Show the display buffer on the screen. You MUST call display() after
//   // drawing commands to make them visible on screen!
//   display.display();
//   delay(2000);
//   // Clear the buffer
//   display.clearDisplay();
//   // Draw a single pixel in white
//   display.drawPixel(10, 10, SSD1306_WHITE);
//   // Show the display buffer on the screen. You MUST call display() after
//   // drawing commands to make them visible on screen!
//   display.display();
//   delay(2000);
//   // Clear the buffer
//   display.clearDisplay();
//   // Draw a single pixel in white
//   display.drawPixel(10, 10, SSD1306_WHITE);
//   // Show the display buffer on the screen. You MUST call display() after
//   // drawing commands to make them visible on screen!
//   display.display();
//   delay(2000);
//   // Clear the buffer
//   display.clearDisplay();
//   // Draw a single pixel in white
//   display.drawPixel(10, 10, SSD1306_WHITE);
//   // Show the display buffer on the screen. You MUST call display() after
//   // drawing commands to make them visible on screen!
//   display.display();
//   delay(2000);
// }

///////////////////////////////////////////////
 // Handles web server page requests

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);


// Constants
const char* ssid = "SMart Bottle"; // Name of access point



// Global variables
AsyncWebServer  server(80); // Object of WebServer(HTTP port, 80 is defualt)
String header; // Variable to store HTTP request
// String outputState = "off"; // Stores LED state


#define SHORT_PRESS_TIME 2000 // 500 milliseconds
// Variables will change:
// the current reading from the input pin
unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;

xTaskHandle configHundle;
///////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////

const char* PARAM_INPUT_1 = "setTime";
const char* PARAM_INPUT_2 = "note";
const char* PARAM_INPUT_3 = "periods";
const char* PARAM_INPUT_4 = "Notice";



void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void StartServer(){
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input4 value on <ESP_IP>/get?input3=<inputMessage>
    if (request->hasParam(PARAM_INPUT_4)) {
      inputMessage = request->getParam(PARAM_INPUT_4)->value();
      inputParam = PARAM_INPUT_4;
      writeIntIntoEEPROM(3, inputMessage.toInt());
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
  
}

// xTask
void configTask(void *pvParameters){
     
  
  while(1){
    vTaskDelay(100);
  }

}
void ringTask(void *pvParameters){
  String ringTime = readStringFromEEPROM(3);
  

  while(1){
    if (ringTime == rtc.getTime()){
      buzz();
      for (int i = 0; i < 10; i++)
      {
        digitalWrite(BUILTIN_LED, HIGH);
        delay(500);
        digitalWrite(BUILTIN_LED, LOW);
        delay(250);
      }
      delay(readIntFromEEPROM(3)*60000);
    }
    vTaskDelay(100);
  }

}

// void longpress( void *pvParameters){
//   int lastState = LOW;  // the previous state frgom the input pin
//   int currentState;     // the current reading from the input pin
//   for(;;){
//    // the current reading from the input pin
//     // read the state of the switch/button:
//     currentState = digitalRead(BUTTON_PIN);

//     if (lastState == HIGH && currentState == LOW){       // button is pressed
//       digitalWrite(BUILTIN_LED, HIGH);
//       pressedTime = millis();
//       } else if (lastState == LOW && currentState == HIGH) { // button is released
//       releasedTime = millis();

//       long pressDuration = releasedTime - pressedTime;

//       if ( pressDuration < SHORT_PRESS_TIME )
//         Serial.println("A short press is detected");
//         if ( !configHundle == NULL ) vTaskDelete(configHundle);
//          xTaskCreate(configTask, "config", 10000, NULL, 1, &configHundle);
//     }

//       {
//         digitalWrite(BUILTIN_LED, HIGH);
//         delay(250);
//         digitalWrite(BUILTIN_LED, LOW);
//         delay(100);
//       }
      
//       for (int i = 0; i < 512; i++) {
//         digitalWrite(BUILTIN_LED, HIGH);
//         EEPROM.write(i, 0);
//         delay(500);
//         digitalWrite(BUILTIN_LED, LOW);
//         delay(100);
//         }
//       EEPROM.commit();
//       delay(2000);
//       // ESP.restart();

//   }
  
// }

///////////////////////////////////////////////////////////
void setup(){
  

  Wire.begin(i1, i2);

  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LEVEL_PIN, INPUT_PULLUP);
  pinMode(LEDN_PIN, OUTPUT);

  for (int i = 0; i < 10; i++)
  {
    digitalWrite(BUILTIN_LED, HIGH);
    delay(300);
    digitalWrite(BUILTIN_LED, LOW);
    delay(100);
  }
  WiFi.softAP(ssid);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(2000);
  StartServer();

  if(SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  Serial.begin(115200);
      xTaskCreate(ringTask, "ring", 10000, NULL, 1, NULL);
  
  SetTime(readStringFromEEPROM(5));
  
  // EEPROM.read(0) == 0
  // if (true){
  // }else{
  //   xTaskCreate(longpress, "longpress", 10000, NULL, 1, NULL);
  // }

      // xTaskCreate(configTask, "config", 10000, NULL, 1, &configHundle);

  // if (EEPROM.read(0) == 0){
    
  // }else{
  //   xTaskCreate(longpress, "longpress", 10000, NULL, 1, NULL);
  // }
 
}

void loop(){
  String ringTime = readStringFromEEPROM(3);
  
  if (ringTime == rtc.getTime()){
      buzz();
      for (int i = 0; i < 10; i++)
      {
        digitalWrite(BUILTIN_LED, HIGH);
        delay(500);
        digitalWrite(BUILTIN_LED, LOW);
        delay(250);
      }
    }
  Serial.println(rtc.getTime());
}