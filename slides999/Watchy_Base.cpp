//Derived from peerdavid's source at: https://github.com/peerdavid/Watchy
#include "Watchy_Base.h"
#include "config999.h"

RTC_DATA_ATTR int twelveMode = 0;
RTC_DATA_ATTR bool sleep_mode = false;
RTC_DATA_ATTR bool darkMode = true;
RTC_DATA_ATTR bool syncNTP = true;
RTC_DATA_ATTR int animMode = 0;
RTC_DATA_ATTR int dateMode = 0;
RTC_DATA_ATTR int wifiMode = 0;
RTC_DATA_ATTR int selectedItem;
RTC_DATA_ATTR int syncIndex = 0;
bool playAnim = false;
bool manualSync = false;
const char *offsetStatus = "success";

//Set this flag to true if you want to monitor Serial logs
RTC_DATA_ATTR bool debugger = false;

const char *menuItems[] = {"Animations", "Sync NTP Time", "Hour Format", "Date Format", "WiFi Mode", "Setup WiFi", "Set Time", "Show Battery"};
int16_t menuOptions = sizeof(menuItems) / sizeof(menuItems[0]);

WatchyBase::WatchyBase() {}

void WatchyBase::init() {

  if (debugger)
    Serial.begin(115200);

  wakeup_reason = esp_sleep_get_wakeup_cause(); //get wake up reason
  Wire.begin(SDA, SCL); //init i2c

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0: //RTC Alarm

      // Handle classical tick
      RTC.alarm(ALARM_2); //resets the alarm flag in the RTC

      if (guiState == WATCHFACE_STATE) {
        RTC.read(currentTime);
        showWatchFace(true); //partial updates on tick
      }
      break;

    case ESP_SLEEP_WAKEUP_EXT1: //button Press + no handling if wakeup
      if (sleep_mode) {
        sleep_mode = false;
        RTC.alarmInterrupt(ALARM_2, true);
        RTC.alarm(ALARM_2); //resets the alarm flag in the RTC

        RTC.read(currentTime);
        showWatchFace(false); //full update on wakeup from sleep mode
        break;
      }

      handleButtonPress();
      break;

    default: //reset
      _rtcConfig();
      _bmaConfig();
      showWatchFace(true); //full update on reset
      break;
  }

  // Sometimes BMA crashes - simply try to reinitialize bma...
  if (sensor.getErrorCode() != 0) {
    sensor.shutDown();
    sensor.wakeUp();
    sensor.softReset();
    _bmaConfig();
  }
  deepSleep();
}

