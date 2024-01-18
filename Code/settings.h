// ###########################################################################################################################################
// #
// # WordClock code for the 3 printables.com WordClock 16x16 matrix projects:
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
// # Compatible with WordClock version: V3.2.0
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
// # LED language layout default: !!! SET YOUR DEFAULT LANGUAGE HERE !!!
// ###########################################################################################################################################
int langLEDlayout_default = 0;  // Can be selected in the configuraration portal later too. This is just the default for the 1st startup
// Available languages:
// ####################
// 0 = DE     - GERMAN
// 1 = EN     - ENGLISH
// 2 = NL     - DUTCH
// 3 = SWE    - SWEDISH
// 4 = IT     - ITALIAN
// 5 = FR     - FRENCH
// 6 = GSW    - SWISS GERMAN
// 7 = CN     - CHINESE
// 8 = SCHWB  - SWABIAN GERMAN
// 9 = BAY    - BAVARIAN
// 10 = LTZ   - LUXEMBURGISH
// NOTE: You may need to use the "Reset WordClock settings"-button to update the value on the device


// ###########################################################################################################################################
// # Hardware settings:
// ###########################################################################################################################################
#define LEDPIN 32                                                                      // ESP32 pin connected to the NeoPixels matrix
#define NUMPIXELS 256                                                                  // How many NeoPixels are attached to the Arduino
#define ROWPIXELS 16                                                                   // How many NeoPixels per row
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);  // Matrix specification


// ###########################################################################################################################################
// # LED defaults:
// ###########################################################################################################################################
int redVal_back_default = 0;              // Default background color RED
int greenVal_back_default = 255;          // Default background color GREEN
int blueVal_back_default = 255;           // Default background color BLUE
int redVal_time_default = 128;            // Default time color RED
int greenVal_time_default = 0;            // Default time color GREEN
int blueVal_time_default = 0;             // Default time color BLUE
int intensity_day_default = 15;           // LED intensity (0..255) in day mode   - Important note: Check power consumption and used power supply capabilities!
int intensity_night_default = 5;          // LED intensity (0..255) in day mode   - Important note: Check power consumption and used power supply capabilities!
int usenightmode_default = 1;             // Use the night mode to reduce LED intensity during set times
String day_time_start_default = "06:15";  // Define day mode start --> time before is then night mode if used
String day_time_stop_default = "22:14";   // Define day mode end --> time after is then night mode if used


// ###########################################################################################################################################
// # Various default settings:
// ###########################################################################################################################################
int useshowip_default = 1;       // Show the current ip at boot
int usesinglemin_default = 1;    // Show the single minutes
int RandomColor_default = 0;     // Change text color every minute in random color
int useStartupText_default = 1;  // Show the "WordClock" text at boot
int maxWiFiconnctiontries = 30;  // Maximum connection tries to logon to the set WiFi. After the amount of tries is reached the WiFi settings will be deleted!
int iHourOffset_default = 1;     // Offset of hours in Offline Mode


// ###########################################################################################################################################
// # Variables declaration in Online Mode:
// ###########################################################################################################################################
#define DEFAULT_AP_NAME "WordClock"  // WiFi access point name of the ESP32


// ###########################################################################################################################################
// # Variables declaration in Offline Mode:
// ###########################################################################################################################################
// NOTE: When you try this setting "UseOfflineModeWithPassword" you may need to remove the known WiFi entries to see the change after some
// seconds and meanwhile disabling WiFi on your device. Some operating systems seem to cache the settings, so you might not see the change first
int UseOfflineModeWithPassword = 1;                      // Choose to open the interal WiFi access point with (1) or without (0) password protection
const char* Offline_SSID = "WordClock in Offline Mode";  // SSID of the internal WiFi access point used in Offline Mode
const char* Offline_PW = "Matrix-16x16";                 // Access point with password protection. Minimum 8 characters needed!
int showOMhints_default = 0;                             // Show or hide the Operation Mode hints on the page


