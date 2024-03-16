// ###########################################################################################################################################
// #
// # WordClock code for the printables.com AWSW WordClock 16x8 LED matrix projects of 2023/2024:
// # https://www.printables.com/de/search/models?q=WordClock%2016x8%20@awsw&ctx=models
// #
// # Code by https://github.com/AWSW-de
// #
// # Released under licenses:
// # GNU General Public License v3.0: https://github.com/AWSW-de/WordClock-16x8-LED-matrix/blob/main/LICENSE and
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
// # - ESP32Time              // by fbiego:                       https://github.com/fbiego/ESP32Time
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
#include <ESP32Time.h>          // Used for the Offline Mode ESP32 time function
#include "settings.h"           // Settings are stored in a seperate file to make to code better readable and to be able to switch to other settings faster


// ###########################################################################################################################################
// # Version number of the code:
// ###########################################################################################################################################
const char* WORD_CLOCK_VERSION = "V3.5.0";


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
Preferences preferences;  // ESP32 flash storage
ESP32Time rtc;            // Setup Offline ESP32 time function
bool updatedevice = true;
bool updatenow = false;
bool updatemode = false;
bool changedvalues = false;
bool WiFIsetup = false;
int iHour, iMinute, iSecond, iDay, iMonth, iYear, intensity, intensity_day, intensity_night, set_web_intensity, intensity_web;
int redVal_back, greenVal_back, blueVal_back, redVal_time, greenVal_time, blueVal_time, langLEDlayout, statusLanguageID, DayNightSectionID, LEDsettingsSectionID;
int usenightmode, statusNightMode, statusNightModeWarnID, useshowip, usesinglemin, useStartupText, statusLabelID, statusNightModeID, intensity_web_HintID, RandomColor, switchRandomColorID, switchSingleMinutesID;
int sliderBrightnessDayID, switchNightModeID, sliderBrightnessNightID, call_day_time_startID, call_day_time_stopID, mySetTimeZone, mySetTimeZoneID, mySetTimeServer, mySetTimeServerID;
int UseOnlineMode, OfflineCurrentHourOffset, iHourOffset, statusTimeFromDevice, statusTimeSetOffline, OfflineModeHint1, OfflineModeHint2, OfflineModeHint3, showOMhints;
uint16_t text_colour_background, text_colour_time, selectLang, timeId;
String iStartTime, selectLangTXT, myTimeZone, myTimeServer, Timezone, NTPserver, day_time_start, day_time_stop, statusNightModeIDtxt;


// ###########################################################################################################################################
// # Setup function that runs once at startup of the ESP:
// ###########################################################################################################################################
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("######################################################################");
  Serial.println("# WordClock startup of version: " + String(WORD_CLOCK_VERSION));
  Serial.println("######################################################################");
  preferences.begin("wordclock", false);         // Init ESP32 flash
  getFlashValues();                              // Read settings from flash
  strip.begin();                                 // Init the LEDs
  strip.show();                                  // Init the LEDs --> Set them to OFF
  intensity = intensity_day;                     // Set the intenity to day mode for startup
  strip.setBrightness(intensity);                // Set LED brightness
  if (UseOnlineMode == 1) WIFI_SETUP();          // ONLINE MODE WiFi login and startup of web services
  if (UseOnlineMode == 0) {                      // OFFLINE MODE
    iHour = 9;                                   // Default hour in Offline Mode
    iMinute = 41;                                // Default minute in Offline Mode
    OfflinePotalSetup();                         // Offline mode setup access point
    rtc.setTime(0, iMinute, iHour, 1, 1, 2024);  // Set time: (ss, mm, hh, DD, MM, YYYY) --> 17th Jan 2021 09:41:00
    updatenow = true;                            // Update the display 1x after startup
    update_display();                            // Update LED display
    Serial.println("######################################################################");
    Serial.println("# WordClock startup in OFFLINE MODE finished...");
    Serial.println("######################################################################");
  }
}


// ###########################################################################################################################################
// # Loop function which runs all the time after the startup was done:
// ###########################################################################################################################################
void loop() {
  if (UseOnlineMode == 1) {  // Online Mode actions:
    if ((WiFIsetup == true) || (testTime == 1)) {
      printLocalTime();                               // Locally get the time (NTP server requests done 1x per hour)
      if (updatedevice == true) {                     // Allow display updates (normal usage)
        if (changedvalues == true) setFlashValues();  // Write settings to flash
        update_display();                             // Update display (1x per minute regulary)
      }
      if (updatemode == true) otaserver.handleClient();  // ESP32 OTA update
    }
  } else {                                               // Offline Mode actions:
    if (debugtexts == 1) Serial.println(rtc.getTime());  // Time string as e.g. 15:24:38
    struct tm timeinfo = rtc.getTimeStruct();
    iHour = timeinfo.tm_hour;
    iMinute = timeinfo.tm_min;
    iSecond = timeinfo.tm_sec;
    if (updatedevice == true) {
      if (changedvalues == true) setFlashValues();  // Write settings to flash
      update_display();
    }
    delay(1000);
  }
  dnsServer.processNextRequest();  // Update the web server
}