void WatchyBase::handleButtonPress() {
  uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();

  //Menu Button
  if (wakeupBit & MENU_BTN_MASK) {
    if (guiState == WATCHFACE_STATE) { //enter menu state if coming from watch face
      vibrate();
      showMenu(menuIndex, false);
    } else if (guiState == MAIN_MENU_STATE) { //if already in menu, then select menu item
      switch (menuIndex)
      {
        case 0:
          animationApp();
          break;
        case 1:
          ntpApp();
          break;
        case 2:
          twelveModeApp();
          break;
        case 3:
          dateModeApp();
          break;
        case 4:
          wifiModeApp();
          break;
        case 5:
          setupWifi();
          break;
        case 6:
          setTime();
          break;
        case 7:
          showBattery();
          break;
        default:
          break;
      }
    } else if (guiState == FW_UPDATE_STATE) {
      updateFWBegin();
    }
  }
  //Back Button
  else if (wakeupBit & BACK_BTN_MASK) {
    if (guiState == MAIN_MENU_STATE) { //exit to watch face if already in menu
      RTC.alarm(ALARM_2); //resets the alarm flag in the RTC
      RTC.read(currentTime);
      showWatchFace(false);
    } else if (guiState == APP_STATE) {
      showMenu(menuIndex, false);//exit to menu if already in app
    } else if (guiState == FW_UPDATE_STATE) {
      showMenu(menuIndex, false);//exit to menu if already in app
    }
  }
  //Up Button
  else if (wakeupBit & UP_BTN_MASK) {
    if (guiState == MAIN_MENU_STATE) { //increment menu index
      menuIndex--;
      if (menuIndex < 0) {
        menuIndex = menuOptions - 1;
      }
      showMenu(menuIndex, true);
    }
  }
  //Down Button
  else if (wakeupBit & DOWN_BTN_MASK) {
    if (guiState == MAIN_MENU_STATE) { //decrement menu index
      menuIndex++;
      if (menuIndex > menuOptions - 1) {
        menuIndex = 0;
      }
      showMenu(menuIndex, true);
    }
  }

  if (IS_BTN_LEFT_UP) {
    //    twelveMode = (twelveMode == 0) ? true : false;
    RTC.read(currentTime);
    vibrate();
    showWatchFace(true);
    return;
  }

  if (IS_BTN_RIGHT_UP) {
    RTC.read(currentTime);
    vibrate();

    darkMode = (!darkMode) ? true : false;
    showWatchFace(true);
    return;
  }

  if (IS_BTN_RIGHT_DOWN) {
    RTC.read(currentTime);
    vibrate();
    if (debugger)
      playAnim = true;
    showWatchFace(true);
    return;
  }

  /***************** fast menu *****************/
  bool timeout = false;
  long lastTimeout = millis();
  pinMode(MENU_BTN_PIN, INPUT);
  pinMode(BACK_BTN_PIN, INPUT);
  pinMode(UP_BTN_PIN, INPUT);
  pinMode(DOWN_BTN_PIN, INPUT);

  while (!timeout) {
    if (millis() - lastTimeout > 5000) {
      timeout = true;
    } else {
      if (digitalRead(MENU_BTN_PIN) == 1) {
        vibrate();
        lastTimeout = millis();
        if (guiState == MAIN_MENU_STATE) { //if already in menu, then select menu item
          switch (menuIndex)
          {
            case 0:
              animationApp();
              break;
            case 1:
              ntpApp();
              break;
            case 2:
              twelveModeApp();
              break;
            case 3:
              dateModeApp();
              break;
            case 4:
              wifiModeApp();
              break;
            case 5:
              setupWifi();
              break;
            case 6:
              setTime();
              break;
            case 7:
              showBattery();
            default:
              break;
          }
        } else if (guiState == FW_UPDATE_STATE) {
          updateFWBegin();
        }
      } else if (digitalRead(BACK_BTN_PIN) == 1) {
        vibrate();
        lastTimeout = millis();
        if (guiState == MAIN_MENU_STATE) { //exit to watch face if already in menu
          RTC.alarm(ALARM_2); //resets the alarm flag in the RTC
          RTC.read(currentTime);
          showWatchFace(false);
          break; //leave loop
        } else if (guiState == APP_STATE) {
          showMenu(menuIndex, false);//exit to menu if already in app
        } else if (guiState == FW_UPDATE_STATE) {
          showMenu(menuIndex, false);//exit to menu if already in app
        }
      } else if (digitalRead(UP_BTN_PIN) == 1) {
        vibrate();
        lastTimeout = millis();
        if (guiState == MAIN_MENU_STATE) { //increment menu index
          menuIndex--;
          if (menuIndex < 0) {
            menuIndex = menuOptions - 1;
          }
          showFastMenu(menuIndex);
        }
      } else if (digitalRead(DOWN_BTN_PIN) == 1) {
        vibrate();
        lastTimeout = millis();
        if (guiState == MAIN_MENU_STATE) { //decrement menu index
          menuIndex++;
          if (menuIndex > menuOptions - 1) {
            menuIndex = 0;
          }
          showFastMenu(menuIndex);
        }
      }
    }
  }

  //  Watchy::handleButtonPress();
  display.hibernate();

}

void WatchyBase::vibrate(uint8_t times, uint32_t delay_time) {
  // Ensure that no false positive double tap is produced
  sensor.enableFeature(BMA423_WAKEUP, false);

  pinMode(VIB_MOTOR_PIN, OUTPUT);
  for (uint8_t i = 0; i < times; i++) {
    delay(delay_time);
    digitalWrite(VIB_MOTOR_PIN, true);
    delay(delay_time);
    digitalWrite(VIB_MOTOR_PIN, false);
  }

  sensor.enableFeature(BMA423_WAKEUP, true);
}

