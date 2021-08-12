#include "Watchy_slides999.h"

#define TIME_FONT PebblecoNumbers_MONO32pt7b

RTC_DATA_ATTR char  step1, step2, step3, step4;
RTC_DATA_ATTR char  stepC1 = '0';
RTC_DATA_ATTR char  stepC2 = '0';
RTC_DATA_ATTR char  stepC3 = '0';
RTC_DATA_ATTR char  stepC4 = '0';
RTC_DATA_ATTR char  month1, month2, date1, date2;
RTC_DATA_ATTR char  hour1, hour2, min1, min2;
RTC_DATA_ATTR char  hourC1 = '0';
RTC_DATA_ATTR char  hourC2 = '0';
RTC_DATA_ATTR char  minC1 = '0';
RTC_DATA_ATTR char  minC2 = '0';
RTC_DATA_ATTR char  monthC1 = '0';
RTC_DATA_ATTR char  monthC2 = '0';
RTC_DATA_ATTR char  dateC1 = '0';
RTC_DATA_ATTR char  dateC2 = '0';
RTC_DATA_ATTR String stepCount;
RTC_DATA_ATTR bool runOnce = true;
RTC_DATA_ATTR bool charging = false;
RTC_DATA_ATTR bool chargeSync = false;
RTC_DATA_ATTR float oldVoltage;
RTC_DATA_ATTR float maxVoltage = 4.2;
RTC_DATA_ATTR float battCalculator;
float lowBattMargin = .9; //Leaves a ~.1 power reserve
char rNum1, rNum2, rNum3, rNum4, rNum5, rNum6, rNum7, rNum8;

WatchySlides999::WatchySlides999() {}

void WatchySlides999::getTime() {

  int Hour = currentTime.Hour;
  if (twelveMode == 0) {
    if (Hour > 12) {
      Hour = Hour - 12;
    } else if (Hour == 0) {
      Hour = 12;
    }
  }

  hour1 = String(Hour / 10).charAt(0);
  hour2 = String(Hour % 10).charAt(0);

  min1 = String(currentTime.Minute / 10).charAt(0);
  min2 = String(currentTime.Minute % 10).charAt(0);
}

void WatchySlides999::getDate() {
  //dateMode adjustment for non-yanks aka brit wanks
  if(dateMode == 0) {
    month1 = (currentTime.Month / 10 != 0) ? String(currentTime.Month / 10).charAt(0) : '0';
    month2 = String(currentTime.Month % 10).charAt(0);
    date1 = (currentTime.Day / 10 != 0) ? String(currentTime.Day / 10).charAt(0) : '0';
    date2 = String(currentTime.Day % 10).charAt(0);
  } else {
    date1 = (currentTime.Month / 10 != 0) ? String(currentTime.Month / 10).charAt(0) : '0';
    date2 = String(currentTime.Month % 10).charAt(0);
    month1 = (currentTime.Day / 10 != 0) ? String(currentTime.Day / 10).charAt(0) : '0';
    month2 = String(currentTime.Day % 10).charAt(0);
  }  
}

void WatchySlides999::getRand() {
  srand(currentTime.Minute * currentTime.Second * currentTime.Wday); //seeds rand() with the current minute causing watchy to display a new random animation once per minute
  String rNumber = String(random(10000000, 99999999));
  rNum1 = rNumber.charAt(0);
  rNum2 = rNumber.charAt(1);
  rNum3 = rNumber.charAt(2);
  rNum4 = rNumber.charAt(3);
  rNum5 = rNumber.charAt(4);
  rNum6 = rNumber.charAt(5);
  rNum7 = rNumber.charAt(6);
  rNum8 = rNumber.charAt(7);
    if (debugger){
      Serial.println("rNumber: " + String(rNumber));
      Serial.println("randomNum1: " + String(rNum1));
      Serial.println("randomNum2: " + String(rNum2));
      Serial.println("randomNum3: " + String(rNum3));
      Serial.println("randomNum4: " + String(rNum4));
      Serial.println("randomNum5: " + String(rNum5));
      Serial.println("randomNum6: " + String(rNum6));
      Serial.println("randomNum7: " + String(rNum7));
      Serial.println("randomNum8: " + String(rNum8));
    }
}