// ###########################################################################################################################################
// # Setup the internal web server configuration page:
// ###########################################################################################################################################
void setupWebInterface() {
  dnsServer.start(DNS_PORT, "*", apIP);
  if (UseOnlineMode == 0) ESPUI.captivePortal = true;  // Offline Mode Captive Portal


  // Section General:
  // ################
  ESPUI.separator("General:");

  // Welcome label:
  ESPUI.label("WordClock configuration", ControlColor::None, "Welcome to the WordClock configuration. Here you can adjust your WordClock settings to your personal needs. Enjoy your WordClock =)");

  // WordClock version:
  ESPUI.label("WordClock software version", ControlColor::None, WORD_CLOCK_VERSION);



  // Section Offline Mode:
  // #####################
  ESPUI.separator("Operation mode:");

  // Status label:
  if (UseOnlineMode == 0) {
    statusLabelID = ESPUI.label("Operation mode", ControlColor::None, "WordClock is used in Offline Mode. Please check time value.");
  } else {
    statusLabelID = ESPUI.label("Operation mode", ControlColor::Dark, "WordClock is used in Online Mode. All functions are available.");
  }

  // Add the invisible "Time" control to update to the current time of your device (PC, smartphone, tablet, ...)
  timeId = ESPUI.addControl(Time, "", "", None, 0, timeCallback);

  // Use online or offline mode:
  ESPUI.switcher("Use WordClock in Online Mode (change forces restart)", &switchOffline, ControlColor::Dark, UseOnlineMode);

  if (UseOnlineMode == 0) {
    // Time read from your device:
    ESPUI.button("Get time from your device", &getTimeCallback, ControlColor::Dark, "Get time from your computer, phone, tablet");
    statusTimeFromDevice = ESPUI.label("Time received from your device", ControlColor::Dark, "-");

    // Hour offset:
    OfflineCurrentHourOffset = ESPUI.addControl(ControlType::Select, "Current hour offset", String(iHourOffset), ControlColor::Dark, Control::noParent, SetOfflineHourOffset);
    ESPUI.addControl(ControlType::Option, "- 9 hours", "-9", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 8 hours", "-8", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 7 hours", "-7", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 6 hours", "-6", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 5 hours", "-5", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 4 hours", "-4", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 3 hours", "-3", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 2 hours", "-2", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "- 1 hours", "-1", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 0 hours", "0", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 1 hours", "1", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 2 hours", "2", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 3 hours", "3", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 4 hours", "4", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 5 hours", "5", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 6 hours", "6", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 7 hours", "7", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 8 hours", "8", ControlColor::Alizarin, OfflineCurrentHourOffset);
    ESPUI.addControl(ControlType::Option, "+ 9 hours", "9", ControlColor::Alizarin, OfflineCurrentHourOffset);

    // Time set on WordClock with the calculated offset:
    statusTimeSetOffline = ESPUI.label("Time set on WordClock", ControlColor::Dark, "-");
  }

  // Operation Mode hints:
  ESPUI.switcher("Show the Operation Mode hints", &switchOMhints, ControlColor::Dark, showOMhints);
  OfflineModeHint1 = ESPUI.label("What is 'Online Mode' ?", ControlColor::Dark, "WordClock uses your set local WiFi to update the time and can use all of the smart functions in your local network. (Normal and recommended operation mode)");
  ESPUI.setPanelStyle(OfflineModeHint1, "width: 95%;");
  OfflineModeHint2 = ESPUI.label("What is 'Offline Mode' ?", ControlColor::Dark, "WordClock does not use your WiFi and it sets up an own, internal access point '" + String(Offline_SSID) + "' you can connect to to control all functions that do not require your network. All smart functions will be disabled and you need to set the time manually after each startup, but you can use the time piece in environments without a local WiFi.");
  ESPUI.setPanelStyle(OfflineModeHint2, "width: 95%;");
  OfflineModeHint3 = ESPUI.label("General usage hints", ControlColor::Dark, "You can switch between both modes without lost of data. In case your browser does not open the WordClock configuration page automatically in any mode after connecting to the access point, please navigate to this URL manually: http://" + IpAddress2String(WiFi.softAPIP()));
  ESPUI.setPanelStyle(OfflineModeHint3, "width: 95%;");
  if (showOMhints == 0) {
    ESPUI.updateVisibility(OfflineModeHint1, false);
    ESPUI.updateVisibility(OfflineModeHint2, false);
    ESPUI.updateVisibility(OfflineModeHint3, false);
  }



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
  intensity_web_HintID = ESPUI.label("Manual settings disabled due to web URL usage:", ControlColor::Alizarin, "Restart WordClock or deactivate web control usage via <a href='http://" + IpAddress2String(WiFi.localIP()) + ":2023/config?INTENSITYviaWEB=0' target='_blank' style='color:#ffffff;'>http://" + IpAddress2String(WiFi.localIP()) + ":2023/config?INTENSITYviaWEB=0</a>");
  ESPUI.updateVisibility(intensity_web_HintID, false);



  // Section LED night mode settings:
  // ################################
  DayNightSectionID = ESPUI.separator("Day/Night LED brightness mode settings:");

  // Use night mode function:
  switchNightModeID = ESPUI.switcher("Use night mode to reduce brightness", &switchNightMode, ControlColor::Dark, usenightmode);

  // Intensity DAY slider selector:
  sliderBrightnessDayID = ESPUI.slider("Brightness during the day", &sliderBrightnessDay, ControlColor::Dark, intensity_day, 0, LEDintensityLIMIT);

  // Intensity NIGHT slider selector:
  sliderBrightnessNightID = ESPUI.slider("Brightness at night", &sliderBrightnessNight, ControlColor::Dark, intensity_night, 0, LEDintensityLIMIT);

  // Warning if day/night time setting is wrong:
  statusNightModeWarnID = ESPUI.label("Night mode settings error", ControlColor::Alizarin, "Wrong setting! 'Day time starts at' needs to be before 'Day time ends after'. Please correct values.");
  ESPUI.updateVisibility(statusNightModeWarnID, false);

  // Night mode status:
  statusNightModeID = ESPUI.label("Night mode status", ControlColor::Dark, "Night mode not used");

  // Day mode start time:
  call_day_time_startID = ESPUI.text("Day time starts at", call_day_time_start, ControlColor::Dark, String(day_time_start));
  ESPUI.setInputType(call_day_time_startID, "time");

  // Day mode stop time:
  call_day_time_stopID = ESPUI.text("Day time ends after", call_day_time_stop, ControlColor::Dark, String(day_time_stop));
  ESPUI.setInputType(call_day_time_stopID, "time");



  // Section Startup:
  // ################
  if (UseOnlineMode == 1) {
    ESPUI.separator("Startup:");

    // Startup WordClock text function:
    ESPUI.switcher("Show the 'WordClock' text on startup", &switchStartupText, ControlColor::Dark, useStartupText);

    // Show IP-address on startup:
    ESPUI.switcher("Show IP-address on startup", &switchShowIP, ControlColor::Dark, useshowip);
  }



  // Section WiFi:
  // #############
  if (UseOnlineMode == 1) {
    ESPUI.separator("WiFi:");

    // WiFi SSID:
    ESPUI.label("SSID", ControlColor::Dark, WiFi.SSID());

    // WiFi signal strength:
    ESPUI.label("Signal", ControlColor::Dark, String(WiFi.RSSI()) + "dBm");

    // Hostname:
    ESPUI.label("Hostname in your router", ControlColor::Dark, "<a href='http://" + String(WiFi.getHostname()) + "' target='_blank' style='color:#ffffff;'>" + String(WiFi.getHostname()) + "</a>");

    // WiFi MAC-address:
    ESPUI.label("MAC address", ControlColor::Dark, WiFi.macAddress());

    // WiFi ip-address:
    ESPUI.label("IP-address", ControlColor::Dark, "<a href='http://" + IpAddress2String(WiFi.localIP()) + "' target='_blank' style='color:#ffffff;'>" + IpAddress2String(WiFi.localIP()) + "</a>");

    // WiFi DNS address:
    ESPUI.label("DNS address", ControlColor::Dark, IpAddress2String(WiFi.dnsIP()));

    // WiFi Gateway address:
    ESPUI.label("Gateway address", ControlColor::Dark, IpAddress2String(WiFi.gatewayIP()));
  }



  // Section Time settings:
  // ######################
  if (UseOnlineMode == 1) {
    ESPUI.separator("Time settings:");

    // WordClock startup time:
    ESPUI.label("WordClock startup time", ControlColor::Dark, iStartTime);

    // NTP time server: (All of these were successfully tested on 06.01.2024)
    mySetTimeServer = ESPUI.addControl(ControlType::Select, "Choose your time server (change forces restart)", String(myTimeServer), ControlColor::Dark, Control::noParent, SetMyTimeServer);
    ESPUI.addControl(ControlType::Option, "Best choice: Your local router (WiFi gateway address)", "Your local router", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "Special: Use value set in 'settings.h'", NTPserver_default, ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "Special: Your local 'Speedport IP' router", "speedport.ip", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "Special: Your local 'Fritz!Box' router", "fritz.box", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "Default: pool.ntp.org", "pool.ntp.org", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "AS: asia.pool.ntp.org", "asia.pool.ntp.org", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "AT: asynchronos.iiss.at", "asynchronos.iiss.at", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "BE: ntp1.oma.be", "ntp1.oma.be", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "BR: ntps1.pads.ufrj.br", "ntps1.pads.ufrj.br", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "CA: time.nrc.ca", "time.nrc.ca", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "CH: ntp.neel.ch", "ntp.neel.ch", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "CL: ntp.shoa.cl", "ntp.shoa.cl", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "CN: ntp.neu.edu.cn", "ntp.neu.edu.cn", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "CZ: ntp.nic.cz", "ntp.nic.cz", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "DE: time.fu-berlin.de", "time.fu-berlin.de", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "ES: hora.roa.es", "hora.roa.es", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "EU: europe.pool.ntp.org", "europe.pool.ntp.org", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "EUS: ntp.i2t.ehu.eus", "ntp.i2t.ehu.eus", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "HU: ntp.atomki.mta.hu", "ntp.atomki.mta.hu", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "IT: ntp1.inrim.it", "ntp1.inrim.it", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "JP: ntp.nict.jp", "ntp.nict.jp", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "MX: cronos.cenam.mx", "cronos.cenam.mx", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "NL: ntp.vsl.nl", "ntp.vsl.nl", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "PL: tempus1.gum.gov.pl", "tempus1.gum.gov.pl", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "RO: ntp1.usv.ro", "ntp1.usv.ro", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "SE: time1.stupi.se", "time1.stupi.se", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "UK: uk.pool.ntp.org", "uk.pool.ntp.org", ControlColor::Alizarin, mySetTimeServer);
    ESPUI.addControl(ControlType::Option, "US: north-america.pool.ntp.org", "north-america.pool.ntp.org", ControlColor::Alizarin, mySetTimeServer);
    mySetTimeServerID = ESPUI.label("Used NTP time server", ControlColor::Dark, NTPserver);

    // Time zone:
    mySetTimeZone = ESPUI.addControl(ControlType::Select, "Choose your time zone (change forces restart)", String(myTimeZone), ControlColor::Dark, Control::noParent, SetMyTimeZone);
    ESPUI.addControl(ControlType::Option, "Use value set in 'settings.h'", Timezone_default, ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "Asia: EET-2EEST,M3.5.5/0,M10.5.5/0", "EET-2EEST,M3.5.5/0,M10.5.5/0", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "Australia: ACST-9:30ACDT,M10.1.0,M4.1.0/3", "ACST-9:30ACDT,M10.1.0,M4.1.0/3", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "Central Europe: CET-1CEST,M3.5.0,M10.5.0/3 (Austria,Denmark,France,Germany,Italy,Netherlands,Poland,Switzerland)", "CET-1CEST,M3.5.0,M10.5.0/3", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "Most of Europe: MET-2METDST,M3.5.0/01,M10.5.0/02", "MET-2METDST,M3.5.0/01,M10.5.0/02", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "New Zealand: NZST-12NZDT,M9.5.0,M4.1.0/3 (Auckland)", "NZST-12NZDT,M9.5.0,M4.1.0/3", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "UK: GMT0BST,M3.5.0/01,M10.5.0/02 (London)", "GMT0BST,M3.5.0/01,M10.5.0/02", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "USA EST: EST5EDT,M3.2.0,M11.1.0", "EST5EDT,M3.2.0,M11.1.0", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "USA CST: CST6CDT,M3.2.0,M11.1.0", "CST6CDT,M3.2.0,M11.1.0", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "USA MST: MST7MDT,M4.1.0,M10.5.0", "MST7MDT,M4.1.0,M10.5.0", ControlColor::Alizarin, mySetTimeZone);
    ESPUI.addControl(ControlType::Option, "Vietnam: ICT-7", "ICT-7", ControlColor::Alizarin, mySetTimeZone);
    mySetTimeZoneID = ESPUI.label("Used time zone value", ControlColor::Dark, Timezone);
  }



  // Section smart home control via web URLs:
  // ########################################
  if (UseOnlineMode == 1) {
    ESPUI.separator("Smart home control via web URLs:");

    // About note:
    ESPUI.label("About note", ControlColor::Dark, "Control WordClock from your smart home environment via web URLs.");

    // Functions note:
    ESPUI.label("Functions", ControlColor::Dark, "You can turn the LEDs off or on via http commands to reduce energy consumption.");

    // Usage note:
    ESPUI.label("Usage hints and examples", ControlColor::Dark, "<a href='http://" + IpAddress2String(WiFi.localIP()) + ":2023' target='_blank' style='color:#ffffff;'>http://" + IpAddress2String(WiFi.localIP()) + ":2023</a> or <a href='http://" + String(WiFi.getHostname()) + ":2023' target='_blank' style='color:#ffffff;'>http://" + String(WiFi.getHostname()) + ":2023</a>");
  }



  // Section Update:
  // ###############
  if (UseOnlineMode == 1) {
    ESPUI.separator("Update:");

    // Update WordClock:
    ESPUI.button("Activate update mode", &buttonUpdate, ControlColor::Dark, "Activate update mode", (void*)1);

    // Update URL
    ESPUI.label("Update URLs", ControlColor::Dark, "<a href='http://" + IpAddress2String(WiFi.localIP()) + ":8080' target='_blank' style='color:#ffffff;'>http://" + IpAddress2String(WiFi.localIP()) + ":8080</a> or <a href='http://" + String(WiFi.getHostname()) + ":8080' target='_blank' style='color:#ffffff;'>http://" + String(WiFi.getHostname()) + ":8080</a>");

    // AWSW software GitHub repository:
    ESPUI.label("Download newer software updates here", ControlColor::Dark, "<a href='https://github.com/AWSW-de/WordClock-16x8-LED-matrix' target='_blank' style='color:#ffffff;'>https://github.com/AWSW-de/WordClock-16x8-LED-matrix</a>");
  }


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
  if (langLEDlayout == 10) selectLangTXT = "Luxembourgish";
  if (langLEDlayout == 11) selectLangTXT = "East German";
  if (langLEDlayout == 12) selectLangTXT = "Austrian (2024 models only)";
  Serial.print("Selected language: ");
  Serial.println(selectLangTXT);

  // Change language:
  selectLang = ESPUI.addControl(ControlType::Select, "Change layout language", String(langLEDlayout), ControlColor::Dark, Control::noParent, call_langauge_select);
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
  ESPUI.addControl(ControlType::Option, "Luxembourgish", "10", ControlColor::Alizarin, selectLang);
  ESPUI.addControl(ControlType::Option, "East German", "11", ControlColor::Alizarin, selectLang);
  ESPUI.addControl(ControlType::Option, "Austrian (2024 models only)", "12", ControlColor::Alizarin, selectLang);

  // Current language:
  statusLanguageID = ESPUI.label("Current layout language", ControlColor::Dark, selectLangTXT);



  // Section Maintenance:
  // ####################
  ESPUI.separator("Maintenance:");

  // Restart WordClock:
  ESPUI.button("Restart", &buttonRestart, ControlColor::Dark, "Restart", (void*)1);

  // Reset WiFi settings:
  if (UseOnlineMode == 1) {
    ESPUI.button("Reset WiFi settings", &buttonWiFiReset, ControlColor::Dark, "Reset WiFi settings", (void*)2);
  }

  // Reset WordClock settings:
  ESPUI.button("Reset settings (except WiFi, language & operation mode)", &buttonWordClockReset, ControlColor::Dark, "Reset WordClock settings", (void*)3);



  // Section License:
  // ####################
  ESPUI.separator("License information:");

  // License information:
  ESPUI.label("License information", ControlColor::Dark, "NonCommercial — You may not use the project for commercial purposes! © 2024 Copyright by <a href='https://github.com/AWSW-de/WordClock-16x8-LED-matrix' target='_blank' style='color:#ffffff;'>AWSW</a>");



  checkforNightMode();       // Check for night mode settings on startup
  ESPUI.begin("WordClock");  // Deploy the page
}