void WatchyBase::_rtcConfig() {
  //https://github.com/JChristensen/DS3232RTC
  RTC.squareWave(SQWAVE_NONE); //disable square wave output
  //RTC.set(compileTime()); //set RTC time to compile time
  RTC.setAlarm(ALM2_EVERY_MINUTE, 0, 0, 0, 0); //alarm wakes up Watchy every minute
  RTC.alarmInterrupt(ALARM_2, true); //enable alarm interrupt
  RTC.read(currentTime);
}

void WatchyBase::_bmaConfig() {

  if (sensor.begin(_readRegister, _writeRegister, delay) == false) {
    //fail to init BMA
    return;
  }

  // Accel parameter structure
  Acfg cfg;
  /*!
      Output data rate in Hz, Optional parameters:
          - BMA4_OUTPUT_DATA_RATE_0_78HZ
          - BMA4_OUTPUT_DATA_RATE_1_56HZ
          - BMA4_OUTPUT_DATA_RATE_3_12HZ
          - BMA4_OUTPUT_DATA_RATE_6_25HZ
          - BMA4_OUTPUT_DATA_RATE_12_5HZ
          - BMA4_OUTPUT_DATA_RATE_25HZ
          - BMA4_OUTPUT_DATA_RATE_50HZ
          - BMA4_OUTPUT_DATA_RATE_100HZ
          - BMA4_OUTPUT_DATA_RATE_200HZ
          - BMA4_OUTPUT_DATA_RATE_400HZ
          - BMA4_OUTPUT_DATA_RATE_800HZ
          - BMA4_OUTPUT_DATA_RATE_1600HZ
  */
  cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
  /*!
      G-range, Optional parameters:
          - BMA4_ACCEL_RANGE_2G
          - BMA4_ACCEL_RANGE_4G
          - BMA4_ACCEL_RANGE_8G
          - BMA4_ACCEL_RANGE_16G
  */
  cfg.range = BMA4_ACCEL_RANGE_2G;
  /*!
      Bandwidth parameter, determines filter configuration, Optional parameters:
          - BMA4_ACCEL_OSR4_AVG1
          - BMA4_ACCEL_OSR2_AVG2
          - BMA4_ACCEL_NORMAL_AVG4
          - BMA4_ACCEL_CIC_AVG8
          - BMA4_ACCEL_RES_AVG16
          - BMA4_ACCEL_RES_AVG32
          - BMA4_ACCEL_RES_AVG64
          - BMA4_ACCEL_RES_AVG128
  */
  cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;

  /*! Filter performance mode , Optional parameters:
      - BMA4_CIC_AVG_MODE
      - BMA4_CONTINUOUS_MODE
  */
  cfg.perf_mode = BMA4_CONTINUOUS_MODE;

  // Configure the BMA423 accelerometer
  sensor.setAccelConfig(cfg);

  // Enable BMA423 accelerometer
  // Warning : Need to use feature, you must first enable the accelerometer
  sensor.enableAccel();

  struct bma4_int_pin_config config ;
  config.edge_ctrl = BMA4_LEVEL_TRIGGER;
  config.lvl = BMA4_ACTIVE_HIGH;
  config.od = BMA4_PUSH_PULL;
  config.output_en = BMA4_OUTPUT_ENABLE;
  config.input_en = BMA4_INPUT_DISABLE;
  // The correct trigger interrupt needs to be configured as needed
  sensor.setINTPinConfig(config, BMA4_INTR1_MAP);

  struct bma423_axes_remap remap_data;
  remap_data.x_axis = 1;
  remap_data.x_axis_sign = 0xFF;
  remap_data.y_axis = 0;
  remap_data.y_axis_sign = 0xFF;
  remap_data.z_axis = 2;
  remap_data.z_axis_sign = 0xFF;
  // Need to raise the wrist function, need to set the correct axis
  sensor.setRemapAxes(&remap_data);

  // Enable BMA423 isStepCounter feature
  sensor.enableFeature(BMA423_STEP_CNTR, true);
  // Enable BMA423 isTilt feature
  sensor.enableFeature(BMA423_TILT, true);
  // Enable BMA423 isDoubleClick feature
  //sensor.enableFeature(BMA423_WAKEUP, true);

  // Reset steps
  //sensor.resetStepCounter();

  // Turn on feature interrupt
  //sensor.enableStepCountInterrupt();
  //sensor.enableTiltInterrupt();
  // It corresponds to isDoubleClick interrupt
  //sensor.enableWakeupInterrupt();
}

