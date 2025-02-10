/**
 * @file main.cpp
 * 
 * @mainpage Template for MQTT messaging based projects.
 * 
 * @section description Description
 * This is a template project that features some standard customizations that
 * I like to use in all of my projects. Included in this code is suport for
 * 1) Logging to serial as well as to a MQTT broker, 
 * 2) Unique device name based on DEVICE_TYPE in platformio.ini as well as the 
 *    MAC address of this device,
 * 3) Access Point scanning and connecting using SSID and password pairs kept 
 *    in the apSecrets.h file,
 * 4) Ability to orecieve incoming MQTT messages and process them as commands,
 * 5) Abiity to publish MQTT messages to an MQTT broker,
 * 
 * @section circuit Circuit
 * An ESP32 SOC based board with an antenna.
 * 
 * @section libraries Libraries
 * 1. PubSubClient by Nick O'Leary (knolleary/PubSubClient@^2.8) for MQTT support.
 * 2. ArduinoJson by Benoit Blanchon (bblanchon/ArduinoJson@^7.2.1) for JSON support.
 * 3. TaskScheduler by Richard Lowe (arkhipenko/TaskScheduler@^3.3.0) for task scheduling.
 * 
 * @section notes Notes
 * - 
 * 
 * @section todo TODO
 * - Add JSON support.
 * - Get OTA working. Code is in but have not got  it working yet
 * - Add unit tests.
 *  
 * @section author Author 
 * - Created by theAgingApprentice (TheAgingApprentice@protonmail.com)
 * 
 * @brief ESP32 PlatformIO Arduino project using MQTT messaging.
 * @details Build your MQTT IOT device project starting with this code.
 * 
 * @version 1.0.0
 * @date 2024-12-27
 * 
 * @copyright Copyright (c) 2023 MIT License (MIT) https://mit-license.org/ 
 */
#include <Arduino.h> // Arduino Core for ESP32. Comes with PlatformIO.
#include <WiFi.h> // For WiFi connection (use for ESP32/ESP8266).
#include <PubSubClient.h> // For MQTT handling.
#include <MqttLogger.h> // For logging to serial and/or MQTT.
#include <apSecrets.h> // Known Access Point SSID and password pairs.
#include <TaskScheduler.h> // Manage scheduding task executin out of loop().
//#include <ArduinoOTA.h> // For OTA update support. Comes with PlatformIO. 
#include <huzzah32GpioPins.h> // Pin names for Adafruit Huzzah32 dev board.
#include <projectPinout.h> // Map application pins development board pins. 
#include <ESP32Servo.h> // Servo control library.

// Define global objects.
WiFiClient espClient; // WiFi client object.
PubSubClient client(espClient); // MQTT client.
Scheduler runner; // Task scheduler.
Servo servoMotor; // Servo motor object.

// Structure for storing Wifi Access Point information.
struct accessPoint 
{
   int32_t rssi;
   String ssid;
   int32_t channel;
   wifi_auth_mode_t encryptionType;
   String password;
   bool status;
}; // accessPoint
accessPoint ap =  {0,"",0,WIFI_AUTH_OPEN,"",false};

// Define global variables.
String clientID = ""; // Unique client ID.
String mqttResponseTopic = ""; // Topic to publish responses to.
String mqttCommandTopic = ""; // Topic to subscribe to for commands.

// Build_flags defined in platformio.ini
unsigned long serialBaudRate = UART_SPEED; // Baud rate for serial port.
const char* mqttServer = MQTT_SERVER; // Your MQTT broker IP or domain
const int mqttPort = MQTT_PORT; // MQTT port (default: 1883)
const char* mqttUser = MQTT_USER; // MQTT username. Not used at present.
const char* mqttPassword = MQTT_PASSWORD; // MQTT password. Not used at present.
const char* deviceType = DEVICE_TYPE; // What this device is.
const int keepAlive = KEEP_ALIVE; // Keep alive time in milli-seconds.
const char* buildVersion = BUILD_VERSION; // Version of the software.

// Configure logging object target based on the value of LOG_TARGET in 
// platformio.ini. 
#define logTarget LOG_TARGET
#if logTarget == 0 // No logging.
   MqttLogger mqttLogger(client,"mqttlogger/log");
