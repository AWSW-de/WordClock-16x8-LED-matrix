// ###########################################################################################################################################
// #
// # WordClock code for the 2 printables WordClock 16x16 matrix projects:
// # https://www.printables.com/de/model/350568-wordclock-16x16-led-matrix-2023-v1
// # https://www.printables.com/de/model/361861-wordclock-16x16-led-matrix-2023-v2
// #
// # Code by https://github.com/AWSW-de
// #
// # Released under license: GNU General Public License v3.0: https://github.com/AWSW-de/WordClock-16x16-LED-matrix-2023/blob/main/LICENSE
// #
// ###########################################################################################################################################
/*

      ___           ___           ___           ___           ___           ___       ___           ___           ___     
     /\__\         /\  \         /\  \         /\  \         /\  \         /\__\     /\  \         /\  \         /\__\    
    /:/ _/_       /::\  \       /::\  \       /::\  \       /::\  \       /:/  /    /::\  \       /::\  \       /:/  /    
   /:/ /\__\     /:/\:\  \     /:/\:\  \     /:/\:\  \     /:/\:\  \     /:/  /    /:/\:\  \     /:/\:\  \     /:/__/     
  /:/ /:/ _/_   /:/  \:\  \   /::\~\:\  \   /:/  \:\__\   /:/  \:\  \   /:/  /    /:/  \:\  \   /:/  \:\  \   /::\__\____ 
 /:/_/:/ /\__\ /:/__/ \:\__\ /:/\:\ \:\__\ /:/__/ \:|__| /:/__/ \:\__\ /:/__/    /:/__/ \:\__\ /:/__/ \:\__\ /:/\:::::\__\
 \:\/:/ /:/  / \:\  \ /:/  / \/_|::\/:/  / \:\  \ /:/  / \:\  \  \/__/ \:\  \    \:\  \ /:/  / \:\  \  \/__/ \/_|:|~~|~   
  \::/_/:/  /   \:\  /:/  /     |:|::/  /   \:\  /:/  /   \:\  \        \:\  \    \:\  /:/  /   \:\  \          |:|  |    
   \:\/:/  /     \:\/:/  /      |:|\/__/     \:\/:/  /     \:\  \        \:\  \    \:\/:/  /     \:\  \         |:|  |    
    \::/  /       \::/  /       |:|  |        \::/__/       \:\__\        \:\__\    \::/  /       \:\__\        |:|  |    
     \/__/         \/__/         \|__|         ~~            \/__/         \/__/     \/__/         \/__/         \|__|    

*/


// ###########################################################################################################################################
// # Includes:
// #
// # You will need to add the following libraries to your Arduino IDE to use the project:
// # - Adafruit NeoPixel      // by Adafruit:                     https://github.com/adafruit/Adafruit_NeoPixel
// # - WiFiManager            // by tablatronix / tzapu:          https://github.com/tzapu/WiFiManager
// # - AsyncTCP               // by me-no-dev:                    https://github.com/me-no-dev/AsyncTCP
// # - ESPAsyncWebServer      // by me-no-dev:                    https://github.com/me-no-dev/ESPAsyncWebServer
// # - ESPUI                  // by s00500:                       https://github.com/s00500/ESPUI
// # - ArduinoJson            // by bblanchon:                    https://github.com/bblanchon/ArduinoJson
// # - LITTLEFS               // by lorol:                        https://github.com/lorol/LITTLEFS
// #
// ###########################################################################################################################################
#include <WiFi.h>               // Used to connect the ESP32 to your WiFi
#include <WiFiManager.h>        // Used for the WiFi Manager option to be able to connect the WordClock to your WiFi without code changes
#include <Adafruit_NeoPixel.h>  // Used to drive the NeoPixel LEDs
#include "time.h"               // Used for NTP time requests
#include <AsyncTCP.h>           // Used for the internal web server
#include <ESPAsyncWebServer.h>  // Used for the internal web server
#include <DNSServer.h>          // Used for the internal web server
#include <ESPUI.h>              // Used for the internal web server
#include <Preferences.h>        // Used to save the configuration to the ESP32 flash
#include <WiFiClient.h>         // Used for update function
#include <WebServer.h>          // Used for update function
#include <Update.h>             // Used for update function
#include "settings.h"           // Settings are stored in a seperate file to make to code better readable and to be able to switch to other settings faster


// ###########################################################################################################################################
// # Version number of the code:
// ###########################################################################################################################################
const char* WORD_CLOCK_VERSION = "V1.6.0";


// ###########################################################################################################################################
// # Internal web server settings:
// ###########################################################################################################################################
AsyncWebServer server(80);  // Web server for config
WebServer ewserver(8080);  
WebServer updserver(2022);  // Web server for OTA updates
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 44, 1);
DNSServer dnsServer;


// ###########################################################################################################################################
// # Declartions and variables used in the functions:
// ###########################################################################################################################################
Preferences preferences;
int langLEDlayout;
int iHour = 0;
int iMinute = 0;
int iSecond = 0;
bool updatedevice = true;
bool updatenow = false;
bool updatemode = false;
bool changedvalues = false;
int WiFiManFix = 0;
String iStartTime = "Failed to obtain time on startup... Please restart...";
int redVal_back, greenVal_back, blueVal_back;
int redVal_time, greenVal_time, blueVal_time;
int Display;
int intensity, intensity_day, intensity_night;
int usenightmode, day_time_start, day_time_stop, statusNightMode;
int useshowip, usesinglemin;
int statusLabelID, statusNightModeID;
char* selectLang;
int RandomColor;


// ###########################################################################################################################################
// # Setup function that runs once at startup of the ESP:
// ###########################################################################################################################################
void setup() {
  Serial.begin(115200);
  delay(500);
  preferences.begin("wordclock", false);  // Init ESP32 flash
  Serial.println("######################################################################");
  Serial.print("# WordClock startup of version: ");
  Serial.println(WORD_CLOCK_VERSION);
  Serial.println("######################################################################");
  getFlashValues();                // Read settings from flash
  strip.begin();                   // Init the LEDs
  intensity = intensity_day;       // Set the intenity to day mode for startup
  strip.setBrightness(intensity);  // Set LED brightness
  if (testTime == 0) {             // If time text test mode is not used:
    WIFI_login();                  // WiFiManager
    WiFiManager1stBootFix();       // WiFi Manager 1st connect fix
    ShowIPaddress();               // Display the current IP-address
    configNTPTime();               // NTP time setup
    setupWebInterface();           // Generate the configuration page
    updatenow = true;              // Update the display 1x after startup
    update_display();              // Update LED display
    handleOTAupdate();             // Start the ESP32 OTA update server
    handleExtraWords();
  }
  Serial.println("######################################################################");
  Serial.println("# WordClock startup finished...");
  Serial.println("######################################################################");
}


// ###########################################################################################################################################
// # Loop function which runs all the time after the startup was done:
// ###########################################################################################################################################
void loop() {
  printLocalTime();                               // Locally get the time (NTP server requests done 1x per hour)
  if (updatedevice == true) {                     // Allow display updates (normal usage)
    if (changedvalues == true) setFlashValues();  // Write settings to flash
    update_display();                             // Update display (1x per minute regulary)
  }
  dnsServer.processNextRequest();                    // Update web server
  ewserver.handleClient();                          // Extra words web server access update
  if (updatemode == true) updserver.handleClient();  // ESP32 OTA updates
}