// ###########################################################################################################################################
// # Read settings from flash:
// ###########################################################################################################################################
void getFlashValues() {
  if (debugtexts == 1) Serial.println("Read settings from flash: START");
  langLEDlayout = preferences.getUInt("langLEDlayout", langLEDlayout_default);
  myTimeZone = preferences.getString("myTimeZone", Timezone_default);
  Timezone = myTimeZone;
  myTimeServer = preferences.getString("myTimeServer", NTPserver_default);
  NTPserver = myTimeServer;
  UseOnlineMode = preferences.getUInt("UseOnlineMode", 1);
  showOMhints = preferences.getUInt("showOMhints", showOMhints_default);
  redVal_time = preferences.getUInt("redVal_time", redVal_time_default);
  greenVal_time = preferences.getUInt("greenVal_time", greenVal_time_default);
  blueVal_time = preferences.getUInt("blueVal_time", blueVal_time_default);
  redVal_back = preferences.getUInt("redVal_back", redVal_back_default);
  greenVal_back = preferences.getUInt("greenVal_back", greenVal_back_default);
  blueVal_back = preferences.getUInt("blueVal_back", blueVal_back_default);
  intensity_day = preferences.getUInt("intensity_day", intensity_day_default);
  intensity_night = preferences.getUInt("intensity_night", intensity_night_default);
  usenightmode = preferences.getUInt("usenightmode", usenightmode_default);
  day_time_start = preferences.getString("day_time_start", day_time_start_default);
  day_time_stop = preferences.getString("day_time_stop", day_time_stop_default);
  iHourOffset = preferences.getUInt("iHourOffset", iHourOffset_default);
  useshowip = preferences.getUInt("useshowip", useshowip_default);
  useStartupText = preferences.getUInt("useStartupText", useStartupText_default);
  usesinglemin = preferences.getUInt("usesinglemin", usesinglemin_default);
  RandomColor = preferences.getUInt("RandomColor", RandomColor_default);
  if (debugtexts == 1) Serial.println("Read settings from flash: END");
}


