/* ---------------------------------------------------------------------------------------------------------------------------------
 * Extended configuration for OnStepX INFREQUENTLY USED options
 *
 *          For more information on setting OnStep up see http://www.stellarjourney.com/index.php?r=site/equipment_onstep 
 *                      and join the OnStep Groups.io at https://groups.io/g/onstep
 * 
 *           *** Read the compiler warnings and errors, they are there to help guard against invalid configurations ***
 *
 * ---------------------------------------------------------------------------------------------------------------------------------
 * ADJUST THE FOLLOWING TO CONFIGURE YOUR CONTROLLER FEATURES ----------------------------------------------------------------------
 * <-Req'd = always must set, <-Often = usually must set, Option = optional, Adjust = adjust as req'd, Infreq = infrequently changed
*/
//      Parameter Name              Value   Default  Notes                                                                      Hint

// =================================================================================================================================
// CONTROLLER ======================================================================================================================

// DEBUG ---------------------------------------------------------------------------------------------------------------------------
//   Debug log message configuration.
//       DEBUG controls general diagnostics message level.
//       DEBUG_xxx enables specific message sets.
//       SERIAL_DEBUG sets the port used for debug output.
//          On a Teensy4, Serial is a good choice since it goes back out the native USB interface and will not consume an actual serial port.
//          Since there is a command processor listening to port Serial, you can type commands into OnStepX from the debug monitor.
//          On ESP32, you need to choose a specific hardware port Serial1, Serial2, etc. that doesn't conflict with the one you use for the SWS or GPS.
//       SERIAL_DEBUG_BAUD sets the data rate.  Ignored when using native USB port "Serial" on Teensy4.
//       DEBUG_HARD_WAIT_FOR_MONITOR lets you force the system to wait for a connected serial monitor (e.g. from vscode) before startup.
// Note: The SERIAL_DEBUG port is often attached to the same SERIAL_A port so it cannot be used for normal
//       LX200 protocol communications.  If this is the case set both to the same (faster) baud rate specified below.
#define DEBUG                         VERBOSE //    OFF, Use ON for background error messages only, use VERBOSE for all           Infreq
                                          //         error and status messages, use CONSOLE for VT100 debug console,
                                          //         or use PROFILER for VT100 task profiler.
#define DEBUG_HARD_WAIT_FOR_MONITOR   OFF // Controls wait-for-debug-monitor connection behavior when DEBUG != OFF  
                                          // OFF: do not wait, n: wait n seconds, ON: wait forever for serial monitor connect.
#define DEBUG_SERVO                   OFF //    OFF, n. Where n=1 to 9 as the designated axis for logging servo activity.     Option
#define DEBUG_ECHO_COMMANDS           ON  //    OFF, Use ON or ERRORS_ONLY to log commands to the debug serial port.          Option
#define SERIAL_DEBUG               Serial // Serial (For Teensy this is USB serial, baud ignored), Use any available h/w serial port. Serial1 or Serial2, etc.              Option
#define SERIAL_DEBUG_BAUD          230400 // 230400, n. Where n=9600,19200,57600,115200,230400,460800 (common baud rates.)    Option
                                          // No effect when SERIAL_DEBUG = Serial (for Teensy USB serial monitor channel)

// NON-VOLATILE STORAGE ------------------------------------------------------------------------------------------------------------
#define NV_WIPE                       OFF //         OFF, Causes the defaults to be written back into NV (FLASH,EEPROM,etc.)  Infreq
                                          //              ***     IMPORTANT: ENABLE THIS OPTION THEN UPLOAD, WAIT A MINUTE    ***
// Warning --->                           //              ***     OR TWO THEN SET THIS OPTION TO OFF AND UPLOAD AGAIN.        ***
                                          //              ***     LEAVING THIS OPTION ENABLED CAN CAUSE EXCESSIVE NV          ***
                                          //              ***     WEAR AND DAMAGE THE MICROCONTROLLER NV SUBSYSTEM !!!        ***

// ESP32 VIRTUAL SERIAL BLUETOOTH AND IP COMMAND CHANNELS --------------------------------------------------------------------------
#define SERIAL_BT_MODE                OFF //    OFF, Use SLAVE to enable the interface (ESP32 only.)                          Option
#define SERIAL_BT_NAME          "OnStepX" //         "OnStepX", Bluetooth device name.                                        Adjust
#define SERIAL_IP_MODE                OFF //    OFF, WIFI_ACCESS_POINT or WIFI_STATION enables interface (ESP32 only.)        Option
#define WEB_SERVER                    OFF //    OFF, ON enables Webserver (for Website plugin)                                Option

// EXTERNAL GPIO SUPPORT -----------------------------------------------------------------------------------------------------------
#define GPIO_DEVICE                   OFF //    OFF, DS2413: for 2-ch or 4-ch using 1-wire gpio's (one or two devices.)       Option
                                          //         SWS: for 8-ch Serial gpio (normally 4 unused encoder pins.)
                                          //         MCP23008: for 8-ch I2C gpio.
                                          //         MCP23017, X9555, or X8575: for 16-ch I2C gpio.
                                          //         SSR74HC595: for up to 32-ch gpio (serial shift register, output only.)
                                          //         Works w/most OnStep features, channels assigned in order pin# 512 and up.

// ---------------------------------------------------------------------------------------------------------------------------------