// ###########################################################################################################################################
// # Setup the internal web server configuration page:
// ###########################################################################################################################################
void setupWebInterface() {
  dnsServer.start(DNS_PORT, "*", apIP);

  // Section General:
  // ################
  ESPUI.separator("General:");

  // Status label:
  statusLabelID = ESPUI.label("Status:", ControlColor::Dark, "Operational");

  // WordClock version:
  ESPUI.label("Version", ControlColor::None, WORD_CLOCK_VERSION);

  // Set layout language:
  if (langLEDlayout == 0) selectLang = "Current layout language: German";
  if (langLEDlayout == 1) selectLang = "Current layout language: English";
  if (langLEDlayout == 2) selectLang = "Current layout language: Dutch";
  if (langLEDlayout == 3) selectLang = "Current layout language: Swedish";
  ESPUI.button(selectLang, &buttonlangChange, ControlColor::Dark, "Change layout language", (void*)4);



  // Section LED settings:
  // #####################
  ESPUI.separator("LED settings:");

  // Time color selector:
  char hex_time[7] = { 0 };
  sprintf(hex_time, "#%02X%02X%02X", redVal_time, greenVal_time, blueVal_time);
  uint16_t text_colour_time;
  text_colour_time = ESPUI.text("Time", colCallTIME, ControlColor::Dark, hex_time);
  ESPUI.setInputType(text_colour_time, "color");

  // Background color selector:
  char hex_back[7] = { 0 };
  sprintf(hex_back, "#%02X%02X%02X", redVal_back, greenVal_back, blueVal_back);
  uint16_t text_colour_background;
  text_colour_background = ESPUI.text("Background", colCallBACK, ControlColor::Dark, hex_back);
  ESPUI.setInputType(text_colour_background, "color");

  // Intensity DAY slider selector: !!! DEFAULT LIMITED TO 128 of 255 !!!
  ESPUI.slider("Brightness during the day", &sliderBrightnessDay, ControlColor::Dark, intensity_day, 0, LEDintensityLIMIT);

  // Use night mode function:
  ESPUI.switcher("Show single minutes to display the minute exact time", &switchSingleMinutes, ControlColor::Dark, usesinglemin);

  // Use night mode function:
  ESPUI.switcher("Use night mode to reduce brightness", &switchNightMode, ControlColor::Dark, usenightmode);

  // Intensity NIGHT slider selector: !!! DEFAULT LIMITED TO 128 of 255 !!!
  ESPUI.slider("Brightness at night", &sliderBrightnessNight, ControlColor::Dark, intensity_night, 0, LEDintensityLIMIT);

  // Night mode status:
  statusNightModeID = ESPUI.label("Night mode status", ControlColor::Dark, "Night mode not used");

  // Day mode start time:
  ESPUI.number("Day time starts at", call_day_time_start, ControlColor::Dark, day_time_start, 0, 11);

  // Day mode stop time:
  ESPUI.number("Day time ends after", call_day_time_stop, ControlColor::Dark, day_time_stop, 12, 23);

  // Use random color mode:
  ESPUI.switcher("Use random text color every new minute", &switchRandomColor, ControlColor::Dark, RandomColor);



  // Section Startup:
  // ################
  ESPUI.separator("Startup:");

  // Startup LED test function:
  // ESPUI.switcher("Show LED test on startup", &switchLEDTest, ControlColor::Dark, useledtest);

  // Show IP-address on startup:
  ESPUI.switcher("Show IP-address on startup", &switchShowIP, ControlColor::Dark, useshowip);



  // Section WiFi:
  // #############
  ESPUI.separator("WiFi:");

  // WiFi SSID:
  ESPUI.label("SSID", ControlColor::Dark, WiFi.SSID());

  // WiFi signal strength:
  ESPUI.label("Signal", ControlColor::Dark, String(WiFi.RSSI()) + "dBm");

  // Hostname:
  ESPUI.label("Hostname", ControlColor::Dark, hostname);

  // WiFi ip-address:
  ESPUI.label("IP-address", ControlColor::Dark, IpAddress2String(WiFi.localIP()));

  // WiFi MAC-address:
  ESPUI.label("MAC address", ControlColor::Dark, WiFi.macAddress());



  // Section Time settings:
  // ######################
  ESPUI.separator("Time settings:");

  // NTP server:
  ESPUI.label("NTP server", ControlColor::Dark, NTPserver);

  // Time zone:
  ESPUI.label("Time zone", ControlColor::Dark, Timezone);

  // Time:
  ESPUI.label("Startup time", ControlColor::Dark, iStartTime);



  // Section Update:
  // ###############
  ESPUI.separator("Update:");

  // Update WordClock:
  ESPUI.button("Activate update mode", &buttonUpdate, ControlColor::Dark, "Activate update mode", (void*)1);

  // Update URL
  ESPUI.label("Update URL", ControlColor::Dark, "http://" + IpAddress2String(WiFi.localIP()) + ":2022/ota");

  // Update User account
  ESPUI.label("Update account", ControlColor::Dark, "Username: WordClock   /   Password: 16x16");



  // Section Maintenance:
  // ####################
  ESPUI.separator("Maintenance:");

  // Restart WordClock:
  ESPUI.button("Restart", &buttonRestart, ControlColor::Dark, "Restart", (void*)1);

  // Reset WiFi settings:
  ESPUI.button("Reset WiFi settings", &buttonWiFiReset, ControlColor::Dark, "Reset WiFi settings", (void*)2);

  // Reset WordClock settings:
  ESPUI.button("Reset WordClock settings", &buttonWordClockReset, ControlColor::Dark, "Reset WordClock settings", (void*)3);



  // Update night mode status text on startup:
  if (usenightmode == 1) {
    if ((iHour <= day_time_stop) && (iHour >= day_time_start)) {
      ESPUI.print(statusNightModeID, "Day time");
    } else {
      ESPUI.print(statusNightModeID, "Night time");
    }
  }


  // Deploy the page:
  ESPUI.begin("WordClock");
}


// ###########################################################################################################################################
// # Read settings from flash:
// ###########################################################################################################################################
void getFlashValues() {
  if (debugtexts == 1) Serial.println("Read settings from flash: START");
  langLEDlayout = preferences.getUInt("langLEDlayout", langLEDlayout_default);
  redVal_time = preferences.getUInt("redVal_time", redVal_time_default);
  greenVal_time = preferences.getUInt("greenVal_time", greenVal_time_default);
  blueVal_time = preferences.getUInt("blueVal_time", blueVal_time_default);
  redVal_back = preferences.getUInt("redVal_back", redVal_back_default);
  greenVal_back = preferences.getUInt("greenVal_back", greenVal_back_default);
  blueVal_back = preferences.getUInt("blueVal_back", blueVal_back_default);
  intensity_day = preferences.getUInt("intensity_day", intensity_day_default);
  intensity_night = preferences.getUInt("intensity_night", intensity_night_default);
  usenightmode = preferences.getUInt("usenightmode", usenightmode_default);
  day_time_start = preferences.getUInt("day_time_start", day_time_start_default);
  day_time_stop = preferences.getUInt("day_time_stop", day_time_stop_default);
  useshowip = preferences.getUInt("useshowip", useshowip_default);
  usesinglemin = preferences.getUInt("usesinglemin", usesinglemin_default);
  RandomColor = preferences.getUInt("RandomColor", RandomColor_default);
  Display = preferences.getUInt("Display", Display_default);
  if (debugtexts == 1) Serial.println("Read settings from flash: END");
}


// ###########################################################################################################################################
// # Write settings to flash:
// ###########################################################################################################################################
void setFlashValues() {
  if (debugtexts == 1) Serial.println("Write settings to flash: START");
  changedvalues = false;
  preferences.putUInt("langLEDlayout", langLEDlayout);
  preferences.putUInt("redVal_time", redVal_time);
  preferences.putUInt("greenVal_time", greenVal_time);
  preferences.putUInt("blueVal_time", blueVal_time);
  preferences.putUInt("redVal_back", redVal_back);
  preferences.putUInt("greenVal_back", greenVal_back);
  preferences.putUInt("blueVal_back", blueVal_back);
  preferences.putUInt("intensity_day", intensity_day);
  preferences.putUInt("intensity_night", intensity_night);
  preferences.putUInt("usenightmode", usenightmode);
  preferences.putUInt("day_time_start", day_time_start);
  preferences.putUInt("day_time_stop", day_time_stop);
  preferences.putUInt("useshowip", useshowip);
  preferences.putUInt("usesinglemin", usesinglemin);
  preferences.putUInt("RandomColor", RandomColor);
  preferences.putUInt("Display", Display);
  if (debugtexts == 1) Serial.println("Write settings to flash: END");
  if (usenightmode == 1) {
    if ((iHour <= day_time_stop) && (iHour >= day_time_start)) {
      ESPUI.print(statusNightModeID, "Day time");
    } else {
      ESPUI.print(statusNightModeID, "Night time");
    }
  } else {
    ESPUI.print(statusNightModeID, "Night mode not used");
  }
  updatenow = true;  // Update display now...
}


// ###########################################################################################################################################
// # GUI: Reset the WordClock settings:
// ###########################################################################################################################################
int WordClockResetCounter = 0;
void buttonWordClockReset(Control* sender, int type, void* param) {
  updatedevice = false;
  delay(1000);
  if (WordClockResetCounter == 0) ResetTextLEDs(strip.Color(255, 0, 0));
  if (WordClockResetCounter == 1) ResetTextLEDs(strip.Color(0, 255, 0));
  switch (type) {
    case B_DOWN:
      break;
    case B_UP:
      if (WordClockResetCounter == 1) {
        Serial.println("Status: WORDCLOCK SETTINGS RESET REQUEST EXECUTED");
        preferences.clear();
        delay(1000);
        preferences.putUInt("langLEDlayout", langLEDlayout_default);
        preferences.putUInt("redVal_time", redVal_time_default);
        preferences.putUInt("greenVal_time", greenVal_time_default);
        preferences.putUInt("blueVal_time", blueVal_time_default);
        preferences.putUInt("redVal_back", redVal_back_default);
        preferences.putUInt("greenVal_back", greenVal_back_default);
        preferences.putUInt("blueVal_back", blueVal_back_default);
        preferences.putUInt("intensity_day", intensity_day_default);
        preferences.putUInt("intensity_night", intensity_night_default);
        preferences.putUInt("useshowip", useshowip_default);
        preferences.putUInt("usenightmode", usenightmode_default);
        preferences.putUInt("day_time_stop", day_time_stop_default);
        preferences.putUInt("day_time_stop", day_time_stop_default);
        preferences.putUInt("usesinglemin", usesinglemin_default);
        preferences.putUInt("RandomColor", RandomColor_default);
        preferences.putUInt("Display", Display_default);
        delay(1000);
        preferences.end();
        back_color();
        Serial.println("####################################################################################################");
        Serial.println("# WORDCLOCK SETTING WERE SET TO DEFAULT... WORDCLOCK WILL NOW RESTART... PLEASE CONFIGURE AGAIN... #");
        Serial.println("####################################################################################################");
        delay(1000);
        ESP.restart();
      } else {
        Serial.println("Status: WORDCLOCK SETTINGS RESET REQUEST");
        ESPUI.print(statusLabelID, "WORDCLOCK SETTINGS RESET REQUESTED");
        ESPUI.updateButton(sender->id, "! Press button once more to apply settings reset !");
        delay(1000);
        WordClockResetCounter = WordClockResetCounter + 1;
      }
      break;
  }
}