uint16_t WatchyBase::_readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)address, (uint8_t)len);
  uint8_t i = 0;
  while (Wire.available()) {
    data[i++] = Wire.read();
  }
  return 0;
}

uint16_t WatchyBase::_writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data, len);
  return (0 !=  Wire.endTransmission());
}

//scrolling menu by Alex Story
void WatchyBase::showMenu(byte menuIndex, bool partialRefresh) {
  display.init(0, false); //_initial_refresh to false to prevent full update on init
  display.setFullWindow();
  display.fillScreen(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);

  int16_t  x1, y1;
  uint16_t w, h;
  int16_t yPos;
  int16_t startPos = 0;
  //Code to move the menu if current selected index out of bounds
  if (menuIndex + MENU_LENGTH > menuOptions)
  {
    startPos = (menuOptions - 1) - (MENU_LENGTH - 1);
  }
  else
  {
    startPos = menuIndex;
  }
  for (int i = startPos; i < MENU_LENGTH + startPos; i++) {
    yPos = 30 + (MENU_HEIGHT * (i - startPos));
    display.setCursor(20, yPos);
    if (i == menuIndex) {
      display.getTextBounds(menuItems[i], 0, yPos, &x1, &y1, &w, &h);
      display.fillRect(x1 - 1, y1 - 10, 200, h + 15, GxEPD_WHITE);
      display.setTextColor(GxEPD_BLACK);
      display.println(menuItems[i]);
    } else {
      display.setTextColor(GxEPD_WHITE);
      display.println(menuItems[i]);
    }
  }

  display.display(partialRefresh);
  //display.hibernate();

  guiState = MAIN_MENU_STATE;
}

//Specific functions by dezign999 start here
//Sub Menu with selectable items by dezign999
void WatchyBase::showFastMenu(byte menuIndex) {
  display.setFullWindow();
  display.fillScreen(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);

  int16_t  x1, y1;
  uint16_t w, h;
  int16_t yPos;
  int16_t startPos = 0;
  if (menuIndex + MENU_LENGTH > menuOptions)
  {
    startPos = (menuOptions - 1) - (MENU_LENGTH - 1);
  }
  else
  {
    startPos = menuIndex;
  }
  for (int i = startPos; i < MENU_LENGTH + startPos; i++) {
    yPos = 30 + (MENU_HEIGHT * (i - startPos));
    display.setCursor(20, yPos);
    if (i == menuIndex) {
      display.getTextBounds(menuItems[i], 0, yPos, &x1, &y1, &w, &h);
      display.fillRect(x1 - 1, y1 - 10, 200, h + 15, GxEPD_WHITE);
      display.setTextColor(GxEPD_BLACK);
      display.println(menuItems[i]);
    } else {
      display.setTextColor(GxEPD_WHITE);
      display.println(menuItems[i]);
    }
  }

  display.display(true);

  guiState = MAIN_MENU_STATE;
}

void WatchyBase::animationApp() {
  display.init(0, false); //_initial_refresh to false to prevent full update on init
  display.setFullWindow();
  display.fillScreen(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_WHITE);
  display.fillScreen(GxEPD_BLACK);

  char *listItems[] = {"Every Minute", "Every Half Hour", "Every Hour", "Off"};
  byte itemCount = sizeof(listItems) / sizeof(listItems[0]);

  uint16_t listIndex = animMode;

  //Send the stored animMode value to set the selected item
  showList(listItems, itemCount, animMode, true, false);

  while (1) {

    if (digitalRead(BACK_BTN_PIN) == 1) {
      vibrate();
      break;
    }

    if (digitalRead(MENU_BTN_PIN) == 1) {
      vibrate();
      animMode = listIndex;
      showList(listItems, itemCount, listIndex, true, true);
    }

    if (digitalRead(DOWN_BTN_PIN) == 1) {
      listIndex == (itemCount - 1) ? (listIndex = 0) : listIndex++;
      vibrate();
      showList(listItems, itemCount, listIndex, false, true);
    }

    if (digitalRead(UP_BTN_PIN) == 1) {
      listIndex == 0 ? (listIndex = (itemCount - 1)) : listIndex--;
      vibrate();
      showList(listItems, itemCount, listIndex, false, true);
    }

  }
  display.hibernate();
  showMenu(menuIndex, false);
}

