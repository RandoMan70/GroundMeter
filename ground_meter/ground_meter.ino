#include <TM1637Display.h>
#include <DallasTemperature.h>

const int LED_BRIGHTNESS=12;

DeviceAddress airMainAddress = {0x28, 0xF5, 0x58, 0xAE, 0x06, 0x00, 0x00, 0x41};
DeviceAddress airReserveAddress = {0x28, 0x49, 0x22, 0x16, 0x0A, 0x00, 0x00, 0xD                                                                                                                                                             D};

DeviceAddress groundMainAddress = {0x28, 0x84, 0x84, 0xAE, 0x06, 0x00, 0xF0, 0x9                                                                                                                                                             1};
DeviceAddress groundReserveAddress = {0x28, 0x23, 0x26, 0x16, 0x0A, 0x00, 0x00,                                                                                                                                                              0xCE};
//DeviceAddress groundReserveAddress = {0x28, 0x4D, 0x31, 0x16, 0x0A, 0x00, 0x00                                                                                                                                                             , 0x33};

//Initialize one-wire bus
OneWire g_oneWire(2);
DallasTemperature g_sensors(&g_oneWire);

//Initialize LEDs
TM1637Display led0(12, 11);
TM1637Display led1(10, 9);
TM1637Display led2(8, 7);
TM1637Display led3(6, 5);

uint8_t deviceCount = 2;//g_sensors.getDeviceCount();

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}


class MeterPoint {
public:
  MeterPoint(DallasTemperature * sensors, DeviceAddress mainAddress, DeviceAddre                                                                                                                                                             ss reserveAddress, TM1637Display *currentLCD, TM1637Display *minLCD);
  void Update();
  static bool ValidTemperature(float value) {
    return value > -100 && value < 80;
  }
  static bool ShowLCD(TM1637Display *lcd, float temperature) {
    lcd->showNumberDec(round(temperature * 10), false);
  }
  static bool InvalidateLCD(TM1637Display *lcd) {
    uint8_t invalid[] = {1 << 6, 1 << 6, 1 << 6, 1 << 6};
    lcd->setSegments(invalid);
  }
private:
  float m_currentTemp;
  float m_minTemp;
  DallasTemperature *m_sensors;
  DeviceAddress m_address;
  DeviceAddress m_reserve_address;
  TM1637Display *m_currentLCD;
  TM1637Display *m_minLCD;
};

MeterPoint::MeterPoint(DallasTemperature * sensors, DeviceAddress address, Devic                                                                                                                                                             eAddress reserveAddress, TM1637Display *currentLCD, TM1637Display *minLCD) :
      m_currentTemp(0), m_minTemp(999), m_sensors(sensors), m_address({0}), m_re                                                                                                                                                             serve_address({0}), m_currentLCD(currentLCD), m_minLCD(minLCD)
{
  m_currentLCD->setBrightness(12);
  m_minLCD->setBrightness(12);
  memcpy(m_address, address, sizeof(m_address));
  memcpy(m_reserve_address, reserveAddress, sizeof(m_reserve_address));
}

void MeterPoint::Update() {
  m_currentTemp = m_sensors->getTempC(m_address);
  if (!ValidTemperature(m_currentTemp)) {
    m_currentTemp = m_sensors->getTempC(m_reserve_address);
  }

  if (!ValidTemperature(m_currentTemp)) {
    InvalidateLCD(m_currentLCD);
    if (!ValidTemperature(m_minTemp)) {
      InvalidateLCD(m_minLCD);
    }
    return;
  }
  if (m_currentTemp < m_minTemp) {
    m_minTemp = m_currentTemp;
  }
/*
  printAddress(m_address);
  Serial.print(" ");
  Serial.print(m_currentTemp);
  Serial.print(" ");
  Serial.println(m_minTemp);
*/

  ShowLCD(m_currentLCD, m_currentTemp);
  ShowLCD(m_minLCD, m_minTemp);
}

void show_sensors() {
  Serial.println("Connected sensors:");
  Serial.println(deviceCount);
  for(uint8_t idx = 0; idx < deviceCount; idx++) {
    DeviceAddress addr;
    g_sensors.getAddress(addr, idx);
    printAddress(addr);
    Serial.println();
  }
}

void setup() {
  Serial.begin(9600);

  led0.setBrightness(LED_BRIGHTNESS);
  led1.setBrightness(LED_BRIGHTNESS);
  led2.setBrightness(LED_BRIGHTNESS);
  led3.setBrightness(LED_BRIGHTNESS);


  g_sensors.begin();
  g_sensors.setResolution(12);
  g_sensors.requestTemperatures();

  Serial.println("Basic temperature monitor is started.");
  show_sensors();
}

MeterPoint AirPoint(&g_sensors, airMainAddress, airReserveAddress, &led0, &led1)                                                                                                                                                             ;
MeterPoint GroundPoint(&g_sensors, groundMainAddress, groundReserveAddress, &led                                                                                                                                                             2, &led3);


void loop() {
  g_sensors.requestTemperatures();

  AirPoint.Update();
  GroundPoint.Update();
  delay(5000);
}