// ###########################################################################################################################################
// # GUI: Change LED layout language:
// ###########################################################################################################################################
int langChangeCounter = 0;
void buttonlangChange(Control* sender, int type, void* param) {
  updatedevice = false;
  delay(1000);
  if (langChangeCounter == 0) ResetTextLEDs(strip.Color(255, 0, 0));
  if (langChangeCounter == 1) ResetTextLEDs(strip.Color(0, 255, 0));
  switch (type) {
    case B_DOWN:
      ESPUI.print(statusLabelID, "WORDCLOCK LAYOUT LANGUAGE CHANGE REQUESTED");
      delay(1000);
      break;
    case B_UP:
      if (langChangeCounter == 1) {
        Serial.println("WORDCLOCK LAYOUT LANGUAGE CHANGE EXECUTED");
        if (langLEDlayout == 0) preferences.putUInt("langLEDlayout", 1);  // DE to EN
        if (langLEDlayout == 1) preferences.putUInt("langLEDlayout", 2);  // EN to NL
        if (langLEDlayout == 2) preferences.putUInt("langLEDlayout", 3);  // NL to SWE
        if (langLEDlayout == 3) preferences.putUInt("langLEDlayout", 0);  // SWE to DE
        delay(1000);
        preferences.end();
        Serial.println("##########################################################################");
        Serial.println("# WORDCLOCK LAYOUT LANGUAGE WAS CHANGED... WORDCLOCK WILL NOW RESTART... #");
        Serial.println("##########################################################################");
        delay(1000);
        ESP.restart();
      } else {
        Serial.println("WORDCLOCK LAYOUT LANGUAGE CHANGE REQUESTED");
        ESPUI.updateButton(sender->id, "! Press button once more to apply the language change and restart !");
        langChangeCounter = langChangeCounter + 1;
      }
      break;
  }
}


// ###########################################################################################################################################
// # Show the IP-address on the display:
// ###########################################################################################################################################
void ShowIPaddress() {
  if (useshowip == 1) {
    Serial.println("Show current IP-address on the display: " + IpAddress2String(WiFi.localIP()));
    int ipdelay = 2000;

    // Testing the digits:
    // for (int i = 0; i < 10; i++) {
    //   back_color();
    //   numbers(i, 3);
    //   numbers(i, 2);
    //   numbers(i, 1);
    //   strip.show();
    //   delay(ipdelay);
    // }

    // Octet 1:
    back_color();
    numbers(getDigit(int(WiFi.localIP()[0]), 2), 3);
    numbers(getDigit(int(WiFi.localIP()[0]), 1), 2);
    numbers(getDigit(int(WiFi.localIP()[0]), 0), 1);
    setLED(160, 160, 1);
    setLED(191, 191, 1);  // 2nd row
    setLED(236, 239, 1);
    setLED(240, 243, 1);  // 2nd row
    strip.show();
    delay(ipdelay);

    // // Octet 2:
    back_color();
    numbers(getDigit(int(WiFi.localIP()[1]), 2), 3);
    numbers(getDigit(int(WiFi.localIP()[1]), 1), 2);
    numbers(getDigit(int(WiFi.localIP()[1]), 0), 1);
    setLED(160, 160, 1);
    setLED(191, 191, 1);  // 2nd row
    setLED(232, 239, 1);
    setLED(240, 247, 1);  // 2nd row
    strip.show();
    delay(ipdelay);

    // // Octet 3:
    back_color();
    numbers(getDigit(int(WiFi.localIP()[2]), 2), 3);
    numbers(getDigit(int(WiFi.localIP()[2]), 1), 2);
    numbers(getDigit(int(WiFi.localIP()[2]), 0), 1);
    setLED(160, 160, 1);
    setLED(191, 191, 1);  // 2nd row
    setLED(228, 239, 1);
    setLED(240, 251, 1);  // 2nd row
    strip.show();
    delay(ipdelay);

    // // Octet 4:
    back_color();
    numbers(getDigit(int(WiFi.localIP()[3]), 2), 3);
    numbers(getDigit(int(WiFi.localIP()[3]), 1), 2);
    numbers(getDigit(int(WiFi.localIP()[3]), 0), 1);
    setLED(224, 239, 1);
    setLED(240, 255, 1);  // 2nd row
    strip.show();
    delay(ipdelay);
  }
}


