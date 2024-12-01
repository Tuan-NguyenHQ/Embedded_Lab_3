#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#define WLAN_SSID "Pixel_6"
#define WLAN_PASS

#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "tuannguyenhoangquoc"
#define AIO_KEY
//setup MQTT
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

//setup publish
//Adafruit_MQTT_Publish light_pub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/led");
Adafruit_MQTT_Publish temp_pub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");

//setup subcribe
//Adafruit_MQTT_Subscribe light_sub = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/led", MQTT_QOS_1);
Adafruit_MQTT_Subscribe temp_sub = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/temp", MQTT_QOS_1);

int led_counter = 0;
int led_status = HIGH;
String tempBuffer = ""; 
void lightcallback(char* value, uint16_t len){
  if(value[0] == '0') Serial.print('a');
  if(value[0] == '1') Serial.print('A');
}

void setup() {
  // put your setup code here, to run once:
  //set pin 2,5 as OUTPUT
  pinMode(2, OUTPUT);
  pinMode(5, OUTPUT);
  //set busy pin HIGH
  digitalWrite(5, HIGH);

  Serial.begin(115200);

  //connect Wifi
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

//   subscribe light feed
//  light_sub.setCallback(lightcallback);
  
  //mqtt.subscribe(&light_sub);
  mqtt.subscribe(&temp_sub);

  //connect MQTT
  while (mqtt.connect() != 0) { 
    mqtt.disconnect();
    delay(500);
  }

  //finish setup, set busy pin LOW
  digitalWrite(5, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  

  //receive packet
  mqtt.processPackets(1);
  
  //read serial
  // if(Serial.available()){
  //   int msg = Serial.read();
  //   if(msg == 'o') Serial.print('O');
  //   else if(msg == 'a') light_pub.publish(0);
  //   else if(msg == 'A') light_pub.publish(1);
  //   else if(msg )
  // }
 while (Serial.available()) {
    
    char c = Serial.read();
    Serial.println(c);
    // Check for start and end of the message
    if (c == '!') {  // Start of message
      tempBuffer = "";
    }

    tempBuffer += c;

    if (c == '#') {  // End of message
      if (tempBuffer.startsWith("!TEMP:")) {
        float temperature = parseTemperature(tempBuffer);
        if (temperature != -999.0) {  // Valid temperature
          temp_pub.publish(temperature);  // Publish temperature to Adafruit IO
        }
        
      }
      tempBuffer = "";  // Clear the buffer for the next message
    }
    // else{
    //   temp_pub.publish(30);
    // }
  }
 


  led_counter++;
  if(led_counter == 100){
    // every 1s

    led_counter = 0;
    //toggle LED
    if(led_status == HIGH){ led_status = LOW;}
    else {led_status = HIGH;}
    digitalWrite(2, led_status);
  }
  delay(10);
}
float parseTemperature(String message) {
  int startIndex = message.indexOf(':') + 1;  // Find ':'
  int endIndex = message.indexOf('#');       // Find '#'
  
  if (startIndex > 0 && endIndex > startIndex) {
    String tempString = message.substring(startIndex, endIndex);
    return tempString.toFloat();  // Convert to float
  }

  return -999.0;  // Return error value if parsing fails
}
// void sendTemperatureToAdafruit(float temperature) {
//   if (temp_sub.publish(temperature)) {
//     Serial.println("Temperature sent to Adafruit IO: " + String(temperature));
//   } else {
//     Serial.println("Failed to send temperature to Adafruit IO");
//   }
// }