// ###########################################################################################################################################
// # Write settings to flash:
// ###########################################################################################################################################
void setFlashValues() {
  if (debugtexts == 1) Serial.println("Write settings to flash: START");
  changedvalues = false;
  preferences.putUInt("langLEDlayout", langLEDlayout);
  preferences.putString("myTimeZone", myTimeZone);
  preferences.putString("myTimeServer", myTimeServer);
  preferences.putUInt("UseOnlineMode", UseOnlineMode);
  preferences.putUInt("showOMhints", showOMhints);
  preferences.putUInt("redVal_time", redVal_time);
  preferences.putUInt("greenVal_time", greenVal_time);
  preferences.putUInt("blueVal_time", blueVal_time);
  preferences.putUInt("redVal_back", redVal_back);
  preferences.putUInt("greenVal_back", greenVal_back);
  preferences.putUInt("blueVal_back", blueVal_back);
  preferences.putUInt("intensity_day", intensity_day);
  preferences.putUInt("intensity_night", intensity_night);
  preferences.putUInt("usenightmode", usenightmode);
  preferences.putString("day_time_start", day_time_start);
  preferences.putString("day_time_stop", day_time_stop);
  preferences.putUInt("iHourOffset", iHourOffset);
  preferences.putUInt("useshowip", useshowip);
  preferences.putUInt("useStartupText", useStartupText);
  preferences.putUInt("usesinglemin", usesinglemin);
  preferences.putUInt("RandomColor", RandomColor);
  if (debugtexts == 1) Serial.println("Write settings to flash: END");
  checkforNightMode();
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
  // Save stored values for WiFi, language and Operation Mode:
  String tempDelWiFiSSID = preferences.getString("WIFIssid");
  String tempDelWiFiPASS = preferences.getString("WIFIpass");
  int tempDelLANG = preferences.getUInt("langLEDlayout");
  int tempOfflineMode = preferences.getUInt("UseOnlineMode");
  preferences.clear();
  delay(100);
  preferences.putUInt("UseOnlineMode", tempOfflineMode);  // Restore Operation Mode
  preferences.putString("WIFIssid", tempDelWiFiSSID);     // Restore entered WiFi SSID
  preferences.putString("WIFIpass", tempDelWiFiPASS);     // Restore entered WiFi password
  preferences.putUInt("langLEDlayout", tempDelLANG);      // Restore entered language
  preferences.putUInt("showOMhints", showOMhints_default);
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
  preferences.putString("day_time_stop", day_time_stop_default);
  preferences.putString("day_time_stop", day_time_stop_default);
  preferences.putUInt("usesinglemin", usesinglemin_default);
  preferences.putUInt("RandomColor", RandomColor_default);
  preferences.putString("myTimeZone", Timezone_default);
  preferences.putString("myTimeServer", NTPserver_default);
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
  if (langLEDlayout == 10) selectLangTXT = "Luxembourgish";
  if (langLEDlayout == 11) selectLangTXT = "East German";
  if (langLEDlayout == 12) selectLangTXT = "Austrian (2024 models only)";
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
  delay(250);
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
  delay(1000);
  ESPUI.updateButton(sender->id, "Update mode active now - Use the update url: >>>");
  if (updatemode == false) {
    updatemode = true;
    int32_t c = strip.Color(0, 0, 255);
    int TextWait = 250;
    showtext('U', TextWait, c);
    showtext('P', TextWait, c);
    showtext('D', TextWait, c);
    showtext('A', TextWait, c);
    showtext('T', TextWait, c);
    showtext('E', TextWait, c);
    showtext(' ', TextWait, c);
    showtext('U', TextWait, c);
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

  if (langLEDlayout == 10) {     // LUXEMBOURGISH:
    setLEDcol(38, 38, color);    // R
    setLEDcol(73, 73, color);    // E
    setLEDcol(103, 103, color);  // S
    setLEDcol(139, 139, color);  // E
    setLEDcol(171, 171, color);  // T
  }

  if (langLEDlayout == 11) {  // EAST GERMAN:
    setLEDcol(5, 9, color);   // RESET
  }

  if (langLEDlayout == 12) {   // AUSTRIAN:
    setLEDcol(11, 15, color);  // RESET
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
#define SLIDER_UPDATE_TIME 150  // Wait at least 100 ms before allowing another slider update --> Bug fix for color slider crashing ESP32
void colCallTIME(Control* sender, int type) {
  static unsigned long last_slider_update = 0;  // Track the time of the last slider update
  if ((millis() - last_slider_update >= SLIDER_UPDATE_TIME)) {
    getRGBTIME(sender->value);
    last_slider_update = millis();
    if (debugtexts == 1) {
      Serial.println(type);
      Serial.println(sender->value);
    }
  }
  return;
}


// ###########################################################################################################################################
// # GUI: Color change for background color:
// ###########################################################################################################################################
#define SLIDER_UPDATE_BACK 150  // Wait at least 100 ms before allowing another slider update --> Bug fix for color slider crashing ESP32
void colCallBACK(Control* sender, int type) {
  static unsigned long last_slider_update = 0;  // Track the time of the last slider update
  if ((type == 10) && (millis() - last_slider_update >= SLIDER_UPDATE_BACK)) {
    getRGBBACK(sender->value);
    last_slider_update = millis();
    if (debugtexts == 1) {
      Serial.println(type);
      Serial.println(sender->value);
    }
  }
  return;
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
  day_time_start = sender->value;
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Time Day Mode Stop
// ###########################################################################################################################################
void call_day_time_stop(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  day_time_stop = sender->value;
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
// # GUI: Use Offline Mode switch:
// ###########################################################################################################################################
void switchOffline(Control* sender, int value) {
  updatedevice = false;
  Serial.println("Offline Mode change: Restart request");
  switch (value) {
    case S_ACTIVE:
      UseOnlineMode = 1;  // Online
      useshowip = useshowip_default;
      break;
    case S_INACTIVE:
      UseOnlineMode = 0;  // Offline
      useshowip = 1;
      break;
  }
  changedvalues = true;
  setFlashValues();  // Save values!
  ClearDisplay();
  ResetTextLEDs(strip.Color(0, 255, 0));
  strip.show();
  Serial.println("Offline Mode change: Perform restart now");
  delay(1000);
  ESP.restart();
}


// ###########################################################################################################################################
// # GUI: Show or hide the Operation Mode hints:
// ###########################################################################################################################################
void switchOMhints(Control* sender, int value) {
  updatedevice = false;
  switch (value) {
    case S_ACTIVE:
      showOMhints = 1;  // Show
      ESPUI.updateVisibility(OfflineModeHint1, true);
      ESPUI.updateVisibility(OfflineModeHint2, true);
      ESPUI.updateVisibility(OfflineModeHint3, true);
      ESPUI.jsonReload();
      break;
    case S_INACTIVE:
      showOMhints = 0;  // Hide
      ESPUI.updateVisibility(OfflineModeHint1, false);
      ESPUI.updateVisibility(OfflineModeHint2, false);
      ESPUI.updateVisibility(OfflineModeHint3, false);
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Manual Offline Mode offset hour setting
// ###########################################################################################################################################
void SetOfflineHourOffset(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  iHourOffset = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Time Zone selection:
// ###########################################################################################################################################
void SetMyTimeZone(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  myTimeZone = sender->value;
  changedvalues = true;
  updatedevice = true;
  setFlashValues();  // Save values!
  ClearDisplay();
  ResetTextLEDs(strip.Color(0, 255, 0));
  strip.show();
  Serial.println("Time Zone change: Perform restart now");
  delay(1000);
  ESP.restart();
}


// ###########################################################################################################################################
// # GUI: Time Server selection:
// ###########################################################################################################################################
void SetMyTimeServer(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  if (sender->value == "Your local router") {
    myTimeServer = IpAddress2String(WiFi.gatewayIP());
  } else {
    myTimeServer = sender->value;
  }
  changedvalues = true;
  updatedevice = true;
  setFlashValues();  // Save values!
  ClearDisplay();
  ResetTextLEDs(strip.Color(0, 255, 0));
  strip.show();
  Serial.println("Time Server change: Perform restart now");
  Serial.println("Time Server set to: " + String(sender->value));
  delay(1000);
  ESP.restart();
}


// ###########################################################################################################################################
// # GUI: Manual time setting from device (PC, tablet, smartphone) as long as the Offline Mode web portal is opened:
// ###########################################################################################################################################
void getTimeCallback(Control* sender, int type) {
  if (type == B_UP) {
    ESPUI.updateTime(timeId);
  }
}


// ###########################################################################################################################################
// # GUI: Manual time setting from device (PC, tablet, smartphone):
// ###########################################################################################################################################
void timeCallback(Control* sender, int type) {
  updatedevice = false;
  if (type == TM_VALUE) {
    // Serial.print("Auto Time by device: ");
    // Serial.println(sender->value);
    // ESPUI.print(statusTimeFromDevice, String(sender->value));

    String calTimestamp = String(sender->value);
    struct tm tm;
    // Test String to get all values:
    // Serial.println("Parsing " + calTimestamp);  // 2024-01-04T18:33:37.294Z
    // #######################################################
    // for (int i = 0; i <= 25; i++) {
    //   Serial.print(i);
    //   Serial.print(" = ");
    //   Serial.println(calTimestamp.substring(i).toInt());
    // }
    // #######################################################

    // Date not used yet:
    // String year = calTimestamp.substring(0, 4);
    // String month = calTimestamp.substring(4, 6);
    // if (month.startsWith("0")) {
    //   month = month.substring(1);
    // }
    // String day = calTimestamp.substring(6, 8);
    // if (day.startsWith("0")) {
    //   month = day.substring(1);
    // }
    // tm.tm_year = year.toInt() - 1900;
    // tm.tm_mon = month.toInt() - 1;
    // tm.tm_mday = day.toInt();
    tm.tm_hour = calTimestamp.substring(11).toInt();  // Hour from string
    tm.tm_min = calTimestamp.substring(14).toInt();   // Minute from string
    tm.tm_sec = calTimestamp.substring(17).toInt();   // Second from string
    iHour = tm.tm_hour;
    iMinute = tm.tm_min;
    iSecond = tm.tm_sec;
    
    // Test a special time:
    if (testspecialtimeOFF == 1) {
      // Serial.println("Special time test active in Offline Mode: " + String(test_hourOFF) + ":" + String(test_minuteOFF) + ":" + String(test_secondOFF));
      iHour = test_hourOFF;
      iMinute = test_minuteOFF;
      iSecond = test_secondOFF;
    }

    ESPUI.print(statusTimeFromDevice, String(iHour) + ":" + String(iMinute) + ":" + String(iSecond));  // Update GUI: Time from device
    if (iHourOffset >= 0) {
      // Serial.println("iHour OLD: " + String(iHour));
      // Serial.println("iHour + Offset: " + String(iHourOffset));  // Set stored hour offset to received hour
      iHour = iHour + iHourOffset;
      // Serial.println("iHour NEW: " + String(iHour));
    }
    if ((iHourOffset <= -1)) {
      // Serial.println("iHour OLD: " + String(iHour));
      // Serial.println("iHour - Offset: " + String(iHourOffset));
      iHour = iHourOffset + iHour;
      // Serial.println("iHour NEW: " + String(iHour));
    }
    int xHour = iHour;
    if (xHour >= 24) xHour = xHour - 24;                                                                 // Mid night corrections because of offset calculation
    if (xHour < 0) xHour = xHour + 24;                                                                   // Mid night corrections because of offset calculation
    ESPUI.print(statusTimeSetOffline, String(xHour) + ":" + String(iMinute) + ":" + String(iSecond));    // Update GUI: Calculated time with offset for negative offset
    if (debugtexts == 1) Serial.println(String(xHour) + ":" + String(iMinute) + ":" + String(iSecond));  // Test output
    rtc.setTime(iSecond, iMinute, xHour, 1, 1, 2024);                                                    // Set time on device RTC: (ss, mm, hh, DD, MM, YYYY) --> Date not used yet
    checkforNightMode();                                                                                 // Night Mode check
    updatenow = true;
    updatedevice = true;
  }
  delay(1000);
}


// ###########################################################################################################################################
// # GUI: Check if Night Mode needs to be set depending on the time:
// ###########################################################################################################################################
void checkforNightMode() {
  // Start time to int:
  String day_time_startH = getValue(day_time_start, ':', 0);
  String day_time_startM = getValue(day_time_start, ':', 1);
  if (debugtexts == 1) Serial.println("day_time_start H part = " + day_time_startH);
  if (debugtexts == 1) Serial.println("day_time_start M part = " + day_time_startM);
  int dt_start_HM = (day_time_startH.toInt() * 100) + day_time_startM.toInt();
  if (debugtexts == 1) Serial.println("dt_start_HM = " + String(dt_start_HM));
  // Stop time to int:
  String day_time_stopH = getValue(day_time_stop, ':', 0);
  String day_time_stopM = getValue(day_time_stop, ':', 1);
  if (debugtexts == 1) Serial.println("day_time_stop H part = " + day_time_stopH);
  if (debugtexts == 1) Serial.println("day_time_stop M part = " + day_time_stopM);
  int dt_stop_HM = (day_time_stopH.toInt() * 100) + day_time_stopM.toInt();
  if (debugtexts == 1) Serial.println("dt_stop_HM = " + String(dt_stop_HM));
  // Current time to int:
  int now_HM = (iHour * 100) + iMinute;
  if (debugtexts == 1) Serial.println("now_HM = " + String(now_HM));

  // Check if start time is before stop time:
  if ((dt_start_HM > dt_stop_HM) || (dt_start_HM == dt_stop_HM)) {
    if (debugtexts == 1) Serial.println("Wrong setting! 'Day time starts' needs to be before 'Day time ends after'. Please correct values.");
    ESPUI.updateVisibility(statusNightModeWarnID, true);  // Show warning
    ESPUI.jsonReload();
  } else {
    ESPUI.updateVisibility(statusNightModeWarnID, false);  // Hide warning
    // Day or Night time:
    if (usenightmode == 1) {
      if ((now_HM >= dt_start_HM) && (now_HM <= dt_stop_HM)) {
        if (debugtexts == 1) Serial.println("Day Time");
        statusNightModeIDtxt = "Day Time";
        ESPUI.print(statusNightModeID, "Day time");
        if ((iHour == 0) && (day_time_startH.toInt() == 23)) {  // Special function if day_time_stop set to 23 and time is 24, so 0...
          statusNightModeIDtxt = "Night time";
          ESPUI.print(statusNightModeID, "Night time");
        }
      } else {
        if (debugtexts == 1) Serial.println("Night Time");
        statusNightModeIDtxt = "Night time";
        ESPUI.print(statusNightModeID, "Night time");
      }
    } else {
      ESPUI.print(statusNightModeID, "Night mode not used");
    }
  }
}


// ###########################################################################################################################################
// # Update the display / time on it:
// ###########################################################################################################################################
void update_display() {
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
    showtext('T', TextWait, c);
    showtext('E', TextWait, c);
    showtext('S', TextWait, c);
    showtext('T', TextWait, c);
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
  if (debugtexts == 1) Serial.println("Update display now: " + String(hours) + ":" + String(minutes) + ":" + String(iSecond));

  // Set LED intensity + DAY/NIGHT MDOE:
  // ##################
  // Start time to int:
  String day_time_startH = getValue(day_time_start, ':', 0);
  String day_time_startM = getValue(day_time_start, ':', 1);
  int dt_start_HM = (day_time_startH.toInt() * 100) + day_time_startM.toInt();
  // Stop time to int:
  String day_time_stopH = getValue(day_time_stop, ':', 0);
  String day_time_stopM = getValue(day_time_stop, ':', 1);
  int dt_stop_HM = (day_time_stopH.toInt() * 100) + day_time_stopM.toInt();
  // Current time to int:
  int now_HM = (iHour * 100) + iMinute;

  // Set intensity:
  if ((usenightmode == 1) && (set_web_intensity == 0)) {
    if ((now_HM >= dt_start_HM) && (now_HM <= dt_stop_HM)) {
      intensity = intensity_day;
      if (statusNightModeIDtxt != "Day time") {
        statusNightModeIDtxt = "Day time";
        ESPUI.print(statusNightModeID, "Day time");
      }
      if ((iHour == 0) && (day_time_startH.toInt() == 23)) {  // Special function if day_time_stop set to 23 and time is 24, so 0...
        intensity = intensity_night;
        if (statusNightModeIDtxt != "Night time") {
          statusNightModeIDtxt = "Night time";
          ESPUI.print(statusNightModeID, "Night time");
        }
      }
    } else {
      intensity = intensity_night;
      if (statusNightModeIDtxt != "Night time") {
        statusNightModeIDtxt = "Night time";
        ESPUI.print(statusNightModeID, "Night time");
      }
    }
  } else {
    if (set_web_intensity == 0) intensity = intensity_day;
    if (set_web_intensity == 1) intensity = intensity_web;
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
  if (testspecialtimeON == 1) {
    Serial.println("Special time test active in Online Mode: " + String(test_hourON) + ":" + String(test_minuteON) + ":" + String(test_secondON));
    iHour = test_hourON;
    iMinute = test_minuteON;
    iSecond = test_secondON;
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
  if (langLEDlayout == 10) {  // LUXEMBOURGISH:

    // ET ASS:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(10, 12, colorRGB);
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
      if (testPrintTimeTexts == 1) Serial.print("FËNNEF ");
    }
    // VÉIREL:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(36, 41, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VÉIREL ");
    }
    // ZÉNG: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(32, 35, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZÉNG ");
    }
    // ZWANZEG:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(72, 78, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZWANZEG ");
    }
    // OP:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(65, 66, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("OP ");
    }
    // VIR:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(68, 70, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VIR ");
    }
    // HALLWER:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(105, 111, colorRGB);
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
            if (testPrintTimeTexts == 1) Serial.print("ENG ");
          }
          break;
        }
      case 2:
        {
          setLEDcol(199, 202, colorRGB);  // ZWOU
          if (testPrintTimeTexts == 1) Serial.print("ZWOU ");
          break;
        }
      case 3:
        {
          setLEDcol(140, 143, colorRGB);  // DRÄI
          if (testPrintTimeTexts == 1) Serial.print("DRÄI ");
          break;
        }
      case 4:
        {
          setLEDcol(203, 207, colorRGB);  // VÉIER
          if (testPrintTimeTexts == 1) Serial.print("VÉIER ");
          break;
        }
      case 5:
        {
          setLEDcol(160, 165, colorRGB);  // FËNNEF
          if (testPrintTimeTexts == 1) Serial.print("FËNNEF ");
          break;
        }
      case 6:
        {
          setLEDcol(166, 170, colorRGB);  // SECHS
          if (testPrintTimeTexts == 1) Serial.print("SECHS ");
          break;
        }
      case 7:
        {
          setLEDcol(99, 103, colorRGB);  // SIWEN
          if (testPrintTimeTexts == 1) Serial.print("SIWEN ");
          break;
        }
      case 8:
        {
          setLEDcol(171, 175, colorRGB);  // AACHT
          if (testPrintTimeTexts == 1) Serial.print("AACHT ");
          break;
        }
      case 9:
        {
          setLEDcol(96, 99, colorRGB);  // NÉNG
          if (testPrintTimeTexts == 1) Serial.print("NÉNG ");
          break;
        }
      case 10:
        {
          setLEDcol(133, 136, colorRGB);  // ZÉNG (Stonnen)
          if (testPrintTimeTexts == 1) Serial.print("ZÉNG ");
          break;
        }
      case 11:
        {
          setLEDcol(128, 132, colorRGB);  // ELLEF
          if (testPrintTimeTexts == 1) Serial.print("ELLEF ");
          break;
        }
      case 12:
        {
          setLEDcol(192, 198, colorRGB);  // ZWIELEF
          if (testPrintTimeTexts == 1) Serial.print("ZWIELEF ");
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(236, 239, colorRGB);  // AUER
      if (testPrintTimeTexts == 1) Serial.print("AUER ");
    }
  }

  // ########################################################### EAST GERMAN:
  if (langLEDlayout == 11) {  // EAST GERMAN:

    // ES IST:
    setLEDcol(7, 8, colorRGB);
    setLEDcol(45, 47, colorRGB);
    if (testPrintTimeTexts == 1) {
      Serial.println("");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(" --> ES IST ");
    }

    // FÜNF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(32, 35, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
    }
    // ZEHN: (Minuten)
    if ((minDiv == 2) || (minDiv == 4) || (minDiv == 8) || (minDiv == 10)) {
      setLEDcol(40, 43, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
    }
    // VIERTEL:
    if (minDiv == 3) {
      setLEDcol(64, 70, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VIERTEL ");
    }
    // DREIVIERTEL:
    if (minDiv == 9) {
      setLEDcol(64, 74, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("DREIVIERTEL ");
    }
    // NACH:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(108, 111, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("NACH ");
    }
    // VOR:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(76, 78, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VOR ");
    }
    // HALB:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 6) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(103, 106, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("HALB ");
    }
    // UM:
    if ((minDiv == 0) || (minDiv == 1) || (minDiv == 2) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(101, 102, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("UM ");
    }

    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 15 hour needs to be counted up:
    // viertel 2 = 13:15
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
            setLEDcol(168, 171, colorRGB);  // EINS
            if (testPrintTimeTexts == 1) Serial.print("EINS ");
          }
          break;
        }
      case 2:
        {
          setLEDcol(172, 175, colorRGB);  // ZWEI
          if (testPrintTimeTexts == 1) Serial.print("ZWEI ");
          break;
        }
      case 3:
        {
          setLEDcol(192, 195, colorRGB);  // DREI
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
          setLEDcol(138, 143, colorRGB);  // SIEBEN
          if (testPrintTimeTexts == 1) Serial.print("SIEBEN ");
          break;
        }
      case 8:
        {
          setLEDcol(199, 202, colorRGB);  // ACHT
          if (testPrintTimeTexts == 1) Serial.print("ACHT ");
          break;
        }
      case 9:
        {
          setLEDcol(96, 99, colorRGB);  // NEUN
          if (testPrintTimeTexts == 1) Serial.print("NEUN ");
          break;
        }
      case 10:
        {
          setLEDcol(133, 136, colorRGB);  // ZEHN (Stunden)
          if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
          break;
        }
      case 11:
        {
          setLEDcol(196, 198, colorRGB);  // ELF
          if (testPrintTimeTexts == 1) Serial.print("ELF ");
          break;
        }
      case 12:
        {
          setLEDcol(203, 207, colorRGB);  // ZWÖLF
          if (testPrintTimeTexts == 1) Serial.print("ZWÖLF ");
          break;
        }
    }
  }

  // ########################################################### AUSTRIAN:
  if (langLEDlayout == 12) {  // AUSTRIAN:

    // ES IST:
    setLEDcol(13, 14, colorRGB);
    setLEDcol(8, 10, colorRGB);
    if (testPrintTimeTexts == 1) {
      Serial.println("");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(" --> ES IST ");
    }

    // FÜNF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(43, 46, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
    }
    // ZEHN: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(76, 79, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
    }
    // VIERTEL:
    if((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(65, 71, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VIERTEL ");
    }
    // UND in FÜNFUNDZWANZIG:
    if ((minDiv == 5) || (minDiv == 7)) {
      setLEDcol(40, 42, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("UND ");
    }
    // ZWANZIG:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(33, 39, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("ZWANZIG ");
    }
    // NACH:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 5)) {
      setLEDcol(108, 111, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("NACH ");
    }
    // VOR:
    if ((minDiv == 7) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(105, 107, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("VOR ");
    }
    // HALB:
    if (minDiv == 6) {
      setLEDcol(96, 99, colorRGB);
      if (testPrintTimeTexts == 1) Serial.print("HALB ");
    }

    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 30 hour needs to be counted up:
    if (iMinute >= 30) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }

    switch (xHour) {
      case 1:
        {
          if (xHour == 1) {
            setLEDcol(165, 167, colorRGB);  // EIN
            if (testPrintTimeTexts == 1) Serial.print("EIN ");
          }
          if ((xHour == 1) && (iMinute > 4)) {
            setLEDcol(164, 167, colorRGB);  // EINS (S in EINS) (just used if not point 1 o'clock)
            if (testPrintTimeTexts == 1) Serial.print("EINS ");
          }
          break;
        }
      case 2:
        {
          setLEDcol(192, 195, colorRGB);  // ZWEI
          if (testPrintTimeTexts == 1) Serial.print("ZWEI ");
          break;
        }
      case 3:
        {
          setLEDcol(168, 171, colorRGB);  // DREI
          if (testPrintTimeTexts == 1) Serial.print("DREI ");
          break;
        }
      case 4:
        {
          setLEDcol(196, 199, colorRGB);  // VIER
          if (testPrintTimeTexts == 1) Serial.print("VIER ");
          break;
        }
      case 5:
        {
          setLEDcol(200, 203, colorRGB);  // FUENF
          if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
          break;
        }
      case 6:
        {
          setLEDcol(160, 164, colorRGB);  // SECHS
          if (testPrintTimeTexts == 1) Serial.print("SECHS ");
          break;
        }
      case 7:
        {
          setLEDcol(135, 140, colorRGB);  // SIEBEN
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
          setLEDcol(128, 131, colorRGB);  // NEUN
          if (testPrintTimeTexts == 1) Serial.print("NEUN ");
          break;
        }
      case 10:
        {
          setLEDcol(131, 134, colorRGB);  // ZEHN (Stunden)
          if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
          break;
        }
      case 11:
        {
          setLEDcol(141, 143, colorRGB);  // ELF
          if (testPrintTimeTexts == 1) Serial.print("ELF ");
          break;
        }
      case 12:
        {
          setLEDcol(203, 207, colorRGB);  // ZWÖLF
          if (testPrintTimeTexts == 1) Serial.print("ZWÖLF ");
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(237, 239, colorRGB);  // UHR
      if (testPrintTimeTexts == 1) Serial.print("UHR ");
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

  // ##################################################### LUXEMBOURGISH:
  if (langLEDlayout == 10) {  // LUXEMBOURGISH:

    switch (minMod) {
      case 1:
        {
          setLEDcol(234, 234, colorRGB);  // +
          setLEDcol(232, 232, colorRGB);  // 1
          setLEDcol(227, 225, colorRGB);  // MIN
          break;
        }
      case 2:
        {
          setLEDcol(234, 234, colorRGB);  // +
          setLEDcol(231, 231, colorRGB);  // 2
          setLEDcol(227, 225, colorRGB);  // MIN
          break;
        }
      case 3:
        {
          setLEDcol(234, 234, colorRGB);  // +
          setLEDcol(230, 230, colorRGB);  // 3
          setLEDcol(227, 225, colorRGB);  // MIN
          break;
        }
      case 4:
        {
          setLEDcol(234, 234, colorRGB);  // +
          setLEDcol(229, 229, colorRGB);  // 4
          setLEDcol(227, 225, colorRGB);  // MIN
          break;
        }
    }
  }

  // ##################################################### EAST GERMAN:
  if (langLEDlayout == 11) {  // EAST GERMAN:

    switch (minMod) {
      case 1:
        {
          setLEDcol(239, 239, colorRGB);  // +
          setLEDcol(237, 237, colorRGB);  // 1
          setLEDcol(227, 232, colorRGB);  // MINUTE
          break;
        }
      case 2:
        {
          setLEDcol(239, 239, colorRGB);  // +
          setLEDcol(236, 236, colorRGB);  // 2
          setLEDcol(226, 232, colorRGB);  // MINUTEN
          break;
        }
      case 3:
        {
          setLEDcol(239, 239, colorRGB);  // +
          setLEDcol(235, 235, colorRGB);  // 3
          setLEDcol(226, 232, colorRGB);  // MINUTEN
          break;
        }
      case 4:
        {
          setLEDcol(239, 239, colorRGB);  // +
          setLEDcol(234, 234, colorRGB);  // 4
          setLEDcol(226, 232, colorRGB);  // MINUTEN
          break;
        }
    }
  }

  // ##################################################### AUSTRIAN:
  if (langLEDlayout == 12) {  // AUSTRIAN:

    switch (minMod) {
      case 1:
        {
          setLEDcol(235, 235, colorRGB);  // +
          setLEDcol(232, 232, colorRGB);  // 1
          setLEDcol(225, 227, colorRGB);  // MIN
          break;
        }
      case 2:
        {
          setLEDcol(235, 235, colorRGB);  // +
          setLEDcol(231, 231, colorRGB);  // 2
          setLEDcol(225, 227, colorRGB);  // MIN
          break;
        }
      case 3:
        {
          setLEDcol(235, 235, colorRGB);  // +
          setLEDcol(230, 230, colorRGB);  // 3
          setLEDcol(225, 227, colorRGB);  // MIN
          break;
        }
      case 4:
        {
          setLEDcol(235, 235, colorRGB);  // +
          setLEDcol(229, 229, colorRGB);  // 4
          setLEDcol(225, 227, colorRGB);  // MIN
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
  Serial.println("Setting up time from: " + NTPserver);

  // Flash TIME LEDs 3x blue:
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

    if (langLEDlayout == 10) {                  // LUXEMBOURGISH:
      setLEDcol(1, 4, strip.Color(0, 0, 255));  // ZÄIT
    }

    if (langLEDlayout == 11) {                  // EAST GERMAN:
      setLEDcol(1, 4, strip.Color(0, 0, 255));  // ZEIT
    }

    if (langLEDlayout == 12) {                  // AUSTRIAN:
      setLEDcol(0, 3, strip.Color(0, 0, 255));  // ZEIT
    }

    strip.show();
    delay(500);
  }


  struct tm timeinfo;
  configTime(0, 0, NTPserver.c_str());
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

    if (langLEDlayout == 10) {                  // LUXEMBOURGISH:
      setLEDcol(1, 4, strip.Color(255, 0, 0));  // ZÄIT
    }

    if (langLEDlayout == 11) {                  // EAST GERMAN:
      setLEDcol(1, 4, strip.Color(255, 0, 0));  // ZEIT
    }

    if (langLEDlayout == 12) {                  // AUSTRIAN:
      setLEDcol(0, 3, strip.Color(255, 0, 0));  // ZEIT
    }

    strip.show();
    delay(250);
    ClearDisplay();
    delay(250);
    ResetTextLEDs(strip.Color(0, 255, 0));
    Serial.println("! Failed to obtain time - Time server could not be reached ! --> RESTART THE DEVICE NOW...");
    delay(250);
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

  if (langLEDlayout == 10) {                  // LUXEMBOURGISH:
    setLEDcol(1, 4, strip.Color(0, 255, 0));  // ZÄIT
  }

  if (langLEDlayout == 11) {                  // EAST GERMAN:
    setLEDcol(1, 4, strip.Color(0, 255, 0));  // ZEIT
  }

  if (langLEDlayout == 12) {                  // AUSTRIAN:
    setLEDcol(0, 3, strip.Color(0, 255, 0));  // ZEIT
  }

  strip.show();
  delay(1000);
  Serial.println("Got the time from NTP server: " + NTPserver);
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
  showtext('W', TextWait, c);
  showtext('O', TextWait, c);
  showtext('R', TextWait, c);
  showtext('D', TextWait, c);
  showtext('C', TextWait, c);
  showtext('L', TextWait, c);
  showtext('O', TextWait, c);
  showtext('C', TextWait, c);
  showtext('K', TextWait, c);
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

  if (langLEDlayout == 10) {  // LUXEMBOURGISH:
    setLEDcol(5, 8, color);   // WIFI
  }

  if (langLEDlayout == 11) {   // EAST GERMAN:
    setLEDcol(36, 39, color);  // WLAN
  }

  if (langLEDlayout == 12) {  // AUSTRIAN:
    setLEDcol(4, 7, color);   // WLAN
  }

  strip.show();
}


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>WordClock Setup</title>
    <style>
      body {
        padding: 25px;
        font-size: 18px;
        background-color: #000;
        color: #fff;
        font-family: Arial, sans-serif;
      }
      h1, p { 
        text-align: center; 
        margin-bottom: 20px;
      }
      input, select { 
        font-size: 18px; 
        min-width: 150px;
      }
      button {
        display: inline-block;
        padding: 15px 25px;
        margin-top: 15px;
        font-size: 18px;
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
      button:hover {
        background-color: #3e8e41;
      }
      button:active {
        background-color: #3e8e41;
        box-shadow: 0 5px #666;
        transform: translateY(4px);
      }
    </style>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <script type="text/javascript">
      function disableButtonAndSubmit() {
        var btn = document.getElementById("submitButton");
        btn.disabled = true;
        setTimeout(function() {
          document.forms["myForm"].submit();
        }, 100);
      }
    </script>
  </head>
  <body>
    <form action="/start" name="myForm">
      <center>
        <h1>Welcome to the WordClock setup</h1>
        <p>Please add your local WiFi credentials and set your language on the next page</p>
        <p><button id="submitButton" type="submit" onclick="disableButtonAndSubmit()">Configure WordClock</button></p>
      </center>
    </form>
  </body>
  </html>
)rawliteral";


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char config_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
  <head>
    <title>WordClock Setup</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script language="JavaScript">
      function updateSSIDInput() {
        var ssidSelect = document.getElementById("mySSIDSelect");
        if (ssidSelect && ssidSelect.options.length > 0) {
          document.getElementById("mySSID").value = ssidSelect.options[ssidSelect.selectedIndex].value;
        }
      }
       function validateForm() {
        var errorParagraph = document.querySelector('.error');
        errorParagraph.style.display = 'none';
        errorParagraph.innerHTML = '';
        if (document.forms["myForm"]["mySSID"].value == "") {
          errorParagraph.innerHTML = "WiFi SSID must be set. ";
          errorParagraph.style.display = 'block';
          return false;
        }
        if (document.forms["myForm"]["myPW"].value == "") {
          errorParagraph.innerHTML = "WiFi password must be set. ";
          errorParagraph.style.display = 'block'; 
          return false;
        }
        if (document.forms["myForm"]["setlanguage"].value == "99") {
          errorParagraph.innerHTML = "Language layout must be set. ";
          errorParagraph.style.display = 'block'; 
          return false;
        }
        return true;
      }
      function disableButtonAndSubmit() {
        if (validateForm()) {
          var btn = document.getElementById("submitButton");
          btn.innerText = 'Restarting WordClock...';
          btn.disabled = true;
          setTimeout(function() {
            document.forms["myForm"].submit();
          }, 1000);
        }
      }
      window.onload = function() {
        var ssidSelect = document.getElementById("mySSIDSelect");
        if (ssidSelect) {
          ssidSelect.addEventListener('change', updateSSIDInput);
        }
      };
    </script>
    <style>
      body {
        padding: 25px;
        font-size: 18px;
        background-color: #000;
        color: #fff;
        font-family: Arial, sans-serif;
      }
      h1, p { 
        text-align: center; 
        margin-bottom: 20px;
      }
      p.error { 
        color: #ff0000; 
        display: none;
      }
      input, select { 
        font-size: 18px; 
        min-width: 150px;
      }
      button {
        display: inline-block;
        padding: 15px 25px;
        margin-top: 15px;
        font-size: 18px;
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
      button:hover { background-color: #3e8e41 }
      button:active {
        background-color: #3e8e41;
        box-shadow: 0 5px #666;
        transform: translateY(4px);
      }
    </style>
  </head>
  <body>
    <form action="/get" name="myForm" onsubmit="return validateForm()">
      <h1>Initial WordClock setup:</h1>
      <!-- Select element will be dynamically added here -->
      <p>
        <label for="mySSID">Enter your WiFi SSID:</label><br />
        <input id="mySSID" name="mySSID" value="" />
      </p>
      <p>
        <label for="myPW">Enter your WiFi password:</label><br/>
        <input type="text" id="myPW" name="myPW" value="" />
      </p>
      <p>
        <label for="setlanguage">Select your language layout:</label><br/>
        <select id="setlanguage" name="setlanguage">
          <option value="99" disabled selected>Choose...</option>
          <option value="0">GERMAN</option>
          <option value="1">ENGLISH</option>
          <option value="2">DUTCH</option>
          <option value="3">SWEDISH</option>
          <option value="4">ITALIAN</option>
          <option value="5">FRENCH</option>
          <option value="6">SWISS GERMAN</option>
          <option value="7">CHINESE</option>
          <option value="8">SWABIAN GERMAN</option>
          <option value="9">BAVARIAN</option>
          <option value="10">LUXEMBOURGISH</option>
          <option value="11">EAST GERMAN</option>
          <option value="12">AUSTRIAN (2024 models only)</option>
        </select>
      </p>
      <p class="error">Errors will be displayed here!</p>
      <p>
        <button id="submitButton" onclick="disableButtonAndSubmit()">Save values</button>
      </p>
    </form>
  </body>
  </html>
)rawliteral";


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char saved_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
  <head>
    <title>WordClock Setup</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body {
        padding: 25px;
        font-size: 18px;
        background-color: #000;
        color: #fff;
        font-family: Arial, sans-serif;
      }
      h1, p { 
        text-align: center; 
        margin-bottom: 20px;
      }
    </style>
  </head>
  <body>
    <h1>Settings saved!</h1>
    <p>WordClock is now trying to connect to the selected WiFi and display the chosen language.</p>
    <p>First the WiFi leds will be lit blue and change to green in case of a successful WiFi connection.</p>
    <p>If the connection fails the WiFi leds will flash red. Then please reconnect to the temporary access point again.</p>
    <p>Please close this page now, rejoin your selected WiFi and enjoy your WordClock. =)</p>
  </body>
  </html>
)rawliteral";


// ###########################################################################################################################################
// # Wifi scan function to help you to setup your WiFi connection
// ###########################################################################################################################################
String ScanWiFi() {
  String html = config_html;
  Serial.println("Scan WiFi networks - START");
  int n = WiFi.scanNetworks();
  Serial.println("WiFi scan done");
  Serial.println("Scan WiFi networks - END");
  Serial.println(" ");
  if (n > 0) {
    Serial.print(n);
    Serial.println(" WiFi networks found:");
    Serial.println(" ");
    String ssidList = "<p><label for=\"mySSISelect\">Found these networks:</label><br /><select id=\"mySSIDSelect\" name=\"mySSIDSelect\"><option value=\"\" disabled selected>Choose...</option>";
    for (int i = 0; i < n; ++i) {
      ssidList += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>";
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "" : "*");
    }
    ssidList += "</select></p>";
    html.replace("<!-- Select element will be dynamically added here -->", ssidList);
  } else {
    Serial.println("No WiFi networks found");
  }
  return html;
}


// ###########################################################################################################################################
// # Captive Portal by AWSW to avoid the usage of the WiFi Manager library to have more control
// ###########################################################################################################################################
const char* PARAM_INPUT_1 = "mySSID";
const char* PARAM_INPUT_2 = "myPW";
const char* PARAM_INPUT_3 = "setlanguage";
const String captiveportalURL = "http://192.168.4.1";
void CaptivePortalSetup() {
  String htmlConfigContent = ScanWiFi();
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

  server.on("/start", HTTP_GET, [htmlConfigContent](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", htmlConfigContent.c_str());
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
      showtext('S', TextWait, c);
      showtext('E', TextWait, c);
      showtext('T', TextWait, c);
      showtext(' ', TextWait, c);
      showtext('W', TextWait, c);
      showtext('I', TextWait, c);
      showtext('F', TextWait, c);
      showtext('I', TextWait, c);
      showtext(' ', TextWait, c);
      CaptivePortalSetup();
      SetWLAN(strip.Color(0, 255, 255));
    } else {
      Serial.println("Try to connect to found WiFi configuration: ");
      WiFi.disconnect();
      int tryCount = 0;
      WiFi.mode(WIFI_STA);
      // WiFi.setHostname(hostname); // Potential ESP32 bug... Does not work anymore :(
      WiFi.begin((const char*)WIFIssid.c_str(), (const char*)WIFIpass.c_str());
      Serial.println("Connecting to WiFi: " + String(WIFIssid));
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
      Serial.println("GW: " + WiFi.gatewayIP().toString());
      Serial.println("ESP32 hostname: " + String(WiFi.getHostname()));
      SetWLAN(strip.Color(0, 255, 0));
      delay(1000);
      configNTPTime();      // NTP time setup
      setupWebInterface();  // Generate the configuration page
      handleLEDupdate();    // LED update via web
      setupOTAupate();      // ESP32 OTA update
      Serial.println("######################################################################");
      Serial.println("# Web interface online at: http://" + IpAddress2String(WiFi.localIP()));
      Serial.println("# Web interface online at: http://" + String(WiFi.getHostname()));
      Serial.println("# HTTP controls online at: http://" + IpAddress2String(WiFi.localIP()) + ":2023");
      Serial.println("# HTTP controls online at: http://" + String(WiFi.getHostname()) + ":2023");
      Serial.println("######################################################################");
      Serial.println("# WordClock startup finished...");
      Serial.println("######################################################################");
      if (useStartupText == 1) callStartText();  // Show "WordClock" startup text
      if (useshowip == 1) ShowIPaddress();       // Display the current IP-address
      Serial.println(" ");
      Serial.println(" ");
      Serial.println(" ");
      updatenow = true;  // Update the display 1x after startup
      update_display();  // Update LED display
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
      <h2>Please select the in the Arduino IDE > "Sketch" ><br/>"Export Compiled Binary (Alt+Ctrl+S)"<br/>to generate the required "Code.ino.bin" file.<br/><br/>
      Select the "Code.ino.bin" file with the "Search" button.<br/><br/>
      Use the "Update" button to start the update.<br/><br/>WordClock will restart automatically.</h2><br/>
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
// # OFFLINE MODE Captive Portal by AWSW
// ###########################################################################################################################################
void OfflinePotalSetup() {
  if (debugtexts == 1) Serial.println("\nCreating WordClock Offline Mode access point...");
  WiFi.mode(WIFI_AP);
  delay(100);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  if (UseOfflineModeWithPassword == 1) {  // Access point with password or not
    WiFi.softAP(Offline_SSID, Offline_PW);
  } else {
    WiFi.softAP(Offline_SSID);
  }
  Serial.println("##############################################################################################################################################################################################################");
  Serial.print("# Offline Mode WiFi access point initialized. Please connect to the WiFi access point and set the current time now. Access point name: '");
  Serial.print(Offline_SSID);
  if (UseOfflineModeWithPassword == 1) {
    Serial.print("' using the password: '");
    Serial.print(Offline_PW);
  }
  Serial.println("'");
  Serial.print("# In case your browser does not open the WordClock configuration page automatically after connecting to the access point, please navigate to this URL manually: http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("##############################################################################################################################################################################################################");

  setupWebInterface();  // Generate the configuration page

  server.on("/connecttest.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /connecttest.txt");
  });
  server.on("msftconnecttest.com", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served msftconnecttest.com");
  });
  server.on("/fwlink", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /fwlink");
  });
  server.on("/wpad.dat", [](AsyncWebServerRequest* request) {
    // request->send(404);
    request->redirect(captiveportalURL);
    Serial.println("Served wpad.dat");
  });
  server.on("/generate_204", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /gernerate_204");
  });
  server.on("/redirect", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /redirect");
  });
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /hotspot-detect.html");
  });
  server.on("/canonical.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /cannonical.html");
  });
  server.on("/success.txt", [](AsyncWebServerRequest* request) {
    // request->send(200);
    request->redirect(captiveportalURL);
    Serial.println("Served /success.txt");
  });
  server.on("/ncsi.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /ncsi.txt");
  });
  server.on("/chrome-variations/seed", [](AsyncWebServerRequest* request) {
    // request->send(200);
    request->redirect(captiveportalURL);
    Serial.println("Served /chrome-variations/seed");
  });
  server.on("/service/update2/json", [](AsyncWebServerRequest* request) {
    // request->send(200);
    request->redirect(captiveportalURL);
    Serial.println("Served /service/update2/json");
  });
  server.on("/chat", [](AsyncWebServerRequest* request) {
    // request->send(404);
    request->redirect(captiveportalURL);
    Serial.println("Served /chat");
  });
  server.on("/startpage", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.println("Served /startpage");
  });
  server.on("/favicon.ico", [](AsyncWebServerRequest* request) {
    // request->send(404);
    request->redirect(captiveportalURL);
    Serial.println("Served /favicon.ico");
  });


  server.on("/", HTTP_ANY, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/html", index_html);
    // AsyncWebServerResponse* response = request->beginResponse(200, "text/html", index_offline_html);
    response->addHeader("Cache-Control", "public,max-age=31536000");
    request->send(response);
    // request->redirect(captiveportalURL);
    Serial.println("Served Basic HTML Page");
  });

  server.onNotFound([](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.print("Web page not found: ");
    Serial.print(request->host());
    Serial.print(" ");
    Serial.print(request->url());
    Serial.print(" sent redirect to " + captiveportalURL + "\n");
  });

  server.begin();
  if (debugtexts == 1) Serial.println("WordClock OFFLINE MODE captive portal web server started");
  ShowOfflineIPaddress();  // Display the current Offline Mode IP-address every time on startup
}