// ###########################################################################################################################################
// # Set the numbers on the display in each single row:
// ###########################################################################################################################################
void numbers(int wert, int segment) {

  // Serial.println(wert);

  switch (segment) {
    case 3:
      {
        switch (wert) {
          case 0:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(79, 79, 1);
              setLED(80, 80, 1);  // 2nd row
              setLED(108, 108, 1);
              setLED(115, 115, 1);  // 2nd row
              setLED(111, 111, 1);
              setLED(112, 112, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(143, 143, 1);
              setLED(144, 144, 1);  // 2nd row
              setLED(172, 175, 1);
              setLED(176, 179, 1);  // 2nd row
              break;
            }
          case 1:
            {
              setLED(44, 44, 1);
              setLED(51, 51, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(108, 108, 1);
              setLED(115, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(172, 172, 1);
              setLED(179, 179, 1);  // 2nd row
              break;
            }
          case 2:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(108, 111, 1);
              setLED(112, 115, 1);  // 2nd row
              setLED(143, 143, 1);
              setLED(144, 144, 1);  // 2nd row
              setLED(172, 175, 1);
              setLED(176, 179, 1);  // 2nd row
              break;
            }
          case 3:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(108, 111, 1);
              setLED(112, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(172, 175, 1);
              setLED(176, 179, 1);  // 2nd row
              break;
            }
          case 4:
            {
              setLED(44, 44, 1);
              setLED(51, 51, 1);  // 2nd row
              setLED(47, 47, 1);
              setLED(48, 48, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(79, 79, 1);
              setLED(80, 80, 1);  // 2nd row
              setLED(108, 111, 1);
              setLED(112, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(172, 172, 1);
              setLED(179, 179, 1);  // 2nd row
              break;
            }
          case 5:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(79, 79, 1);
              setLED(80, 80, 1);  // 2nd row
              setLED(108, 111, 1);
              setLED(112, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(172, 175, 1);
              setLED(176, 179, 1);  // 2nd row
              break;
            }
          case 6:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(79, 79, 1);
              setLED(80, 80, 1);  // 2nd row
              setLED(108, 111, 1);
              setLED(112, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(143, 143, 1);
              setLED(144, 144, 1);  // 2nd row
              setLED(172, 175, 1);
              setLED(176, 179, 1);  // 2nd row
              break;
            }
          case 7:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(108, 108, 1);
              setLED(115, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(172, 172, 1);
              setLED(179, 179, 1);  // 2nd row
              break;
            }
          case 8:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(79, 79, 1);
              setLED(80, 80, 1);  // 2nd row
              setLED(108, 111, 1);
              setLED(112, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(143, 143, 1);
              setLED(144, 144, 1);  // 2nd row
              setLED(172, 175, 1);
              setLED(176, 179, 1);  // 2nd row
              break;
            }
          case 9:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(79, 79, 1);
              setLED(80, 80, 1);  // 2nd row
              setLED(108, 111, 1);
              setLED(112, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(172, 175, 1);
              setLED(176, 179, 1);  // 2nd row
              break;
            }
        }
        break;
      }

    case 2:
      {
        switch (wert) {
          case 0:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(74, 74, 1);
              setLED(85, 85, 1);  // 2nd row
              setLED(103, 103, 1);
              setLED(120, 120, 1);  // 2nd row
              setLED(106, 106, 1);
              setLED(117, 117, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(138, 138, 1);
              setLED(149, 149, 1);  // 2nd row
              setLED(167, 170, 1);
              setLED(181, 184, 1);  // 2nd row
              break;
            }
          case 1:
            {
              setLED(39, 39, 1);
              setLED(56, 56, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(103, 103, 1);
              setLED(120, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(167, 167, 1);
              setLED(184, 184, 1);  // 2nd row
              break;
            }
          case 2:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(103, 106, 1);
              setLED(117, 120, 1);  // 2nd row
              setLED(138, 138, 1);
              setLED(149, 149, 1);  // 2nd row
              setLED(167, 170, 1);
              setLED(181, 184, 1);  // 2nd row
              break;
            }
          case 3:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(103, 106, 1);
              setLED(117, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(167, 170, 1);
              setLED(181, 184, 1);  // 2nd row
              break;
            }
          case 4:
            {
              setLED(39, 39, 1);
              setLED(56, 56, 1);  // 2nd row
              setLED(42, 42, 1);
              setLED(53, 53, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(74, 74, 1);
              setLED(85, 85, 1);  // 2nd row
              setLED(103, 106, 1);
              setLED(117, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(167, 167, 1);
              setLED(184, 184, 1);  // 2nd row
              break;
            }
          case 5:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(74, 74, 1);
              setLED(85, 85, 1);  // 2nd row
              setLED(103, 106, 1);
              setLED(117, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(167, 170, 1);
              setLED(181, 184, 1);  // 2nd row
              break;
            }
          case 6:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(74, 74, 1);
              setLED(85, 85, 1);  // 2nd row
              setLED(103, 106, 1);
              setLED(117, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(138, 138, 1);
              setLED(149, 149, 1);  // 2nd row
              setLED(167, 170, 1);
              setLED(181, 184, 1);  // 2nd row
              break;
            }
          case 7:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(103, 103, 1);
              setLED(120, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(167, 167, 1);
              setLED(184, 184, 1);  // 2nd row
              break;
            }
          case 8:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(74, 74, 1);
              setLED(85, 85, 1);  // 2nd row
              setLED(103, 106, 1);
              setLED(117, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(138, 138, 1);
              setLED(149, 149, 1);  // 2nd row
              setLED(167, 170, 1);
              setLED(181, 184, 1);  // 2nd row
              break;
            }
          case 9:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(74, 74, 1);
              setLED(85, 85, 1);  // 2nd row
              setLED(103, 106, 1);
              setLED(117, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(167, 170, 1);
              setLED(181, 184, 1);  // 2nd row
              break;
            }
        }
        break;
      }

    case 1:
      {
        switch (wert) {
          case 0:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(69, 69, 1);
              setLED(90, 90, 1);  // 2nd row
              setLED(98, 98, 1);
              setLED(125, 125, 1);  // 2nd row
              setLED(101, 101, 1);
              setLED(122, 122, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(133, 133, 1);
              setLED(154, 154, 1);  // 2nd row
              setLED(162, 165, 1);
              setLED(186, 189, 1);  // 2nd row
              break;
            }
          case 1:
            {
              setLED(34, 34, 1);
              setLED(61, 61, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(98, 98, 1);
              setLED(125, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(162, 162, 1);
              setLED(189, 189, 1);  // 2nd row
              break;
            }
          case 2:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(98, 101, 1);
              setLED(122, 125, 1);  // 2nd row
              setLED(133, 133, 1);
              setLED(154, 154, 1);  // 2nd row
              setLED(162, 165, 1);
              setLED(186, 189, 1);  // 2nd row
              break;
            }
          case 3:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(98, 101, 1);
              setLED(122, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(162, 165, 1);
              setLED(186, 189, 1);  // 2nd row
              break;
            }
          case 4:
            {
              setLED(34, 34, 1);
              setLED(61, 61, 1);  // 2nd row
              setLED(37, 37, 1);
              setLED(58, 58, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(69, 69, 1);
              setLED(90, 90, 1);  // 2nd row
              setLED(98, 101, 1);
              setLED(122, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(162, 162, 1);
              setLED(189, 189, 1);  // 2nd row
              break;
            }
          case 5:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(69, 69, 1);
              setLED(90, 90, 1);  // 2nd row
              setLED(98, 101, 1);
              setLED(122, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(162, 165, 1);
              setLED(186, 189, 1);  // 2nd row
              break;
            }
          case 6:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(69, 69, 1);
              setLED(90, 90, 1);  // 2nd row
              setLED(98, 101, 1);
              setLED(122, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(133, 133, 1);
              setLED(154, 154, 1);  // 2nd row
              setLED(162, 165, 1);
              setLED(186, 189, 1);  // 2nd row
              break;
            }
          case 7:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(98, 98, 1);
              setLED(125, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(162, 162, 1);
              setLED(189, 189, 1);  // 2nd row
              break;
            }
          case 8:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(69, 69, 1);
              setLED(90, 90, 1);  // 2nd row
              setLED(98, 101, 1);
              setLED(122, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(133, 133, 1);
              setLED(154, 154, 1);  // 2nd row
              setLED(162, 165, 1);
              setLED(186, 189, 1);  // 2nd row
              break;
            }
          case 9:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(69, 69, 1);
              setLED(90, 90, 1);  // 2nd row
              setLED(98, 101, 1);
              setLED(122, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(162, 165, 1);
              setLED(186, 189, 1);  // 2nd row
              break;
            }
        }
        break;
      }
  }
}


// ###########################################################################################################################################
// # Get a digit from a number at position pos: (Split IP-address octets in single digits)
// ###########################################################################################################################################
int getDigit(int number, int pos) {
  return (pos == 0) ? number % 10 : getDigit(number / 10, --pos);
}


// ###########################################################################################################################################
// # GUI: Restart the WordClock:
// ###########################################################################################################################################
int ResetCounter = 0;
void buttonRestart(Control* sender, int type, void* param) {
  updatedevice = false;
  delay(1000);
  if (changedvalues == true) {
    setFlashValues();  // Write settings to flash
    delay(1000);
  }
  preferences.end();
  if (ResetCounter == 0) ResetTextLEDs(strip.Color(255, 0, 0));
  if (ResetCounter == 1) ResetTextLEDs(strip.Color(0, 255, 0));
  switch (type) {
    case B_DOWN:
      break;
    case B_UP:
      if (ResetCounter == 1) {
        delay(1000);
        ESP.restart();
      } else {
        Serial.println("Status: Restart request");
        ESPUI.print(statusLabelID, "RESTART REQUESTED");
        ESPUI.updateButton(sender->id, "! Press button once more to apply restart !");
        ResetCounter = ResetCounter + 1;
      }
      break;
  }
}


// ###########################################################################################################################################
// # GUI: Reset the WiFi settings of the WordClock:
// ###########################################################################################################################################
int WIFIResetCounter = 0;
void buttonWiFiReset(Control* sender, int type, void* param) {
  updatedevice = false;
  if (WIFIResetCounter == 0) ResetTextLEDs(strip.Color(255, 0, 0));
  if (WIFIResetCounter == 1) ResetTextLEDs(strip.Color(0, 255, 0));
  switch (type) {
    case B_DOWN:
      break;
    case B_UP:
      if (WIFIResetCounter == 1) {
        Serial.println("Status: WIFI SETTINGS RESET REQUEST EXECUTED");
        WiFi.disconnect();
        delay(1000);
        WiFiManager manager;
        manager.resetSettings();
        Serial.println("####################################################################################################");
        Serial.println("# WIFI SETTING WERE SET TO DEFAULT... WORDCLOCK WILL NOW RESTART... PLEASE CONFIGURE WIFI AGAIN... #");
        Serial.println("####################################################################################################");
        delay(500);
        ESP.restart();
      } else {
        preferences.putUInt("WiFiManFix", 0);                 // WiFi Manager Fix Reset
        preferences.putUInt("useshowip", useshowip_default);  // Show IP-address again
        delay(100);
        preferences.end();
        Serial.println("Status: WIFI SETTINGS RESET REQUEST");
        ESPUI.print(statusLabelID, "WORDCLOCK WIFI SETTINGS RESET REQUEST");
        ESPUI.updateButton(sender->id, "! Press button once more to apply WiFi reset !");
        WIFIResetCounter = WIFIResetCounter + 1;
      }
      break;
  }
}


// ###########################################################################################################################################
// # GUI: Update the WordClock:
// ###########################################################################################################################################
void buttonUpdate(Control* sender, int type, void* param) {
  preferences.end();
  updatedevice = false;
  delay(1000);
  updatemode = true;
  delay(1000);
  back_color();
  strip.show();
  Serial.println(String("param: ") + String(int(param)));
  switch (type) {
    case B_DOWN:
      Serial.println("Status: Update request");
      ESPUI.print(statusLabelID, "Update requested");
      break;
    case B_UP:
      Serial.println("Status: Update executed");
      ESPUI.updateButton(sender->id, "Update mode active now - Use the update url: >>>");
      break;
  }
}


// ###########################################################################################################################################
// # Show a LED output for RESET in the different languages:
// ###########################################################################################################################################
void ResetTextLEDs(uint32_t color) {
  updatedevice = false;
  delay(1000);
  back_color();

  if (langLEDlayout == 0) {      // DE:
    setLEDcol(137, 138, color);  // RE
    setLEDcol(149, 150, color);  // 2nd row
    setLEDcol(167, 168, color);  // SE
    setLEDcol(183, 184, color);  // 2nd row
    setLEDcol(227, 227, color);  // T
    setLEDcol(252, 252, color);  // 2nd row
  }

  if (langLEDlayout == 1) {      // EN:
    setLEDcol(100, 101, color);  // RE
    setLEDcol(122, 123, color);  // 2nd row
    setLEDcol(174, 175, color);  // SE
    setLEDcol(176, 177, color);  // 2nd row
    setLEDcol(227, 227, color);  // T
    setLEDcol(252, 252, color);  // 2nd row
  }

  if (langLEDlayout == 2) {      // NL:
    setLEDcol(33, 33, color);    // R
    setLEDcol(62, 62, color);    // 2nd row
    setLEDcol(96, 97, color);    // ES
    setLEDcol(126, 127, color);  // 2nd row
    setLEDcol(164, 164, color);  // E
    setLEDcol(187, 187, color);  // 2nd row
    setLEDcol(227, 227, color);  // T
    setLEDcol(252, 252, color);  // 2nd row
  }

  if (langLEDlayout == 3) {    // SWE:
    setLEDcol(67, 71, color);  // R
    setLEDcol(88, 92, color);  // 2nd row
  }

  strip.show();
}


// ###########################################################################################################################################
// # Actual function, which controls 1/0 of the LED with color value:
// ###########################################################################################################################################
void setLEDcol(int ledNrFrom, int ledNrTo, uint32_t color) {
  if (ledNrFrom > ledNrTo) {
    setLED(ledNrTo, ledNrFrom, 1);  // Sets LED numbers in correct order
  } else {
    for (int i = ledNrFrom; i <= ledNrTo; i++) {
      if ((i >= 0) && (i < NUMPIXELS))
        strip.setPixelColor(i, color);
    }
  }
}


// ###########################################################################################################################################
// # GUI: Night mode switch:
// ###########################################################################################################################################
void switchNightMode(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      usenightmode = 1;
      if ((iHour <= day_time_stop) && (iHour >= day_time_start)) {
        intensity = intensity_day;
      } else {
        intensity = intensity_night;
      }
      break;
    case S_INACTIVE:
      intensity = intensity_day;
      usenightmode = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Single minutes switch:
// ###########################################################################################################################################
void switchSingleMinutes(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      usesinglemin = 1;
      break;
    case S_INACTIVE:
      usesinglemin = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Use random color mode:
// ###########################################################################################################################################
void switchRandomColor(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      RandomColor = 1;
      break;
    case S_INACTIVE:
      RandomColor = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Show IP-ADdress switch:
// ###########################################################################################################################################
void switchShowIP(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      useshowip = 1;
      break;
    case S_INACTIVE:
      useshowip = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # Update the display / time on it:
// ###########################################################################################################################################
void update_display() {
  if (debugtexts == 1) Serial.println("Time: " + iStartTime);

  if (usenightmode == 1) {
    if ((iHour <= day_time_stop) && (iHour >= day_time_start)) {
      intensity = intensity_day;
    } else {
      intensity = intensity_night;
    }
  } else {
    intensity = intensity_day;
  }
  strip.setBrightness(intensity);

  if (testTime == 0) {  // Show the current time:
    show_time(iHour, iMinute);
  } else {  // TEST THE DISPLAY TIME OUTPUT:
    strip.setBrightness(33);
    for (int i = 0; i <= 12; i++) {  // 12 hours only:
      show_time(i, 0);
      delay(1000);
    }
    for (int i = 0; i <= 3; i++) {  // Hours 0 to 3 with all minute texts:
      for (int y = 0; y < 60; y++) {
        show_time(i, y);
        delay(500);
      }
    }
  }
}


// ###########################################################################################################################################
// # Display hours and minutes text function:
// ###########################################################################################################################################
uint32_t colorRGB;
void show_time(int hours, int minutes) {
  static int lastHourSet = -1;
  static int lastMinutesSet = -1;
  if ((lastHourSet == hours && lastMinutesSet == minutes) && updatenow == false) {  // Reduce display updates to new minutes and new config updates
    return;
  }

  if (updatenow = true) {
    updatenow = false;
    lastHourSet = hours;
    lastMinutesSet = minutes;
    if (debugtexts == 1) {
      Serial.print("Update display now: ");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(":");
      Serial.println(iSecond);
    }
  }

  // Set background color:
  back_color();

  // Static text color or random color mode:
  if (RandomColor == 0) colorRGB = strip.Color(redVal_time, greenVal_time, blueVal_time);
  if (RandomColor == 1) colorRGB = strip.Color(random(255), random(255), random(255));
  if (Display == 0) colorRGB = strip.Color(0, 0, 0);

  // Display time:
  iHour = hours;
  iMinute = minutes;

  // Test a special time:
  // iHour = 23;
  // iMinute = 25;

  // divide minute by 5 to get value for display control
  int minDiv = iMinute / 5;
  if (usesinglemin == 1) showMinutes(iMinute);

  // ########################################################### DE:
  if (langLEDlayout == 0) {  // DE:

    // ES IST:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(16, 17, colorRGB);  // 2nd row
    setLEDcol(10, 12, colorRGB);
    setLEDcol(19, 21, colorRGB);  // 2nd row

    // FÜNF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(76, 79, colorRGB);
      setLEDcol(80, 83, colorRGB);  // 2nd row
    }
    // VIERTEL:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(69, 75, colorRGB);
      setLEDcol(84, 90, colorRGB);  // 2nd row
    }
    // ZEHN: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(32, 35, colorRGB);
      setLEDcol(60, 63, colorRGB);  // 2nd row
    }
    // ZWANZIG:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(41, 47, colorRGB);
      setLEDcol(48, 54, colorRGB);  // 2nd row
    }
    // NACH:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(64, 67, colorRGB);
      setLEDcol(92, 95, colorRGB);  // 2nd row
    }
    // VOR:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(109, 111, colorRGB);
      setLEDcol(112, 114, colorRGB);  // 2nd row
    }
    // HALB:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(104, 107, colorRGB);
      setLEDcol(116, 119, colorRGB);  // 2nd row
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 25 hour needs to be counted up:
    // fuenf vor halb 2 = 13:25
    if (iMinute >= 25) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          if (xHour == 1) {
            setLEDcol(169, 171, colorRGB);  // EIN
            setLEDcol(180, 182, colorRGB);  // 2nd row
          }
          if ((xHour == 1) && (iMinute > 4)) {
            setLEDcol(168, 171, colorRGB);  // EINS (S in EINS) (just used if not point 1 o'clock)
            setLEDcol(180, 183, colorRGB);  // 2nd row
          }
          break;
        }
      case 2:
        {
          setLEDcol(140, 143, colorRGB);  // ZWEI
          setLEDcol(144, 147, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(136, 139, colorRGB);  // DREI
          setLEDcol(148, 151, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(128, 131, colorRGB);  // VIER
          setLEDcol(156, 159, colorRGB);  // 2nd row
          break;
        }
      case 5:
        {
          setLEDcol(160, 163, colorRGB);  // FUENF
          setLEDcol(188, 191, colorRGB);  // 2nd row
          break;
        }
      case 6:
        {
          setLEDcol(164, 168, colorRGB);  // SECHS
          setLEDcol(183, 187, colorRGB);  // 2nd row
          break;
        }
      case 7:
        {
          setLEDcol(202, 207, colorRGB);  // SIEBEN
          setLEDcol(208, 213, colorRGB);  // 2nd row
          break;
        }
      case 8:
        {
          setLEDcol(172, 175, colorRGB);  // ACHT
          setLEDcol(176, 179, colorRGB);  // 2nd row
          break;
        }
      case 9:
        {
          setLEDcol(132, 135, colorRGB);  // NEUN
          setLEDcol(152, 155, colorRGB);  // 2nd row
          break;
        }
      case 10:
        {
          setLEDcol(99, 102, colorRGB);   // ZEHN (Stunden)
          setLEDcol(121, 124, colorRGB);  // 2nd row
          break;
        }
      case 11:
        {
          setLEDcol(96, 98, colorRGB);    // ELF
          setLEDcol(125, 127, colorRGB);  // 2nd row
          break;
        }
      case 12:
        {
          setLEDcol(197, 201, colorRGB);  // ZWÖLF
          setLEDcol(214, 218, colorRGB);  // 2nd row
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(192, 194, colorRGB);  // UHR
      setLEDcol(221, 223, colorRGB);  // 2nd row
    }
  }

  // ########################################################### EN:
  if (langLEDlayout == 1) {  // EN:

    // IT IS:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(16, 17, colorRGB);  // 2nd row
    setLEDcol(11, 12, colorRGB);
    setLEDcol(19, 20, colorRGB);  // 2nd row

    // FIVE: (Minutes)                         // x:05 + x:25 + x:35 + x:55
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(38, 41, colorRGB);
      setLEDcol(54, 57, colorRGB);  // 2nd row
    }
    // QUARTER:                                // x:15 + X:45
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(72, 78, colorRGB);
      setLEDcol(81, 87, colorRGB);  // 2nd row
    }
    // A:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(5, 5, colorRGB);
      setLEDcol(26, 26, colorRGB);  // 2nd row
    }
    // TEN: (Minutes)                          // x:10 + x:50
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(0, 2, colorRGB);
      setLEDcol(29, 31, colorRGB);  // 2nd row
    }
    // TWENTY:                                 // x:20 + x:25 + x:35 + x:40
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(42, 47, colorRGB);
      setLEDcol(48, 53, colorRGB);  // 2nd row
    }
    // PAST:                                   // x:05 + x:10 + x:15 + x:20 + x:25 + x:30
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 5) || (minDiv == 6)) {
      setLEDcol(66, 69, colorRGB);
      setLEDcol(90, 93, colorRGB);  // 2nd row
    }
    // TO:                                     // x:35 + x:40 + x:45 + x:50 + x:55
    if ((minDiv == 7) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(65, 66, colorRGB);
      setLEDcol(93, 94, colorRGB);  // 2nd row
    }
    // HALF:                                   // x:30
    if ((minDiv == 6)) {
      setLEDcol(3, 6, colorRGB);
      setLEDcol(25, 28, colorRGB);  // 2nd row
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 35 hour needs to be counted up:
    // Twenty five to two = 13:35
    if (iMinute >= 35) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          setLEDcol(201, 203, colorRGB);  // ONE
          setLEDcol(212, 214, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(105, 107, colorRGB);  // TWO
          setLEDcol(116, 118, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(99, 103, colorRGB);   // THREE
          setLEDcol(120, 124, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(128, 131, colorRGB);  // FOUR
          setLEDcol(156, 159, colorRGB);  // 2nd row
          break;
        }
      case 5:
        {
          setLEDcol(108, 111, colorRGB);  // FIVE
          setLEDcol(112, 115, colorRGB);  // 2nd row
          break;
        }
      case 6:
        {
          setLEDcol(163, 165, colorRGB);  // SIX
          setLEDcol(186, 188, colorRGB);  // 2nd row
          break;
        }
      case 7:
        {
          setLEDcol(171, 175, colorRGB);  // SEVEN
          setLEDcol(176, 180, colorRGB);  // 2nd row
          break;
        }
      case 8:
        {
          setLEDcol(166, 170, colorRGB);  // EIGHT
          setLEDcol(181, 185, colorRGB);  // 2nd row
          break;
        }
      case 9:
        {
          setLEDcol(204, 207, colorRGB);  // NINE
          setLEDcol(208, 211, colorRGB);  // 2nd row
          break;
        }
      case 10:
        {
          setLEDcol(96, 98, colorRGB);    // TEN
          setLEDcol(125, 127, colorRGB);  // 2nd row
          break;
        }
      case 11:
        {
          setLEDcol(138, 143, colorRGB);  // ELEVEN
          setLEDcol(144, 149, colorRGB);  // 2nd row
          break;
        }
      case 12:
        {
          setLEDcol(132, 137, colorRGB);  // TWELVE
          setLEDcol(150, 155, colorRGB);  // 2nd row
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(193, 199, colorRGB);  // O'CLOCK
      setLEDcol(216, 222, colorRGB);  // 2nd row
    }
  }

  // ########################################################### NL:
  if (langLEDlayout == 2) {  // NL:

    // HET IS:
    setLEDcol(13, 15, colorRGB);
    setLEDcol(16, 18, colorRGB);  // 2nd row
    setLEDcol(10, 11, colorRGB);
    setLEDcol(20, 21, colorRGB);  // 2nd row

    // VIJF: (Minuten) x:05, x:25, x:35, x:55
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(0, 3, colorRGB);
      setLEDcol(28, 31, colorRGB);  // 2nd row
    }
    // KWART: x:15, x:45
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(38, 42, colorRGB);
      setLEDcol(53, 57, colorRGB);  // 2nd row
    }
    // TIEN: (Minuten) x:10, x:50
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(44, 47, colorRGB);
      setLEDcol(48, 51, colorRGB);  // 2nd row
    }
    // TIEN: (TIEN VOOR HALF, TIEN OVER HALF) x:20, x:40 (on request not set to TWINTIG OVER)
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(44, 47, colorRGB);
      setLEDcol(48, 51, colorRGB);  // 2nd row
    }
    // OVER: x:05, x:10, x:15, x:35, x:40
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(33, 36, colorRGB);
      setLEDcol(59, 62, colorRGB);  // 2nd row
    }
    // VOOR: x:20, x:25, x:45, x:50, x:55
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(64, 67, colorRGB);
      setLEDcol(92, 95, colorRGB);  // 2nd row
    }
    // HALF:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 6) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(107, 110, colorRGB);
      setLEDcol(113, 116, colorRGB);  // 2nd row
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 20 hour needs to be counted up:
    // tien voor half 2 = 13:20
    if (iMinute >= 20) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          setLEDcol(99, 101, colorRGB);   // EEN
          setLEDcol(122, 124, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(203, 206, colorRGB);  // TWEE
          setLEDcol(209, 212, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(164, 167, colorRGB);  // DRIE
          setLEDcol(184, 187, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(198, 201, colorRGB);  // VIER
          setLEDcol(214, 217, colorRGB);  // 2nd row
          break;
        }
      case 5:
        {
          setLEDcol(160, 163, colorRGB);  // VIJF
          setLEDcol(188, 191, colorRGB);  // 2nd row
          break;
        }
      case 6:
        {
          setLEDcol(96, 98, colorRGB);    // ZES
          setLEDcol(125, 127, colorRGB);  // 2nd row
          break;
        }
      case 7:
        {
          setLEDcol(129, 133, colorRGB);  // ZEVEN
          setLEDcol(154, 158, colorRGB);  // 2nd row
          break;
        }
      case 8:
        {
          setLEDcol(102, 105, colorRGB);  // ACHT
          setLEDcol(118, 121, colorRGB);  // 2nd row
          break;
        }
      case 9:
        {
          setLEDcol(171, 175, colorRGB);  // NEGEN
          setLEDcol(176, 180, colorRGB);  // 2nd row
          break;
        }
      case 10:
        {
          setLEDcol(140, 143, colorRGB);  // TIEN (Stunden)
          setLEDcol(144, 147, colorRGB);  // 2nd row
          break;
        }
      case 11:
        {
          setLEDcol(168, 170, colorRGB);  // ELF
          setLEDcol(181, 183, colorRGB);  // 2nd row
          break;
        }
      case 12:
        {
          setLEDcol(134, 139, colorRGB);  // TWAALF
          setLEDcol(148, 153, colorRGB);  // 2nd row
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(193, 195, colorRGB);  // UUR
      setLEDcol(220, 222, colorRGB);  // 2nd row
    }
  }

  // ########################################################### SWE:
  if (langLEDlayout == 3) {  // SWE:

    // KLOCKAN ÄR:
    setLEDcol(9, 15, colorRGB);
    setLEDcol(16, 22, colorRGB);  // 2nd row
    setLEDcol(5, 6, colorRGB);
    setLEDcol(25, 26, colorRGB);  // 2nd row

    // FEM: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(64, 66, colorRGB);
      setLEDcol(93, 95, colorRGB);  // 2nd row
    }
    // KVART:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(36, 40, colorRGB);
      setLEDcol(55, 59, colorRGB);  // 2nd row
    }
    // TIO: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(72, 74, colorRGB);
      setLEDcol(85, 87, colorRGB);  // 2nd row
    }
    // TJUGO:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(75, 79, colorRGB);
      setLEDcol(80, 84, colorRGB);  // 2nd row
    }
    // ÖVER:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(108, 111, colorRGB);
      setLEDcol(112, 115, colorRGB);  // 2nd row
    }
    // I:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(97, 97, colorRGB);
      setLEDcol(126, 126, colorRGB);  // 2nd row
    }
    // HALV:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(140, 143, colorRGB);
      setLEDcol(144, 147, colorRGB);  // 2nd row
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 25 hour needs to be counted up:
    // fuenf vor halb 2 = 13:25
    if (iMinute >= 25) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          if (xHour == 1) {
            setLEDcol(170, 172, colorRGB);  // ETT
            setLEDcol(179, 181, colorRGB);  // 2nd row
          }
          break;
        }
      case 2:
        {
          setLEDcol(160, 162, colorRGB);  // TVÅ
          setLEDcol(189, 191, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(132, 134, colorRGB);  // TRE
          setLEDcol(153, 155, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(197, 200, colorRGB);  // FYRA
          setLEDcol(218, 218, colorRGB);  // 2nd row
          break;
        }
      case 5:
        {
          setLEDcol(194, 196, colorRGB);  // FEM
          setLEDcol(219, 221, colorRGB);  // 2nd row
          break;
        }
      case 6:
        {
          setLEDcol(201, 203, colorRGB);  // SEX
          setLEDcol(212, 214, colorRGB);  // 2nd row
          break;
        }
      case 7:
        {
          setLEDcol(173, 175, colorRGB);  // SJU
          setLEDcol(176, 178, colorRGB);  // 2nd row
          break;
        }
      case 8:
        {
          setLEDcol(128, 131, colorRGB);  // ÅTTA
          setLEDcol(156, 159, colorRGB);  // 2nd row
          break;
        }
      case 9:
        {
          setLEDcol(135, 137, colorRGB);  // NIO
          setLEDcol(150, 152, colorRGB);  // 2nd row
          break;
        }
      case 10:
        {
          setLEDcol(168, 170, colorRGB);  // TIO (Stunden)
          setLEDcol(181, 183, colorRGB);  // 2nd row
          break;
        }
      case 11:
        {
          setLEDcol(204, 207, colorRGB);  // ELVA
          setLEDcol(208, 211, colorRGB);  // 2nd row
          break;
        }
      case 12:
        {
          setLEDcol(163, 166, colorRGB);  // TOLV
          setLEDcol(185, 188, colorRGB);  // 2nd row
          break;
        }
    }
  }

  strip.show();
}