void WatchyBase::twelveModeApp() {

  char *listItems[] = {"12 Hour", "24 Hour"};
  byte itemCount = sizeof(listItems) / sizeof(listItems[0]);

  uint16_t listIndex = twelveMode;

  //Send the stored value to set the selected item
  showList(listItems, itemCount, twelveMode, true, false);

  while (1) {

    if (digitalRead(BACK_BTN_PIN) == 1) {
      vibrate();
      break;
    }

    if (digitalRead(MENU_BTN_PIN) == 1) {
      twelveMode = (listIndex == 0) ? 0 : 1;
      vibrate();
      showList(listItems, itemCount, listIndex, true, true);
    }

    if (digitalRead(DOWN_BTN_PIN) == 1) {
      listIndex = (listIndex == 0) ? 1 : 0;
      vibrate();
      showList(listItems, itemCount, listIndex, false, true);
      if (debugger)
        Serial.println(String(listIndex));
      RTC.read(currentTime);
    }

    if (digitalRead(UP_BTN_PIN) == 1) {
      listIndex = (listIndex == 0) ? 1 : 0;
      vibrate();
      showList(listItems, itemCount, listIndex, false, true);
      if (debugger)
        Serial.println(String(listIndex));
      RTC.read(currentTime);
    }

  }
  display.hibernate();
  showMenu(menuIndex, false);
}

void WatchyBase::dateModeApp() {

  char *listItems[] = {"MM/DD", "DD/HH"};
  byte itemCount = sizeof(listItems) / sizeof(listItems[0]);

  uint16_t listIndex = dateMode;

  //Send the stored value to set the selected item
  showList(listItems, itemCount, dateMode, true, false);

  while (1) {

    if (digitalRead(BACK_BTN_PIN) == 1) {
      vibrate();
      break;
    }

    if (digitalRead(MENU_BTN_PIN) == 1) {
      dateMode = (listIndex == 0) ? 0 : 1;
      vibrate();
      showList(listItems, itemCount, listIndex, true, true);
    }

    if (digitalRead(DOWN_BTN_PIN) == 1) {
      listIndex = (listIndex == 0) ? 1 : 0;
      vibrate();
      showList(listItems, itemCount, listIndex, false, true);
      if (debugger)
        Serial.println(String(listIndex));
      RTC.read(currentTime);
    }

    if (digitalRead(UP_BTN_PIN) == 1) {
      listIndex = (listIndex == 0) ? 1 : 0;
      vibrate();
      showList(listItems, itemCount, listIndex, false, true);
      if (debugger)
        Serial.println(String(listIndex));
      RTC.read(currentTime);
    }

  }
  display.hibernate();
  showMenu(menuIndex, false);
}

void WatchyBase::ntpApp() {

  char *listItems[] = {"When Charging", "Off", "Now (Press Menu)"};
  byte itemCount = sizeof(listItems) / sizeof(listItems[0]);

  uint16_t listIndex = syncIndex;

  bool synced = false;

  //Send the stored animMode value to set the selected item
  showList(listItems, itemCount, syncIndex, true, false);

  while (1) {

    if (digitalRead(BACK_BTN_PIN) == 1) {
      vibrate();
      break;
    }

    if (digitalRead(DOWN_BTN_PIN) == 1) {
      listIndex == (itemCount - 1) ? (listIndex = 0) : listIndex++;
      synced = false;
      vibrate();
      showList(listItems, itemCount, listIndex, false, true);
    }

    if (digitalRead(UP_BTN_PIN) == 1) {
      listIndex == 0 ? (listIndex = (itemCount - 1)) : listIndex--;
      syncNTP = (listIndex == 0) ? true : false;
      synced = false;
      vibrate();
      showList(listItems, itemCount, listIndex, false, true);
    }

    if (digitalRead(MENU_BTN_PIN) == 1) {
      vibrate();
      if (debugger)
        Serial.println("ntp listIndex: " + String(listIndex));

      if (listIndex != 2) {

        syncIndex = listIndex;
        showList(listItems, itemCount, listIndex, true, true);

      } else if (listIndex == 2) {

        showList(listItems, itemCount, listIndex, false, true);

        display.setTextColor(GxEPD_WHITE);
        display.setCursor(20, 120);
        display.println("Checking WiFi");
        display.display(true);

        manualSync = true;

        syncNtpTime();

        synced = true;
      }
    }

  }

  syncNTP = (syncIndex == 1) ? true : false;

  display.hibernate();
  showMenu(menuIndex, false);

}