// ###########################################################################################################################################
// # Show the Offline Mode IP-address on the display:
// ###########################################################################################################################################
void ShowOfflineIPaddress() {
  if (useshowip == 1) {
    // Serial.println("Show current IP-address on the display: " + IpAddress2String(WiFi.softAPIP()));
    int ipdelay = 2000;

    // Testing the digits:
    // for (int i = 0; i < 10; i++) {
    //   back_color();
    //   // numbers(i, 3);
    //   // numbers(i, 2);
    //   numbers(i, 1);
    //   strip.show();
    //   delay(ipdelay);
    // }

    // Octet 1:
    ClearDisplay();
    numbers(getDigit(int(WiFi.softAPIP()[0]), 2), 3);
    numbers(getDigit(int(WiFi.softAPIP()[0]), 1), 2);
    numbers(getDigit(int(WiFi.softAPIP()[0]), 0), 1);
    setLED(191, 191, 1);
    setLED(0, 3, 1);
    setLED(240, 243, 1);
    strip.show();
    delay(ipdelay);

    // // Octet 2:
    ClearDisplay();
    numbers(getDigit(int(WiFi.softAPIP()[1]), 2), 3);
    numbers(getDigit(int(WiFi.softAPIP()[1]), 1), 2);
    numbers(getDigit(int(WiFi.softAPIP()[1]), 0), 1);
    setLED(191, 191, 1);
    setLED(0, 7, 1);
    setLED(240, 247, 1);
    strip.show();
    delay(ipdelay);

    // // Octet 3:
    ClearDisplay();
    numbers(getDigit(int(WiFi.softAPIP()[2]), 2), 3);
    numbers(getDigit(int(WiFi.softAPIP()[2]), 1), 2);
    numbers(getDigit(int(WiFi.softAPIP()[2]), 0), 1);
    setLED(191, 191, 1);
    setLED(0, 11, 1);
    setLED(240, 251, 1);
    strip.show();
    delay(ipdelay);

    // // Octet 4:
    ClearDisplay();
    numbers(getDigit(int(WiFi.softAPIP()[3]), 2), 3);
    numbers(getDigit(int(WiFi.softAPIP()[3]), 1), 2);
    numbers(getDigit(int(WiFi.softAPIP()[3]), 0), 1);
    setLED(0, 15, 1);
    setLED(240, 255, 1);
    strip.show();
    delay(ipdelay);
  }
}



