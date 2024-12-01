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
Adafruit_MQTT_Publish temp_pub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");

//setup subcribe
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
  
 while (Serial.available()) {
    
    char c = Serial.read();
    Serial.println(c);
    // Check for start and end of the message
    if (c == '!') {  // Start of message
      tempBuffer = "";
    }

    tempBuffer += c;

    if (c == '#') {  
      if (tempBuffer.startsWith("!TEMP:")) {
        float temperature = parseTemperature(tempBuffer);
        if (temperature != -999.0) {  
          temp_pub.publish(temperature);  
        }
        
      }
      tempBuffer = "";  
    }
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
  int startIndex = message.indexOf(':') + 1;  
  int endIndex = message.indexOf('#');       
  
  if (startIndex > 0 && endIndex > startIndex) {
    String tempString = message.substring(startIndex, endIndex);
    return tempString.toFloat();  
  }

  return -999.0;
}