#elif logTarget == 1 // Log messages to terminal only.
   MqttLogger mqttLogger(client,"mqttlogger/log",MqttLoggerMode::SerialOnly);
#elif logTarget == 2 // Log messages to both terminal and MQTT broker.
   MqttLogger mqttLogger(client,"mqttlogger/log",MqttLoggerMode::MqttAndSerial);
#elif logTarget == 3 // Log messages to MQTT broker only.
   MqttLogger mqttLogger(client,"mqttlogger/log",MqttLoggerMode::MqttOnly);
#else // For all other logging value settings, log messages to terminal only.
   MqttLogger mqttLogger(client,"mqttlogger/log",MqttLoggerMode::SerialOnly);
#endif

// Log themed compiler macros mapped to different MqttLogger functions or if 
// the targhet is set to 0 map the logging macros to do nothing. This makes it
// easy to control logging behaviour at compile time by simply setting 1 
// varibale in PlatformIO.   
#if logTarget == 0
   #define LOG(msg) // Map to nothing, effectively suppressing all logging.
   #define LOGNF(msg) // Map to nothing, effectively suppressing all logging.
   #define LOGLN(msg) // Map to nothing, effectively suppressing all logging.
   #define LOGLNF(msg) // Map to nothing, effectively suppressing all logging.
#else
   #define LOG(msg) mqttLogger.print(String("<") + __FUNCTION__ + "> " + msg)
   #define LOGNF(msg) mqttLogger.print(msg)
   #define LOGLN(msg) mqttLogger.println(String("<") + __FUNCTION__ + "> " + msg)
   #define LOGLNF(msg) mqttLogger.println(msg)
#endif

// Forward function declarations.
void mqttSendKeepAlive();
void mqttCheckIncoming();
//void otaCheck();
void mqttIncomingCallback(char* topic, byte* payload, unsigned int length); 
void stop();
void goForward();
void goBackward();
void motorControl();
String getPassword(String lAP);
Task t1(keepAlive, TASK_FOREVER, &mqttSendKeepAlive);
Task t2(keepAlive, TASK_FOREVER, &mqttCheckIncoming);
//Task t3(keepAlive, TASK_FOREVER, &otaCheck);
Task t4(keepAlive, TASK_FOREVER, &motorControl);
int servoForward = 170;
int servoBackward = 10;
int servoStop = 90;


/**
 * @brief Handle OTA updates.
 * 
 * @details This function is called in the loop() function to handle OTA 
 * updates. It is a non-blocking function that will return immediately if
 * there is no OTA update in progress. Details on how to initiate an OTA
 * update can be found in the PlatformIO documentation. See also
 * https://lonelybinary.com/blogs/learn/esp32-ota-using-platformio and
 * https://techoverflow.net/2023/01/29/minimal-platformio-esp32-arduinoota-example/
 * 
 * @param NA No parameters are passed in.
 * 
 * @return NA No return value.
 */
//void otaCheck() 
//{
//   ArduinoOTA.handle();  
//} // otaCheck()

/** 
 * @brief Check for incoming MQTT messages.
 * 
 * @param NA No parameters are passed in.
 * 
 */
void mqttCheckIncoming() 
{
   client.loop();  
} // mqttSendKeepAlive()

/** 
 * @brief Issue MQTT keepalive messages.
 * 
 * @param NA No parameters are passed in.
 * 
 * @return NA No return value.
 */
void mqttSendKeepAlive() 
{
   LOGLN(millis());
   LOGLNF(WiFi.localIP().toString());
   String msg = "Build version = ";
   msg += buildVersion;
   client.publish(mqttResponseTopic.c_str(), msg.c_str());  
} // mqttSendKeepAlive()

/**
 * @brief Call back function to process incoming MQTT messages.
 * 
 * @param topic The MQTT topic that the incomming message was sent to.
 * @param payload The content of the incoming MQTT message.
 * @param length The length of the incoming message.
 * 
 * @return NA No return value.
 */
