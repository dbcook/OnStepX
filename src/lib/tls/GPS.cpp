// -----------------------------------------------------------------------------------
// Time/Location source GPS support
// uses the specified serial port

#include "GPS.h"

#if defined(TIME_LOCATION_SOURCE) && TIME_LOCATION_SOURCE == GPS

#ifdef TLS_TIMELIB
  #include <TimeLib.h> // https://github.com/PaulStoffregen/Time/archive/master.zip
#endif

#ifndef SERIAL_GPS
  #error "SERIAL_GPS must be set to the serial port object if TIME_LOCATION_SOURCE GPS is used"
#endif

#ifndef SERIAL_GPS_BAUD
  #error "SERIAL_GPS_BAUD must be set to the baud rate if TIME_LOCATION_SOURCE GPS is used"
#endif

#if SERIAL_GPS == SoftSerial || SERIAL_GPS == HardSerial
  #ifndef SERIAL_GPS_RX
    #error "SERIAL_GPS_RX must be set to the serial port RX pin if SoftSerial or HardSerial is used"
  #endif
  #ifndef SERIAL_GPS_TX
    #error "SERIAL_GPS_TX must be set to the serial port TX pin if SoftSerial or HardSerial is used"
  #endif
#endif

#include "PPS.h"
#include "../tasks/OnTask.h"

#include <TinyGPS++.h> // http://arduiniana.org/libraries/tinygpsplus/
TinyGPSPlus gps;

// provide for using software serial
#if SERIAL_GPS == SoftSerial
  #include <SoftwareSerial.h>
  #undef SERIAL_GPS
  SoftwareSerial SWSerialGPS(SERIAL_GPS_RX, SERIAL_GPS_TX);
  #define SERIAL_GPS SWSerialGPS
  #define SERIAL_GPS_RXTX_SET
#endif

// provide for using hardware serial
#if SERIAL_GPS == HardSerial
  #include <HardwareSerial.h>
  #undef SERIAL_GPS
  HardwareSerial HWSerialGPS(SERIAL_GPS_RX, SERIAL_GPS_TX);
  #define SERIAL_GPS HWSerialGPS
  #define SERIAL_GPS_RXTX_SET
#endif

void gpsPoll() {
  #if TIME_LOCATION_PPS_SENSE != OFF
    if (pps.synced) {
  #endif

  if (!tls.isReady()) tls.poll();

  #if TIME_LOCATION_PPS_SENSE != OFF
    }
  #endif
}

// initialize
bool TimeLocationSource::init() {
  #if defined(SERIAL_GPS_RX) && defined(SERIAL_GPS_TX) && !defined(SERIAL_GPS_RXTX_SET)
    SERIAL_GPS.begin(SERIAL_GPS_BAUD, SERIAL_8N1, SERIAL_GPS_RX, SERIAL_GPS_TX);
  #else
    SERIAL_GPS.begin(SERIAL_GPS_BAUD);
  #endif

  // check to see if the GPS is present
  // *** dbc: I changed the logic here to fix somee problems.
  //     I was seeing "Site, falling back to Date/Time from NV" before the sentences ever start 
  //     because Site.init() expects the date/time to be available by the time it's called.
  // 
  //  We need a new strategy here.
  //    If enabled, the GPS TLS should alway run and make maximum effort to provide accurate position and time.
  //    Time and location are handled separately.
  //    Priority of time setting:
  //      0) If no date/time source available, allow basic tracking but disallow GOTO ops
  //      1) Last known date/time from NV.  Time from NV is only useful for GOTO on a quick reboot of OnStep.
  //      2) battery backed RTC date/time (onboard or outboard, sets system clock at boot)
  //      3) Manually entered date/time (independent fields, sets system clock when entered via UI)
  //      4) system clock
  //      5) NTP time source (runtime enable/disable, valid NTP time defeats all lower priority sources if enabled)
  //      6) GPS time source (runtime enable/disable, valid GPS sentence defeats all lower priority sources if enabled)
  //      7) GPS time source with PPS (runtime enable/disable, valid GPS+PPS defeats all lower priority sources if enabled)
  //    Priority of location setting:  (look at altitude handling!)
  //      0) If no location source available, allow basic tracking but disallow GOTO ops.
  //      1) Last known location from NV (loaded at boot)
  //      2) Manually entered location (active until overwritten by enabled GPS source)
  //      3) GPS location source (runtime enable/disable, valid GPS sentence always wins if enabled)
  //   Notes:
  //      If GPS location and time are available, there is probably no reason ever to not use them.  Typical GPS
  //      accuracy of a few meters is a fraction of an arcsec (28m == 1 arcsec)
  //      Continuous update of GPS location and time (with appropriate smoothing) eliminates issues
  //      with local clock drift when OnStep is left running for a long time.  It's also a small step
  //      toward tracking from mobile platforms.
  //      If you want manual overrides to have effect, you need to turn off NTP and GPS sources.
  // ***

  // Wait a reasonable time until we see GPS input chars
  unsigned long timeout = millis() + 5000UL;
  while(SERIAL_GPS.available() <= 0) {
    if ((long)(millis() - timeout) > 0) {
      VLF("WRN: TLS, GPS serial RX interface is quiet!");
      return false;
    }
    tasks.yield(50);
  }
  // collect a whole GPS sentence
  timeout = millis() + 5000UL;
  while (SERIAL_GPS.available() > 0) {
    if (gps.encode(SERIAL_GPS.read())) break;
    if ((long)(millis() - timeout) > 0) {
      VLF("WRN: TLS, GPS serial RX interface no NMEA sentences detected!");
      return false;
    }
    Y;
  }
  // Launch the GPS poller to monitor the GPS messages.  It waits for a fix with suitable HDOP
  // and sets the ready flag when it sees one.
  // Site init will load time/loc from NV and start a longish (10min) but not infinite timer to
  // receive a GPS fix via the 'gpsChk' task.
  VF("MSG: TLS, GPS start monitor task (rate 10ms priority 7)... ");
  if (tasks.add(1, 0, true, 7, gpsPoll, "gpsPoll")) { VLF("success"); active = true; } else { VLF("FAILED!"); }

  return active;
}

