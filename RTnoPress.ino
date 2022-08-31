#include <UART.h>
#include <RTno.h>
#include <Servo.h>
#include "HX711_verKi.hpp"

#define DT 8
#define SCK 9
HX711_verKi lc(DT, SCK);

#define lim 2
#define solPin 3
Servo sol;
volatile bool pinDownFlag = LOW;
#define PIN_UP 90
#define PIN_DOWN 10
#define PIN_RESET 160

//////////////////////////////////////////
// This function is called at first
//////////////////////////////////////////
void rtcconf(config_str& conf, exec_cxt_str& exec_cxt) {
  conf._default.connection_type = ConnectionTypeSerial1;
  conf._default.baudrate = 57600;
  exec_cxt.periodic.type = ProxySynchronousExecutionContext;
}

TimedBoolean m_BLTon;
InPort<TimedBoolean> m_BLTonIn("BLTon", m_BLTon);
TimedBoolean m_reoffset;
InPort<TimedBoolean> m_reoffsetIn("reoffset", m_reoffset);


TimedBoolean m_touch;
OutPort<TimedBoolean> m_touchOut("touch", m_touch);
TimedDouble m_load;
OutPort<TimedDouble> m_loadOut("load", m_load);

//////////////////////////////////////////
// on_initialize
//////////////////////////////////////////
int RTno::onInitialize() {
  addInPort(m_BLTonIn);
  addInPort(m_reoffsetIn);
  addOutPort(m_touchOut);
  addOutPort(m_loadOut);

  return RTC_OK;
}

////////////////////////////////////////////
// on_activated
////////////////////////////////////////////
int RTno::onActivated() {
  attachInterrupt(digitalPinToInterrupt(lim), Z_LimitChanged, CHANGE);
  sol.attach(solPin);
  delay(5000);
  lc.setOffset(50);
  return RTC_OK;
}

/////////////////////////////////////////////
// on_deactfivated
/////////////////////////////////////////////
int RTno::onDeactivated()
{

  return RTC_OK;
}

//////////////////////////////////////////////
// on_execute
//////////////////////////////////////////////
int RTno::onExecute() {
  m_load.data = lc.readDataAve(1); // 単位[g]
  m_loadOut.write();
  //delay(1);

  if (m_BLTonIn.isNew()) {
    m_BLTonIn.read();
    if (m_BLTon.data) {
      sol.write(PIN_DOWN);
      pinDownFlag = true;
      m_BLTon.data = false;
    }
  }

  if (m_reoffsetIn.isNew()) {
    m_reoffsetIn.read();
    if(m_reoffset.data == true) lc.setOffset(25);
  }
  return RTC_OK;
}


//////////////////////////////////////
// on_error
///////////////////////////////////////
int RTno::onError()
{
  return RTC_OK;
}

////////////////////////////////////////
// on_reset
///////////////////////////////////////
int RTno::onReset()
{
  return RTC_OK;
}



////////////////////////////////////////
// myFunction
///////////////////////////////////////
//Zmin変化時割り込み処理
void Z_LimitChanged()
{
  if (digitalRead(lim)) {
    if (pinDownFlag) {
      sol.write(PIN_UP);
      pinDownFlag = false;
      m_touch.data = true;
      m_touchOut.write();
    }
  }
}