// ###########################################################################################################################################
// # Special NTP time server and time zone settings:
// ###########################################################################################################################################
// Time server:
String NTPserver_default = "pool.ntp.org";  // Here you can set your own specific time server address if needed. Many others can be selected in the configuraration portal
// Choose the closest one to you here: https://gist.github.com/mutin-sa/eea1c396b1e610a2da1e5550d94b0453
// PS: The closest NTP time server to you might be your local router which can be selected in the configuraration portal too =)
//
// Time zone:
String Timezone_default = "CET-1CEST,M3.5.0,M10.5.0/3";  // Here you can set your own specific time zone if needed. Many others can be selected in the configuraration portal
// Please send me a message with your time zone settings to add them to this list in future releases. Thanks in advance! =)
// You can check a list of timezone string variables here:  https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv


// ###########################################################################################################################################
// # Test functions:
// ###########################################################################################################################################
int debugtexts = 0;          // Show more debug information in serial monitor
int ShowTimeEachSecond = 0;  // Show current time each second in serial monitor
int testTime = 0;            // LED time text output test
int testPrintTimeTexts = 0;  // Print the time texts shown on the display in the serial monitor (Currently DE + IT only)
// Test a special time Online Mode:
int testspecialtimeON = 0;  // Use test a special time function
int test_hourON = 23;       // Test a special time: hour
int test_minuteON = 38;     // Test a special time: minute
int test_secondON = 45;     // Test a special time: second
// Test a special time Offline Mode:
int testspecialtimeOFF = 0;  // Use test a special time function
int test_hourOFF = 0;        // Test a special time: hour
int test_minuteOFF = 38;     // Test a special time: minute
int test_secondOFF = 45;     // Test a special time: second


// ###########################################################################################################################################
// #
// # !!! DANGER ZONE: DO NOT PLAY WITH THIS VALUE IF YOU ARE NOT AWARE WHAT YOU ARE DOING - SERIOSLY !!!
// #
// # !!! PLEASE READ THIS CAREFULLY TO AVOID DAMAGE TO THE ESP32 AND THE OTHER COMPONENTS !!! YOU HAVE BEEN WARNED !!!
// #
// ###########################################################################################################################################
// # LED intensity setting:
// ###########################################################################################################################################
#define LEDintensityLIMIT 50  // of 255
                              /* Do NOT change this if you don't know how much power drain this may cause to avoid damage !!! 
                              Limit the intensity level to be able to select in the configuration to avoid to much power drain and to avoid hardware failures over time. 
                              In case you want to use the device over longer times with a higher intensity or even higher, you will need to consider a much more powerful 
                              power supply, a better cable to connect the device and you have to power the matrix directly to 5V of the power supply, not over the ESP32! 
                              Otherwise the components fail directly or over time. Therefore this setting is limited and should not be raised. 
                              In case you really need a higher value, you need to make sure to power the matrix directly so that the power is not served over the ESP32 
                              to the matrix to avoid its damage over time or directly. 
                              Specifications for these matrix say that "60mA (per pixel at full brightness" (white color) is used. Multiple this by 256 and it takes 15.36A 
                              Calculation: 16x16 = 256 LEDs >>> 256x60mA = 15.36A (without the rest of the electronics = ESP32) >>> ~16A * 5V = ~80W power drain... 
                              Think of the heat emited by the LEDs then too... 
                              Conclusion: 
                              - You should not go higher here. 
                              - Measuremets showed that with the set intensity limit of 50 the 5V/3A power supply is enough to use with THIS software. That does not mean that this 
                                device will not fail, if you try to use it with other software "to shortly test other LED software" to see the result. Keep this in mind too. 
                              - Please think about if you really need such a high intensity value. The WordClock's used from me run at a maximum intensity of 22 which is 
                                really bright enoigh in my eyes and all levels above 48 i could not see really an advantage anymore that the display is better able to view... 
                              - Also useing white background color does not seem to be a good idea because to white set LEDs take the most power... 
                              - Make sure to use propper components and settings! 
                              - As allways: You are using this device at your own risk! */


// ###########################################################################################################################################
// # EOF - You have successfully reached the end of the code - well done ;-)
// ###########################################################################################################################################