void WatchySlides999::getSteps() {

  if (currentTime.Hour == 0 && currentTime.Minute == 0)
    sensor.resetStepCounter();

  if (debugger)
    stepCount = 10667;
  else
    stepCount = sensor.getCounter();
  
  if (stepCount.length() == 5) {
    step1 = stepCount.charAt(0);
    step2 = stepCount.charAt(1);
    step3 = '.';
    step4 = stepCount.charAt(2);
  } else if (stepCount.length() == 4) {
    step1 = stepCount.charAt(0);
    step2 = stepCount.charAt(1);
    step3 = stepCount.charAt(2);
    step4 = stepCount.charAt(3);
  } else if (stepCount.length() == 3) {
    step1 = '0';
    step2 = stepCount.charAt(0);
    step3 = stepCount.charAt(1);
    step4 = stepCount.charAt(2);
  } else if (stepCount.length() == 2) {
    step1 = '0';
    step2 = '0';
    step3 = stepCount.charAt(0);
    step4 = stepCount.charAt(1);
  } else {
    step1 = '0';
    step2 = '0';
    step3 = '0';
    step4 = stepCount.charAt(0);
  }

  if (debugger) {
    Serial.println("stepCount: " + String(stepCount));
    Serial.println("step1: " + String(step1));
    Serial.println("step2: " + String(step2));
    Serial.println("step3: " + String(step3));
    Serial.println("step4: " + String(step4));
  }
}

void WatchySlides999::updateCache() {
  getSteps();
  getDate();

  monthC1 = month1;
  monthC2 = month2;
  dateC1 = date1;
  dateC2 = date2;
  hourC1 = hour1;
  hourC2 = hour2;
  minC1 = min1;
  minC2 = min2;
  stepC1 = step1;
  stepC2 = step2;
  stepC3 = step3;
  stepC4 = step4;
}

void WatchySlides999::drawTime() {

  getTime();

  display.setFont(&TIME_FONT);

  display.setTextColor((darkMode) ? GxEPD_BLACK : GxEPD_WHITE);

  //Hour
  display.setCursor(11, 121);
  display.print(hour1);
  display.setCursor(57, 121);
  display.print(hour2);

  //Minute
  display.setCursor(103, 121);
  display.print(min1);
  display.setCursor(149, 121);
  display.print(min2);
}

void WatchySlides999::drawDate() {

  getDate();

  display.setFont(&TIME_FONT);

  display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);

  //Month
  display.setCursor(11, 58);
  display.print(month1);
  display.setCursor(57, 58);
  display.print(month2);

  //Date
  display.setCursor(103, 58);
  display.print(date1);
  display.setCursor(149, 58);
  display.print(date2);

}

void WatchySlides999::drawSteps() {

  getSteps();

  display.setFont(&TIME_FONT);

  display.fillRect(0, 135, 200, 60, !darkMode ? GxEPD_WHITE : GxEPD_BLACK);

  //Steps
  display.setCursor(11, 185);
  display.print(step1);
  display.setCursor(57, 185);
  display.print(step2);
  display.setCursor(103, 185);
  display.print(step3);
  display.setCursor(149, 185);
  display.print(step4);
}

