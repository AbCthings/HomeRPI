 /*
 * @author: Alessandro Bortoletto
 * @date: 21/06/2019
 * @version: 0.0.3
 * @brief: This is the firmware of the NodeMCU (ESP8266) field controller, used for control and acquisition of the irrgation system state variables
 * @detailed: todo
 */
 
/* ******* INCLUDES ******* */
//Library DHT11 temperature and humidity sensor
#include "DHT.h"
//Library NTP
#include <NTPClient.h>
//Library Wifi
#include <ESP8266WiFi.h>
//Library Wifi UDP (for NTP)
#include <WiFiUdp.h>
//Library MQTT
#include <PubSubClient.h>
//Library for EEPROM (non volatile memory)
#include <EEPROM.h>
//Library for making an http client (to perform requests)
#include <ESP8266HTTPClient.h>
//Library for making OTA update
#include <ESP8266httpUpdate.h>

/* ******* DEFINTIONS ******* */
//Definition and declarations for DHT11
#define DHTTYPE DHT11
#define TEMPERATURE D2 //GPIO for data
#define VCC_TEMPERATURE D5 //GPIO for VCC to sensor, in order to disable power before deepstate
DHT dht(TEMPERATURE, DHTTYPE);

//Definitions and declarations for NTP time synchronization
#define NTP_OFFSET   7200      // In seconds, this is the offset with respect to GMT (for Italy is GMT+2)
#define NTP_INTERVAL 60000    // In miliseconds, this is the synch time period of the NTP service
#define NTP_ADDRESS  "europe.pool.ntp.org"
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

//Pump GPIO
#define PUMP D1

//Fan GPIO
#define FAN D4

//Definitions for Wifi
const char* ssid = "FASTWEB-HKMUI0";
const char* password = "2ED6W6FODV";

//Definitions for MQTT
const char* mqtt_server = "192.168.1.127";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[75];
int value = 0;

//Declaration for active and deep sleeping times
unsigned long startTime; // Variable where to save the starting instant of the operations
int operationsTime = 15 * 1000; // Time to allow the operations to conclude. In milliseconds!  
int deepSleepTime = 45 * 1000000; // Tine of sleeping. In MICROseconds!

//Declaration of variables useful to handle the firmware version
const char *latestReleaseChar;
char currentReleaseChar[6];

//Declare variables for temperature and humidity
float t;
float h;

//Declare variable for UNIX timestamp
unsigned long epcohTime;

/* ******* USER FUNCTIONS ******* */

//Wifi Setup
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");   
    // AVOID INFINITE LOOP IN TRYING TO CONNECT TO WIFI
    //If a lot of time elapsed (>operationsTime) without MQTT connection, then go to sleep)
    if(millis()-startTime>operationsTime) ESP.deepSleep(deepSleepTime);    
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//MQTT callbacks
void callback(char* topic, byte* payload, unsigned int length) {
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // Print the content of the message one char per time
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Check which topic has been called
  if(strcmp(topic,"HomeRPI/control/pump") == 0) {
    // Check the content of the message and act properly
    if ((char)payload[0] == '1') {
      Serial.println("Turn ON pump.");
      digitalWrite(PUMP, LOW);
      snprintf(msg, 75, "1");
      client.publish("HomeRPI/status/pump", msg);
    }   
    if ((char)payload[0] == '0') {
      Serial.println("Turn OFF pump.");
      digitalWrite(PUMP, HIGH);
      snprintf(msg, 75, "0");
      client.publish("HomeRPI/status/pump", msg); 
    }  
  }
  else if(strcmp(topic,"HomeRPI/control/fan") == 0) {  
    // Check the content of the message and act properly
    if ((char)payload[0] == '1') {
      Serial.println("Turn ON fan.");
      digitalWrite(FAN, LOW);
      snprintf(msg, 75, "1");
      client.publish("HomeRPI/status/fan", msg);      
    }   
    if ((char)payload[0] == '0') {
      Serial.println("Turn OFF fan.");
      digitalWrite(FAN, HIGH);
      snprintf(msg, 75, "0");
      client.publish("HomeRPI/status/fan", msg); 
    }
  }
  else {
    Serial.println("Message on unmapped topic received.");
  }
  
}

