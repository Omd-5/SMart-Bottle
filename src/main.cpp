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
#include <FreeRTOS.h>

ESP32Time rtc(-3840); // Create a Real Time Clock object to be used throughout the code

#define i1 11
#define i2 9
#define BUTTON_PIN       21  // GIOP21 pin connected to button
#define LEDN_PIN          5
#define BUZZER_PIN       7  // GIOP22 pin connected to buzzer
#define LEVEL_PIN        13  // GIOP23 pin connected to level sensor

//EEPROM Section-----------------------------

int ADrStartTime = 1;// String
int ADrEndTime = 2;// String
int ADrIntervals = 3;// Int


void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

void writeTimeToEEPROM() {
  // Get the current time
  time_t now = time(nullptr);

  // Write the time to the EEPROM
  EEPROM.begin(64);
  EEPROM.put(0, now);
  EEPROM.commit();
  EEPROM.end();
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
unsigned long period_in_minutes(tm earlier, tm later){
  int earlier_hours = earlier.tm_hour;
  int earlier_minutes = earlier.tm_min;
  int later_hours = later.tm_hour;
  int later_minutes = later.tm_min;
  return (later_hours * 60 + later_minutes) - (earlier_hours * 60 + earlier_minutes);
  }


//#FINISHED 🎉🎉🎉🎉🎉🎉🎉🎉🎉🎉
tm getTimeformString(String timeString){
  String  cut = timeString.substring(0, 5);
  const char *time_details = cut.c_str();
  struct tm tim;
  strptime(time_details, "%H:%M", &tim);
 
  // Serial.println(tim.tm_sec);
  return tim;
}

void SetTime(tm time){
  rtc.setTime(0,
  time.tm_min,
  time.tm_hour,
   0,0,0);
  Serial.println("setTime");
  Serial.println(time.tm_hour);
  Serial.println(time.tm_min);
}
/// ///////////////////////////////////////
// void buzz(){
//   for (int i = 0; i < 5; i++)
//   {
    
//     delay(200);
//   }
  
// }

String processor(const String& var){
  // Serial.println(var);
  if(var == "TIME"){ 
    return rtc.getHour() + ":" + rtc.getMinute();
  }
  return String();
}


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


// // #define SHORT_PRESS_TIME 2000 // 500 milliseconds
// // Variables will change:
// // the current reading from the input pin
// unsigned long pressedTime  = 0;
// unsigned long releasedTime = 0;

// xTaskHandle configHundle;
///////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////



void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "<h3>>Not found</h3> <br> <a href=\"/\">Return to Home Page</a>");
}



const char* INPUT_TIME = "time";


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
    String setTime = "00:00";
    String startAt = "00:00";
    String endAt = "00:00";
    int interval = 0;
    // GET input4 value on <ESP_IP>/get?input3=<inputMessage>
    if (request->hasParam(INPUT_TIME)) {
      setTime = request->getParam(INPUT_TIME)->value();
      inputMessage = setTime;
      writeStringToEEPROM(3, setTime);
    }
    delay(1000);
    if (request->hasParam("startAt")) {
       startAt = request->getParam("startAt")->value();
       inputMessage = startAt;
      // inputParam = "startAt";
      writeStringToEEPROM(ADrStartTime, startAt);
    }
    delay(1000);
    if (request->hasParam("endAt")) {
       endAt = request->getParam("endAt")->value();
        inputMessage = endAt;
      // inputParam = "endAt";
      writeStringToEEPROM(ADrEndTime, endAt);
    }
    delay(1000);
    if (request->hasParam("periods")) {
      interval = request->getParam("periods")->value().toInt();
      inputMessage = interval;
      writeIntIntoEEPROM(ADrIntervals, interval);
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    SetTime(getTimeformString(setTime));
    Serial.println(inputMessage);
    Serial.println(readStringFromEEPROM(ADrStartTime));
    Serial.println(readStringFromEEPROM(ADrEndTime));
    Serial.print(readIntFromEEPROM(ADrIntervals));
    
    request->send(200, "text/html", "<h3>HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>"+"</h2>"+
                                     "<script>setTimeout(function() {window.location.href = \"/\";}, 3000); </script>");
  });
  server.onNotFound(notFound);
  server.begin();
  
}

// // xTask
// void configTask(void *pvParameters){
     
  
//   while(1){
//     vTaskDelay(100);
//   }

// }


// void ringTask(void *pvParameters){
  


//   while(1){
//         Serial.println("ringTask run!!!");
        

// }

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
 bool start = false;
void setup(){
    EEPROM.begin(512);
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
    Serial.println("Start");
    // rtc.begin();
    delay(5000);
    Serial.println("ringTask");
    delay(5000);
    // xTaskCreate(ringTask, "ring", 2048, NULL, 1, NULL);

    
}

void loop(){
        delay(5000);
 
        tm startTime = getTimeformString(readStringFromEEPROM(ADrStartTime));
        Serial.println("startTime:");
        
        tm endTime = getTimeformString(readStringFromEEPROM(ADrEndTime));
        Serial.println("endTime:");
        Serial.print(endTime.tm_hour);

        int intervals = readIntFromEEPROM(ADrIntervals);
        Serial.println("startTime:");
        Serial.print(startTime.tm_hour);

        int delays =  (int)period_in_minutes(endTime, startTime) / intervals;
        Serial.println("delays:");
        Serial.print(delays);
      if (true){
        for (int i = 0; i < 10; i++)
        {
          Serial.println("ringing..");
          digitalWrite(BUILTIN_LED, HIGH);
          digitalWrite(LEDN_PIN, HIGH);
          tone(BUZZER_PIN, 4100, 1000);
          delay(500);
          tone(BUZZER_PIN, 3200, 1000);
          digitalWrite(BUILTIN_LED, LOW);
          digitalWrite(BUILTIN_LED, LOW);
          delay(250);
        }
        Serial.println(delays);
        delay(delays * 60000);
      }
      if (startTime.tm_hour == rtc.getHour() && startTime.tm_min == rtc.getMinute()){
        start = true;
      }else if (endTime.tm_hour == rtc.getHour() && endTime.tm_min == rtc.getMinute()){
        start = false;
      }
      delay(10000);
  }