// ###########################################################################################################################################
// # Display extra minutes function:
// ###########################################################################################################################################
void showMinutes(int minutes) {
  int minMod = (minutes % 5);
  // Serial.println(minMod);

  // ##################################################### DE:
  if (langLEDlayout == 0) {  // DE:

    switch (minMod) {
      case 1:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(243, 243, colorRGB);  // 2nd row
          setLEDcol(226, 231, colorRGB);  // MINUTE
          setLEDcol(248, 253, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(244, 244, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(245, 245, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(246, 246, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
    }
  }

  // ##################################################### EN:
  if (langLEDlayout == 1) {  // EN:
    switch (minMod) {
      case 1:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(243, 243, colorRGB);  // 2nd row
          setLEDcol(226, 231, colorRGB);  // MINUTE
          setLEDcol(248, 253, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(244, 244, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTES
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(245, 245, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTES
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(246, 246, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTES
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
    }
  }

  // ##################################################### NL:
  if (langLEDlayout == 2) {  // NL:

    switch (minMod) {
      case 1:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(243, 243, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTEN (set to this on request, because there was no space for the extra word "minuut")
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(244, 244, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(245, 245, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(246, 246, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
    }
  }

  // ##################################################### SWE:
  if (langLEDlayout == 3) {  // SWE:

    switch (minMod) {
      case 1:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(243, 243, colorRGB);  // 2nd row
          setLEDcol(227, 231, colorRGB);  // MINUT
          setLEDcol(248, 252, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(244, 244, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTER
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(245, 245, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTER
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(246, 246, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTER
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
    }
  }
}


// ###########################################################################################################################################
// # Background color function: SET ALL LEDs OFF
// ###########################################################################################################################################
void back_color() {
  uint32_t c0;

   if (Display == 0) {
    c0 = strip.Color(0, 0, 0);
   } else {
    c0 = strip.Color(redVal_back, greenVal_back, blueVal_back);

   }
  
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, c0);
  }
  delay(500);
}


// ###########################################################################################################################################
// # Startup WiFi text function:
// ###########################################################################################################################################
void SetWLAN(uint32_t color) {
  Serial.println("Show text WLAN/WIFI...");

  if (langLEDlayout == 0) {  // DE:
    for (uint16_t i = 5; i < 9; i++) {
      strip.setPixelColor(i, color);
    }
    for (uint16_t i = 23; i < 27; i++) {  // 2nd row
      strip.setPixelColor(i, color);
    }
  }

  if (langLEDlayout == 1) {  // EN:
    for (uint16_t i = 7; i < 11; i++) {
      strip.setPixelColor(i, color);
    }
    for (uint16_t i = 21; i < 25; i++) {  // 2nd row
      strip.setPixelColor(i, color);
    }
  }

  if (langLEDlayout == 2) {  // NL:
    for (uint16_t i = 75; i < 79; i++) {
      strip.setPixelColor(i, color);
    }
    for (uint16_t i = 81; i < 85; i++) {  // 2nd row
      strip.setPixelColor(i, color);
    }
  }

  if (langLEDlayout == 3) {  // SWE:
    for (uint16_t i = 0; i < 4; i++) {
      strip.setPixelColor(i, color);
    }
    for (uint16_t i = 28; i < 32; i++) {  // 2nd row
      strip.setPixelColor(i, color);
    }
  }

  strip.show();
}


// ###########################################################################################################################################
// # Wifi Manager setup and reconnect function that runs once at startup and during the loop function of the ESP:
// ###########################################################################################################################################
void WIFI_login() {
  Serial.print("Try to connect to WiFi: ");
  Serial.println(WiFi.SSID());
  SetWLAN(strip.Color(0, 0, 255));
  WiFi.setHostname(hostname);
  bool WiFires;
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(AP_TIMEOUT);
  WiFires = wifiManager.autoConnect(DEFAULT_AP_NAME);
  if (!WiFires) {
    Serial.print("Failed to connect to WiFi: ");
    Serial.println(WiFi.SSID());
    SetWLAN(strip.Color(255, 0, 0));
    delay(1000);
  } else {
    Serial.print("Connected to WiFi: ");
    Serial.println(WiFi.SSID());
    SetWLAN(strip.Color(0, 255, 0));
    delay(1000);
  }
}


// ###########################################################################################################################################
// # WiFi Manager 1st connect fix: (Needed after the 1st login to your router - Restart the device once to be able to reach the web page...)
// ###########################################################################################################################################
void WiFiManager1stBootFix() {
  WiFiManFix = preferences.getUInt("WiFiManFix", 0);
  if (WiFiManFix == 0) {
    Serial.println("######################################################################");
    Serial.println("# ESP restart needed because of WiFi Manager Fix");
    Serial.println("######################################################################");
    SetWLAN(strip.Color(0, 255, 0));
    preferences.putUInt("WiFiManFix", 1);
    delay(1000);
    preferences.end();
    delay(1000);
    ESP.restart();
  }
}


// ###########################################################################################################################################
// # Actual function, which controls 1/0 of the LED:
// ###########################################################################################################################################
void setLED(int ledNrFrom, int ledNrTo, int switchOn) {
  if (ledNrFrom > ledNrTo) {
    setLED(ledNrTo, ledNrFrom, switchOn);  // Sets LED numbers in correct order
  } else {
    for (int i = ledNrFrom; i <= ledNrTo; i++) {
      if ((i >= 0) && (i < NUMPIXELS))
        strip.setPixelColor(i, strip.Color(redVal_time, greenVal_time, blueVal_time));
    }
  }
  if (switchOn == 0) {
    for (int i = ledNrFrom; i <= ledNrTo; i++) {
      if ((i >= 0) && (i < NUMPIXELS))
        strip.setPixelColor(i, strip.Color(0, 0, 0));  // Switch LEDs off
    }
  }
}


// ###########################################################################################################################################
// # NTP time functions:
// ###########################################################################################################################################
void configNTPTime() {
  initTime(Timezone);
  printLocalTime();
}
// ###########################################################################################################################################
void setTimezone(String timezone) {
  Serial.printf("Setting timezone to %s\n", timezone.c_str());
  setenv("TZ", timezone.c_str(), 1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}
// ###########################################################################################################################################
void initTime(String timezone) {
  struct tm timeinfo;
  Serial.println("Setting up time");
  configTime(0, 0, NTPserver);
  if (!getLocalTime(&timeinfo)) {
    back_color();
    Serial.println("Failed to obtain time");
    ESPUI.print(statusLabelID, "Failed to obtain time");

    if (langLEDlayout == 0) {  // DE:
      setLEDcol(1, 4, strip.Color(255, 0, 0));
      setLEDcol(27, 30, strip.Color(255, 0, 0));  // 2nd row
    }

    if (langLEDlayout == 1) {  // EN:
      setLEDcol(33, 36, strip.Color(255, 0, 0));
      setLEDcol(59, 62, strip.Color(255, 0, 0));  // 2nd row
    }

    if (langLEDlayout == 2) {  // NL:
      setLEDcol(69, 72, strip.Color(255, 0, 0));
      setLEDcol(87, 90, strip.Color(255, 0, 0));  // 2nd row
    }

    if (langLEDlayout == 3) {  // SWE:
      setLEDcol(96, 98, strip.Color(255, 0, 0));
      setLEDcol(125, 127, strip.Color(255, 0, 0));  // 2nd row
    }

    strip.show();
    delay(1000);
    ESP.restart();
    return;
  } else {
    back_color();
    Serial.println("Failed to obtain time");
    ESPUI.print(statusLabelID, "Failed to obtain time");

    if (langLEDlayout == 0) {  // DE:
      setLEDcol(1, 4, strip.Color(0, 255, 0));
      setLEDcol(27, 30, strip.Color(0, 255, 0));  // 2nd row
    }

    if (langLEDlayout == 1) {  // EN:
      setLEDcol(33, 36, strip.Color(0, 255, 0));
      setLEDcol(59, 62, strip.Color(0, 255, 0));  // 2nd row
    }

    if (langLEDlayout == 2) {  // NL:
      setLEDcol(69, 72, strip.Color(0, 255, 0));
      setLEDcol(87, 90, strip.Color(0, 255, 0));  // 2nd row
    }

    if (langLEDlayout == 3) {  // SWE:
      setLEDcol(96, 98, strip.Color(0, 255, 0));
      setLEDcol(125, 127, strip.Color(0, 255, 0));  // 2nd row
    }

    strip.show();
    delay(1000);
  }
  Serial.println("Got the time from NTP");
  setTimezone(timezone);
}
// ###########################################################################################################################################
void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time 1");
    return;
  }
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S zone %Z %z ");
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  iStartTime = String(timeStringBuff);
  // Serial.println(iStartTime);
  iHour = timeinfo.tm_hour;
  iMinute = timeinfo.tm_min;
  iSecond = timeinfo.tm_sec;
  // Serial.print("Time: ");
  // Serial.print(iHour);
  // Serial.print(":");
  // Serial.print(iMinute);
  // Serial.print(":");
  // Serial.println(iSecond);
  delay(1000);
}
// ###########################################################################################################################################
void setTime(int yr, int month, int mday, int hr, int minute, int sec, int isDst) {
  struct tm tm;
  tm.tm_year = yr - 1900;  // Set date
  tm.tm_mon = month - 1;
  tm.tm_mday = mday;
  tm.tm_hour = hr;  // Set time
  tm.tm_min = minute;
  tm.tm_sec = sec;
  tm.tm_isdst = isDst;  // 1 or 0
  time_t t = mktime(&tm);
  Serial.printf("Setting time: %s", asctime(&tm));
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);
}
// ###########################################################################################################################################


// ###########################################################################################################################################
// # GUI: Convert hex color value to RGB int values - TIME:
// ###########################################################################################################################################
void getRGBTIME(String hexvalue) {
  updatedevice = false;
  delay(1000);
  hexvalue.toUpperCase();
  char c[7];
  hexvalue.toCharArray(c, 8);
  int red = hexcolorToInt(c[1], c[2]);
  int green = hexcolorToInt(c[3], c[4]);
  int blue = hexcolorToInt(c[5], c[6]);
  redVal_time = red;
  greenVal_time = green;
  blueVal_time = blue;
  changedvalues = true;
  updatedevice = true;
}
// ###########################################################################################################################################
// # GUI: Convert hex color value to RGB int values - BACKGROUND:
// ###########################################################################################################################################
void getRGBBACK(String hexvalue) {
  updatedevice = false;
  delay(1000);
  hexvalue.toUpperCase();
  char c[7];
  hexvalue.toCharArray(c, 8);
  int red = hexcolorToInt(c[1], c[2]);
  int green = hexcolorToInt(c[3], c[4]);
  int blue = hexcolorToInt(c[5], c[6]);
  // Serial.print("red: ");
  // Serial.println(red);
  // Serial.print("green: ");
  // Serial.println(green);
  // Serial.print("blue: ");
  // Serial.println(blue);
  redVal_back = red;
  greenVal_back = green;
  blueVal_back = blue;
  changedvalues = true;
  updatedevice = true;
}

// ###########################################################################################################################################
// # GUI: Convert hex color value to RGB int values - helper function:
// ###########################################################################################################################################
int hexcolorToInt(char upper, char lower) {
  int uVal = (int)upper;
  int lVal = (int)lower;
  uVal = uVal > 64 ? uVal - 55 : uVal - 48;
  uVal = uVal << 4;
  lVal = lVal > 64 ? lVal - 55 : lVal - 48;
  return uVal + lVal;
}


// ###########################################################################################################################################
// # GUI: Color change for time color:
// ###########################################################################################################################################
void colCallTIME(Control* sender, int type) {
  getRGBTIME(sender->value);
}
// ###########################################################################################################################################
// # GUI: Color change for background color:
// ###########################################################################################################################################
void colCallBACK(Control* sender, int type) {
  // Serial.print("BACK Col: ID: ");
  // Serial.print(sender->id);
  // Serial.print(", Value: ");
  // Serial.println(sender->value);
  getRGBBACK(sender->value);
}


// ###########################################################################################################################################
// # GUI: Slider change for LED intensity: DAY
// ###########################################################################################################################################
void sliderBrightnessDay(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  intensity_day = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Slider change for LED intensity: NIGHT
// ###########################################################################################################################################
void sliderBrightnessNight(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  intensity_night = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Time Day Mode Start
// ###########################################################################################################################################
void call_day_time_start(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  day_time_start = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Time Day Mode Stop
// ###########################################################################################################################################
void call_day_time_stop(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  day_time_stop = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Convert IP-address value to string:
// ###########################################################################################################################################
String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]);
}


// ###########################################################################################################################################
// # ESP32 OTA update:
// ###########################################################################################################################################
const char* loginIndex =
  "<form name='loginForm'>"
  "<table width='20%' bgcolor='A09F9F' align='center'>"
  "<tr>"
  "<td colspan=2>"
  "<center><font size=4><b>WordClock Update Login Page</b></font></center>"
  "<br>"
  "</td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td>Username:</td>"
  "<td><input type='text' size=25 name='userid'><br></td>"
  "</tr>"
  "<br>"
  "<br>"
  "<tr>"
  "<td>Password:</td>"
  "<td><input type='Password' size=25 name='pwd'><br></td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
  "</tr>"
  "</table>"
  "</form>"
  "<script>"
  "function check(form)"
  "{"
  "if(form.userid.value=='WordClock' && form.pwd.value=='16x16')"
  "{"
  "window.open('/serverIndex')"
  "}"
  "else"
  "{"
  " alert('Error Password or Username')/*displays error message*/"
  "}"
  "}"
  "</script>";

const char* serverIndex =
  "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
  "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
  "<input type='file' name='update'>"
  "<input type='submit' value='Update'>"
  "</form>"
  "<div id='prg'>progress: 0%</div>"
  "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
  "},"
  "error: function (a, b, c) {"
  "}"
  "});"
  "});"
  "</script>";

void handleOTAupdate() {
  // OTA update server pages urls:
  updserver.on("/", HTTP_GET, []() {
    updserver.sendHeader("Connection", "close");
    updserver.send(200, "text/html", "WordClock web server on port 2022 is up. Please use the shown url and account credentials to update...");
  });

  updserver.on("/ota", HTTP_GET, []() {
    updserver.sendHeader("Connection", "close");
    updserver.send(200, "text/html", loginIndex);
  });

  updserver.on("/serverIndex", HTTP_GET, []() {
    updserver.sendHeader("Connection", "close");
    updserver.send(200, "text/html", serverIndex);
  });

  // handling uploading firmware file:
  updserver.on(
    "/update", HTTP_POST, []() {
      updserver.sendHeader("Connection", "close");
      updserver.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    },
    []() {
      HTTPUpload& upload = updserver.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {  //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        // flashing firmware to ESP
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {  //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
          delay(1000);
        } else {
          Update.printError(Serial);
        }
      }
    });
  updserver.begin();
}

// ###########################################################################################################################################
// # Extra Words web update server:
// ###########################################################################################################################################
void handleExtraWords() {
  updatedevice = false;
  delay(1000);

  // OTA update server pages urls:
  ewserver.on("/", HTTP_GET, []() {
    ewserver.sendHeader("Connection", "close");
    ewserver.send(200, "text/html", "WordClock web server on port 8080 is up...Display | DisplayOn | DisplayOff | ColorWipe | Rainbow | TheaterChase | PulseWhite");
  });
 // ################################################################## DE:

    ewserver.on("/Display", HTTP_GET, []() {
      ewserver.sendHeader("Connection", "close");
      if (Display == 0) {
        Display = 1;
        ewserver.send(200, "text/html", "Display text set active");
      } else {
        Display = 0;
        ewserver.send(200, "text/html", "Display text set inactive");
      }
      changedvalues = true;
    });

   ewserver.on("/DisplayOn", HTTP_GET, []() {
      ewserver.sendHeader("Connection", "close");
        Display = 1;
        ewserver.send(200, "text/html", "Display text set active");
      changedvalues = true;
    });

    ewserver.on("/DisplayOff", HTTP_GET, []() {
      ewserver.sendHeader("Connection", "close");
        Display = 0;
        ewserver.send(200, "text/html", "Display text set inactive");
      changedvalues = true;
    });

    ewserver.on("/ColorWipe", HTTP_GET, []() {
      ewserver.sendHeader("Connection", "close");
        ewserver.send(200, "text/html", "ColorWipe");
        colorWipe(strip.Color(  0, 255,   0), 5);
        changedvalues = true;
    });

   ewserver.on("/Rainbow", HTTP_GET, []() {
      ewserver.sendHeader("Connection", "close");
        ewserver.send(200, "text/html", "RainBow");
        rainbow(10);
        changedvalues = true;
    });

    ewserver.on("/TheaterChase", HTTP_GET, []() {
      ewserver.sendHeader("Connection", "close");
        ewserver.send(200, "text/html", "TheaterChase");
        theaterChase(strip.Color(redVal_time, greenVal_time, blueVal_time), 50); // Blue
        changedvalues = true;
    });

        ewserver.on("/PulseWhite", HTTP_GET, []() {
      ewserver.sendHeader("Connection", "close");
        ewserver.send(200, "text/html", "PulseWhite");
        pulseWhite(1);
        changedvalues = true;
    });

  ewserver.begin();
  updatedevice = true;
}

void colorWipe(uint32_t color, int wait) {
   updatedevice = false;
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
    updatedevice = true;
}

void rainbow(uint8_t wait) {
   updatedevice = false;
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
        strip.rainbow(firstPixelHue);
      strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
  updatedevice = true;
}

void theaterChase(uint32_t c, uint8_t wait) {
  updatedevice = false;
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
 updatedevice = true;
}

void pulseWhite(uint8_t wait) {
 updatedevice = false;
  for(int j=0; j<256; j++) { // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    strip.fill(strip.Color(j, j, j, strip.gamma8(j)));
    strip.show();
    delay(wait);
  }

  for(int j=255; j>=0; j--) { // Ramp down from 255 to 0
    strip.fill(strip.Color(j, j, j, strip.gamma8(j)));
    strip.show();
    delay(wait);
  }
   updatedevice = true;
}


// ###########################################################################################################################################
// # EOF - You have successfully reached the end of the code - well done ;-)
// ###########################################################################################################################################
