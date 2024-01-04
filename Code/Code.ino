// ###########################################################################################################################################
// #
// # WordClock code for the 3 printables WordClock 16x16 matrix projects:
// # https://www.printables.com/de/model/350568-wordclock-16x16-led-matrix-2023-v1
// # https://www.printables.com/de/model/361861-wordclock-16x16-led-matrix-2023-v2
// # https://www.printables.com/de/model/450556-wordclock-16x16-led-matrix-2023-v3
// #
// # Code by https://github.com/AWSW-de
// #
// # Released under licenses:
// # GNU General Public License v3.0: https://github.com/AWSW-de/WordClock-16x16-LED-matrix-2023/blob/main/LICENSE and
// # Creative Commons Attribution-NonCommercial 3.0 Unported License http://creativecommons.org/licenses/by-nc/3.0
// # Important: NonCommercial — You may not use the material for commercial purposes !
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
// # - AsyncTCP               // by me-no-dev:                    https://github.com/me-no-dev/AsyncTCP
// # - ESPAsyncWebServer      // by me-no-dev:                    https://github.com/me-no-dev/ESPAsyncWebServer
// # - ESPUI                  // by s00500:                       https://github.com/s00500/ESPUI/archive/refs/tags/2.2.3.zip
// # - ArduinoJson            // by bblanchon:                    https://github.com/bblanchon/ArduinoJson
// # - LITTLEFS               // by lorol:                        https://github.com/lorol/LITTLEFS
// #
// ###########################################################################################################################################
#include <WiFi.h>               // Used to connect the ESP32 to your WiFi
#include <WebServer.h>          // ESP32 OTA update function
#include <Update.h>             // ESP32 OTA update function
#include <Adafruit_NeoPixel.h>  // Used to drive the NeoPixel LEDs
#include "time.h"               // Used for NTP time requests
#include <AsyncTCP.h>           // Used for the internal web server
#include <ESPAsyncWebServer.h>  // Used for the internal web server
#include <DNSServer.h>          // Used for the internal web server
#include <ESPUI.h>              // Used for the internal web server
#include "esp_log.h"            // Disable WiFi debug warnings
#include <Preferences.h>        // Used to save the configuration to the ESP32 flash
#include "settings.h"           // Settings are stored in a seperate file to make to code better readable and to be able to switch to other settings faster


// ###########################################################################################################################################
// # Version number of the code:
// ###########################################################################################################################################
const char* WORD_CLOCK_VERSION = "V2.8.0";


// ###########################################################################################################################################
// # Internal web server settings:
// ###########################################################################################################################################
AsyncWebServer server(80);       // Web server for config
WebServer otaserver(8080);       // Web OTA ESP32 update server
AsyncWebServer ledserver(2023);  // Web server for HTML commands
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;


// ###########################################################################################################################################
// # Declartions and variables used in the functions:
// ###########################################################################################################################################
Preferences preferences;
int langLEDlayout, statusLanguageID;
int iHour = 0;
int iMinute = 0;
int iSecond = 0;
int iDay = 23;
int iMonth = 11;
int iYear = 2022;
bool updatedevice = true;
bool updatenow = false;
bool updatemode = false;
bool changedvalues = false;
String iStartTime = " ";
int redVal_back, greenVal_back, blueVal_back;
int redVal_time, greenVal_time, blueVal_time;
int intensity, intensity_day, intensity_night, intensity_web;
int set_web_intensity = 0;
int usenightmode, day_time_start, day_time_stop, statusNightMode;
int useshowip, usesinglemin, useStartupText;
int statusLabelID, statusNightModeID, intensity_web_HintID, DayNightSectionID, LEDsettingsSectionID;
int sliderBrightnessDayID, switchNightModeID, sliderBrightnessNightID, call_day_time_startID, call_day_time_stopID;
int RandomColor;
uint16_t text_colour_background;
uint16_t text_colour_time;
int switchRandomColorID, switchSingleMinutesID;
bool WiFIsetup = false;
String selectLangTXT;
uint16_t selectLang;



// ###########################################################################################################################################
// # Setup function that runs once at startup of the ESP:
// ###########################################################################################################################################
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("######################################################################");
  Serial.print("# WordClock startup of version: ");
  Serial.println(WORD_CLOCK_VERSION);
  Serial.println("######################################################################");
  preferences.begin("wordclock", false);  // Init ESP32 flash
  getFlashValues();                       // Read settings from flash
  strip.begin();                          // Init the LEDs
  strip.show();                           // Init the LEDs --> Set them to OFF
  intensity = intensity_day;              // Set the intenity to day mode for startup
  strip.setBrightness(intensity);         // Set LED brightness
  WIFI_SETUP();                           // WiFi login and startup of web services
}