void mqttIncomingCallback(char* topic, byte* payload, unsigned int length) 
{
   payload[length] = '\0';
   String strTopic = String((char*)topic);
   String msg = (char*)payload;
   LOG("Received message: ");
   LOGLNF(msg);
   if(msg == "forward")
   {
      goForward();
   } // if
   else if(msg == "backward")
   {
      goBackward();
   }  // if
   else if(msg == "stop")
   {
      stop();
   }  // if
   else
   {
      LOGLN("Unknown command.");
   } // else
} // mqttIncomingCallback()

/**
 * @brief Returns a string contaning the password for an Access Point.
 * 
 * @param lAP SSID to use to look up the password.
 * 
 * @return Access Point password. If the AP is nt known return "unknown".
 */
String getPassword(String lAP)
{
   String password = "unknown";
   String msg = "Check to see if ";
   msg += lAP;
   msg += " is a known network name.";
   LOGLN(msg);
   int apArraySize = sizeof(apSecrets)/sizeof(apSecrets[0]);
   for (int x = 0; x < apArraySize; x++) 
   {
      if(lAP == apSecrets[x].ssid)
      {
         password = apSecrets[x].pwd;
         msg = lAP;
         msg += " is a known Access Point. Setting password.";
      } // if
   } // for
   if(password == "unknown")
   {
         msg = lAP;
         msg += " is NOT a known Access Point.";
   } //if
   LOGLN(msg);
   return password;
} // aaWifi::getPassword()

/**
 * @brief Returns a string contaning a unique ID for this device.
 * 
 * @param NA No parameters are passed in.
 * 
 * @return uniqueID A String contianing a unique ID for ths device.
 */
String getUniqueID()
{
   String uniqueID = String(deviceType);
   uniqueID += WiFi.macAddress().c_str();
   return uniqueID;
} // getUniqueID()

/**
 * @brief Look for Wifi Access Points, put the best in a global structure.
 * 
 * @param NA No parameters are passed in.
 * 
 * @return True if a known AP is found and False if not.
 */
bool scanForAp()
{
   bool validAP = false;
   LOGLN("Scan for nearby Access Points.");
   ap.rssi = -99; // Initialize RSSI to indicating no AP found.
   ap.ssid = "null"; // Initilize SSID to indicate no AP found.
   int n = WiFi.scanNetworks(); // Look for access points.
   LOGLN("Scan complete.");
   if(n == 0) 
   {
      LOGLN("No networks found.");
   } // if
   else 
   {
      String cnt = String(n);
      cnt += " networks found."; 
      LOGLN(cnt);
      for (int i = 0; i < n; ++i) 
      {
         // Print SSID and RSSI for each network found
         String op = "Count = ";
         op += String(i + 1);
         LOGLN(op);
         String ssid = "   SSID  = ";
         ssid += String(WiFi.SSID(i).c_str());
         LOGLN(ssid);
         String rssi = "   RSSI = ";
         rssi += String(WiFi.RSSI(i));
         LOGLN(rssi);
         int32_t slvl = WiFi.RSSI(i);
         if(slvl >= -30)
         {
            LOGLN("   RSSI rating = Amazing.");
         } // if
         if((slvl >= -54) && (slvl <= -29))
         {
            LOGLN("   RSSI rating = Amazing.");
         } // if
         else if((slvl >= -66) && (slvl <= -55))
         {
            LOGLN("   RSSI rating = Very good.");
         } // else if
         else if((slvl >= -70) && (slvl <= -67))
         {
            LOGLN("   RSSI rating = Fairly Good.");
         } // else if
         else if((slvl >= -79) && (slvl <= -70))
         {
            LOGLN("   RSSI rating = Okay.");
         } // else if
         else if((slvl >= -89) && (slvl <= -80))
         {
            LOGLN("   RSSI rating = Not good.");
         } // else if
         else if(slvl <= -90)
         {
            LOGLN("   RSSI rating = Extremely weak signal (unusable).");
         } // else if
         String chan = "   Channel = ";
         chan += String(WiFi.channel(i));
         LOGLN(chan);
         ap.channel = WiFi.channel(i);
         ap.encryptionType = WiFi.encryptionType(i);
         switch (WiFi.encryptionType(i))
         {
            case WIFI_AUTH_OPEN:
               LOGLN("   Auth = open");
               break;
            case WIFI_AUTH_WEP:
               LOGLN("   Auth = WEP");
               break;
            case WIFI_AUTH_WPA_PSK:
               LOGLN("   Auth = WPA");
               break;
            case WIFI_AUTH_WPA2_PSK:
               LOGLN("   Auth = WPA2");
               break;
            case WIFI_AUTH_WPA_WPA2_PSK:
               LOGLN("   Auth = WPA+WPA2");
               break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
               LOGLN("   Auth = WPA2-EAP");
               break;
            case WIFI_AUTH_WPA3_PSK:
               LOGLN("   Auth = WPA3");
               break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
               LOGLN("   Auth = WPA2+WPA3");
               break;
            case WIFI_AUTH_WAPI_PSK:
               LOGLN("   Auth = WAPI");
               break;
            default:
               LOGLN("   Auth = unknown");
         } // switch()
         if(slvl > ap.rssi) // Save AP with best RSSI.
         {
            validAP = true;
            ap.rssi = slvl;
            ap.ssid = String(WiFi.SSID(i).c_str());
            ap.password = getPassword(WiFi.SSID(i).c_str());
            LOGLN("   NOTE: This is now the best access point.");
         } // if
         delay(10);
      } // for
   } // else
   WiFi.scanDelete(); // Delete the scan result to free memory.
   return validAP;
} // aaWifi::scanForAP()

