#include <Arduino.h>
#include <WiFi.h> // Connect to WiFi network
#include <EEPROM.h> // read and write from flash memory
// #include <WebServer.h> /.. Used AsyncWebServer instead
#include <ESP32Time.h> // Real Time library
#include <ESPAsyncWebServer.h> // Asynchronous Web Server Library
#include <SPIFFS.h> // File system library for internal ESP32 memory
#include <AsyncTCP.h> // Asynchronous TCP Library
// #include <Adafruit_GFX.h> // Core graphics library
// #include <Adafruit_SSD1306.h> // Hardware-specific library for SSD1306
// #include <Wire.h> // I2C library
#include <ESP32Servo.h>
#include <FreeRTOS.h>
// #include <TimeLib.h>
#include <EEPROM.h>



// Define the EEPROM address where the timer value will be stored
#define TIMER_ADDR 0

// Define the duration of the timer in seconds
// #define TIMER_DURATION 60


// ESP32Time rtc(-3840); // Create a Real Time Clock object to be used throughout the code

#define i1 11
#define i2 9
#define BUTTON_PIN       21  // GIOP21 pin connected to button
#define LEDN_PIN          5
#define BUZZER_PIN       7  // GIOP22 pin connected to buzzer
#define LEVEL_PIN        11  // GIOP23 pin connected to level sensor

//EEPROM Section-----------------------------


void storeInt(int value){
  EEPROM.begin(sizeof(int));
  EEPROM.write(0, value);
  EEPROM.commit();
}



int readInt(){
  int storedValue = EEPROM.read(0);
  Serial.println(storedValue);
  return storedValue;
}// Read the integer value from the EEPROM


// 0 7450 - 7250
// 1 7150
// 2 7050
// 3 6720
// 4 6670

//6600
int level(int read){
  if (read > 7450){
    return 0;
  }else if (read > 7250){
    return 1;
  }else if(read > 7150){
    return 2;
  }else if(read > 7050){
    return 3;
  }else if(read > 6720){
    return 4;
  }else if(read > 6695){
    return 5;
  }else if(read > 6670){
    return 6;
  }else if(read > 6000){
    return 7;
  }else if(read > 5700){
    return -1;
  }
  return -2;
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

// void SetTime(tm time){
//   rtc.setTime(0,
//   time.tm_min,
//   time.tm_hour,
//    0,0,0);
//   Serial.println("setTime");
//   Serial.println(time.tm_hour);
//   Serial.println(time.tm_min);
// }
/// ///////////////////////////////////////
// void buzz(){
//   for (int i = 0; i < 5; i++)
//   {
    
//     delay(200);
//   }
  
// }

String processor(const String& var){
  // Serial.println(var);
  if(var == "LEVEL"){
    switch (level(analogRead(LEVEL_PIN)))
    {
    case 0:
      return "You did it!!✨, 0%";
      break;
    case 1:
      return "Almost there!!🔥, 20%";
      break;
    case 2:
      return "Keep Going!!🔥, 30%";
      break;
    case 3:
      return "Drink More!!💧, 40%";
      break;
    case 4:
      return "Water is Life🍃, 60%";
      break;
    case 5:
      return "Drink Drink Drink💪, 80%";
      break;
    case 6:
      return "Start your day!! ☀️, 95%";
      break;
    case 7:
      return "Drink me, 100%";
      break;
    default:
      break;
    }
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
    String setTime;
    int interval = 0;
    // GET input4 value on <ESP_IP>/get?input3=<inputMessage>
    if (request->hasParam(INPUT_TIME)) {
      setTime = request->getParam(INPUT_TIME)->value();
      inputMessage = setTime;
      storeInt(atoi(setTime.c_str()));
      // writeStringToEEPROM(3, setTime);
    }
    // delay(1000);

    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
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
    // Wire.begin(i1, i2);
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
  int time = readInt();
  Serial.println(time);
  delay(time *60000 );
  Serial.println("ring");
  for (int i = 0; i < 10; i++)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LEDN_PIN, HIGH);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(500);
     digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LEDN_PIN, LOW);
    digitalWrite(BUILTIN_LED, LOW);
    delay(300);

  }
}