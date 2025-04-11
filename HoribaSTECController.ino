const int VERSION = 2;

#include <Wire.h>

#include <Adafruit_MCP4725.h>
#include <Vrekrer_scpi_parser.h>
#include <EEPROM.h>


Adafruit_MCP4725 dac;



const int scale_addr = 0x10;
const int offset_addr = 0x20;


volatile double offset = 0.0;
volatile double scale = 4095/100;

int magic = 0;


volatile bool debug = false;

int16_t control_voltage = 0;
int32_t flow_set = 0;

SCPI_Parser instrument;

void setup() {
  // setup serial communication
  Serial.begin(9600);

  // setup I2C
  Wire.begin();

  // setup DAC
  dac.begin(0x60);
  dac.setVoltage(control_voltage, true);

  instrument.SetCommandTreeBase(F("FLOw"));
  instrument.RegisterCommand(F(":CURrent?"), &GetCurrentFlow);
  instrument.RegisterCommand(F(":SET"), &SetSetFlow);
  instrument.RegisterCommand(F(":SET?"), &GetSetFlow);

  instrument.SetCommandTreeBase(F("SYSTem"));
  instrument.RegisterCommand(F(":DEBug"), &SetDebug);
  instrument.RegisterCommand(F(":RESet"), &ResetEEPROM);

  instrument.SetCommandTreeBase(F("SYSTem:FLOw"));
  instrument.RegisterCommand(F(":OFFSet"), &SetOffset);
  instrument.RegisterCommand(F(":OFFSet?"), &GetOffset);
  instrument.RegisterCommand(F(":SCALe"), &SetScale);
  instrument.RegisterCommand(F(":SCALe?"), &GetScale);

  EEPROM.get(0, magic);

  if (magic == VERSION) {
    EEPROM.get(offset_addr, offset);
    EEPROM.get(scale_addr, scale);

    Serial.println("Configuration loaded!");

  } else {
    EEPROM.put(offset_addr, offset);
    EEPROM.put(scale_addr, scale);
    EEPROM.put(0, VERSION);
    Serial.println("No configuration found, set defaults!");
  }

  Serial.println("Setup completed!");
}

void loop() {




  // debug output
  if (debug) {
    Serial.print("control_voltage:");
    Serial.print(control_voltage);
    Serial.print(",");
    Serial.print("flow_set:");
    Serial.print(flow_set);
    Serial.print(",");
    Serial.print("flow_cur:");
    Serial.println(readFlow());
  }

  instrument.ProcessInput(Serial, "\n");
}



float readFlow() {
  return 0.;
}


void GetCurrentFlow(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(readFlow());
}

void SetSetFlow(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  if (parameters.Size() > 0) {
    flow_set = constrain(String(parameters[0]).toInt(), 0, 100);
    control_voltage = constrain(uint16_t(scale*flow_set + offset), 0, 4095);
    dac.setVoltage(control_voltage, true);
  }
}
void GetSetFlow(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(flow_set);
}


void SetOffset(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  if (parameters.Size() > 0) {
    offset = constrain(String(parameters[0]).toDouble(), 0, 10000);
    EEPROM.put(offset_addr, offset);
  }
}
void GetOffset(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(offset);
}
void SetScale(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  if (parameters.Size() > 0) {
    scale = constrain(String(parameters[0]).toDouble(), 0, 10000);
    EEPROM.put(scale_addr, scale);
  }
}
void GetScale(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(scale);
}



void SetDebug(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  if (parameters.Size() > 0) {
    debug = constrain(String(parameters[0]).toInt(), 0, 2);
  }
}


void ResetEEPROM(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  EEPROM.put(scale_addr, double(0));
  EEPROM.put(offset_addr, double(0));
  EEPROM.put(0, 0);
  interface.println("EEPROM resetted. Please restart device.");
}

void DoNothing(SCPI_C commands, SCPI_P parameters, Stream& interface) {
}