void WatchyBase::wifiModeApp() {

  char *listItems[] = {"Multi APs", "Default WiFi"};
  byte itemCount = sizeof(listItems) / sizeof(listItems[0]);

  uint16_t listIndex = wifiMode;
  uint8_t selectCounter = 0;

  //Send the stored value to set the selected item
  showList(listItems, itemCount, wifiMode, true, false);

  while (1) {



    if (digitalRead(BACK_BTN_PIN) == 1) {
      vibrate();
      break;
    }

    if (digitalRead(MENU_BTN_PIN) == 1) {
      wifiMode = (listIndex == 0) ? 0 : 1;
      selectCounter++;
      vibrate();
      if(selectCounter < 2)
        showList(listItems, itemCount, listIndex, true, true);
      if (wifiMode == 1) {
        display.setTextColor(GxEPD_WHITE);
        display.setCursor(20, 90);
        display.println("SSID Reset");
        display.setCursor(20, 105);
        display.println("Press Menu");
        display.setCursor(20, 120);
        display.println("to Setup WiFi");
        display.display(true);
      }
      if (selectCounter == 2) {
        setupWifi();
      }
    }

    if (digitalRead(DOWN_BTN_PIN) == 1) {
      listIndex = (listIndex == 0) ? 1 : 0;
      vibrate();
      showList(listItems, itemCount, listIndex, false, true);
      if (debugger)
        Serial.println(String(listIndex));
    }

    if (digitalRead(UP_BTN_PIN) == 1) {
      listIndex = (listIndex == 0) ? 1 : 0;
      vibrate();
      showList(listItems, itemCount, listIndex, false, true);
      if (debugger)
        Serial.println(String(listIndex));
    }

  }
  display.hibernate();
  showMenu(menuIndex, false);
}

void WatchyBase::showList(char *listItems[], byte itemCount, byte listIndex, bool selected, bool partialRefresh) {
  display.init(0, false); //_initial_refresh to false to prevent full update on init
  display.setFullWindow();
  display.fillScreen(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);

  if (selected)
    selectedItem = listIndex;

  if (debugger) {
    Serial.print("listItem: ");
    Serial.println(listItems[listIndex]);
    Serial.println("listOption length: " + String(itemCount));
    Serial.print("Selected: ");
    Serial.println((selected) ? "true" : "false");
    Serial.println("Selected Item: " + String(selectedItem));
  }

  int16_t  x1, y1;
  uint16_t w, h;
  int16_t yPos;

  for (int i = 0; i < itemCount; i++) {
    yPos = 30 + (MENU_HEIGHT * i);
    display.setCursor(20, yPos);
    if (i == listIndex) {
      display.getTextBounds(listItems[i], 0, yPos, &x1, &y1, &w, &h);
      display.fillRect(x1 - 1, y1 - 10, 200, h + 15, GxEPD_WHITE);
      display.setTextColor(GxEPD_BLACK);
      display.println(listItems[i]);
      if (i == selectedItem) {
        display.setCursor(5, yPos);
        display.println(">");
      }
    } else {
      display.setTextColor(GxEPD_WHITE);
      display.println(listItems[i]);
      if (i == selectedItem) {
        display.setCursor(5, yPos);
        display.println(">");
      }
    }
  }

  display.display(partialRefresh);
  guiState = APP_STATE;
}

