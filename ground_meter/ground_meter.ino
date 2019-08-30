#include <TM1637Display.h>
#include <DallasTemperature.h>

const int LED_BRIGHTNESS=12;

//Sensors addresses
DeviceAddress airAddress={0x28, 0xF5, 0x58, 0xAE, 0x06, 0x00, 0x00, 0x41};
DeviceAddress groundAddress={0x28, 0x1B, 0xE1, 0x39, 0x02, 0x00, 0x00, 0x3E};

//Initialize one-wire bus
OneWire g_oneWire(13);
DallasTemperature g_sensors(&g_oneWire);

//Initialize LEDs
TM1637Display led0(12, 11);
TM1637Display led1(10, 9);
TM1637Display led2(8, 7);
TM1637Display led3(6, 5);

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
  MeterPoint(DallasTemperature * sensors, DeviceAddress address, TM1637Display *currentLCD, TM1637Display *minLCD);
  void Update();
private:
  float m_currentTemp;
  float m_minTemp;
  DallasTemperature *m_sensors;
  DeviceAddress m_address;
  TM1637Display *m_currentLCD;
  TM1637Display *m_minLCD;  
};

MeterPoint::MeterPoint(DallasTemperature * sensors, DeviceAddress address, TM1637Display *currentLCD, TM1637Display *minLCD) :
      m_currentTemp(0), m_minTemp(999), m_sensors(sensors), m_address({0}), m_currentLCD(currentLCD), m_minLCD(minLCD)
{
  m_currentLCD->setBrightness(12);
  m_minLCD->setBrightness(12);
  memcpy(m_address, address, sizeof(m_address));
}

void MeterPoint::Update() {
  m_currentTemp = m_sensors->getTempC(m_address);
  if (m_currentTemp < m_minTemp) {
    m_minTemp = m_currentTemp;
  }

  m_currentLCD->showNumberDec(round(m_currentTemp * 10), false);
  m_minLCD->showNumberDec(round(m_minTemp * 10), false);
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

  uint8_t deviceCount = 2;//g_sensors.getDeviceCount();
  Serial.println("Basic temperature monitor is started. Connected sensors:");
  Serial.println(deviceCount);
  for(uint8_t idx = 0; idx < deviceCount; idx++) {
    DeviceAddress addr;
    g_sensors.getAddress(addr, idx);
    printAddress(addr);
    Serial.println();    
    //Serial.print("
  }
}

MeterPoint AirPoint(&g_sensors, airAddress, &led0, &led1);
MeterPoint GroundPoint(&g_sensors, groundAddress, &led2, &led3);


void loop() {
  g_sensors.requestTemperatures();

  AirPoint.Update();
  GroundPoint.Update();
  delay(1000);
}
