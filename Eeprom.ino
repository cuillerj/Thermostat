uint8_t ReadByte(int address)
{
#if defined(debugEeprom)
  Serial.print("eeprom ");
  Serial.print(address);
  Serial.print(":");
  Serial.println(EEPROM.read(address), HEX);
#endif
  return EEPROM.read(address);
}
uint8_t WriteByte(int address, uint8_t value)
{
#if defined(debugEeprom)
  Serial.print("update eeprom ");
  Serial.print(address);
  Serial.print(":");
  Serial.println(value, HEX);
#endif
  EEPROM.update(address, value);
  delay(5);
}
void LoadParameters()
{
  if ( EEPROM.read(eepromVersionAddr) != eepromVersion)
  {
#if defined(debugEeprom)
    Serial.print("invalid eeprom version:0x");
    Serial.println(EEPROM.read(eepromVersionAddr), HEX);
    Serial.println("default coded values will be used");
#endif
    PrintEeprom();
    return;
  }
#if defined(debugEeprom)
  Serial.print("unitGroup:0x");
  Serial.print(EEPROM.read(eepromUnitIdAddr), HEX);
  Serial.print(" unitId:0x");
  Serial.println(EEPROM.read(eepromUnitIdAddr + 1), HEX);
#endif
  bitWrite(diagByte, diagEeprom, 0);
  runningMode = EEPROM.read(eepromLastModeAddr);
  unitGroup = EEPROM.read(eepromUnitIdAddr);
  unitId = EEPROM.read(eepromUnitIdAddr + 1);
  for (uint8_t i = 0; i < tempListSize; i++)
  {
    if ( EEPROM.read(eepromTemperatureListAddr + i) != 0xff)
    {
      temperatureList[i] = EEPROM.read(eepromTemperatureListAddr + i);
#if defined(debugEeprom)
      Serial.print(" t:");
      Serial.print(temperatureList[i]);
#endif
    }
#if defined(debugEeprom)
    Serial.println();
#endif

  }
  for (uint8_t i = 0; i < registerSize; i++)
  {
    thermostatRegister[i] = EEPROM.read(eepromRegistersAddr + i);
#if defined(debugEeprom)
    Serial.print(" reg:");
    Serial.print(thermostatRegister[i]);
#endif
  }
#if defined(debugEeprom)
  Serial.println();
#endif
  for (int i = 0; i < scheduleSize; i++)
  {
    Schedule[i] = EEPROM.read(eepromScheduleAddr + i);
#if defined(debugEeprom)
    Serial.print(" s:");
    Serial.print(Schedule[i]);
#endif
  }
#if defined(debugEeprom)
  Serial.println();
#endif
}

void InitConfiguration()
{
  /*
     must be run once during the installation
  */
#if defined(debugEeprom)
  Serial.println("init eeprom... wait... ");
#endif
  WriteByte(eepromVersionAddr, eepromVersion);
  WriteByte(eepromUnitIdAddr, unitGroup);
  WriteByte(eepromUnitIdAddr + 1, unitId);
  InitTemparatures();
  WriteByte(eepromLastModeAddr, modeWeek);
  WriteByte(eepromRegistersAddr + reactivityRegister, reactivity);
  WriteByte(eepromRegistersAddr + sizeAnticipationRegister, sizeAnticipation);
  WriteByte(eepromRegistersAddr + maximumTemperatureRegister, maximumTemperature);
  WriteByte(eepromRegistersAddr + absenceTemperatureReductionRegister, outHomeTemperatureDecrease);
  WriteByte(eepromRegistersAddr + KpPIDRegister, KpPID);
  WriteByte(eepromRegistersAddr + KiPIDRegister, KiPID);
  WriteByte(eepromRegistersAddr + KdPIDRegister, KpdPID);
  WriteByte(eepromRegistersAddr + thresholdPIDRegister, thresholdPID);
  SaveSchedul();
#if defined(debugEeprom)
  Serial.println("end init eeprom");
#endif
  PrintEeprom();
}
void InitTemparatures()
{
  for (int i = 0; i < tempListSize; i++) {
    WriteByte(eepromTemperatureListAddr + i, temperatureList[i]);
  }
}
void PrintEeprom()
{
  for (int i = 0; i < 300; i++) {
    ReadByte(i);
  }
}
void SaveCurrentMode() {
  WriteByte(eepromLastModeAddr, runningMode);
}
void SaveRegister(int  regId, uint8_t value) {
  if (regId < registerSize && regId >= 0) {
    WriteByte(eepromRegistersAddr + regId, value);
  }
}
void SaveRegisters() {
  WriteByte(eepromRegistersAddr + reactivityRegister, thermostatRegister[reactivityRegister]);
  WriteByte(eepromRegistersAddr + sizeAnticipationRegister, thermostatRegister[sizeAnticipationRegister]);
  WriteByte(eepromRegistersAddr + maximumTemperatureRegister, thermostatRegister[maximumTemperatureRegister]);
  WriteByte(eepromRegistersAddr + absenceTemperatureReductionRegister, thermostatRegister[absenceTemperatureReductionRegister]);
  WriteByte(eepromRegistersAddr + KpPIDRegister, thermostatRegister[KpPIDRegister]);
  WriteByte(eepromRegistersAddr + KiPIDRegister, thermostatRegister[KiPIDRegister]);
  WriteByte(eepromRegistersAddr + KdPIDRegister, thermostatRegister[KdPIDRegister]);
  WriteByte(eepromRegistersAddr + thresholdPIDRegister, thermostatRegister[thresholdPIDRegister]);
}
void SaveSchedul() {
  for (int i = 0; i < scheduleSize; i++) {
    WriteByte(eepromScheduleAddr + i, Schedule[i]);
  }
}