void WatchySlides999::drawAnim() {

  display.setFont(&TIME_FONT);

  for (uint8_t i = 0; i < 9; i++) {

    if (i == 0) {

      display.fillScreen((darkMode) ? GxEPD_BLACK : GxEPD_WHITE);
      display.fillRect(0, 70, 200, 60, darkMode ? GxEPD_WHITE : GxEPD_BLACK);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Month
      display.setCursor(11, 58);
      display.print(hourC1);

      display.setCursor(57, 58);
      display.print(monthC2);

      //Date
      display.setCursor(103, 58);
      display.print(dateC1);

      display.setCursor(149, 58);
      display.print(dateC2);

      display.setTextColor((!darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Hour
      display.setCursor(11, 121);
      display.print(stepC1);

      display.setCursor(57, 121);
      display.print(hourC2);

      //Minute
      display.setCursor(103, 121);
      display.print(minC1);

      display.setCursor(149, 121);
      display.print(minC2);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Steps
      display.setCursor(11, 185);
      display.print(rNum1);

      display.setCursor(57, 185);
      display.print(stepC2);

      display.setCursor(103, 185);
      display.print(stepC3);

      display.setCursor(149, 185);
      display.print(stepC4);

      display.display(true);

    } else if (i == 1) {

      display.fillScreen((darkMode) ? GxEPD_BLACK : GxEPD_WHITE);
      display.fillRect(0, 70, 200, 60, darkMode ? GxEPD_WHITE : GxEPD_BLACK);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Month
      display.setCursor(11, 58);
      display.print(stepC1);

      display.setCursor(57, 58);
      display.print(monthC2);

      //Date
      display.setCursor(103, 58);
      display.print(minC1);

      display.setCursor(149, 58);
      display.print(dateC2);

      display.setTextColor((!darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Hour
      display.setCursor(11, 121);
      display.print(rNum1);

      display.setCursor(57, 121);
      display.print(hourC2);

      //Minute
      display.setCursor(103, 121);
      display.print(stepC3);

      display.setCursor(149, 121);
      display.print(minC2);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Steps
      display.setCursor(11, 185);
      display.print(rNum2);

      display.setCursor(57, 185);
      display.print(stepC2);

      display.setCursor(103, 185);
      display.print(rNum5);

      display.setCursor(149, 185);
      display.print(stepC4);

      display.display(true);

    } else if (i == 2) {

      getDate();

      display.fillScreen((darkMode) ? GxEPD_BLACK : GxEPD_WHITE);
      display.fillRect(0, 70, 200, 60, darkMode ? GxEPD_WHITE : GxEPD_BLACK);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Month
      display.setCursor(11, 58);
      display.print(rNum1);

      display.setCursor(57, 58);
      display.print(monthC2);

      //Date
      display.setCursor(103, 58);
      display.print(stepC3);

      display.setCursor(149, 58);
      display.print(dateC2);

      display.setTextColor((!darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Hour
      display.setCursor(11, 121);
      display.print(rNum2);

      display.setCursor(57, 121);
      display.print(hourC2);

      //Minute
      display.setCursor(103, 121);
      display.print(rNum5);

      display.setCursor(149, 121);
      display.print(minC2);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Steps
      display.setCursor(11, 185);
      display.print(month1);

      display.setCursor(57, 185);
      display.print(stepC2);

      display.setCursor(103, 185);
      display.print(rNum6);

      display.setCursor(149, 185);
      display.print(stepC4);

      display.display(true);

    } else if (i == 3) {

      getSteps();
      
      getTime();

      display.fillScreen((darkMode) ? GxEPD_BLACK : GxEPD_WHITE);
      display.fillRect(0, 70, 200, 60, darkMode ? GxEPD_WHITE : GxEPD_BLACK);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Month
      display.setCursor(11, 58);
      display.print(rNum2);

      display.setCursor(57, 58);
      display.print(monthC2);

      //Date
      display.setCursor(103, 58); //Column 3 Start
      display.print(rNum5);

      display.setCursor(149, 58);
      display.print(dateC2);

      display.setTextColor((!darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Hour
      display.setCursor(11, 121);
      display.print(month1);

      display.setCursor(57, 121);
      display.print(hourC2);

      //Minute
      display.setCursor(103, 121);
      display.print(rNum6);

      display.setCursor(149, 121);
      display.print(minC2);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Steps
      display.setCursor(11, 185);
      display.print(hour1);

      display.setCursor(57, 185);
      display.print(stepC2);

      display.setCursor(103, 185);
      display.print(date1);

      display.setCursor(149, 185);
      display.print(stepC4);

      display.display(true);

    } else if (i == 4) {

      display.fillScreen((darkMode) ? GxEPD_BLACK : GxEPD_WHITE);
      display.fillRect(0, 70, 200, 60, darkMode ? GxEPD_WHITE : GxEPD_BLACK);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Month
      display.setCursor(11, 58);
      display.print(month1);

      display.setCursor(57, 58);
      display.print(rNum3);

      //Date
      display.setCursor(103, 58);
      display.print(rNum6);

      display.setCursor(149, 58);
      display.print(dateC2);

      display.setTextColor((!darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Hour
      display.setCursor(11, 121);
      display.print(hour1);

      display.setCursor(57, 121);
      display.print(monthC2);

      //Minute
      display.setCursor(103, 121);
      display.print(date1);

      display.setCursor(149, 121);
      display.print(minC2);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Steps
      display.setCursor(11, 185);
      display.print(step1);

      display.setCursor(57, 185);
      display.print(hourC2);

      display.setCursor(103, 185);
      display.print(min1);

      display.setCursor(149, 185);
      display.print(stepC4);

      display.display(true);

    } else if (i == 5) {

      display.fillScreen((darkMode) ? GxEPD_BLACK : GxEPD_WHITE);
      display.fillRect(0, 70, 200, 60, darkMode ? GxEPD_WHITE : GxEPD_BLACK);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Month
      display.setCursor(11, 58);
      display.print(month1);

      display.setCursor(57, 58);
      display.print(rNum4);

      //Date
      display.setCursor(103, 58);
      display.print(date1);

      display.setCursor(149, 58);
      display.print(rNum7);

      display.setTextColor((!darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Hour
      display.setCursor(11, 121);
      display.print(hour1);

      display.setCursor(57, 121);
      display.print(rNum3);

      //Minute
      display.setCursor(103, 121);
      display.print(min1);

      display.setCursor(149, 121);
      display.print(dateC2);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Steps
      display.setCursor(11, 185);
      display.print(step1);

      display.setCursor(57, 185);
      display.print(minC2);

      display.setCursor(103, 185);
      display.print(step3);

      display.setCursor(149, 185);
      display.print(minC2); //Column 4 start

      display.display(true);

    } else if (i == 6) {

      display.fillScreen((darkMode) ? GxEPD_BLACK : GxEPD_WHITE);
      display.fillRect(0, 70, 200, 60, darkMode ? GxEPD_WHITE : GxEPD_BLACK);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Month
      display.setCursor(11, 58);
      display.print(month1);

      display.setCursor(57, 58);
      display.print(step2);

      //Date
      display.setCursor(103, 58);
      display.print(date1);

      display.setCursor(149, 58);
      display.print(rNum8);

      display.setTextColor((!darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Hour
      display.setCursor(11, 121);
      display.print(hour1);

      display.setCursor(57, 121);
      display.print(rNum4);

      //Minute
      display.setCursor(103, 121);
      display.print(min1);

      display.setCursor(149, 121);
      display.print(rNum7);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Steps
      display.setCursor(11, 185);
      display.print(step1);

      display.setCursor(57, 185);
      display.print(rNum3);

      display.setCursor(103, 185);
      display.print(step3);

      display.setCursor(149, 185);
      display.print(minC2); //Column 4 start

      display.display(true);

    } else if (i == 7) {

      display.fillScreen((darkMode) ? GxEPD_BLACK : GxEPD_WHITE);
      display.fillRect(0, 70, 200, 60, darkMode ? GxEPD_WHITE : GxEPD_BLACK);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Month
      display.setCursor(11, 58);
      display.print(month1);

      display.setCursor(57, 58);
      display.print(hour2);

      //Date
      display.setCursor(103, 58);
      display.print(date1);

      display.setCursor(149, 58);
      display.print(step4);

      display.setTextColor((!darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Hour
      display.setCursor(11, 121);
      display.print(hour1);

      display.setCursor(57, 121);
      display.print(step2);

      //Minute
      display.setCursor(103, 121);
      display.print(min1);

      display.setCursor(149, 121);
      display.print(rNum8);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Steps
      display.setCursor(11, 185);
      display.print(step1);

      display.setCursor(57, 185);
      display.print(rNum4);

      display.setCursor(103, 185);
      display.print(step3);

      display.setCursor(149, 185);
      display.print(rNum7);

      display.display(true);

    } else if (i == 8) {

      display.fillScreen((darkMode) ? GxEPD_BLACK : GxEPD_WHITE);
      display.fillRect(0, 70, 200, 60, darkMode ? GxEPD_WHITE : GxEPD_BLACK);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Month
      display.setCursor(11, 58);
      display.print(month1);

      display.setCursor(57, 58);
      display.print(month2);

      //Date
      display.setCursor(103, 58);
      display.print(date1);

      display.setCursor(149, 58);
      display.print(min2);

      display.setTextColor((!darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Hour
      display.setCursor(11, 121);
      display.print(hour1);

      display.setCursor(57, 121);
      display.print(hour2);

      //Minute
      display.setCursor(103, 121);
      display.print(min1);

      display.setCursor(149, 121);
      display.print(step4);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Steps
      display.setCursor(11, 185);
      display.print(step1);

      display.setCursor(57, 185);
      display.print(step2);

      display.setCursor(103, 185);
      display.print(step3);

      display.setCursor(149, 185);
      display.print(rNum8);

      display.display(true);

    } else if (i == 9) {

      display.fillScreen((darkMode) ? GxEPD_BLACK : GxEPD_WHITE);
      display.fillRect(0, 70, 200, 60, darkMode ? GxEPD_WHITE : GxEPD_BLACK);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Month
      display.setCursor(11, 58);
      display.print(month1);

      display.setCursor(57, 58);
      display.print(month2);

      //Date
      display.setCursor(103, 58);
      display.print(date1);

      display.setCursor(149, 58);
      display.print(date2);

      display.setTextColor((!darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Hour
      display.setCursor(11, 121);
      display.print(hour1);

      display.setCursor(57, 121);
      display.print(hour2);

      //Minute
      display.setCursor(103, 121);
      display.print(min1);

      display.setCursor(149, 121);
      display.print(min2);

      display.setTextColor((darkMode) ? GxEPD_WHITE : GxEPD_BLACK);
      //Steps
      display.setCursor(11, 185);
      display.print(step1);

      display.setCursor(57, 185);
      display.print(step2);

      display.setCursor(103, 185);
      display.print(step3);

      display.setCursor(149, 185);
      display.print(step4);

      display.display(true);

    }

  }
  updateCache();
}

void WatchySlides999::checkBattery() {

  float battery =  analogReadMilliVolts(ADC_PIN) / 1000.0f * 2.0f;

//  maxVoltage = (maxVoltage < battery) ? battery : 4.2;
  charging = (currentTime.Minute % 2 == 0) && (battery > oldVoltage) ? true : (battery < oldVoltage) ? false : charging;
  oldVoltage = (oldVoltage == 0) ? battery : (battery > oldVoltage) ? battery : oldVoltage;

  //battery bar logic
  //    if(charging)
  //    battCalculator = maxVoltage - lowBattMargin;
  //    float batt = (battery - battCalculator) / lowBattMargin;
//        float batt = 100*(battery - 3.6)/(4.2 - 3.6);
  //    oldVoltage = (oldVoltage == 0) ? batt : ((batt - oldVoltage) > .01)  ? batt : oldVoltage;

  if (!chargeSync && charging || battery >= maxVoltage && !chargeSync) {
    if (debugger)
      Serial.println("Charging NTP Sync");
    chargeSync = true;
    syncNtpTime();
  } else if (oldVoltage > (battery + .02) & chargeSync && !charging) {
    if (debugger)
      Serial.println("Resetting Charge settings");
    chargeSync = false;
    oldVoltage = 0;
  }

  if (debugger) {
    Serial.println("maxVoltage: " + String(maxVoltage));
    //      Serial.println("battCalculator: " + String(battCalculator));
//    Serial.println("batt: " + String(batt));
    Serial.println("battery: " + String(battery));
    Serial.println("oldVoltage: " + String(oldVoltage));
    Serial.println((charging == true) ? "charging" : "not charging");
    Serial.print("chargeSync: ");
    Serial.println((chargeSync) ? "true" : "false");
  }


}

void WatchySlides999::drawWatchFace() {

  if (syncNTP && !runOnce)
    checkBattery();

  if (runOnce) {
    runOnce = false;
    playAnim = true;
  }

  display.fillScreen((darkMode) ? GxEPD_BLACK : GxEPD_WHITE);
  display.fillRect(0, 70, 200, 60, darkMode ? GxEPD_WHITE : GxEPD_BLACK);

  if (playAnim) {
    getRand();
    drawAnim();
    playAnim = false;
  }
  //  if (syncNTP && currentTime.Hour == 0 && currentTime.Minute == 0) { //Sync NTP at Midnight
  //    if (debugger)
  //      Serial.println("Getting new NTP time");
  //    syncNtpTime();
  //  } else
  if (animMode == 0 && currentTime.Second == 0 || animMode == 1 && currentTime.Minute % 30 == 0 || animMode == 2 && currentTime.Minute == 0) {
    if (debugger)
      Serial.println("updating cache");
    getRand();
    drawAnim();
  }

  display.fillScreen((darkMode) ? GxEPD_BLACK : GxEPD_WHITE);
  display.fillRect(0, 70, 200, 60, darkMode ? GxEPD_WHITE : GxEPD_BLACK);
  drawTime();
  drawDate();
  drawSteps();
}