// ###########################################################################################################################################
// # Loop function which runs all the time after the startup was done:
// ###########################################################################################################################################
void loop() {
  if ((WiFIsetup == true) || (testTime == 1)) {
    printLocalTime();                               // Locally get the time (NTP server requests done 1x per hour)
    if (updatedevice == true) {                     // Allow display updates (normal usage)
      if (changedvalues == true) setFlashValues();  // Write settings to flash
      update_display();                             // Update display (1x per minute regulary)
    }
    if (updatemode == true) otaserver.handleClient();  // ESP32 OTA update
  }
  dnsServer.processNextRequest();  // Update the web server
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



  // Section LED settings:
  // #####################
  LEDsettingsSectionID = ESPUI.separator("LED settings:");

  // Time color selector:
  char hex_time[7] = { 0 };
  sprintf(hex_time, "#%02X%02X%02X", redVal_time, greenVal_time, blueVal_time);
  text_colour_time = ESPUI.text("Time", colCallTIME, ControlColor::Dark, hex_time);
  ESPUI.setInputType(text_colour_time, "color");

  // Background color selector:
  char hex_back[7] = { 0 };
  sprintf(hex_back, "#%02X%02X%02X", redVal_back, greenVal_back, blueVal_back);
  text_colour_background = ESPUI.text("Background", colCallBACK, ControlColor::Dark, hex_back);
  ESPUI.setInputType(text_colour_background, "color");

  // Use random color mode:
  switchRandomColorID = ESPUI.switcher("Use random text color every new minute", &switchRandomColor, ControlColor::Dark, RandomColor);
  if (RandomColor == 1) {
    ESPUI.updateVisibility(text_colour_time, false);
    ESPUI.updateVisibility(text_colour_background, false);
  }

  // Show single minutes to display the minute exact time:
  switchSingleMinutesID = ESPUI.switcher("Show single minutes to display the minute exact time", &switchSingleMinutes, ControlColor::Dark, usesinglemin);

  // Show note when intensity is currently controlled via web-url usage and these internal settings get disabled:
  intensity_web_HintID = ESPUI.label("Manual settings disabled due to web URL usage:", ControlColor::Alizarin, "Restart WordClock or deactivate web control usage via http://" + IpAddress2String(WiFi.localIP()) + ":2023/config?LEDs=1");
  ESPUI.updateVisibility(intensity_web_HintID, false);



  // Section LED night mode settings:
  // ################################
  DayNightSectionID = ESPUI.separator("Day/Night LED brightness mode settings:");

  // Use night mode function:
  switchNightModeID = ESPUI.switcher("Use night mode to reduce brightness", &switchNightMode, ControlColor::Dark, usenightmode);

  // Intensity DAY slider selector: !!! DEFAULT LIMITED TO 64 of 255 !!!
  sliderBrightnessDayID = ESPUI.slider("Brightness during the day", &sliderBrightnessDay, ControlColor::Dark, intensity_day, 0, LEDintensityLIMIT);

  // Intensity NIGHT slider selector: !!! DEFAULT LIMITED TO 64 of 255 !!!
  sliderBrightnessNightID = ESPUI.slider("Brightness at night", &sliderBrightnessNight, ControlColor::Dark, intensity_night, 0, LEDintensityLIMIT);

  // Night mode status:
  statusNightModeID = ESPUI.label("Night mode status", ControlColor::Dark, "Night mode not used");

  // Day mode start time:
  call_day_time_startID = ESPUI.number("Day time starts at", call_day_time_start, ControlColor::Dark, day_time_start, 0, 11);

  // Day mode stop time:
  call_day_time_stopID = ESPUI.number("Day time ends after", call_day_time_stop, ControlColor::Dark, day_time_stop, 12, 23);



  // Section Startup:
  // ################
  ESPUI.separator("Startup:");

  // Startup WordClock text function:
  ESPUI.switcher("Show the 'WordClock' text on startup", &switchStartupText, ControlColor::Dark, useStartupText);

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



  // Section smart home control via web URLs:
  // ########################################
  ESPUI.separator("Smart home control via web URLs:");

  // About note:
  ESPUI.label("About note", ControlColor::Dark, "Control WordClock from your smart home environment via web URLs.");

  // Functions note:
  ESPUI.label("Functions", ControlColor::Dark, "You can turn the LEDs off or on via http commands to reduce energy consumption.");

  // Usage note:
  ESPUI.label("Usage hints and examples", ControlColor::Dark, "http://" + IpAddress2String(WiFi.localIP()) + ":2023");



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
  ESPUI.label("Update URL", ControlColor::Dark, "http://" + IpAddress2String(WiFi.localIP()) + ":8080");

  // AWSW software GitHub repository:
  ESPUI.label("Download newer software updates here", ControlColor::Dark, "https://github.com/AWSW-de/WordClock-16x16-LED-matrix-2023");



  // Section Language:
  // #################
  ESPUI.separator("Language:");

  // Set layout language in drop down field:
  if (langLEDlayout == 0) selectLangTXT = "German";
  if (langLEDlayout == 1) selectLangTXT = "English";
  if (langLEDlayout == 2) selectLangTXT = "Dutch";
  if (langLEDlayout == 3) selectLangTXT = "Swedish";
  if (langLEDlayout == 4) selectLangTXT = "Italian";
  if (langLEDlayout == 5) selectLangTXT = "French";
  if (langLEDlayout == 6) selectLangTXT = "Swiss German";
  if (langLEDlayout == 7) selectLangTXT = "Chinese";
  if (langLEDlayout == 8) selectLangTXT = "Swabian German";
  if (langLEDlayout == 9) selectLangTXT = "Bavarian";
  if (langLEDlayout == 10) selectLangTXT = "Luxemburgish";
  Serial.print("Selected language: ");
  Serial.println(selectLangTXT);

  // Change language:
  selectLang = ESPUI.addControl(ControlType::Select, "Change layout language", selectLangTXT, ControlColor::Dark, Control::noParent, call_langauge_select);
  ESPUI.addControl(ControlType::Option, "German", "0", ControlColor::Alizarin, selectLang);
  ESPUI.addControl(ControlType::Option, "English", "1", ControlColor::Alizarin, selectLang);
  ESPUI.addControl(ControlType::Option, "Dutch", "2", ControlColor::Alizarin, selectLang);
  ESPUI.addControl(ControlType::Option, "Swedish", "3", ControlColor::Alizarin, selectLang);
  ESPUI.addControl(ControlType::Option, "Italian", "4", ControlColor::Alizarin, selectLang);
  ESPUI.addControl(ControlType::Option, "French", "5", ControlColor::Alizarin, selectLang);
  ESPUI.addControl(ControlType::Option, "Swiss German", "6", ControlColor::Alizarin, selectLang);
  ESPUI.addControl(ControlType::Option, "Chinese", "7", ControlColor::Alizarin, selectLang);
  ESPUI.addControl(ControlType::Option, "Swabian German", "8", ControlColor::Alizarin, selectLang);
  ESPUI.addControl(ControlType::Option, "Bavarian", "9", ControlColor::Alizarin, selectLang);
  ESPUI.addControl(ControlType::Option, "Luxemburgish", "10", ControlColor::Alizarin, selectLang);

  // Current language:
  statusLanguageID = ESPUI.label("Current layout language", ControlColor::Dark, selectLangTXT);



  // Section Maintenance:
  // ####################
  ESPUI.separator("Maintenance:");

  // Restart WordClock:
  ESPUI.button("Restart", &buttonRestart, ControlColor::Dark, "Restart", (void*)1);

  // Reset WiFi settings:
  ESPUI.button("Reset WiFi settings", &buttonWiFiReset, ControlColor::Dark, "Reset WiFi settings", (void*)2);

  // Reset WordClock settings:
  ESPUI.button("Reset WordClock settings (except WiFi)", &buttonWordClockReset, ControlColor::Dark, "Reset WordClock settings", (void*)3);



  // Section License:
  // ####################
  ESPUI.separator("License information:");

  // License information:
  ESPUI.label("License information", ControlColor::Dark, "NonCommercial — You may not use the project for commercial purposes!");



  // Update night mode status text on startup:
  if (usenightmode == 1) {
    if ((iHour >= day_time_start) && (iHour <= day_time_stop)) {
      ESPUI.print(statusNightModeID, "Day time");
      if ((iHour == 0) && (day_time_stop == 23)) ESPUI.print(statusNightModeID, "Night time");  // Special function if day_time_stop set to 23 and time is 24, so 0...
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
  // if (debugtexts == 1) Serial.println("Read settings from flash: START");
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
  useStartupText = preferences.getUInt("useStartupText", useStartupText_default);
  usesinglemin = preferences.getUInt("usesinglemin", usesinglemin_default);
  RandomColor = preferences.getUInt("RandomColor", RandomColor_default);
  // if (debugtexts == 1) Serial.println("Read settings from flash: END");
}


// ###########################################################################################################################################
// # Write settings to flash:
// ###########################################################################################################################################
void setFlashValues() {
  // if (debugtexts == 1) Serial.println("Write settings to flash: START");
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
  preferences.putUInt("useStartupText", useStartupText);
  preferences.putUInt("usesinglemin", usesinglemin);
  preferences.putUInt("RandomColor", RandomColor);
  // if (debugtexts == 1) Serial.println("Write settings to flash: END");
  if (usenightmode == 1) {
    if ((iHour >= day_time_start) && (iHour <= day_time_stop)) {
      ESPUI.print(statusNightModeID, "Day time");
      if ((iHour == 0) && (day_time_stop == 23)) ESPUI.print(statusNightModeID, "Night time");  // Special function if day_time_stop set to 23 and time is 24, so 0...
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
void buttonWordClockReset(Control* sender, int type, void* param) {
  updatedevice = false;
  delay(100);
  ResetTextLEDs(strip.Color(0, 255, 0));
  delay(1000);
  Serial.println("Status: WORDCLOCK SETTINGS RESET REQUEST EXECUTED");
  // Save stored values for WiFi:
  String tempDelWiFiSSID = preferences.getString("WIFIssid");
  String tempDelWiFiPASS = preferences.getString("WIFIpass");
  preferences.clear();
  delay(100);
  preferences.putString("WIFIssid", tempDelWiFiSSID);  // Restore entered WiFi SSID
  preferences.putString("WIFIpass", tempDelWiFiPASS);  // Restore entered WiFi password
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
  preferences.putUInt("useStartupText", useStartupText_default);
  preferences.putUInt("usenightmode", usenightmode_default);
  preferences.putUInt("day_time_stop", day_time_stop_default);
  preferences.putUInt("day_time_stop", day_time_stop_default);
  preferences.putUInt("usesinglemin", usesinglemin_default);
  preferences.putUInt("RandomColor", RandomColor_default);
  delay(100);
  preferences.end();
  Serial.println("####################################################################################################");
  Serial.println("# WORDCLOCK SETTING WERE SET TO DEFAULT... WORDCLOCK WILL NOW RESTART... PLEASE CONFIGURE AGAIN... #");
  Serial.println("####################################################################################################");
  ClearDisplay();
  strip.show();
  delay(250);
  ESP.restart();
}


// ###########################################################################################################################################
// # GUI: Language selection
// ###########################################################################################################################################
void call_langauge_select(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  langLEDlayout = sender->value.toInt();
  // Set layout language in drop down field:
  if (langLEDlayout == 0) selectLangTXT = "German";
  if (langLEDlayout == 1) selectLangTXT = "English";
  if (langLEDlayout == 2) selectLangTXT = "Dutch";
  if (langLEDlayout == 3) selectLangTXT = "Swedish";
  if (langLEDlayout == 4) selectLangTXT = "Italian";
  if (langLEDlayout == 5) selectLangTXT = "French";
  if (langLEDlayout == 6) selectLangTXT = "Swiss German";
  if (langLEDlayout == 7) selectLangTXT = "Chinese";
  if (langLEDlayout == 8) selectLangTXT = "Swabian German";
  if (langLEDlayout == 9) selectLangTXT = "Bavarian";
  if (langLEDlayout == 10) selectLangTXT = "Luxemburgish";
  if (debugtexts == 1) {
    Serial.print("Selected language ID: ");
    Serial.println(langLEDlayout);
    Serial.print("Selected language: ");
    Serial.println(selectLangTXT);
  }
  ESPUI.print(statusLanguageID, selectLangTXT);
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # Clear the display:
// ###########################################################################################################################################
void ClearDisplay() {
  uint32_t c0 = strip.Color(0, 0, 0);
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, c0);
  }
}


// ###########################################################################################################################################
// # Show the IP-address on the display:
// ###########################################################################################################################################
void ShowIPaddress() {
  Serial.println("Show current IP-address on the display: " + IpAddress2String(WiFi.localIP()));
  int ipdelay = 2000;

  // Testing the digits:
  // for (int i = 0; i < 10; i++) {
  //   ClearDisplay();
  //   numbers(i, 3);
  //   numbers(i, 2);
  //   numbers(i, 1);
  //   strip.show();
  //   delay(ipdelay);
  // }

  // Octet 1:
  ClearDisplay();
  numbers(getDigit(int(WiFi.localIP()[0]), 2), 3);
  numbers(getDigit(int(WiFi.localIP()[0]), 1), 2);
  numbers(getDigit(int(WiFi.localIP()[0]), 0), 1);
  setLED(160, 160, 1);
  setLED(236, 239, 1);
  strip.show();
  delay(ipdelay);

  // // Octet 2:
  ClearDisplay();
  numbers(getDigit(int(WiFi.localIP()[1]), 2), 3);
  numbers(getDigit(int(WiFi.localIP()[1]), 1), 2);
  numbers(getDigit(int(WiFi.localIP()[1]), 0), 1);
  setLED(160, 160, 1);
  setLED(232, 239, 1);
  strip.show();
  delay(ipdelay);

  // // Octet 3:
  ClearDisplay();
  numbers(getDigit(int(WiFi.localIP()[2]), 2), 3);
  numbers(getDigit(int(WiFi.localIP()[2]), 1), 2);
  numbers(getDigit(int(WiFi.localIP()[2]), 0), 1);
  setLED(160, 160, 1);
  setLED(228, 239, 1);
  strip.show();
  delay(ipdelay);

  // // Octet 4:
  ClearDisplay();
  numbers(getDigit(int(WiFi.localIP()[3]), 2), 3);
  numbers(getDigit(int(WiFi.localIP()[3]), 1), 2);
  numbers(getDigit(int(WiFi.localIP()[3]), 0), 1);
  setLED(224, 239, 1);
  strip.show();
  delay(ipdelay);
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
              setLED(76, 76, 1);
              setLED(79, 79, 1);
              setLED(108, 108, 1);
              setLED(111, 111, 1);
              setLED(140, 140, 1);
              setLED(143, 143, 1);
              setLED(172, 175, 1);
              break;
            }
          case 1:
            {
              setLED(44, 44, 1);
              setLED(76, 76, 1);
              setLED(108, 108, 1);
              setLED(140, 140, 1);
              setLED(172, 172, 1);
              break;
            }
          case 2:
            {
              setLED(44, 47, 1);
              setLED(76, 76, 1);
              setLED(108, 111, 1);
              setLED(143, 143, 1);
              setLED(172, 175, 1);
              break;
            }
          case 3:
            {
              setLED(44, 47, 1);
              setLED(76, 76, 1);
              setLED(108, 111, 1);
              setLED(140, 140, 1);
              setLED(172, 175, 1);
              break;
            }
          case 4:
            {
              setLED(44, 44, 1);
              setLED(47, 47, 1);
              setLED(76, 76, 1);
              setLED(79, 79, 1);
              setLED(108, 111, 1);
              setLED(140, 140, 1);
              setLED(172, 172, 1);
              break;
            }
          case 5:
            {
              setLED(44, 47, 1);
              setLED(79, 79, 1);
              setLED(108, 111, 1);
              setLED(140, 140, 1);
              setLED(172, 175, 1);
              break;
            }
          case 6:
            {
              setLED(44, 47, 1);
              setLED(79, 79, 1);
              setLED(108, 111, 1);
              setLED(140, 140, 1);
              setLED(143, 143, 1);
              setLED(172, 175, 1);
              break;
            }
          case 7:
            {
              setLED(44, 47, 1);
              setLED(76, 76, 1);
              setLED(108, 108, 1);
              setLED(140, 140, 1);
              setLED(172, 172, 1);
              break;
            }
          case 8:
            {
              setLED(44, 47, 1);
              setLED(76, 76, 1);
              setLED(79, 79, 1);
              setLED(108, 111, 1);
              setLED(140, 140, 1);
              setLED(143, 143, 1);
              setLED(172, 175, 1);
              break;
            }
          case 9:
            {
              setLED(44, 47, 1);
              setLED(76, 76, 1);
              setLED(79, 79, 1);
              setLED(108, 111, 1);
              setLED(140, 140, 1);
              setLED(172, 175, 1);
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
              setLED(71, 71, 1);
              setLED(74, 74, 1);
              setLED(103, 103, 1);
              setLED(106, 106, 1);
              setLED(135, 135, 1);
              setLED(138, 138, 1);
              setLED(167, 170, 1);
              break;
            }
          case 1:
            {
              setLED(39, 39, 1);
              setLED(71, 71, 1);
              setLED(103, 103, 1);
              setLED(135, 135, 1);
              setLED(167, 167, 1);
              break;
            }
          case 2:
            {
              setLED(39, 42, 1);
              setLED(71, 71, 1);
              setLED(103, 106, 1);
              setLED(138, 138, 1);
              setLED(167, 170, 1);
              break;
            }
          case 3:
            {
              setLED(39, 42, 1);
              setLED(71, 71, 1);
              setLED(103, 106, 1);
              setLED(135, 135, 1);
              setLED(167, 170, 1);
              break;
            }
          case 4:
            {
              setLED(39, 39, 1);
              setLED(42, 42, 1);
              setLED(71, 71, 1);
              setLED(74, 74, 1);
              setLED(103, 106, 1);
              setLED(135, 135, 1);
              setLED(167, 167, 1);
              break;
            }
          case 5:
            {
              setLED(39, 42, 1);
              setLED(74, 74, 1);
              setLED(103, 106, 1);
              setLED(135, 135, 1);
              setLED(167, 170, 1);
              break;
            }
          case 6:
            {
              setLED(39, 42, 1);
              setLED(74, 74, 1);
              setLED(103, 106, 1);
              setLED(135, 135, 1);
              setLED(138, 138, 1);
              setLED(167, 170, 1);
              break;
            }
          case 7:
            {
              setLED(39, 42, 1);
              setLED(71, 71, 1);
              setLED(103, 103, 1);
              setLED(135, 135, 1);
              setLED(167, 167, 1);
              break;
            }
          case 8:
            {
              setLED(39, 42, 1);
              setLED(71, 71, 1);
              setLED(74, 74, 1);
              setLED(103, 106, 1);
              setLED(135, 135, 1);
              setLED(138, 138, 1);
              setLED(167, 170, 1);
              break;
            }
          case 9:
            {
              setLED(39, 42, 1);
              setLED(71, 71, 1);
              setLED(74, 74, 1);
              setLED(103, 106, 1);
              setLED(135, 135, 1);
              setLED(167, 170, 1);
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
              setLED(66, 66, 1);
              setLED(69, 69, 1);
              setLED(98, 98, 1);
              setLED(101, 101, 1);
              setLED(130, 130, 1);
              setLED(133, 133, 1);
              setLED(162, 165, 1);
              break;
            }
          case 1:
            {
              setLED(34, 34, 1);
              setLED(66, 66, 1);
              setLED(98, 98, 1);
              setLED(130, 130, 1);
              setLED(162, 162, 1);
              break;
            }
          case 2:
            {
              setLED(34, 37, 1);
              setLED(66, 66, 1);
              setLED(98, 101, 1);
              setLED(133, 133, 1);
              setLED(162, 165, 1);
              break;
            }
          case 3:
            {
              setLED(34, 37, 1);
              setLED(66, 66, 1);
              setLED(98, 101, 1);
              setLED(130, 130, 1);
              setLED(162, 165, 1);
              break;
            }
          case 4:
            {
              setLED(34, 34, 1);
              setLED(37, 37, 1);
              setLED(66, 66, 1);
              setLED(69, 69, 1);
              setLED(98, 101, 1);
              setLED(130, 130, 1);
              setLED(162, 162, 1);
              break;
            }
          case 5:
            {
              setLED(34, 37, 1);
              setLED(69, 69, 1);
              setLED(98, 101, 1);
              setLED(130, 130, 1);
              setLED(162, 165, 1);
              break;
            }
          case 6:
            {
              setLED(34, 37, 1);
              setLED(69, 69, 1);
              setLED(98, 101, 1);
              setLED(130, 130, 1);
              setLED(133, 133, 1);
              setLED(162, 165, 1);
              break;
            }
          case 7:
            {
              setLED(34, 37, 1);
              setLED(66, 66, 1);
              setLED(98, 98, 1);
              setLED(130, 130, 1);
              setLED(162, 162, 1);
              break;
            }
          case 8:
            {
              setLED(34, 37, 1);
              setLED(66, 66, 1);
              setLED(69, 69, 1);
              setLED(98, 101, 1);
              setLED(130, 130, 1);
              setLED(133, 133, 1);
              setLED(162, 165, 1);
              break;
            }
          case 9:
            {
              setLED(34, 37, 1);
              setLED(66, 66, 1);
              setLED(69, 69, 1);
              setLED(98, 101, 1);
              setLED(130, 130, 1);
              setLED(162, 165, 1);
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
void buttonRestart(Control* sender, int type, void* param) {
  updatedevice = false;
  ResetTextLEDs(strip.Color(0, 255, 0));
  if (changedvalues == true) setFlashValues();  // Write settings to flash
  preferences.end();
  delay(250);
  ESP.restart();
}


// ###########################################################################################################################################
// # GUI: Reset the WiFi settings of the WordClock:
// ###########################################################################################################################################
void buttonWiFiReset(Control* sender, int type, void* param) {
  updatedevice = false;
  Serial.println("Status: WIFI SETTINGS RESET REQUEST");
  ResetTextLEDs(strip.Color(0, 255, 0));
  WiFi.disconnect();  // DISCONNECT FROM WIFI
  delay(1000);
  preferences.putString("WIFIssid", "");                // Reset WiFi SSID
  preferences.putString("WIFIpass", "");                // Reste WiFi password
  preferences.putUInt("useshowip", useshowip_default);  // Show IP-address again
  preferences.end();
  Serial.println("Status: WIFI SETTINGS RESET REQUEST EXECUTED");
  Serial.println("####################################################################################################");
  Serial.println("# WIFI SETTING WERE SET TO DEFAULT... WORDCLOCK WILL NOW RESTART... PLEASE CONFIGURE WIFI AGAIN... #");
  Serial.println("####################################################################################################");
  delay(500);
  ESP.restart();
}


// ###########################################################################################################################################
// # GUI: Update the WordClock:
// ###########################################################################################################################################
void buttonUpdate(Control* sender, int type, void* param) {
  preferences.end();
  updatedevice = false;
  ESPUI.print(statusLabelID, "Update requested");
  ESPUI.updateButton(sender->id, "Update mode active now - Use the update url: >>>");
  if (updatemode == false) {
    updatemode = true;
    int32_t c = strip.Color(0, 0, 255);
    int TextWait = 500;
    showtext("U", TextWait, c);
    showtext("P", TextWait, c);
    showtext("D", TextWait, c);
    showtext("A", TextWait, c);
    showtext("T", TextWait, c);
    showtext("E", TextWait, c);
    setLED(0, 0, 1);
    setLED(15, 15, 1);
    setLED(239, 239, 1);
    setLED(224, 224, 1);
    int myArray[50];
    memset(myArray, 0, sizeof(myArray));
    int myArray2[] = { 42, 53, 74, 85, 106, 117, 138, 149, 170, 181, 169, 182, 183, 168, 167, 184, 166, 185, 186, 165, 154, 133, 122, 101, 90, 69, 58, 37 };  // U
    memcpy(myArray, myArray2, sizeof(myArray2));
    for (int element : myArray) {
      if (element != 0) {
        strip.setPixelColor(element, c);
      }
    }
    strip.show();
    Serial.println("Status: Update request");
  }
}


// ###########################################################################################################################################
// # Show a LED output for RESET in the different languages:
// ###########################################################################################################################################
void ResetTextLEDs(uint32_t color) {
  updatedevice = false;
  delay(1000);
  ClearDisplay();

  if (langLEDlayout == 0) {      // DE:
    setLEDcol(137, 138, color);  // RE
    setLEDcol(167, 168, color);  // SE
    setLEDcol(227, 227, color);  // T
  }

  if (langLEDlayout == 1) {      // EN:
    setLEDcol(100, 101, color);  // RE
    setLEDcol(174, 175, color);  // SE
    setLEDcol(227, 227, color);  // T
  }

  if (langLEDlayout == 2) {      // NL:
    setLEDcol(33, 33, color);    // R
    setLEDcol(96, 97, color);    // ES
    setLEDcol(164, 164, color);  // E
    setLEDcol(227, 227, color);  // T
  }

  if (langLEDlayout == 3) {    // SWE:
    setLEDcol(67, 71, color);  // R
  }

  if (langLEDlayout == 4) {    // IT:
    setLEDcol(11, 11, color);  // R
    setLEDcol(9, 9, color);    // E
    setLEDcol(45, 47, color);  // SET
  }

  if (langLEDlayout == 5) {    // FR:
    setLEDcol(11, 13, color);  // RES
    setLEDcol(5, 5, color);    // E
    setLEDcol(36, 36, color);  // T
  }

  if (langLEDlayout == 6) {    // GSW:
    setLEDcol(11, 15, color);  // RESET
  }

  if (langLEDlayout == 7) {    // CN:
    setLEDcol(38, 39, color);  // RESET 重置
  }

  if (langLEDlayout == 8) {      // SWABIAN GERMAN:
    setLEDcol(40, 41, color);    // RE
    setLEDcol(133, 134, color);  // SE
    setLEDcol(204, 204, color);  // T
  }

  if (langLEDlayout == 9) {      // BAVARIAN:
    setLEDcol(106, 106, color);  // R
    setLEDcol(133, 133, color);  // E
    setLEDcol(175, 175, color);  // S
    setLEDcol(170, 170, color);  // E
    setLEDcol(160, 160, color);  // T
  }

  if (langLEDlayout == 10) {     // LTZ:
    setLEDcol(38, 38, color);    // R
    setLEDcol(57, 57, color);    // 2nd row
    setLEDcol(73, 73, color);    // E
    setLEDcol(86, 86, color);    // 2nd row
    setLEDcol(103, 103, color);  // S
    setLEDcol(120, 120, color);  // 2nd row
    setLEDcol(139, 139, color);  // E
    setLEDcol(148, 148, color);  // 2nd row
    setLEDcol(171, 171, color);  // T
    setLEDcol(180, 180, color);  // 2nd row
  }

  strip.show();
}


// ###########################################################################################################################################
// # Actual function, which controls 1/0 of the LED and their sibling with color value:
// ###########################################################################################################################################
void setLEDcol(int ledNrFrom, int ledNrTo, uint32_t color) {
  if (ledNrFrom > ledNrTo) {
    setLEDcol(ledNrTo, ledNrFrom, color);  // Sets LED numbers in correct order
  } else {
    for (int i = ledNrFrom; i <= ledNrTo; i++) {
      if ((i >= 0) && (i < NUMPIXELS)) {
        strip.setPixelColor(i, color);
        int pairedLED = getPairedLED(i);
        if ((pairedLED >= 0) && (pairedLED < NUMPIXELS))
          strip.setPixelColor(pairedLED, color);
      }
    }
  }
}


// ###########################################################################################################################################
// # Get the sibling led for a two-led lit character (with 32 leds / 16 chars per row):
// ###########################################################################################################################################
int getPairedLED(int ledNumber) {
  const int ledsPerLine = ROWPIXELS * 2;
  int row = ledNumber / ledsPerLine;
  int positionInRow = ledNumber % ledsPerLine;
  return row * ledsPerLine + (ledsPerLine - 1 - positionInRow);
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
      if ((iHour >= day_time_start) && (iHour <= day_time_stop)) {
        intensity = intensity_day;
        if ((iHour == 0) && (day_time_stop == 23)) intensity = intensity_night;  // Special function if day_time_stop set to 23 and time is 24, so 0...
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
      ESPUI.updateVisibility(text_colour_background, false);
      ESPUI.updateVisibility(text_colour_time, false);
      redVal_back = 0;
      greenVal_back = 0;
      blueVal_back = 0;
      break;
    case S_INACTIVE:
      RandomColor = 0;
      ESPUI.updateVisibility(text_colour_background, true);
      ESPUI.updateVisibility(text_colour_time, true);
      ESPUI.jsonReload();
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
// # GUI: Show WordClock text switch:
// ###########################################################################################################################################
void switchStartupText(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      useStartupText = 1;
      break;
    case S_INACTIVE:
      useStartupText = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # Update the display / time on it:
// ###########################################################################################################################################
void update_display() {
  // if (debugtexts == 1) Serial.println("Time: " + iStartTime);

  // Show the current time or use the time text test function:
  if (testTime == 0) {  // Show the current time:
    show_time(iHour, iMinute);
  } else {  // TEST THE DISPLAY TIME OUTPUT:
    ClearDisplay();
    strip.show();
    Serial.println(" ");
    Serial.println("Show 'TEST' text...");
    strip.setBrightness(25);
    redVal_back = 0;
    greenVal_back = 0;
    blueVal_back = 0;
    usenightmode = 0;
    uint32_t c = strip.Color(redVal_time, greenVal_time, blueVal_time);
    int TextWait = 500;
    showtext("T", TextWait, c);
    showtext("E", TextWait, c);
    showtext("S", TextWait, c);
    showtext("T", TextWait, c);
    for (int i = 1; i <= 12; i++) {  // 12 hours only:
      show_time(i, 0);
      delay(3000);
    }
    Serial.println(" ");
    Serial.println(" ");
    Serial.println(" ");
    for (int i = 0; i <= 55; i += 5) {  // 5 minutes steps only:
      show_time(9, i);
      delay(3000);
    }
    Serial.println(" ");
    Serial.println(" ");
    Serial.println(" ");
    for (int i = 9; i <= 12; i++) {  // Hours 9 to 12 with all minute texts:
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
static int lastHourSet = -1;
static int lastMinutesSet = -1;
void show_time(int hours, int minutes) {

  if ((lastHourSet == hours && lastMinutesSet == minutes) && updatenow == false) {  // Reduce display updates to new minutes and new config updates
    return;
  }

  updatenow = false;
  lastHourSet = hours;
  lastMinutesSet = minutes;

  // Show current time of display update:
  // if (debugtexts == 1) Serial.println("Update display now: " + String(hours) + ":" + String(minutes) + ":" + String(iSecond));

  // Night/Day mode intensity setting:
  if ((usenightmode == 1) && (set_web_intensity == 0)) {
    if ((iHour >= day_time_start) && (iHour <= day_time_stop)) {
      intensity = intensity_day;
      if ((iHour == 0) && (day_time_stop == 23)) intensity = intensity_night;  // Special function if day_time_stop set to 23 and time is 24, so 0...
    } else {
      intensity = intensity_night;
    }
    if (testDayNightmode == 1) {  // Test day/night times function:
      Serial.println("############################################################################################");
      Serial.println("Current time day/night test: " + String(hours) + ":" + String(minutes) + ":" + String(iSecond));
      Serial.println("Current settings: day_time_start: " + String(day_time_start) + " day_time_stop: " + String(day_time_stop));
      for (int i = 0; i < 24; i++) {
        String daynightvar = "-";
        if ((i >= day_time_start) && (i <= day_time_stop)) {
          daynightvar = "Day time";
          if ((i == 0) && (day_time_stop == 23)) daynightvar = "Night time";
        } else {
          daynightvar = "Night time";
        }
        Serial.println("Current hour: " + String(i) + " --> " + daynightvar);
      }
      testDayNightmode = 0;  // Show the list 1x only
      Serial.println("############################################################################################");
    }
  } else {  // Control intensity by WordClock settings or via HTML command:
    if (set_web_intensity == 0) intensity = intensity_day;
    if (set_web_intensity == 1) intensity = 0;
  }
  strip.setBrightness(intensity);

  // Set background color:
  back_color();

  // Static text color or random color mode:
  if (RandomColor == 0) colorRGB = strip.Color(redVal_time, greenVal_time, blueVal_time);
  if (RandomColor == 1) colorRGB = strip.Color(random(255), random(255), random(255));

  // Display time:
  iHour = hours;
  iMinute = minutes;

  // Test a special time:
  if (testspecialtime == 1) {
    Serial.println("Special time test active: " + String(test_hour) + ":" + String(test_minute));
    iHour = test_hour;
    iMinute = test_minute;
  }

  // divide minute by 5 to get value for display control
  int minDiv = iMinute / 5;
  if (usesinglemin == 1) showMinutes(iMinute);

  // ########################################################### DE:
  if (langLEDlayout == 0) {  // DE:

    // ES IST:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(10, 12, colorRGB);
    if (testPrintTimeTexts == 1) {
      Serial.println("");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(" --> ES IST ");
    }

    // FÜNF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(76, 79, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
    }
    // VIERTEL:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(69, 75, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VIERTEL ");
    }
    // ZEHN: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(32, 35, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
    }
    // ZWANZIG:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(41, 47, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZWANZIG ");
    }
    // NACH:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(64, 67, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("NACH ");
    }
    // VOR:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(109, 111, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VOR ");
    }
    // HALB:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(104, 107, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("HALB ");
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
            if (testPrintTimeTexts == 1) Serial.print("EIN ");
          }
          if ((xHour == 1) && (iMinute > 4)) {
            setLEDcol(168, 171, colorRGB);  // EINS (S in EINS) (just used if not point 1 o'clock)
            if (testPrintTimeTexts == 1) Serial.print("EINS ");
          }
          break;
        }
      case 2:
        {
          setLEDcol(140, 143, colorRGB);  // ZWEI
          if (testPrintTimeTexts == 1) Serial.print("ZWEI ");
          break;
        }
      case 3:
        {
          setLEDcol(136, 139, colorRGB);  // DREI
          if (testPrintTimeTexts == 1) Serial.print("DREI ");
          break;
        }
      case 4:
        {
          setLEDcol(128, 131, colorRGB);  // VIER
          if (testPrintTimeTexts == 1) Serial.print("VIER ");
          break;
        }
      case 5:
        {
          setLEDcol(160, 163, colorRGB);  // FUENF
          if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
          break;
        }
      case 6:
        {
          setLEDcol(164, 168, colorRGB);  // SECHS
          if (testPrintTimeTexts == 1) Serial.print("SECHS ");
          break;
        }
      case 7:
        {
          setLEDcol(202, 207, colorRGB);  // SIEBEN
          if (testPrintTimeTexts == 1) Serial.print("SIEBEN ");
          break;
        }
      case 8:
        {
          setLEDcol(172, 175, colorRGB);  // ACHT
          if (testPrintTimeTexts == 1) Serial.print("ACHT ");
          break;
        }
      case 9:
        {
          setLEDcol(132, 135, colorRGB);  // NEUN
          if (testPrintTimeTexts == 1) Serial.print("NEUN ");
          break;
        }
      case 10:
        {
          setLEDcol(99, 102, colorRGB);  // ZEHN (Stunden)
          if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
          break;
        }
      case 11:
        {
          setLEDcol(96, 98, colorRGB);  // ELF
          if (testPrintTimeTexts == 1) Serial.print("ELF ");
          break;
        }
      case 12:
        {
          setLEDcol(197, 201, colorRGB);  // ZWÖLF
          if (testPrintTimeTexts == 1) Serial.print("ZWÖLF ");
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(192, 194, colorRGB);  // UHR
      if (testPrintTimeTexts == 1) Serial.print("UHR ");
    }
  }

  // ########################################################### EN:
  if (langLEDlayout == 1) {  // EN:

    // IT IS:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(11, 12, colorRGB);

    // FIVE: (Minutes)                         // x:05 + x:25 + x:35 + x:55
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(38, 41, colorRGB);
    }
    // QUARTER:                                // x:15 + X:45
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(72, 78, colorRGB);
    }
    // A:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(5, 5, colorRGB);
    }
    // TEN: (Minutes)                          // x:10 + x:50
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(0, 2, colorRGB);
    }
    // TWENTY:                                 // x:20 + x:25 + x:35 + x:40
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(42, 47, colorRGB);
    }
    // PAST:                                   // x:05 + x:10 + x:15 + x:20 + x:25 + x:30
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 5) || (minDiv == 6)) {
      setLEDcol(66, 69, colorRGB);
    }
    // TO:                                     // x:35 + x:40 + x:45 + x:50 + x:55
    if ((minDiv == 7) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(65, 66, colorRGB);
    }
    // HALF:                                   // x:30
    if ((minDiv == 6)) {
      setLEDcol(3, 6, colorRGB);
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
          break;
        }
      case 2:
        {
          setLEDcol(105, 107, colorRGB);  // TWO
          break;
        }
      case 3:
        {
          setLEDcol(99, 103, colorRGB);  // THREE
          break;
        }
      case 4:
        {
          setLEDcol(128, 131, colorRGB);  // FOUR
          break;
        }
      case 5:
        {
          setLEDcol(108, 111, colorRGB);  // FIVE
          break;
        }
      case 6:
        {
          setLEDcol(163, 165, colorRGB);  // SIX
          break;
        }
      case 7:
        {
          setLEDcol(171, 175, colorRGB);  // SEVEN
          break;
        }
      case 8:
        {
          setLEDcol(166, 170, colorRGB);  // EIGHT
          break;
        }
      case 9:
        {
          setLEDcol(204, 207, colorRGB);  // NINE
          break;
        }
      case 10:
        {
          setLEDcol(96, 98, colorRGB);  // TEN
          break;
        }
      case 11:
        {
          setLEDcol(138, 143, colorRGB);  // ELEVEN
          break;
        }
      case 12:
        {
          setLEDcol(132, 137, colorRGB);  // TWELVE
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(193, 199, colorRGB);  // O'CLOCK
    }
  }

  // ########################################################### NL:
  if (langLEDlayout == 2) {  // NL:

    // HET IS:
    setLEDcol(13, 15, colorRGB);
    setLEDcol(10, 11, colorRGB);
    // VIJF: (Minuten) x:05, x:25, x:35, x:55
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(0, 3, colorRGB);
    }
    // KWART: x:15, x:45
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(38, 42, colorRGB);
    }
    // TIEN: (Minuten) x:10, x:50
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(44, 47, colorRGB);
    }
    // TIEN: (TIEN VOOR HALF, TIEN OVER HALF) x:20, x:40 (on request not set to TWINTIG OVER)
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(44, 47, colorRGB);
    }
    // OVER: x:05, x:10, x:15, x:35, x:40
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(33, 36, colorRGB);
    }
    // VOOR: x:20, x:25, x:45, x:50, x:55
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(64, 67, colorRGB);
    }
    // HALF:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 6) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(107, 110, colorRGB);
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
          setLEDcol(99, 101, colorRGB);  // EEN
          break;
        }
      case 2:
        {
          setLEDcol(203, 206, colorRGB);  // TWEE
          break;
        }
      case 3:
        {
          setLEDcol(164, 167, colorRGB);  // DRIE
          break;
        }
      case 4:
        {
          setLEDcol(198, 201, colorRGB);  // VIER
          break;
        }
      case 5:
        {
          setLEDcol(160, 163, colorRGB);  // VIJF
          break;
        }
      case 6:
        {
          setLEDcol(96, 98, colorRGB);  // ZES
          break;
        }
      case 7:
        {
          setLEDcol(129, 133, colorRGB);  // ZEVEN
          break;
        }
      case 8:
        {
          setLEDcol(102, 105, colorRGB);  // ACHT
          break;
        }
      case 9:
        {
          setLEDcol(171, 175, colorRGB);  // NEGEN
          break;
        }
      case 10:
        {
          setLEDcol(140, 143, colorRGB);  // TIEN (Stunden)
          break;
        }
      case 11:
        {
          setLEDcol(168, 170, colorRGB);  // ELF
          break;
        }
      case 12:
        {
          setLEDcol(134, 139, colorRGB);  // TWAALF
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(193, 195, colorRGB);  // UUR
    }
  }

  // ########################################################### SWE:
  if (langLEDlayout == 3) {  // SWE:

    // KLOCKAN ÄR:
    setLEDcol(9, 15, colorRGB);
    setLEDcol(5, 6, colorRGB);
    // FEM: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(64, 66, colorRGB);
    }
    // KVART:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(36, 40, colorRGB);
    }
    // TIO: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(72, 74, colorRGB);
    }
    // TJUGO:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(75, 79, colorRGB);
    }
    // ÖVER:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(108, 111, colorRGB);
    }
    // I:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(97, 97, colorRGB);
    }
    // HALV:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(140, 143, colorRGB);
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
          }
          break;
        }
      case 2:
        {
          setLEDcol(160, 162, colorRGB);  // TVÅ
          break;
        }
      case 3:
        {
          setLEDcol(132, 134, colorRGB);  // TRE
          break;
        }
      case 4:
        {
          setLEDcol(197, 200, colorRGB);  // FYRA
          break;
        }
      case 5:
        {
          setLEDcol(194, 196, colorRGB);  // FEM
          break;
        }
      case 6:
        {
          setLEDcol(201, 203, colorRGB);  // SEX
          break;
        }
      case 7:
        {
          setLEDcol(173, 175, colorRGB);  // SJU
          break;
        }
      case 8:
        {
          setLEDcol(128, 131, colorRGB);  // ÅTTA
          break;
        }
      case 9:
        {
          setLEDcol(135, 137, colorRGB);  // NIO
          break;
        }
      case 10:
        {
          setLEDcol(168, 170, colorRGB);  // TIO (Stunden)
          break;
        }
      case 11:
        {
          setLEDcol(204, 207, colorRGB);  // ELVA
          break;
        }
      case 12:
        {
          setLEDcol(163, 166, colorRGB);  // TOLV
          break;
        }
    }
  }

  // ########################################################### IT:
  if (langLEDlayout == 4) {  // IT:

    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 40 hour needs to be counted up:
    if (iMinute >= 40) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }

    // SONO LE:
    if (xHour > 1) {                // NOTE: Displayed only from 2 to 23
      setLEDcol(9, 10, colorRGB);   // LE
      setLEDcol(12, 15, colorRGB);  // SONO
      if (testPrintTimeTexts == 1) {
        Serial.println("");
        Serial.print(hours);
        Serial.print(":");
        Serial.print(minutes);
        Serial.print(" --> SONO LE ");
      }
    }

    switch (xHour) {
      case 1:
        {
          setLEDcol(0, 0, colorRGB);      // È
          setLEDcol(104, 108, colorRGB);  // L’UNA
          if (testPrintTimeTexts == 1) Serial.print("È L’UNA ");
          break;
        }
      case 2:
        {
          setLEDcol(101, 103, colorRGB);  // DUE
          if (testPrintTimeTexts == 1) Serial.print("DUE ");
          break;
        }
      case 3:
        {
          setLEDcol(109, 111, colorRGB);  // TRE
          if (testPrintTimeTexts == 1) Serial.print("TRE ");
          break;
        }
      case 4:
        {
          setLEDcol(73, 79, colorRGB);  // QUATTRO
          if (testPrintTimeTexts == 1) Serial.print("QUATTRO ");
          break;
        }
      case 5:
        {
          setLEDcol(64, 69, colorRGB);  // CINQUE
          if (testPrintTimeTexts == 1) Serial.print("CINQUE ");
          break;
        }
      case 6:
        {
          setLEDcol(40, 42, colorRGB);  // SEI
          if (testPrintTimeTexts == 1) Serial.print("SEI ");
          break;
        }
      case 7:
        {
          setLEDcol(43, 47, colorRGB);  // SETTE
          if (testPrintTimeTexts == 1) Serial.print("SETTE ");
          break;
        }
      case 8:
        {
          setLEDcol(70, 73, colorRGB);  // OTTO
          if (testPrintTimeTexts == 1) Serial.print("OTTO ");
          break;
        }
      case 9:
        {
          setLEDcol(97, 100, colorRGB);  // NOVE
          if (testPrintTimeTexts == 1) Serial.print("NOVE ");
          break;
        }
      case 10:
        {
          setLEDcol(138, 142, colorRGB);  // DIECI
          if (testPrintTimeTexts == 1) Serial.print("DIECI ");
          break;
        }
      case 11:
        {
          setLEDcol(1, 6, colorRGB);  // UNDICI
          if (testPrintTimeTexts == 1) Serial.print("UNDICI ");
          break;
        }
      case 12:
        {
          setLEDcol(34, 39, colorRGB);  // DODICI
          if (testPrintTimeTexts == 1) Serial.print("DODICI ");
          break;
        }
    }

    // E:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(134, 134, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("E ");
    }
    // MENO:
    if ((minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(132, 135, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("MENO ");
    }
    // 5/55: CINQUE
    if ((minDiv == 1) || (minDiv == 11)) {
      setLEDcol(162, 167, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("CINQUE ");
    }
    // 15/45: UN QUARTO
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(128, 129, colorRGB);  // UN
      setLEDcol(234, 239, colorRGB);  // QUARTO
      if (testPrintTimeTexts == 1) Serial.print("UN QUARTO ");
    }
    // 10/50: DIECI
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(192, 196, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("DIECI ");
    }
    // 20/40: VENTI
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(203, 207, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VENTI ");
    }
    // 25: VENTICINQUE
    if (minDiv == 5) {
      setLEDcol(197, 207, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VENTICINQUE ");
    }
    // 30: TRENTA
    if (minDiv == 6) {
      setLEDcol(168, 173, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("TRENTA ");
    }
    // 35: TRENTACINQUE
    if (minDiv == 7) {
      setLEDcol(162, 173, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("TRENTACINQUE ");
    }
  }

  // ########################################################### FR:
  if (langLEDlayout == 5) {  // FR:

    // IL EST:
    setLEDcol(14, 15, colorRGB);  // IL
    setLEDcol(10, 12, colorRGB);  // EST
    // CINQ: (Minutes) x:05, x:25, x:35, x:55
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(197, 200, colorRGB);
    }
    // ET QUART: x:15,
    if ((minDiv == 3)) {
      setLEDcol(171, 172, colorRGB);  // ET
      setLEDcol(193, 197, colorRGB);  // QUART
    }
    // LE QUART: x:45
    if ((minDiv == 9)) {
      setLEDcol(172, 173, colorRGB);  // LE
      setLEDcol(193, 197, colorRGB);  // QUART
    }
    // DIX: (Minutes) x:10, x:50
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(167, 169, colorRGB);
    }
    // VINGT: x:20, x:40
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(202, 206, colorRGB);
    }
    // VINGT-: x:25, x:35
    if ((minDiv == 5) || (minDiv == 7)) {
      setLEDcol(201, 206, colorRGB);
    }
    // MOINS: x:35, x:40 x:45, x:50, x:55
    if ((minDiv == 7) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(128, 132, colorRGB);
    }
    // ET DEMIE: x:30
    if ((minDiv == 6)) {
      setLEDcol(171, 172, colorRGB);  // ET
      setLEDcol(161, 165, colorRGB);  // DEMIE
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 35 hour needs to be counted up:
    if (iMinute >= 35) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          setLEDcol(141, 143, colorRGB);  // UNE
          setLEDcol(135, 139, colorRGB);  // HEURE
          break;
        }
      case 2:
        {
          setLEDcol(96, 99, colorRGB);    // DEUX
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 3:
        {
          setLEDcol(107, 111, colorRGB);  // TROIS
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 4:
        {
          setLEDcol(42, 47, colorRGB);    // QUATRE
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 5:
        {
          setLEDcol(1, 4, colorRGB);      // CINQ
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 6:
        {
          setLEDcol(64, 66, colorRGB);    // SIX
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 7:
        {
          setLEDcol(104, 107, colorRGB);  // SEPT
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 8:
        {
          setLEDcol(32, 35, colorRGB);    // HUIT
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 9:
        {
          setLEDcol(100, 103, colorRGB);  // NEUF
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 10:
        {
          setLEDcol(77, 79, colorRGB);    // DIX
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 11:
        {
          setLEDcol(5, 8, colorRGB);      // ONZE
          setLEDcol(134, 139, colorRGB);  // HEURES
          break;
        }
      case 12:
        {
          // MINUIT (0) or MIDI (12)
          if (iHour == 0 || (iHour == 23 && iMinute >= 35)) setLEDcol(36, 41, colorRGB);   // MINUIT (0)
          if (iHour == 12 || (iHour == 11 && iMinute >= 35)) setLEDcol(73, 76, colorRGB);  // MIDI (12)
          break;
        }
    }
  }

  // ########################################################### GSW:
  if (langLEDlayout == 6) {  // GSW:

    // ES ISCH:
    setLEDcol(13, 14, colorRGB);  // ES
    setLEDcol(4, 7, colorRGB);    // ISCH
    // FÜÜF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(44, 47, colorRGB);
    }
    // VIERTEL:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(72, 78, colorRGB);
    }
    // ZÄH: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(34, 36, colorRGB);
    }
    // ZWÄNZG:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(65, 70, colorRGB);
    }
    // AB:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(110, 111, colorRGB);
    }
    // VOR:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(107, 109, colorRGB);
    }
    // HALBI:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(101, 105, colorRGB);
    }


    // set hour from 1 to 12 (at noon, or midnight)
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
          setLEDcol(128, 130, colorRGB);  // EIS
          break;
        }
      case 2:
        {
          setLEDcol(136, 139, colorRGB);  // ZWEI
          break;
        }
      case 3:
        {
          setLEDcol(96, 99, colorRGB);  // DRÜÜ
          break;
        }
      case 4:
        {
          setLEDcol(160, 164, colorRGB);  // VIERI
          break;
        }
      case 5:
        {
          setLEDcol(171, 175, colorRGB);  // FÜÜFI
          break;
        }
      case 6:
        {
          setLEDcol(165, 170, colorRGB);  // SÄCHSI
          break;
        }
      case 7:
        {
          setLEDcol(202, 207, colorRGB);  // SIEBNI
          break;
        }
      case 8:
        {
          setLEDcol(192, 196, colorRGB);  // ACHTI
          break;
        }
      case 9:
        {
          setLEDcol(131, 135, colorRGB);  // NÜÜNI
          break;
        }
      case 10:
        {
          setLEDcol(197, 201, colorRGB);  // ZÄHNI (Stunden)
          break;
        }
      case 11:
        {
          setLEDcol(140, 143, colorRGB);  // ELFI
          break;
        }
      case 12:
        {
          setLEDcol(233, 238, colorRGB);  // ZWÖLFI
          break;
        }
    }
  }

  // ########################################################### CN:
  if (langLEDlayout == 7) {  // CN:

    // IT IS: 现在 时间
    setLEDcol(44, 45, colorRGB);
    setLEDcol(40, 41, colorRGB);
    // 零五分                         // x:05
    if ((minDiv == 1)) {
      setLEDcol(101, 103, colorRGB);
    }
    // 十分                         // x:10
    if ((minDiv == 2)) {
      setLEDcol(98, 99, colorRGB);
    }
    // 十五分                         // x:15
    if ((minDiv == 3)) {
      setLEDcol(138, 140, colorRGB);
    }
    // 二十分                         // x:20
    if ((minDiv == 4)) {
      setLEDcol(98, 100, colorRGB);
    }
    // 二十五分                         // x:25
    if ((minDiv == 5)) {
      setLEDcol(138, 141, colorRGB);
    }
    // 三十分                         // x:30
    if ((minDiv == 6)) {
      setLEDcol(135, 137, colorRGB);
    }
    // 三十五分                         // x:35
    if ((minDiv == 7)) {
      setLEDcol(170, 173, colorRGB);
    }
    // 四十分                         // x:40
    if ((minDiv == 8)) {
      setLEDcol(132, 134, colorRGB);
    }
    // 四十五分                         // x:45
    if ((minDiv == 9)) {
      setLEDcol(166, 169, colorRGB);
    }
    // 五十分                         // x:50
    if ((minDiv == 10)) {
      setLEDcol(163, 165, colorRGB);
    }
    // 五十五分                         // x:55
    if ((minDiv == 11)) {
      setLEDcol(202, 205, colorRGB);
    }

    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0) {
      xHour = 12;
    }


    switch (xHour) {
      case 1:
        {
          setLEDcol(75, 76, colorRGB);  // 一点
          break;
        }
      case 2:
        {
          setLEDcol(72, 73, colorRGB);  // 二点
          break;
        }
      case 3:
        {
          setLEDcol(36, 37, colorRGB);  // 三点
          break;
        }
      case 4:
        {
          setLEDcol(34, 35, colorRGB);  // 四点
          break;
        }
      case 5:
        {
          setLEDcol(70, 71, colorRGB);  // 五点
          break;
        }
      case 6:
        {
          setLEDcol(68, 69, colorRGB);  // 六点
          break;
        }
      case 7:
        {
          setLEDcol(66, 67, colorRGB);  // 七点
          break;
        }
      case 8:
        {
          setLEDcol(108, 109, colorRGB);  // 八点
          break;
        }
      case 9:
        {
          setLEDcol(106, 107, colorRGB);  // 九点
          break;
        }
      case 10:
        {
          setLEDcol(104, 105, colorRGB);  // 十点
          break;
        }
      case 11:
        {
          setLEDcol(75, 77, colorRGB);  // 十一点
          break;
        }
      case 12:
        {
          setLEDcol(72, 74, colorRGB);  // 十二点
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(162, 162, colorRGB);  // 整
    }
  }

  // ########################################################### SWABIAN GERMAN:
  if (langLEDlayout == 8) {  // SWABIAN GERMAN:

    // ES ISCH:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(9, 12, colorRGB);
    if (testPrintTimeTexts == 1) {
      Serial.println("");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(" --> ES ISCH ");
    }

    // FÜNF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(76, 79, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
    }

    // VIERTL:
    if (minDiv == 3) {
      setLEDcol(0, 5, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VIERTL ");
    }

    // DREIVIERTL:
    if (minDiv == 9) {
      setLEDcol(33, 42, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("DREIVIERTL ");
    }

    // ZEHN: (Minuten)
    if ((minDiv == 2) || (minDiv == 4) || (minDiv == 8) || (minDiv == 10)) {
      setLEDcol(71, 74, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
    }

    // NACH:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(65, 68, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("NACH ");
    }

    // VOR:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(109, 111, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VOR ");
    }

    // HALB:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 6) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(102, 105, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("HALB ");
    }

    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 25 hour needs to be counted up:
    // fuenf vor halb 2 = 13:15
    if (iMinute >= 15) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }

    switch (xHour) {
      case 1:
        {
          if (xHour == 1) {
            setLEDcol(165, 168, colorRGB);  // OISE
            if (testPrintTimeTexts == 1) Serial.print("OISE ");
          }
          break;
        }
      case 2:
        {
          setLEDcol(160, 164, colorRGB);  // ZWOIE
          if (testPrintTimeTexts == 1) Serial.print("ZWOIE ");
          break;
        }
      case 3:
        {
          setLEDcol(235, 239, colorRGB);  // DREIE
          if (testPrintTimeTexts == 1) Serial.print("DREIE ");
          break;
        }
      case 4:
        {
          setLEDcol(128, 132, colorRGB);  // VIERE
          if (testPrintTimeTexts == 1) Serial.print("VIERE ");
          break;
        }
      case 5:
        {
          setLEDcol(139, 143, colorRGB);  // FÜNFE
          if (testPrintTimeTexts == 1) Serial.print("FÜNFE ");
          break;
        }
      case 6:
        {
          setLEDcol(133, 138, colorRGB);  // SECHSE
          if (testPrintTimeTexts == 1) Serial.print("SECHSE ");
          break;
        }
      case 7:
        {
          setLEDcol(169, 175, colorRGB);  // SIEBENE
          if (testPrintTimeTexts == 1) Serial.print("SIEBENE ");
          break;
        }
      case 8:
        {
          setLEDcol(203, 207, colorRGB);  // ACHTE
          if (testPrintTimeTexts == 1) Serial.print("ACHTE ");
          break;
        }
      case 9:
        {
          setLEDcol(192, 196, colorRGB);  // NEUNE
          if (testPrintTimeTexts == 1) Serial.print("NEUNE ");
          break;
        }
      case 10:
        {
          setLEDcol(96, 100, colorRGB);  // ZEHNE (Stunden)
          if (testPrintTimeTexts == 1) Serial.print("ZEHNE ");
          break;
        }
      case 11:
        {
          setLEDcol(232, 235, colorRGB);  // ELFE
          if (testPrintTimeTexts == 1) Serial.print("ELFE ");
          break;
        }
      case 12:
        {
          setLEDcol(197, 202, colorRGB);  // ZWÖLFE
          if (testPrintTimeTexts == 1) Serial.print("ZWÖLFE ");
          break;
        }
    }
  }

  // ########################################################### BAVARIAN:
  if (langLEDlayout == 9) {  // BAVARIAN:

    // ES IS:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(9, 10, colorRGB);
    if (testPrintTimeTexts == 1) {
      Serial.println("");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(" --> ES IS ");
    }

    // FÜNF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(33, 36, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
    }

    // VIERTL:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(37, 42, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VIERTL ");
    }

    // ZWANZIG:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(72, 78, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZWANZIG ");
    }

    // ZEHN: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(44, 47, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
    }

    // NOCH:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(64, 67, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("NOCH ");
    }

    // VOA:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(68, 70, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VOA ");
    }

    // HOIBE:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(107, 111, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("HOIBE ");
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
            setLEDcol(102, 105, colorRGB);  // OANS
            if (testPrintTimeTexts == 1) Serial.print("OANS ");
          }
          break;
        }
      case 2:
        {
          setLEDcol(161, 164, colorRGB);  // ZWOA
          if (testPrintTimeTexts == 1) Serial.print("ZWOA ");
          break;
        }
      case 3:
        {
          setLEDcol(165, 168, colorRGB);  // DREI
          if (testPrintTimeTexts == 1) Serial.print("DREI ");
          break;
        }
      case 4:
        {
          if (iMinute < 5) {
            setLEDcol(194, 196, colorRGB);  // VIA
            if (testPrintTimeTexts == 1) Serial.print("VIA ");
          } else {
            setLEDcol(192, 196, colorRGB);  // VIARE
            if (testPrintTimeTexts == 1) Serial.print("VIARE ");
          }
          break;
        }
      case 5:
        {
          if (iMinute < 5) {
            setLEDcol(98, 101, colorRGB);  // FÜNF
            if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
          } else {
            setLEDcol(97, 101, colorRGB);  // FÜNFE
            if (testPrintTimeTexts == 1) Serial.print("FÜNFE ");
          }
          break;
        }
      case 6:
        {
          if (iMinute < 5) {
            setLEDcol(203, 207, colorRGB);  // SECKS
            if (testPrintTimeTexts == 1) Serial.print("SECKS ");
          } else {
            setLEDcol(202, 207, colorRGB);  // SECKSE
            if (testPrintTimeTexts == 1) Serial.print("SECKSE ");
          }
          break;
        }
      case 7:
        {
          if (iMinute < 5) {
            setLEDcol(171, 175, colorRGB);  // SIEBN
            if (testPrintTimeTexts == 1) Serial.print("SIEBN ");
          } else {
            setLEDcol(170, 175, colorRGB);  // SIEBNE
            if (testPrintTimeTexts == 1) Serial.print("SIEBNE ");
          }
          break;
        }
      case 8:
        {
          if (iMinute < 5) {
            setLEDcol(129, 132, colorRGB);  // ACHT
            if (testPrintTimeTexts == 1) Serial.print("ACHT ");
          } else {
            setLEDcol(128, 132, colorRGB);  // ACHTE
            if (testPrintTimeTexts == 1) Serial.print("ACHTE ");
          }
          break;
        }
      case 9:
        {
          if (iMinute < 5) {
            setLEDcol(198, 201, colorRGB);  // NEIN
            if (testPrintTimeTexts == 1) Serial.print("NEIN ");
          } else {
            setLEDcol(197, 201, colorRGB);  // NEINE
            if (testPrintTimeTexts == 1) Serial.print("NEINE ");
          }
          break;
        }
      case 10:
        {
          if (iMinute < 5) {
            setLEDcol(134, 137, colorRGB);  // ZEHN (Stunden)
            if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
          } else {
            setLEDcol(133, 137, colorRGB);  // ZEHNE (Stunden)
            if (testPrintTimeTexts == 1) Serial.print("ZEHNE ");
          }
          break;
        }
      case 11:
        {
          if (iMinute < 5) {
            setLEDcol(237, 239, colorRGB);  // EJF
            if (testPrintTimeTexts == 1) Serial.print("EJF ");
          } else {
            setLEDcol(236, 239, colorRGB);  // EJFE
            if (testPrintTimeTexts == 1) Serial.print("EJFE ");
          }
          break;
        }
      case 12:
        {
          if (iMinute < 5) {
            setLEDcol(139, 143, colorRGB);  // ZWÄIF
            if (testPrintTimeTexts == 1) Serial.print("ZWÄIF ");
          } else {
            setLEDcol(138, 143, colorRGB);  // ZWÄIFE
            if (testPrintTimeTexts == 1) Serial.print("ZWÄIFE ");
          }
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(232, 234, colorRGB);  // UAH
      if (testPrintTimeTexts == 1) Serial.print("UAH ");
    }
  }

  // ########################################################### LU:
  if (langLEDlayout == 10) {  // LTZ:

    // ET ASS:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(16, 17, colorRGB);  // 2nd row
    setLEDcol(10, 12, colorRGB);
    setLEDcol(19, 21, colorRGB);  // 2nd row
    if (testPrintTimeTexts == 1) {
      Serial.println("");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(" --> ET ASS ");
    }

    // FËNNEF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(42, 47, colorRGB);
      setLEDcol(48, 53, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("FËNNEF ");
    }
    // VÉIREL:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(36, 41, colorRGB);
      setLEDcol(54, 59, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("VÉIREL ");
    }
    // ZÉNG: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(32, 35, colorRGB);
      setLEDcol(60, 63, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("ZÉNG ");
    }
    // ZWANZEG:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(72, 78, colorRGB);
      setLEDcol(81, 87, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("ZWANZEG ");
    }
    // OP:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(65, 66, colorRGB);
      setLEDcol(93, 94, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("OP ");
    }
    // VIR:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(68, 70, colorRGB);
      setLEDcol(89, 91, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("VIR ");
    }
    // HALLWER:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(105, 111, colorRGB);
      setLEDcol(112, 118, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("HALLWER ");
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
            setLEDcol(137, 139, colorRGB);  // ENG
            setLEDcol(148, 150, colorRGB);  // 2nd row
            if (testPrintTimeTexts == 1) Serial.print("ENG ");
          }
          break;
        }
      case 2:
        {
          setLEDcol(199, 202, colorRGB);  // ZWOU
          setLEDcol(213, 216, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("ZWOU ");
          break;
        }
      case 3:
        {
          setLEDcol(140, 143, colorRGB);  // DRÄI
          setLEDcol(144, 147, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("DRÄI ");
          break;
        }
      case 4:
        {
          setLEDcol(203, 207, colorRGB);  // VÉIER
          setLEDcol(208, 212, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("VÉIER ");
          break;
        }
      case 5:
        {
          setLEDcol(160, 165, colorRGB);  // FËNNEF
          setLEDcol(186, 191, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("FËNNEF ");
          break;
        }
      case 6:
        {
          setLEDcol(166, 170, colorRGB);  // SECHS
          setLEDcol(181, 185, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("SECHS ");
          break;
        }
      case 7:
        {
          setLEDcol(99, 103, colorRGB);   // SIWEN
          setLEDcol(120, 124, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("SIWEN ");
          break;
        }
      case 8:
        {
          setLEDcol(171, 175, colorRGB);  // AACHT
          setLEDcol(176, 180, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("AACHT ");
          break;
        }
      case 9:
        {
          setLEDcol(96, 99, colorRGB);    // NÉNG
          setLEDcol(124, 127, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("NÉNG ");
          break;
        }
      case 10:
        {
          setLEDcol(133, 136, colorRGB);  // ZÉNG (Stonnen)
          setLEDcol(151, 154, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("ZÉNG ");
          break;
        }
      case 11:
        {
          setLEDcol(128, 132, colorRGB);  // ELLEF
          setLEDcol(155, 159, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("ELLEF ");
          break;
        }
      case 12:
        {
          setLEDcol(192, 198, colorRGB);  // ZWIELEF
          setLEDcol(217, 223, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("ZWIELEF ");
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(236, 239, colorRGB);  // AUER
      setLEDcol(240, 243, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("AUER ");
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
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(226, 231, colorRGB);  // MINUTE
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(225, 231, colorRGB);  // MINUTEN
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
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(226, 231, colorRGB);  // MINUTE
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(225, 231, colorRGB);  // MINUTES
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(225, 231, colorRGB);  // MINUTES
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(225, 231, colorRGB);  // MINUTES
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
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(225, 231, colorRGB);  // MINUTEN (set to this on request, because there was no space for the extra word "minuut")
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(225, 231, colorRGB);  // MINUTEN
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
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(227, 231, colorRGB);  // MINUT
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(225, 231, colorRGB);  // MINUTER
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(225, 231, colorRGB);  // MINUTER
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(225, 231, colorRGB);  // MINUTER
          break;
        }
    }
  }

  // ##################################################### IT:
  if (langLEDlayout == 4) {  // IT:

    switch (minMod) {
      case 1:
        {
          setLEDcol(232, 232, colorRGB);  // +
          setLEDcol(230, 230, colorRGB);  // 1
          setLEDcol(225, 225, colorRGB);  // M
          break;
        }
      case 2:
        {
          setLEDcol(232, 232, colorRGB);  // +
          setLEDcol(229, 229, colorRGB);  // 2
          setLEDcol(225, 225, colorRGB);  // M
          break;
        }
      case 3:
        {
          setLEDcol(232, 232, colorRGB);  // +
          setLEDcol(228, 228, colorRGB);  // 3
          setLEDcol(225, 225, colorRGB);  // M
          break;
        }
      case 4:
        {
          setLEDcol(232, 232, colorRGB);  // +
          setLEDcol(227, 227, colorRGB);  // 4
          setLEDcol(225, 225, colorRGB);  // M
          break;
        }
    }
  }

  // ##################################################### FR:
  if (langLEDlayout == 5) {  // FR:
    switch (minMod) {
      case 1:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(226, 231, colorRGB);  // MINUTE
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(225, 231, colorRGB);  // MINUTES
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(225, 231, colorRGB);  // MINUTES
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(225, 231, colorRGB);  // MINUTES
          break;
        }
    }
  }

  // ##################################################### GSW:
  if (langLEDlayout == 6) {  // GSW:

    switch (minMod) {
      case 1:
        {
          setLEDcol(231, 231, colorRGB);  // +
          setLEDcol(229, 229, colorRGB);  // 1
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 2:
        {
          setLEDcol(231, 231, colorRGB);  // +
          setLEDcol(228, 228, colorRGB);  // 2
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 3:
        {
          setLEDcol(231, 231, colorRGB);  // +
          setLEDcol(227, 227, colorRGB);  // 3
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 4:
        {
          setLEDcol(231, 231, colorRGB);  // +
          setLEDcol(226, 226, colorRGB);  // 4
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
    }
  }

  // ########################################################### CN:
  if (langLEDlayout == 7) {  // CN:
    switch (minMod) {
      case 1:
        {
          setLEDcol(200, 200, colorRGB);  // 加
          setLEDcol(199, 199, colorRGB);  // 一
          setLEDcol(194, 195, colorRGB);  // 分钟
          break;
        }
      case 2:
        {
          setLEDcol(200, 200, colorRGB);  // 加
          setLEDcol(198, 198, colorRGB);  // 二
          setLEDcol(194, 195, colorRGB);  // 分钟
          break;
        }
      case 3:
        {
          setLEDcol(200, 200, colorRGB);  // 加
          setLEDcol(197, 197, colorRGB);  // 三
          setLEDcol(194, 195, colorRGB);  // 分钟
          break;
        }
      case 4:
        {
          setLEDcol(200, 200, colorRGB);  // 加
          setLEDcol(196, 196, colorRGB);  // 四
          setLEDcol(194, 195, colorRGB);  // 分钟
          break;
        }
    }
  }

  // ##################################################### SWABIAN:
  if (langLEDlayout == 8) {  // SWABIAN:

    switch (minMod) {
      case 1:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(229, 229, colorRGB);  // 1
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 2:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(228, 228, colorRGB);  // 2
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 3:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(227, 227, colorRGB);  // 3
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 4:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(226, 226, colorRGB);  // 4
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
    }
  }

  // ##################################################### BAVARIAN:
  if (langLEDlayout == 9) {  // BAVARIAN:

    switch (minMod) {
      case 1:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(229, 229, colorRGB);  // 1
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 2:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(228, 228, colorRGB);  // 2
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 3:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(227, 227, colorRGB);  // 3
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
      case 4:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(226, 226, colorRGB);  // 4
          setLEDcol(224, 224, colorRGB);  // M
          break;
        }
    }
  }

  // ##################################################### LTZ:
  if (langLEDlayout == 10) {  // LTZ:

    switch (minMod) {
      case 1:
        {
          setLEDcol(234, 234, colorRGB);  // +
          setLEDcol(245, 245, colorRGB);  // 2nd row
          setLEDcol(232, 232, colorRGB);  // 1
          setLEDcol(247, 247, colorRGB);  // 2nd row
          setLEDcol(227, 225, colorRGB);  // MIN
          setLEDcol(252, 254, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(234, 234, colorRGB);  // +
          setLEDcol(245, 245, colorRGB);  // 2nd row
          setLEDcol(231, 231, colorRGB);  // 2
          setLEDcol(248, 248, colorRGB);  // 2nd row
          setLEDcol(227, 225, colorRGB);  // MIN
          setLEDcol(252, 254, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(234, 234, colorRGB);  // +
          setLEDcol(245, 245, colorRGB);  // 2nd row
          setLEDcol(230, 230, colorRGB);  // 3
          setLEDcol(249, 249, colorRGB);  // 2nd row
          setLEDcol(227, 225, colorRGB);  // MIN
          setLEDcol(252, 254, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(234, 234, colorRGB);  // +
          setLEDcol(245, 245, colorRGB);  // 2nd row
          setLEDcol(229, 229, colorRGB);  // 4
          setLEDcol(250, 250, colorRGB);  // 2nd row
          setLEDcol(227, 225, colorRGB);  // MIN
          setLEDcol(252, 254, colorRGB);  // 2nd row
          break;
        }
    }
  }
}


// ###########################################################################################################################################
// # Background color function: SET ALL LEDs OFF
// ###########################################################################################################################################
void back_color() {
  uint32_t c0 = strip.Color(redVal_back, greenVal_back, blueVal_back);  // Background color
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, c0);
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
      if ((i >= 0) && (i < NUMPIXELS)) {
        strip.setPixelColor(i, strip.Color(redVal_time, greenVal_time, blueVal_time));
        int pairedLED = getPairedLED(i);
        if ((pairedLED >= 0) && (pairedLED < NUMPIXELS))
          strip.setPixelColor(pairedLED, strip.Color(redVal_time, greenVal_time, blueVal_time));
      }
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
  delay(1000);
}
// ###########################################################################################################################################
void initTime(String timezone) {
  Serial.println("Setting up time");

  for (int i = 0; i < 3; i++) {
    ClearDisplay();
    strip.show();
    delay(500);

    if (langLEDlayout == 0) {  // DE:
      setLEDcol(1, 4, strip.Color(0, 0, 255));
    }

    if (langLEDlayout == 1) {  // EN:
      setLEDcol(33, 36, strip.Color(0, 0, 255));
    }

    if (langLEDlayout == 2) {  // NL:
      setLEDcol(69, 72, strip.Color(0, 0, 255));
    }

    if (langLEDlayout == 3) {  // SWE:
      setLEDcol(96, 98, strip.Color(0, 0, 255));
    }

    if (langLEDlayout == 4) {                       // IT:
      setLEDcol(111, 111, strip.Color(0, 0, 255));  // T
      setLEDcol(96, 97, strip.Color(0, 0, 255));    // EM
      setLEDcol(131, 131, strip.Color(0, 0, 255));  // P
      setLEDcol(234, 234, strip.Color(0, 0, 255));  // O
    }

    if (langLEDlayout == 5) {                       // FR:
      setLEDcol(10, 10, strip.Color(0, 0, 255));    // T
      setLEDcol(41, 42, strip.Color(0, 0, 255));    // EM
      setLEDcol(105, 105, strip.Color(0, 0, 255));  // p
      setLEDcol(128, 128, strip.Color(0, 0, 255));  // S
    }

    if (langLEDlayout == 6) {                   // GSW:
      setLEDcol(0, 3, strip.Color(0, 0, 255));  // ZIIT
    }

    if (langLEDlayout == 7) {  // CN:
      setLEDcol(40, 41, strip.Color(0, 0, 255));
    }

    if (langLEDlayout == 8) {                       // SWABIAN:
      setLEDcol(73, 74, strip.Color(0, 0, 255));    // ZE
      setLEDcol(131, 131, strip.Color(0, 0, 255));  // I
      setLEDcol(204, 204, strip.Color(0, 0, 255));  // T
    }

    if (langLEDlayout == 9) {                   // BAVARIAN:
      setLEDcol(5, 8, strip.Color(0, 0, 255));  // ZEID
    }

    if (langLEDlayout == 10) {                    // LTZ:
      setLEDcol(1, 4, strip.Color(0, 0, 255));    // ZÄIT
      setLEDcol(27, 30, strip.Color(0, 0, 255));  // 2nd row
    }
    strip.show();
    delay(500);
  }


  struct tm timeinfo;
  configTime(0, 0, NTPserver);
  delay(500);

  while (!getLocalTime(&timeinfo)) {
    delay(1000);

    if (langLEDlayout == 0) {  // DE:
      setLEDcol(1, 4, strip.Color(255, 0, 0));
    }

    if (langLEDlayout == 1) {  // EN:
      setLEDcol(33, 36, strip.Color(255, 0, 0));
    }

    if (langLEDlayout == 2) {  // NL:
      setLEDcol(69, 72, strip.Color(255, 0, 0));
    }

    if (langLEDlayout == 3) {  // SWE:
      setLEDcol(96, 98, strip.Color(255, 0, 0));
    }

    if (langLEDlayout == 4) {                       // IT:
      setLEDcol(111, 111, strip.Color(255, 0, 0));  // T
      setLEDcol(96, 97, strip.Color(255, 0, 0));    // EM
      setLEDcol(131, 131, strip.Color(255, 0, 0));  // P
      setLEDcol(234, 234, strip.Color(255, 0, 0));  // O
    }

    if (langLEDlayout == 5) {                       // FR:
      setLEDcol(10, 10, strip.Color(255, 0, 0));    // T
      setLEDcol(41, 42, strip.Color(255, 0, 0));    // EM
      setLEDcol(105, 105, strip.Color(255, 0, 0));  // p
      setLEDcol(128, 128, strip.Color(255, 0, 0));  // S
    }

    if (langLEDlayout == 6) {                   // GSW:
      setLEDcol(0, 3, strip.Color(255, 0, 0));  // ZIIT
    }

    if (langLEDlayout == 7) {  // CN:
      setLEDcol(40, 41, strip.Color(255, 0, 0));
    }

    if (langLEDlayout == 8) {                       // SWABIAN:
      setLEDcol(73, 74, strip.Color(255, 0, 0));    // ZE
      setLEDcol(131, 131, strip.Color(255, 0, 0));  // I
      setLEDcol(204, 204, strip.Color(255, 0, 0));  // T
    }

    if (langLEDlayout == 9) {                   // BAVARIAN:
      setLEDcol(5, 8, strip.Color(255, 0, 0));  // ZEID
    }

    if (langLEDlayout == 10) {                    // LTZ:
      setLEDcol(1, 4, strip.Color(255, 0, 0));    // ZÄIT
      setLEDcol(27, 30, strip.Color(255, 0, 0));  // 2nd row
    }

    strip.show();
    delay(250);
    ClearDisplay();
    delay(250);

    Serial.println("! Failed to obtain time - Time server could not be reached ! --> RESTART THE DEVICE NOW...");
    ESP.restart();
  }

  // Time successfully received:
  ClearDisplay();
  if (langLEDlayout == 0) {  // DE:
    setLEDcol(1, 4, strip.Color(0, 255, 0));
  }

  if (langLEDlayout == 1) {  // EN:
    setLEDcol(33, 36, strip.Color(0, 255, 0));
  }

  if (langLEDlayout == 2) {  // NL:
    setLEDcol(69, 72, strip.Color(0, 255, 0));
  }

  if (langLEDlayout == 3) {  // SWE:
    setLEDcol(96, 98, strip.Color(0, 255, 0));
  }

  if (langLEDlayout == 4) {                       // IT:
    setLEDcol(111, 111, strip.Color(0, 255, 0));  // T
    setLEDcol(96, 97, strip.Color(0, 255, 0));    // EM
    setLEDcol(131, 131, strip.Color(0, 255, 0));  // P
    setLEDcol(234, 234, strip.Color(0, 255, 0));  // O
  }

  if (langLEDlayout == 5) {                       // FR:
    setLEDcol(10, 10, strip.Color(0, 255, 0));    // T
    setLEDcol(41, 42, strip.Color(0, 255, 0));    // EM
    setLEDcol(105, 105, strip.Color(0, 255, 0));  // p
    setLEDcol(128, 128, strip.Color(0, 255, 0));  // S
  }

  if (langLEDlayout == 6) {                   // GSW:
    setLEDcol(0, 3, strip.Color(0, 255, 0));  // ZIIT
  }

  if (langLEDlayout == 7) {  // CN:
    setLEDcol(40, 41, strip.Color(0, 255, 0));
  }

  if (langLEDlayout == 8) {                       // SWABIAN:
    setLEDcol(73, 74, strip.Color(0, 255, 0));    // ZE
    setLEDcol(131, 131, strip.Color(0, 255, 0));  // I
    setLEDcol(204, 204, strip.Color(0, 255, 0));  // T
  }

  if (langLEDlayout == 9) {                   // BAVARIAN:
    setLEDcol(5, 8, strip.Color(0, 255, 0));  // ZEID
  }

  if (langLEDlayout == 10) {                    // LTZ:
    setLEDcol(1, 4, strip.Color(255, 0, 0));    // ZÄIT
    setLEDcol(27, 30, strip.Color(255, 0, 0));  // 2nd row
  }

  strip.show();
  delay(1000);
  Serial.println("Got the time from NTP");
  setTimezone(timezone);
}
// ###########################################################################################################################################
void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S zone %Z %z ");
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  iStartTime = String(timeStringBuff);
  iHour = timeinfo.tm_hour;
  iMinute = timeinfo.tm_min;
  iSecond = timeinfo.tm_sec;
  if (ShowTimeEachSecond == 1) {
    Serial.print("Current time: ");
    Serial.print(iHour);
    Serial.print(":");
    Serial.print(iMinute);
    Serial.print(":");
    Serial.println(iSecond);
  }
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
// # HTML command web server:
// ###########################################################################################################################################
int ew = 0;  // Current extra word
String ledstatus = "ON";
void handleLEDupdate() {  // LED server pages urls:

  ledserver.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {  // Show a manual how to use these links:
    String message = "WordClock web configuration and querry options examples:\n\n";
    message = message + "General:\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023 --> Shows this text\n\n";
    message = message + "Get the status of the WordClock LEDs:\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023/status --> Show the status of the LEDs (0 = OFF and 1 = ON).\n\n";
    message = message + "Turn the LEDs OFF or ON:\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023/config?LEDs=0 --> LED intensity is set to OFF which will turn the display off.\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023/config?LEDs=1 --> LED intensity is set to ON which will turn the display on again...\n";
    request->send(200, "text/plain", message);
  });

  ledserver.on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {  // Configure background and time texts color and intensity:
    int paramsNr = request->params();
    // Serial.println(paramsNr);
    for (int i = 0; i < paramsNr; i++) {
      AsyncWebParameter* p = request->getParam(i);
      // Serial.print("Param name: ");
      // Serial.println(p->name());
      // Serial.print("Param value: ");
      // Serial.println(p->value());
      // Serial.println("------------------");
      if ((p->value().toInt() >= 0) && (p->value().toInt() <= 1)) {
        if ((String(p->name()) == "LEDs") && (p->value().toInt() == 0)) {
          set_web_intensity = 1;
          ledstatus = "OFF";
          ESPUI.updateVisibility(intensity_web_HintID, true);
          ESPUI.updateVisibility(statusNightModeID, false);
          ESPUI.updateVisibility(sliderBrightnessDayID, false);
          ESPUI.updateVisibility(switchNightModeID, false);
          ESPUI.updateVisibility(sliderBrightnessNightID, false);
          ESPUI.updateVisibility(call_day_time_startID, false);
          ESPUI.updateVisibility(call_day_time_stopID, false);
          ESPUI.updateVisibility(text_colour_time, false);
          ESPUI.updateVisibility(text_colour_background, false);
          ESPUI.updateVisibility(switchRandomColorID, false);
          ESPUI.updateVisibility(DayNightSectionID, false);
          ESPUI.updateVisibility(switchSingleMinutesID, false);
          ESPUI.jsonReload();
        }
        if ((String(p->name()) == "LEDs") && (p->value().toInt() == 1)) {
          set_web_intensity = 0;
          ledstatus = "ON";
          ESPUI.updateVisibility(intensity_web_HintID, false);
          ESPUI.updateVisibility(statusNightModeID, true);
          ESPUI.updateVisibility(sliderBrightnessDayID, true);
          ESPUI.updateVisibility(switchNightModeID, true);
          ESPUI.updateVisibility(sliderBrightnessNightID, true);
          ESPUI.updateVisibility(call_day_time_startID, true);
          ESPUI.updateVisibility(call_day_time_stopID, true);
          ESPUI.updateVisibility(text_colour_time, true);
          ESPUI.updateVisibility(text_colour_background, true);
          ESPUI.updateVisibility(switchRandomColorID, true);
          ESPUI.updateVisibility(DayNightSectionID, true);
          ESPUI.updateVisibility(switchSingleMinutesID, true);
        }
        changedvalues = true;
        updatenow = true;
      } else {
        request->send(200, "text/plain", "INVALID VALUES - MUST BE BETWEEN 0 and 1");
      }
    }
    request->send(200, "text/plain", "WordClock LEDs set to: " + ledstatus);
  });

  ledserver.on("/status", HTTP_GET, [](AsyncWebServerRequest* request) {  // Show the status of all extra words and the color for the background and time texts:
    String message = ledstatus;
    request->send(200, "text/plain", message);
  });

  ledserver.begin();
}


// ###########################################################################################################################################
// # Startup LED test function
// ###########################################################################################################################################
void callStartText() {
  Serial.println("Show 'WordClock' startup text...");
  uint32_t c = strip.Color(redVal_time, greenVal_time, blueVal_time);
  int TextWait = 500;
  showtext("W", TextWait, c);
  showtext("o", TextWait, c);
  showtext("r", TextWait, c);
  showtext("d", TextWait, c);
  showtext("C", TextWait, c);
  showtext("l", TextWait, c);
  showtext("o", TextWait, c);
  showtext("c", TextWait, c);
  showtext("k", TextWait, c);
}


// ###########################################################################################################################################
// # Text output function:
// ###########################################################################################################################################
void showtext(String letter, int wait, uint32_t c) {
  ClearDisplay();

  int myArray[50];
  memset(myArray, 0, sizeof(myArray));

  if (letter == "W") {
    int myArray2[] = { 42, 53, 74, 85, 106, 117, 138, 149, 170, 181, 169, 182, 183, 168, 151, 136, 135, 152, 167, 184, 166, 185, 186, 165, 154, 133, 122, 101, 90, 69, 58, 37 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "o") {
    int myArray2[] = { 106, 117, 138, 149, 170, 181, 169, 182, 183, 168, 167, 184, 166, 185, 186, 165, 154, 133, 122, 101, 102, 121, 120, 103, 104, 119, 118, 105 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "r") {
    int myArray2[] = { 154, 133, 122, 101, 102, 121, 120, 103, 104, 119, 118, 105, 106, 117, 138, 149, 170, 181 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "d") {
    int myArray2[] = { 102, 121, 120, 103, 104, 119, 118, 105, 106, 117, 138, 149, 170, 181, 169, 182, 183, 168, 167, 184, 166, 185, 186, 165, 154, 133, 122, 101, 90, 69, 58, 37 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "C") {
    int myArray2[] = { 58, 37, 57, 38, 39, 56, 40, 55, 41, 54, 42, 53, 74, 85, 106, 117, 138, 149, 170, 181, 169, 182, 183, 168, 167, 184, 166, 185, 186, 165 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "l") {
    int myArray2[] = { 42, 53, 74, 85, 106, 117, 138, 149, 170, 181 };  // , 169, 182, 183, 168
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "c") {
    int myArray2[] = { 122, 101, 102, 121, 120, 103, 104, 119, 118, 105, 106, 117, 138, 149, 170, 181, 169, 182, 183, 168, 167, 184, 166, 185, 186, 165 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "k") {
    int myArray2[] = { 42, 53, 74, 85, 106, 117, 138, 149, 170, 181, 105, 118, 104, 119, 71, 88, 37, 58, 134, 153, 165, 186 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "S") {
    int myArray2[] = { 37, 38, 39, 40, 41, 42, 53, 54, 55, 56, 57, 58, 74, 85, 101, 102, 103, 104, 105, 106, 117, 118, 119, 120, 121, 122, 133, 154, 165, 166, 167, 168, 169, 170, 181, 182, 183, 184, 185, 186 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "E") {
    int myArray2[] = { 37, 38, 39, 40, 41, 42, 53, 54, 55, 56, 57, 58, 74, 85, 101, 102, 103, 104, 105, 106, 117, 118, 119, 120, 121, 122, 138, 149, 165, 166, 167, 168, 169, 170, 181, 182, 183, 184, 185, 186 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "T") {
    int myArray2[] = { 37, 38, 39, 40, 41, 42, 43, 52, 53, 54, 55, 56, 57, 58, 72, 87, 104, 119, 136, 151, 168, 183 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "I") {
    int myArray2[] = { 40, 55, 72, 87, 104, 119, 136, 151, 168, 183 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "F") {
    int myArray2[] = { 37, 38, 39, 40, 41, 42, 53, 54, 55, 56, 57, 58, 74, 85, 101, 102, 103, 104, 105, 106, 117, 118, 119, 120, 121, 122, 138, 149, 170, 181 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "U") {
    int myArray2[] = { 42, 53, 74, 85, 106, 117, 138, 149, 170, 181, 169, 182, 183, 168, 167, 184, 166, 185, 186, 165, 154, 133, 122, 101, 90, 69, 58, 37 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "P") {
    int myArray2[] = { 37, 38, 39, 40, 41, 42, 53, 54, 55, 56, 57, 58, 74, 85, 69, 90, 101, 102, 103, 104, 105, 106, 117, 118, 119, 120, 121, 122, 138, 149, 170, 181 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "D") {
    int myArray2[] = { 38, 39, 40, 41, 42, 53, 54, 55, 56, 57, 74, 85, 69, 90, 101, 106, 117, 122, 138, 149, 170, 181, 133, 154, 169, 182, 183, 168, 167, 184, 166, 185 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == "A") {
    int myArray2[] = { 37, 38, 39, 40, 41, 42, 53, 54, 55, 56, 57, 58, 74, 85, 69, 90, 101, 102, 103, 104, 105, 106, 117, 118, 119, 120, 121, 122, 138, 149, 170, 181, 133, 154, 165, 186 };
    memcpy(myArray, myArray2, sizeof(myArray2));
  }

  if (letter == " ") {
    int myArray2[] = { 255 };
    memcpy(myArray, myArray2, sizeof(myArray2));
    c = strip.Color(0, 0, 0);
  }

  for (int element : myArray) {
    if (element != 0) {
      strip.setPixelColor(element, c);
    }
  }

  strip.show();
  delay(wait);
  ClearDisplay();
}


// ###########################################################################################################################################
// # Startup WiFi text function:
// ###########################################################################################################################################
void SetWLAN(uint32_t color) {
  // if (debugtexts == 1) Serial.println("Show text WLAN/WIFI...");
  ClearDisplay();

  if (langLEDlayout == 0) {  // DE:
    setLEDcol(5, 8, color);  // WIFI
  }

  if (langLEDlayout == 1) {   // EN:
    setLEDcol(7, 10, color);  // WIFI
  }

  if (langLEDlayout == 2) {    // NL:
    setLEDcol(75, 78, color);  // WIFI
  }

  if (langLEDlayout == 3) {  // SWE:
    setLEDcol(0, 3, color);  // WIFI
  }

  if (langLEDlayout == 4) {      // IT:
    setLEDcol(233, 233, color);  // W
    setLEDcol(231, 231, color);  // I
    setLEDcol(226, 226, color);  // F
    setLEDcol(224, 224, color);  // I
  }

  if (langLEDlayout == 5) {      // FR:
    setLEDcol(239, 239, color);  // W
    setLEDcol(237, 237, color);  // I
    setLEDcol(232, 232, color);  // F
    setLEDcol(224, 224, color);  // I
  }

  if (langLEDlayout == 6) {   // GSW:
    setLEDcol(7, 10, color);  // WIFI
  }

  if (langLEDlayout == 7) {    // CN:
    setLEDcol(42, 43, color);  // WIFI
  }

  if (langLEDlayout == 8) {    // SWABIAN:
    setLEDcol(12, 13, color);  // WI
    setLEDcol(7, 8, color);    // FI
  }

  if (langLEDlayout == 9) {    // BAVARIAN:
    setLEDcol(10, 13, color);  // WIFI
  }

  if (langLEDlayout == 10) {   // LTZ:
    setLEDcol(5, 8, color);    // WIFI
    setLEDcol(23, 26, color);  // 2nd row
  }

  strip.show();
}


// ###########################################################################################################################################
// # Wifi scan function to help you to setup your WiFi connection
// ###########################################################################################################################################
void ScanWiFi() {
  Serial.println("Scan WiFi networks - START");
  int n = WiFi.scanNetworks();
  Serial.println("WiFi scan done");
  Serial.println(" ");
  if (n == 0) {
    Serial.println("No WiFi networks found");
  } else {
    Serial.print(n);
    Serial.println(" WiFi networks found:");
    Serial.println(" ");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("Scan WiFi networks - END");
}


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char index_html[] PROGMEM = R"=====(
  <!DOCTYPE html><html><head><title>WordClock</title></head>
          <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
  
   <style>
    .button {
      display: inline-block;
      padding: 15px 25px;
      font-size: 24px;
      cursor: pointer;
      text-align: center;
      text-decoration: none;
      outline: none;
      color: #fff;
      background-color: #4CAF50;
      border: none;
      border-radius: 15px;
      box-shadow: 0 9px #999;
    }
    .button:hover {background-color: #3e8e41}
    .button:active {
      background-color: #3e8e41;
      box-shadow: 0 5px #666;
      transform: translateY(4px);
    }
    </style>
  
  <body>
    <form action="/start" name="myForm">
      <center><b><h1>Welcome to the WordClock setup</h1></b>
      <h2>Please add your local WiFi credentials<br/><br/>and set your language on the next page</h2><br/>
      <input type="submit" value="Configure WordClock" class="button">
     </center></form></body>
  </html>
 )=====";


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char config_html[] PROGMEM = R"rawliteral(
 <!DOCTYPE HTML><html><head><title>WordClock</title>
 <meta name="viewport" content="width=device-width, initial-scale=1">
  <script language="JavaScript">
  <!--
  function validateForm() {
  var x = document.forms["myForm"]["mySSID"].value;
  if (x == "") {
    alert("WiFi SSID must be set");
    return false;
  }
  var y = document.forms["myForm"]["myPW"].value;
  if (y == "") {
    alert("WiFi password must be set");
    return false;
  }
  } 
  //-->
  </script>
  </head>
  
   <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
  
   <style>
    .button {
      display: inline-block;
      padding: 15px 25px;
      font-size: 24px;
      cursor: pointer;
      text-align: center;
      text-decoration: none;
      outline: none;
      color: #fff;
      background-color: #4CAF50;
      border: none;
      border-radius: 15px;
      box-shadow: 0 9px #999;
    }
    .button:hover {background-color: #3e8e41}
    .button:active {
      background-color: #3e8e41;
      box-shadow: 0 5px #666;
      transform: translateY(4px);
    }
    </style>
  
  <body>
  <form action="/get" name="myForm" onsubmit="return validateForm()" >
    <center><b><h1>Initial WordClock setup:</h1></b>
    <label for="mySSID">Enter your WiFi SSID:</label><br/>
    <input type="text" id="mySSID" name="mySSID" value="" style="width: 200px;" /><br/><br/>
    <label for="myPW">Enter your WiFi password:</label><br/>
    <input type="text" id="myPW" name="myPW" value="" style="width: 200px;" /><br/><br/>
    <label for="setlanguage">Select your language layout:</label><br/>
    <select id="setlanguage" name="setlanguage" style="width: 200px;">
    <option value=0 selected>GERMAN</option>
    <option value=1>ENGLISH</option>
    <option value=2>DUTCH</option>
    <option value=3>SWEDISH</option>
    <option value=4>ITALIAN</option>
    <option value=5>FRENCH</option>
    <option value=6>SWISS GERMAN</option>
    <option value=7>CHINESE</option>
    <option value=8>SWABIAN GERMAN</option>
    <option value=9>BAVARIAN</option>
    <option value=10>LUXEMBURGISH</option>
    </select><br/><br/>
    <input type="submit" value="Save values and start WordClock" class="button">
  </center></form></body></html>)rawliteral";


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char saved_html[] PROGMEM = R"rawliteral(
 <!DOCTYPE HTML><html><head>
  <title>Initial WordClock setup</title>
  <meta name="viewport" content="width=device-width, initial-scale=1"></head>
    <style>
  body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
    }
  </style>
  <body>
    <center><h2><b>Settings saved...<br><br>
    WordClock will now try to connect to the named WiFi with the set language.<br>
    If it failes the WIFI leds will flash red and then please try to connect to the temporary access point again.<br>
    Please close this page now and enjoy your WordClock. =)</h2></b>
 </body></html>)rawliteral";


// ###########################################################################################################################################
// # Captive Portal by AWSW to avoid the usage of the WiFi Manager library to have more control
// ###########################################################################################################################################
const char* PARAM_INPUT_1 = "mySSID";
const char* PARAM_INPUT_2 = "myPW";
const char* PARAM_INPUT_3 = "setlanguage";
const String captiveportalURL = "http://192.168.4.1";
void CaptivePotalSetup() {
  ScanWiFi();
  const char* temp_ssid = "WordClock";
  const char* temp_password = "";
  WiFi.softAP(temp_ssid, temp_password);
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("#################################################################################################################################################################################");
  Serial.print("# Temporary WiFi access point initialized. Please connect to the WiFi access point now and set your local WiFi credentials and WordClock language. Access point name: ");
  Serial.println(temp_ssid);
  Serial.print("# In case your browser does not open the WordClock setup page automatically after connecting to the access point, please navigate to this URL manually to http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("#################################################################################################################################################################################");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  dnsServer.start(53, "*", WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    String inputMessage;
    String inputParam;
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      // Serial.println(inputMessage);
      preferences.putString("WIFIssid", inputMessage);  // Save entered WiFi SSID
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
      // Serial.println(inputMessage);
      preferences.putString("WIFIpass", inputMessage);  // Save entered WiFi password
      inputMessage = request->getParam(PARAM_INPUT_3)->value();
      inputParam = PARAM_INPUT_3;
      // Serial.println(inputMessage);
      preferences.putUInt("langLEDlayout", inputMessage.toInt());  // Save entered layout language
      delay(250);
      preferences.end();
    } else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    request->send_P(200, "text/html", saved_html);
    ResetTextLEDs(strip.Color(0, 255, 0));
    delay(1000);
    ESP.restart();
  });

  server.on("/start", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", config_html);
  });

  server.on("/connecttest.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("msftconnecttest.com", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("microsoft.com", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
   server.on("/fwlink", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/wpad.dat", [](AsyncWebServerRequest* request) {
    request->send(404);
  });
  server.on("/generate_204", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/redirect", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/canonical.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/success.txt", [](AsyncWebServerRequest* request) {
    request->send(200);
  });
  server.on("/ncsi.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/chrome-variations/seed", [](AsyncWebServerRequest* request) {
    request->send(200);
  });
  server.on("/service/update2/json", [](AsyncWebServerRequest* request) {
    request->send(200);
  });
  server.on("/chat", [](AsyncWebServerRequest* request) {
    request->send(404);
  });
  server.on("/startpage", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/favicon.ico", [](AsyncWebServerRequest* request) {
    request->send(404);
  });

  server.on("/", HTTP_ANY, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/html", index_html);
    response->addHeader("Cache-Control", "public,max-age=31536000");
    request->send(response);
    Serial.println("Served Basic HTML Page");
  });

  server.onNotFound([](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.print("URL not found: ");
    Serial.print(request->host());
    Serial.print(" ");
    Serial.print(request->url());
    Serial.print(" sent redirect to " + captiveportalURL + "\n");
  });

  server.begin();
  Serial.println("WordClock Captive Portal web server started");
}


// ###########################################################################################################################################
// # Wifi setup and reconnect function that runs once at startup and during the loop function of the ESP:
// ###########################################################################################################################################
void WIFI_SETUP() {
  Serial.println(" ");
  esp_log_level_set("wifi", ESP_LOG_WARN);  // Disable WiFi debug warnings
  if (testTime == 0) {                      // If time text test mode is NOT used:
    String WIFIssid = preferences.getString("WIFIssid");
    bool WiFiConfigEmpty = false;
    if (WIFIssid == "") {
      // Serial.println("WIFIssid empty");
      WiFiConfigEmpty = true;
    } else {
      // Serial.print("WIFIssid = ");
      // Serial.println(WIFIssid);
    }
    String WIFIpass = preferences.getString("WIFIpass");
    if (WIFIpass == "") {
      // Serial.println("WIFIpass empty");
      WiFiConfigEmpty = true;
    } else {
      // Serial.print("WIFIpass = ");
      // Serial.println(WIFIpass);
    }
    if (WiFiConfigEmpty == true) {
      Serial.println("Show SET WIFI...");
      uint32_t c = strip.Color(0, 255, 255);
      int TextWait = 500;
      showtext("S", TextWait, c);
      showtext("E", TextWait, c);
      showtext("T", TextWait, c);
      showtext(" ", TextWait, c);
      showtext("W", TextWait, c);
      showtext("I", TextWait, c);
      showtext("F", TextWait, c);
      showtext("I", TextWait, c);
      showtext(" ", TextWait, c);
      SetWLAN(strip.Color(0, 255, 255));
      CaptivePotalSetup();
    } else {
      Serial.println("Try to connect to found WiFi configuration: ");
      WiFi.disconnect();
      int tryCount = 0;
      WiFi.mode(WIFI_STA);
      WiFi.begin((const char*)WIFIssid.c_str(), (const char*)WIFIpass.c_str());
      Serial.println("Connecting to WiFi " + String(WIFIssid));
      while (WiFi.status() != WL_CONNECTED) {
        SetWLAN(strip.Color(0, 0, 255));
        tryCount = tryCount + 1;
        Serial.print("Connection try #: ");
        Serial.println(tryCount);
        if (tryCount >= maxWiFiconnctiontries - 10) {
          SetWLAN(strip.Color(255, 0, 0));
        }
        if (tryCount == maxWiFiconnctiontries) {
          Serial.println("\n\nWIFI CONNECTION ERROR: If the connection still can not be established please check the WiFi settings or location of the device.\n\n");
          preferences.putString("WIFIssid", "");  // Reset entered WiFi ssid
          preferences.putString("WIFIpass", "");  // Reset entered WiFi password
          preferences.end();
          delay(250);
          Serial.println("WiFi settings deleted because in " + String(maxWiFiconnctiontries) + " tries the WiFi connection could not be established. Temporary WordClock access point will be started to reconfigure WiFi again.");
          ESP.restart();
        }
        delay(1000);
        SetWLAN(strip.Color(0, 0, 0));
        delay(500);
      }
      Serial.println(" ");
      WiFIsetup = true;
      Serial.print("Successfully connected now to WiFi SSID: ");
      Serial.println(WiFi.SSID());
      Serial.println("IP: " + WiFi.localIP().toString());
      Serial.println("DNS: " + WiFi.dnsIP().toString());
      SetWLAN(strip.Color(0, 255, 0));
      delay(1000);
      if (useStartupText == 1) callStartText();  // Show "WordClock" startup text
      if (useshowip == 1) ShowIPaddress();       // Display the current IP-address
      configNTPTime();                           // NTP time setup
      setupWebInterface();                       // Generate the configuration page
      updatenow = true;                          // Update the display 1x after startup
      update_display();                          // Update LED display
      handleLEDupdate();                         // LED update via web
      setupOTAupate();                           // ESP32 OTA update
      Serial.println("######################################################################");
      Serial.println("# Web interface online at: http://" + IpAddress2String(WiFi.localIP()));
      Serial.println("# HTTP controls online at: http://" + IpAddress2String(WiFi.localIP()) + ":2023");
      Serial.println("######################################################################");
      Serial.println("# WordClock startup finished...");
      Serial.println("######################################################################");
      Serial.println(" ");
    }
  }
}

// ###########################################################################################################################################
// # ESP32 OTA update:
// ###########################################################################################################################################
const char otaserverIndex[] PROGMEM = R"=====(
  <!DOCTYPE html><html><head><title>WordClock</title></head>
      <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <body>
    <form method='POST' action='/update' enctype='multipart/form-data'>
      <center><b><h1>WordClock software update</h1></b>
      <h2>Please select the in the Arduino IDE > "Sketch" ><br/>"Export Compiled Binary (Alt+Ctrl+S)"<br/>to generate the required .BIN file.<br/>
      Use the "Update" button 1x to start the update.<br/><br/>WordClock will restart automatically.</h2><br/>
      <input type='file' name='update'>       <input type='submit' value='Update'>
     </center></form></body>
  </html>
 )=====";


const char otaNOK[] PROGMEM = R"=====(
  <!DOCTYPE html><html><head><title>WordClock</title></head>
          <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
   <style>
    .button {
      display: inline-block;
      padding: 15px 25px;
      font-size: 24px;
      cursor: pointer;
      text-align: center;
      text-decoration: none;
      outline: none;
      color: #fff;
      background-color: #4CAF50;
      border: none;
      border-radius: 15px;
      box-shadow: 0 9px #999;
    }
    .button:hover {background-color: #3e8e41}
    .button:active {
      background-color: #3e8e41;
      box-shadow: 0 5px #666;
      transform: translateY(4px);
    }
    </style>
    <body>
      <center><b><h1>WordClock software update</h1></b>
      <h2>ERROR: Software update FAILED !!!<br/><br/>WordClock will restart automatically.</h2><br/>
      </center></body>
  </html>
 )=====";


const char otaOK[] PROGMEM = R"=====(
  <!DOCTYPE html><html><head><title>WordClock</title></head>
          <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
   <style>
    .button {
      display: inline-block;
      padding: 15px 25px;
      font-size: 24px;
      cursor: pointer;
      text-align: center;
      text-decoration: none;
      outline: none;
      color: #fff;
      background-color: #4CAF50;
      border: none;
      border-radius: 15px;
      box-shadow: 0 9px #999;
    }
    .button:hover {background-color: #3e8e41}
    .button:active {
      background-color: #3e8e41;
      box-shadow: 0 5px #666;
      transform: translateY(4px);
    }
    </style>
    <body>
      <center><b><h1>WordClock software update</h1></b>
      <h2>Software update done =)<br/><br/>WordClock will restart automatically.</h2><br/>
      </center></body>
  </html>
 )=====";


void setupOTAupate() {
  otaserver.on("/", HTTP_GET, []() {
    otaserver.sendHeader("Connection", "close");
    otaserver.send(200, "text/html", otaserverIndex);
  });

  otaserver.on(
    "/update", HTTP_POST, []() {
      otaserver.sendHeader("Connection", "close");
      if (Update.hasError()) {
        otaserver.send(200, "text/html", otaNOK);
        ResetTextLEDs(strip.Color(255, 0, 0));
      } else {
        otaserver.send(200, "text/html", otaOK);
        ResetTextLEDs(strip.Color(0, 255, 0));
      }
      delay(3000);
      ESP.restart();
    },
    []() {
      HTTPUpload& upload = otaserver.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.setDebugOutput(true);
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin()) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          Serial.printf("Update success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      } else {
        Serial.printf("Update failed unexpectedly (likely broken connection): status=%d\n", upload.status);
      }
    });
  otaserver.begin();
}


// ###########################################################################################################################################
// # EOF - You have successfully reached the end of the code - well done ;-)
// ###########################################################################################################################################