// ###########################################################################################################################################
// # Split String into an Array (DayNightModeFunction)
// ###########################################################################################################################################
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


// ###########################################################################################################################################
// # Text output function: (modified and original by Dreso)
// ###########################################################################################################################################
// # Matrix treated with coordinates from 1,1 (top,left) to 16,16 (bottom, right)
// # each element with two Leds aligned above each other  (16x16 Matrix treated as 8x16 Matrix)
// #  __________________       ______________
// # | 1,1 | 1,2 | 1,3 | .... | 1,15 | 1,16 |
// # |-----|-----|-----|       -------------|
// # | 2,1 | 2,2 |...................| 2,16 |
// # |-----|-----|-----|       -------------|
// # | 3,1 |-----|
// # |-----|-----|-----|       -------------|
// # |...................
// # |-----|-----|-----|       -------------|
// # | 8,1 | 8,2 | 8,3 | ...  | 8,15 | 8,16 |
// # |_____|_____|______     _|______|______|
#define MAXROWS 8
#define MAXCOLUMS 16


// ##############################################################################################
// #Duocolor Pixel
// ##############################################################################################
void DrawPixelX(int row, int colum, uint32_t c1, uint32_t c2) {
  if (row > 0) row--;                                                      // internal index begins with 0 and external with 1
  if (colum > 0) colum--;                                                  // internal index begins with 0
  strip.setPixelColor(row * 2 * MAXCOLUMS + (MAXCOLUMS - 1 - colum), c1);  // upper led
  strip.setPixelColor(row * 2 * MAXCOLUMS + (MAXCOLUMS + colum), c2);      // lower led // (2023 WordClock)
}