void WatchyBase::syncNtpTime() {


  if (wifiMode == 0) {
    if (debugger)
      Serial.println("Trying wifi999");
    wifi999();
    if (debugger)
      Serial.println("Connected to wifi999");
  } else {
    if (debugger)
      Serial.println("Trying WiFi");
    connectWiFi();
    if (debugger)
      Serial.println("Connected to WiFi");
  }

  if (WiFi.status() == WL_CONNECTED) {

    if (debugger)
      Serial.println("Offset: " + String(gmtOffset));

    if (gmtOffset == 0) { //Get Time Offset
      HTTPClient http;
      http.setConnectTimeout(3000);//3 second max timeout

      String gmtOffsetURL = "http://ip-api.com/json/?fields=33570816";

      http.begin(gmtOffsetURL.c_str());
      int httpResponseCode = http.GET();
      if (httpResponseCode == 200) {
        String payload = http.getString();
        JSONVar responseObject = JSON.parse(payload);
        offsetStatus = responseObject["status"];
        gmtOffset = int(responseObject["offset"]);
        if (strcmp(offsetStatus, "success") != 0) {
          if (manualSync) {
            display.setCursor(20, 145);
            display.println("Offset Failed");
          }
        }
      } else {
        //http error
        if (manualSync) {
          display.setCursor(20, 145);
          display.println("Offset Failed");
        }
      }
    }

    time_t t;
    bool syncFailed = false;

    configTime(gmtOffset, 0, ntpServer);

    int i = 0;
    while (!sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED && i < 20) {
      if (debugger)
        Serial.print(".");
      delay(1000);
      i++;
      if (i == 10) {
        if (manualSync) {
          display.setCursor(20, 145);
          display.println("Failed to sync");
        }
        syncFailed = true;
      }
    }

    if (!syncFailed) {
      time_t tnow = time(nullptr);
      struct tm *local = localtime(&tnow);
      if (manualSync) {
        display.setCursor(20, 145);
        display.println("Offset: " + String(gmtOffset));
        display.setCursor(20, 160);
        display.print(local->tm_year + 1900);
        display.print("-");
        if ((local->tm_mon + 1) < 10) {
          display.print("0");
        }
        display.print(local->tm_mon + 1);
        display.print("-");
        if (local->tm_mday < 10) {
          display.print("0");
        }
        display.print(local->tm_mday);
        display.setCursor(20, 175);
        if (local->tm_hour < 10) {
          display.print("0");
        }
        display.print(local->tm_hour);
        display.print(":");
        if (local->tm_min < 10) {
          display.print("0");
        }
        display.print(local->tm_min);
      }

      currentTime.Year = local->tm_year + YEAR_OFFSET - 2040; //This change matches watchy defaults
      currentTime.Month = local->tm_mon + 1;
      currentTime.Day = local->tm_mday;
      currentTime.Hour = local->tm_hour;
      currentTime.Minute = local->tm_min;
      currentTime.Second = local->tm_sec;
      currentTime.Wday = local->tm_wday + 1;
      RTC.write(currentTime);
      RTC.read(currentTime);

    }
  } else {
    if (manualSync) {
      display.setCursor(20, 145);
      display.println("No WiFi Found");
    }
  }

  disableWiFi();
  manualSync = false;
  display.display(true);
}

void WatchyBase::disableWiFi() {
  WiFi.mode(WIFI_OFF);
  WIFI_CONFIGURED = false;
  btStop();
  if (debugger)
    Serial.println("WiFi Turned Off. IP Check: " + WiFi.localIP());
}

bool WatchyBase::wifi999() {

  int i, n;

  for (n = 0; n < accessPointCount; n++) {
    Serial.println("Trying: " + String(accessPoints[n]));
    WiFi.begin(accessPoints[n], apPasswords[n]);
    i = 0;
    while (i < 10 && WiFi.status() != WL_CONNECTED) {
      i++;
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
      break;
    if (debugger) {
      Serial.print(F("\nConnecting to "));
      Serial.println(accessPoints[n]);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (debugger) {
      Serial.println(F("\nWiFi connected, your IP address is "));
      Serial.println(WiFi.localIP());
    }
    WIFI_CONFIGURED = true;
  } else {
    WIFI_CONFIGURED = false;
    //    disableWiFi();
  }

  if (debugger) {
    Serial.print("WIFI_CONFIGURED = ");
    Serial.println((WIFI_CONFIGURED == 1) ? "true" : "false");
  }
  return WIFI_CONFIGURED;
}