void TimeLocationSource::set(JulianDate ut1) {
  ut1 = ut1;
}

void TimeLocationSource::set(int year, int month, int day, int hour, int minute, int second) {
  #ifdef TLS_TIMELIB
    setTime(hour, minute, second, day, month, year);
  #else
    (void)year; (void)month; (void)day; (void)hour; (void)minute; (void)second;
  #endif
}

void TimeLocationSource::get(JulianDate &ut1) {
  if (!ready) return;
  if (!timeIsValid()) return;

  GregorianDate greg; greg.year = gps.date.year(); greg.month = gps.date.month(); greg.day = gps.date.day();
  ut1 = calendars.gregorianToJulianDay(greg);
  // DUT1 = UT1 − UTC
  // UT1 = DUT1 + UTC
  ut1.hour = gps.time.hour() + gps.time.minute()/60.0 + (gps.time.second() + DUT1)/3600.0;

  // adjust date/time for DUT1 as needed
  if (ut1.hour >= 24.0L) { ut1.hour -= 24.0L; ut1.day += 1.0L; } else
  if (ut1.hour < 0.0L) { ut1.hour += 24.0L; ut1.day -= 1.0L; }
}

void TimeLocationSource::getSite(double &latitude, double &longitude, float &elevation) {
  if (!ready) return;
  if (!siteIsValid()) return;

  latitude = gps.location.lat();
  longitude = -gps.location.lng();
  elevation = gps.altitude.meters();
}

void TimeLocationSource::poll() {
  while (SERIAL_GPS.available() > 0) {
    int c = SERIAL_GPS.read();
    gps.encode(c);
    //Serial.printf("%c", c);
  }

  if (gps.location.isValid() && siteIsValid()) {
    if (gps.date.isValid() && gps.time.isValid() && timeIsValid() && gps.hdop.isValid() && (gps.hdop.hdop() < GPS_HDOP_UPPER_LIMIT)) {
      if (waitIsValid()) {  // nerfed, no hard wait anymore, we look at HDOP right away
        VLF("MSG: TLS, GPS date/time/location is ready");
        VLF("MSG: GPS HDOP = "); VL(gps.hdop.hdop());

        VLF("MSG: TLS, closing GPS serial port");
        SERIAL_GPS.end();

        VLF("MSG: TLS, stopping GPS monitor task");
        tasks.setDurationComplete(tasks.getHandleByName("gpsPoll"));

        #ifdef TLS_TIMELIB
          setTime(gps.time.hour(), gps.time.minute(), gps.time.second(), gps.date.day(), gps.date.month(), gps.date.year());
        #endif

        ready = true;
      }
    }
  }
}

// starts keeping track of the wait once (PPS is synced, if applicable) and GPS has a lock 
bool TimeLocationSource::waitIsValid() {
  return true; // nerf the hard wait
  if (startTime == 0) startTime = millis();
  unsigned long t = millis() - startTime;
  return (t/1000UL)/60UL >= GPS_MIN_WAIT_MINUTES;
}

bool TimeLocationSource::timeIsValid() {
  if (gps.date.year() <= 3000 && gps.date.month() >= 1 && gps.date.month() <= 12 && gps.date.day() >= 1 && gps.date.day() <= 31 &&
      gps.time.hour() <= 23 && gps.time.minute() <= 59 && gps.time.second() <= 59) return true; else return false;
}

bool TimeLocationSource::siteIsValid() {
  if (gps.location.lat() >= -90 && gps.location.lat() <= 90 && gps.location.lng() >= -360 && gps.location.lng() <= 360) return true; else return false;
}

TimeLocationSource tls;

#endif