// ##############################################################################################
// # Monocolor Line Drawing
// ##############################################################################################
void DrawLine(int startrow, int startcolum, int endrow, int endcolum, uint32_t c) {
  DrawLineX(startrow, startcolum, endrow, endcolum, c, c);  //same color for both Leds
}


// ##############################################################################################
// # Monocolor Pixel
// ##############################################################################################
void DrawPixel(int row, int colum, uint32_t c) {
  DrawPixelX(row, colum, c, c);  //same color for both Leds
}


// ##############################################################################################
// # Draw Line with possible different colours on the two leds per element
// #
// # Diagonal Lines are also possible
// # Boundaries are not checked in this routine
// # Endpoints can also be smaller in number than the start
// ##############################################################################################
void DrawLineX(int startrow, int startcolum, int endrow, int endcolum, uint32_t c1, uint32_t c2) {
  int dx, dy;
  if (endcolum > startcolum) {
    dx = 1;
  } else {
    if (endcolum < startcolum) {
      dx = -1;
    } else {
      dx = 0;
    }
  }
  if (endrow > startrow) {
    dy = 1;
  } else {
    if (endrow < startrow) {
      dy = -1;
    } else {
      dy = 0;
    }
  }

  int actrow = startrow, actcolum = startcolum;
  bool bDone = false;
  while (!bDone) {
    DrawPixelX(actrow, actcolum, c1, c2);
    if (dx == 0 && dy == 0) {
      bDone = true;
    }
    actrow += dy;
    actcolum += dx;
    if (actrow == endrow) {
      dy = 0;
    }
    if (actcolum == endcolum) {
      dx = 0;
    }
  }
}


