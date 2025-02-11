// GuideRateRheostat plugin (experimental!)

#include "GuideRateRheostat.h"
#include "../../Common.h"
#include "../../lib/serial/Serial_Local.h"
#include "../../lib/tasks/OnTask.h"
#include "../../lib/convert/Convert.h"
#include "../../telescope/mount/goto/Goto.h"

void rheostatWrapper() { guideRateRheostat.loop(); }

void GuideRateRheostat::init() {
  VLF("MSG: Plugins, starting: GuideRateRheostat");

  // start a task that runs periodically (I chose 200 msec) at priority level 7
  tasks.add(200, 0, true, 7, rheostatWrapper);
}

// The rate won't change while you're holding down a paddle button.
// Adjustments needed so we can hold a direction button and adjust the rate
// instead of having a large change take effect for the next button press,
// which could be unexpected.
void GuideRateRheostat::loop() {
  #if RHEOSTAT_PIN != OFF
    float voltage = (analogRead(RHEOSTAT_PIN)/1023.0F)*3.3F;
    float resistance;
    if (voltage >= 3.3F) {
      resistance = RHEOSTAT_R1; 
    } else {
      resistance = (voltage*RHEOSTAT_R1)/(3.3F - voltage);
    } 

    if (abs(resistance - lastResistance) > RHEOSTAT_R2*(RHEOSTAT_CHANGE_THRESHOLD/100.0F)) {
      char s[40];
      float slowestRate = (15.0F*RHEOSTAT_RATE_MINIMUM)/3600.0F;
      float fastestRate = radToDegF(goTo.rate)*RHEOSTAT_RATE_MAXIMUM;

      float rate = pow(resistance/RHEOSTAT_R2, RHEOSTAT_EXPONENTIAL)*fastestRate;

      if (rate < slowestRate) rate = slowestRate;
      if (rate > fastestRate) rate = fastestRate;

      sprintF(s, ":RA%1.3f#", rate);
      SERIAL_LOCAL.transmit(s);
      sprintF(s, ":RE%1.3f#", rate);
      SERIAL_LOCAL.transmit(s);
      lastResistance = resistance;
    }
  #endif
}

GuideRateRheostat guideRateRheostat;