/**
 * @brief Connect to the Access Point stored in the gobal AP structure.
 * 
 * @param NA No parameters are passed in.
 * 
 * @return NA No return value.
 */
void wifiConnect()
{
   if(scanForAp())
   {
      LOG("Connecting to WiFi. SSID: ");
      LOGNF(ap.ssid);
      WiFi.begin(ap.ssid, ap.password);
      while(WiFi.status() != WL_CONNECTED)
      {
         delay(500);
         LOGNF(".");
      } // while()
      LOGLNF("");
      LOG("WiFi connected. Assigned IP address: ");
      LOGLNF(WiFi.localIP().toString());
   } // if
   else
   {
      LOGLN("No network Access Point found.");
   } // else
} //  wifiConnect()

/** 
 * @brief (Re)establish an MQTT client connection to a broker.
 * 
 * @param NA No parameters are passed in.
 * 
 * @return NA No return value.
 */
void reconnect()
{
   while (!client.connected())
   {
      clientID = getUniqueID();
      LOG("Attempting MQTT connection as ");
      LOGNF(clientID);
      LOGNF("...");
      // Attempt to connect
      if (client.connect(clientID.c_str()))
      {
         // as we have a connection here, this will be the first message published to the mqtt server
         LOGLNF("connected."); 
         mqttCommandTopic = clientID + "/cmd";
         mqttResponseTopic = clientID + "/rsp";
         client.setCallback(mqttIncomingCallback);
         client.subscribe(mqttCommandTopic.c_str());
      } // if
      else
      {
         LOG("failed, rc=");
         LOGNF(client.state());
         LOGLNF(" try again in 5 seconds");
         delay(5000);
      } // else
   } // while()
} // reconnect()

/**
 * @brief Spins motor clockwise (from motor's perspecive).
 * 
 */
void stop() 
{
   // LM298N Motor Controller.
   digitalWrite(enA1, LOW);
   digitalWrite(inA1, LOW);
   digitalWrite(inA2, LOW);
   // DRV8871 Motor Controller.
   digitalWrite(inB1, LOW);
   digitalWrite(inB2, LOW);
   digitalWrite(inC1, LOW);
   digitalWrite(inC2, LOW);
   // Servo motor
   servoMotor.write(servoStop); // Put Servo motor in stop position.
} // stop()

/**
 * @brief Spins motor clockwise (from motor's perspecive).
 * 
 */
void goForward() 
{
   // LM298N Motor Controller.
   digitalWrite(enA1, HIGH);
   digitalWrite(inA1, LOW);
   digitalWrite(inA2, HIGH);
   // DRV8871 Motor Controller.
   digitalWrite(inB1, LOW);
   digitalWrite(inB2, HIGH);
   digitalWrite(inC1, LOW);
   digitalWrite(inC2, HIGH);
   // Servo motor
   servoMotor.write(servoForward); // Put Servo motor in forward position.
} // goForward()

/**
 * @brief Spins motor counter-clockwise (from motor's perspecive).
 * 
 */