//MQTT topics subscribe
void reconnect() {
  
  // Loop until we're reconnected
  while (!client.connected()) {
    
    Serial.print("Attempting MQTT connection... ");
    
    // Create a random client ID
    String clientId = "HomeRPI-NodeMCU-MQTTClient-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str())) { 
      Serial.println("MQTT connection established!");
      
      // Once connected, publish an announcement...
      client.publish("HomeRPI/status/nodemcu", "MQTT connected!");
      
      // ... and resubscribe
      //
      // SUBSCRIBE HERE TO THE NECESSARY TOPICS
      //
      client.subscribe("HomeRPI/control/pump");
      client.subscribe("HomeRPI/control/fan");
      
    } else {  
      Serial.print("MQTT connection failed, rc=");
      Serial.print(client.state());
      Serial.println("Trying again in 2 seconds...");
      
      // AVOID INFINITE LOOP IN TRYING TO CONNECT TO MQTT
      //If a lot of time elapsed (>operationsTime) without MQTT connection, then go to sleep)
      if(millis()-startTime>operationsTime) ESP.deepSleep(deepSleepTime);
      
      // Wait 2 seconds before retrying
      delay(2000);
    } 
  }  
}

/* ******* SETUP ******* */
void setup() {

  // Save the instant of start of the operations --> WILL BE USED TO REBOOT AFTER A CERTAIN TIME
  startTime = millis();
  
  //Pump GPIO setup
  pinMode(PUMP, OUTPUT);
  digitalWrite(PUMP,HIGH); //Turn OFF at startup
  //Fan GPIO setup
  pinMode(FAN, OUTPUT);
  digitalWrite(FAN,HIGH); //Turn OFF at startup
  //DHT11 setup
  pinMode(VCC_TEMPERATURE, OUTPUT); //Set power to the sensor
  digitalWrite(VCC_TEMPERATURE,HIGH); // --> i didn't understand why with pin D5 HIGH is on and LOW is off
  dht.begin(); //Start the sensor loop
  //Serial COM begin
  Serial.begin(9600);
  //NTP setup
  timeClient.begin();
  //Wifi setup
  setup_wifi();
  //MQTT setup
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  //NTP time synch
  //NB The time synch is performed every NTP_INTERVAL milliseconds
  timeClient.update();
  String formattedTime = timeClient.getFormattedTime();
  epcohTime =  timeClient.getEpochTime();

  //Acquire temperature and humidity
  h = dht.readHumidity();
  t = dht.readTemperature();
  Serial.println("[" + formattedTime + "," + epcohTime + "]");       
  Serial.print("Current humidity = ");
  Serial.print(h);
  Serial.print("%  ");
  Serial.print("temperature = ");
  Serial.print(t); 
  Serial.println("C  ");

  /* **************************** OTA HERE ******************************  */
  //Check the presence of UPDATES OTA
  
  // each byte of the EEPROM can only hold a value from 0 to 255.
  EEPROM.begin(64);
  // the version will be of type "X.X.X", so the total number of byte to read will be 5+1 (the last is the terminator)
  for (int i = 0; i<6 ; i++ ) {
      currentReleaseChar[i] = EEPROM.read(i);
  }
  // convert the array of char into string, using the "string" constructor
  String currentRelease(currentReleaseChar);
  Serial.print("Current firmware release: " + currentRelease + "\n");
  
  // Check latest release
  String latestRelease = ""; // to save the retrieved latest release number
  int httpCode; // to save if the response from web server was successful
  
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status    
    HTTPClient http;  //Declare an object of class HTTPClient
    http.begin("http://192.168.1.127:3000/HomeRPI/latest");  //Specify request destination
    httpCode = http.GET();                                                                  //Send the request
   
    if (httpCode > 0) { //Check the returning code
   
      String payload = http.getString();   //Get the request response payload
  
      latestRelease = payload; //Save into variable for further request of binary OS
  
      //Serial.println(payload);                     //Print the response payload
   
    }
   
    http.end();   //Close connection
  }
  Serial.print("Latest available firmware release: " + latestRelease + "\n");

  // convert the latestRelease string to array of char
  latestReleaseChar = latestRelease.c_str();
  
  //If the latest update is different from the current update AND the get was successful (this means that the latestRelease has been updated correctly), then update
  if( ( strcmp(latestReleaseChar,currentReleaseChar)!=0 ) && ( httpCode == 200 ) ){
    
    //Update the version of the release in the EEPROM
    // write each byte (6 in total, since the version is of type X.X.X) in the EEPROM
    for(int i = 0; i<6 ; i++){
      EEPROM.write(i, latestReleaseChar[i]);  
    }
    // save the changes to the flash
    Serial.print("Updated EEPROM with latest release: " + latestRelease + "\n");
    Serial.print("*** WARNING: THIS OPERATION IS REDUCING EEPROM LIFETIME! ***\n");
    EEPROM.commit();

    /* REAL UPDATE DONE HERE */
    // Get the binary of the release and update
    t_httpUpdate_return ret = ESPhttpUpdate.update("192.168.1.127", 3000, "/HomeRPI/releases/HomeRPI_NodeMCU_" + latestRelease + ".bin", "optional current version string here");
    
    switch(ret) {
       case HTTP_UPDATE_FAILED:
            Serial.println("[update] Update failed.");
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("[update] Update no Update.");
            break;
        case HTTP_UPDATE_OK:
            // This will not be called because the ESP will be rebooted!
            Serial.println("[update] Update ok.");
            break;
    }

    //IF WE ARRIVED HERE, THE OTA UPDATE FAILED! SO rollback to previous version in the EEPROM!
    // The update failed: rollback to previous version in the EEPROM!
    // write each byte (6 in total, since the version is of type X.X.X) in the EEPROM
    for(int i = 0; i<6 ; i++){
      EEPROM.write(i, currentReleaseChar[i]);
    }
    // save the changes to the flash
    Serial.print("FIRMWARE UPDATE FAILED: rollback EEPROM with current release: " + currentRelease + "\n");
    Serial.print("*** WARNING: THIS OPERATION IS REDUCING EEPROM LIFETIME! ***\n");
    EEPROM.commit();
  
  }

  /* **************************** END OTA HERE ******************************  */
  
}