// ###########################################################################################################################################
// # Text output function:
// ###########################################################################################################################################
void showtext(char letter, int wait, uint32_t c) {
  ClearDisplay();
  uint32_t c2;
  letter = toupper(letter);  // only uppercase possible at the moment

  if (letter == 'A') {
    DrawLine(3, 6, 6, 6, c);
    DrawLine(3, 11, 6, 11, c);
    DrawLine(2, 7, 2, 10, c);
    DrawLine(4, 6, 4, 11, c);
  }
  if (letter == 'B') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(2, 7, 2, 10, c);
    DrawPixel(3, 11, c);
    DrawLine(4, 7, 4, 10, c);
    DrawPixel(5, 11, c);
    DrawLine(6, 7, 6, 10, c);
  }
  if (letter == 'C') {
    DrawLine(2, 7, 2, 11, c);
    DrawLine(3, 6, 5, 6, c);
    DrawLine(6, 7, 6, 11, c);
  }
  if (letter == 'D') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(3, 11, 5, 11, c);
    DrawLine(2, 7, 2, 10, c);
    DrawLine(6, 7, 6, 10, c);
  }
  if (letter == 'E') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(2, 6, 2, 11, c);
    DrawLine(4, 6, 4, 10, c);
    DrawLine(6, 6, 6, 11, c);
  }
  if (letter == 'F') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(2, 7, 2, 11, c);
    DrawLine(4, 7, 4, 10, c);
  }
  if (letter == 'G') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(4, 11, 6, 11, c);
    DrawLine(2, 6, 2, 11, c);
    DrawLine(4, 9, 4, 11, c);
    DrawLine(6, 6, 6, 11, c);
  }
  if (letter == 'H') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(2, 11, 6, 11, c);
    DrawLine(4, 6, 4, 11, c);
  }
  if (letter == 'I') {
    DrawLine(2, 7, 2, 9, c);
    DrawLine(3, 8, 5, 8, c);
    DrawLine(6, 7, 6, 9, c);
  }
  if (letter == 'J') {
    DrawLine(2, 6, 2, 10, c);
    DrawLine(3, 10, 5, 10, c);
    DrawLine(6, 7, 6, 9, c);
    DrawPixel(5, 6, c);
  }
  if (letter == 'K') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(4, 7, 4, 9, c);
    DrawLine(4, 9, 2, 11, c);
    DrawLine(4, 9, 6, 11, c);
  }
  if (letter == 'L') {
    //"L"
    DrawLine(2, 6, 6, 6, c);
    DrawLine(6, 6, 6, 11, c);
  }
  if (letter == 'M') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(2, 11, 6, 11, c);
    DrawPixel(3, 7, c);
    DrawLine(4, 8, 4, 9, c);
    DrawLine(3, 10, 3, 10, c);
  }
  if (letter == 'N') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(2, 11, 6, 11, c);
    DrawLine(3, 7, 6, 10, c);
  }
  if (letter == 'O') {
    DrawLine(3, 6, 5, 6, c);
    DrawLine(3, 11, 5, 11, c);
    DrawLine(2, 7, 2, 10, c);
    DrawLine(6, 7, 6, 10, c);
  }
  if (letter == 'P') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(3, 11, 3, 11, c);
    DrawLine(2, 6, 2, 10, c);
    DrawLine(4, 6, 4, 10, c);
  }
  if (letter == 'Q') {
    DrawLine(3, 6, 5, 6, c);
    DrawLine(3, 11, 5, 11, c);
    DrawLine(2, 7, 2, 10, c);
    DrawLine(6, 7, 6, 10, c);
    DrawLine(5, 10, 6, 11, c);
  }
  if (letter == 'R') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(3, 11, 4, 11, c);
    DrawLine(2, 6, 2, 10, c);
    DrawLine(4, 6, 4, 11, c);
    DrawLine(5, 10, 6, 11, c);
  }
  if (letter == 'S') {
    DrawLine(3, 6, 3, 6, c);
    DrawLine(5, 11, 5, 11, c);
    DrawLine(2, 7, 2, 11, c);
    DrawLine(4, 7, 4, 10, c);
    DrawLine(6, 6, 6, 10, c);
  }
  if (letter == 'T') {
    DrawLine(2, 6, 2, 10, c);
    DrawLine(3, 8, 6, 8, c);
  }
  if (letter == 'U') {
    DrawLine(2, 6, 5, 6, c);
    DrawLine(2, 11, 5, 11, c);
    DrawLine(6, 7, 6, 10, c);
  }
  if (letter == 'V') {
    DrawLine(2, 6, 3, 6, c);
    DrawLine(2, 11, 3, 11, c);
    DrawLine(4, 6, 6, 8, c);
    DrawLine(6, 9, 4, 11, c);
  }
  if (letter == 'W') {
    DrawLine(2, 6, 6, 6, c);
    DrawLine(2, 11, 6, 11, c);
    DrawLine(5, 7, 4, 8, c);
    DrawLine(4, 9, 5, 10, c);
  }
  if (letter == 'X') {
    DrawLine(2, 6, 4, 8, c);
    DrawLine(4, 9, 2, 11, c);
    DrawLine(6, 6, 4, 8, c);
    DrawLine(4, 9, 6, 11, c);
  }
  if (letter == 'Y') {
    DrawLine(2, 6, 4, 8, c);
    DrawLine(4, 8, 2, 10, c);
    DrawLine(5, 8, 6, 8, c);
  }
  if (letter == 'Z') {
    DrawLine(2, 6, 2, 10, c);
    DrawLine(3, 9, 5, 7, c);
    DrawLine(6, 6, 6, 10, c);
  }

  if (letter == ' ') {
    // already cleareed at top
  }
  strip.show();
  delay(wait);
}


// ###########################################################################################################################################
// # EOF - You have successfully reached the end of the code - well done ;-)
// ###########################################################################################################################################