void goBackward() 
{
   digitalWrite(enA1, HIGH);
   digitalWrite(inA1, HIGH);
   digitalWrite(inA2, LOW);
   // DRV8871 Motor Controller.
   digitalWrite(inB1, HIGH);
   digitalWrite(inB2, LOW);
   digitalWrite(inC1, HIGH);
   digitalWrite(inC2, LOW);
   // Servo motor
   servoMotor.write(servoBackward); // put Servo motor in backward position.
} // goBackward()

/**
 * @brief Simple little routine to spin DC motors forward and backward for 
 * testing. Not used, just here to show all the commands in one spot.
 */
void motorControl()
{
//   stop();
//   delay(1000);
//   goForward();
//   delay(1000);
//   stop();
//   delay(1000);
//   goBackward();
//   delay(1000);
} // motorControl()

/**
 * @brief Standard Arduino start up function.
 * 
 * @param NA No parameters are passed in.
 * 
 * @return NA No return value.
 */
void setup()
{
   Serial.begin(serialBaudRate);
   LOGLN("Start of setup.");
   LOGLN("Initialized Wifi interface.");   
   wifiConnect();
   LOGLN("Connect to MQTT broker.");   
   client.setServer(mqttServer, mqttPort);
   LOGLN("Initialized scheduler");
   runner.init();
   // Add tasks to scheduler.
   LOG("Add t1 task to send keep-alive messages every ");
   LOGNF(keepAlive);
   LOGLNF(" milliseconds.");
   runner.addTask(t1); 
   
   LOG("Add task t2 to check for incoming MQTT messages every ");
   LOGNF(keepAlive);
   LOGLNF(" milliseconds.");
   runner.addTask(t2);
   
//   LOG("Add task t3 to check for OTA messages every ");
//   LOGNF(keepAlive);
//   LOGLNF(" milliseconds.");
//   runner.addTask(t3); 

   LOG("Add task t4 to manage motors every ");
   LOGNF(keepAlive);
   LOGLNF(" milliseconds.");
   runner.addTask(t4); 

   LOGLN("Wait 5 seconds for task setup to complete.");
   delay(5000);

   // Enabe tasks in scheduler.
   LOG("Enable t1 task to send keep-alive messages every ");
   LOGNF(keepAlive);
   LOGLNF(" milliseconds.");   
   t1.enable();

   LOGLN("Enabled t2 to check for incoming MQTT messages every ");
   LOGNF(keepAlive);
   LOGLNF(" milliseconds.");   
   t2.enable();

//   LOGLN("Enabling OTA Feature.");
//   ArduinoOTA.setPassword("lonelybinary");
//   ArduinoOTA.begin();

//   LOGLN("Enabled t3 to check for incoming OTA messages every ");
//   LOGNF(keepAlive);
//   LOGLNF(" milliseconds.");   
//   t3.enable();

   LOGLN("Set up DC motor control pins.");
   pinMode(enA1, OUTPUT);
   pinMode(inA1, OUTPUT);
   pinMode(inA2, OUTPUT);
   pinMode(inB1, OUTPUT);
   pinMode(inB2, OUTPUT);
   pinMode(inC1, OUTPUT);
   pinMode(inC2, OUTPUT);

   LOGLN("Set up Servo motor control pin.");
	// Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	servoMotor.setPeriodHertz(50);    // standard 50 hz servo
	servoMotor.attach(servoPin, 500, 2400); // attaches the servo on pin 18 to the servo object
   servoMotor.write(servoStop); // Put Servo motor in stop position.
	// using default min/max of 1000us and 2000us
	// different servos may require different min/max settings
	// for an accurate 0 to 180 sweep
   LOGLN("Enabled t4 to manage motors every ");
   LOGNF(keepAlive);
   LOGLNF(" milliseconds.");   
   t4.enable();

   LOGLN("End of setup.");
} // setup()

/** 
 * @brief Standard Arduino main logic loop for this program.
 * 
 * @param NA No parameters are passed in.
 * 
 * @return NA No return value.
 */
void loop()
{
   if (!client.connected()) // Make sure there is an MQTT broker connection.
   {
      reconnect(); // If there is no active MQTT broker connection set one up.
   } // if
   runner.execute(); // Run the scheduled tasks.
} // loop()