/* ******* LOOP ******* */
bool oncePerLoop = true; //This variable is useful to perform some operations (such as sending MQTT log messages) only ONCE per each working cycle
void loop() {

    // Work --> LET THE MQTT SUBSCRIPTIONS OPERATE FOR A FEW SECONDS
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    //THIS OPERATIONS ARE PERFORMED ONLY ONCE PER LOOP!
    if(oncePerLoop==true){
      //Send via MQTT log on current and latest releases
      snprintf(msg, 75, "Current firmware: %s", currentReleaseChar);
      client.publish("HomeRPI/status/nodemcu", msg);

      //Send temperature and humidity measures via MQTT
      snprintf(msg, 75, "{[%lu,%f]}", epcohTime, t);
      client.publish("HomeRPI/status/temperature", msg);
      snprintf(msg, 75, "{[%lu,%f]}", epcohTime, h);
      client.publish("HomeRPI/status/humidity", msg);

      //set to false oncePerLoop
      oncePerLoop = false;
    }

    // Deep sleep mode
    // You can have deepSleep forever with ESP.deepSleep(0) --> To restart, just send an high signal to the RST pin of the NodeMCU (with an external radio receiver?)
    if(millis()-startTime>operationsTime){
      snprintf(msg, 75, "I will go to deep sleep for %d seconds...", deepSleepTime/1000000);
      client.publish("HomeRPI/status/nodemcu", msg);

      //Turn everything OFF before going to sleep --> I act on the MQTT topic WITH RETENTION, so that GPIO will NOT turn on when the NodeMCU will boot again
      //pump
      snprintf(msg, 75, "0");
      client.publish("HomeRPI/control/pump", msg, true); //RETAINED
      snprintf(msg, 75, "0");
      client.publish("HomeRPI/status/pump", msg); 
      //fan
      snprintf(msg, 75, "0");
      client.publish("HomeRPI/control/fan", msg, true); //RETAINED
      snprintf(msg, 75, "0");
      client.publish("HomeRPI/status/fan", msg); 
      //delay to let the messages be sent
      delay(2000);
      
      //go to sleep
      ESP.deepSleep(deepSleepTime); //NOTE THAT THIS WILL TURN OFF ALL THE GPIOs!
      //HERE THE NODEMCU WILL BE REBOOTED!
    }
    
}
