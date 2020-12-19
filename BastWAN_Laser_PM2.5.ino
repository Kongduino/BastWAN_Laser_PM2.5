#include "Seeed_HM330X.h"

HM330X sensor;
uint8_t buf[30];

const char *str[] = {
  "sensor num: ",
  "PM1.0 concentration (CF=1): ",
  "PM2.5 concentration (CF=1): ",
  "PM10  concentration (CF=1): ",
  "PM1.0 concentration: ",
  "PM2.5 concentration: ",
  "PM10  concentration: ",
};

void hexDump(uint8_t *buf, uint16_t len) {
  String s = "|", t = "| |";
  Serial.println(F("  +------------------------------------------------+ +----------------+"));
  Serial.println(F("  |.0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .a .b .c .d .e .f | |   A S C I I    |"));
  Serial.println(F("  +------------------------------------------------+ +----------------+"));
  for (uint16_t i = 0; i < len; i += 16) {
    for (uint8_t j = 0; j < 16; j++) {
      if (i + j >= len) {
        s = s + "   "; t = t + " ";
      } else {
        char c = buf[i + j];
        if (c < 16) s = s + "0";
        s = s + String(c, HEX) + " ";
        if (c < 32 || c > 127) t = t + ".";
        else t = t + (char)c;
      }
    }
    uint8_t index = i / 16;
    Serial.print(index, HEX); Serial.write('.');
    Serial.println(s + t + "|");
    s = "|"; t = "| |";
  }
  Serial.println(F("  +------------------------------------------------+ +----------------+\n"));
}

HM330XErrorCode print_result(const char* str, uint16_t value) {
  if (NULL == str) return ERROR_PARAM;
  SerialUSB.print(str);
  SerialUSB.print(value);
  SerialUSB.println(" ug/m3");
  return NO_ERROR;
}

/*parse buf with 29 uint8_t-data*/
HM330XErrorCode parse_result(uint8_t *data) {
  uint16_t value = 0;
  if (NULL == data) return ERROR_PARAM;
  value = (uint16_t)data[2] << 8 | data[3];
  SerialUSB.print(str[0]);
  SerialUSB.println(value);
  for (int i = 2; i < 8; i++) {
    if (i == 2) SerialUSB.println("Standard particulate matter\n---------------------------");
    else if (i == 5) SerialUSB.println("Atmospheric environment\n-----------------------");
    value = (uint16_t)data[i * 2] << 8 | data[i * 2 + 1];
    print_result(str[i - 1], value);
  }
  return NO_ERROR;
}

HM330XErrorCode parse_result_value(uint8_t *data) {
  if (NULL == data) return ERROR_PARAM;
  hexDump(data, 29);
  uint8_t sum = 0;
  for (int i = 0; i < 28; i++) {
    sum += data[i];
  }
  if (sum != data[28]) {
    SerialUSB.println("wrong checkSum!!!!");
    return ERROR_OTHERS;
  }
  SerialUSB.println(" ");
  SerialUSB.println(" ");
  return NO_ERROR;
}

/*30s*/
void setup() {
  SerialUSB.begin(115200);
  delay(2500);
  SerialUSB.flush();
  delay(500);
  SerialUSB.println("Serial start");
  SerialUSB.print("HM330X init\n");
  if (sensor.init()) {
    SerialUSB.println("HM330X init failed!!!");
    while (1);
  }
}

void loop() {
  if (sensor.read_sensor_value(buf, 29)) {
    SerialUSB.println("HM330X read result failed!!!");
  }
  HM330XErrorCode err = parse_result_value(buf);
  if (err == NO_ERROR) {
    parse_result(buf);
    SerialUSB.println(" ");
    SerialUSB.println(" ");
    SerialUSB.println(" ");
  }
  delay(5000);
}
