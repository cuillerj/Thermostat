uint8_t ReadByte(int address)
{
#if defined(debugOn)
  Serial.print("eeprom ");
  Serial.print(address);
  Serial.print(":");
  Serial.println(EEPROM.read(address), HEX);
#endif
  return EEPROM.read(address);
}
uint8_t WriteByte(int address, uint8_t value)
{
  EEPROM.update(address, value);
  delay(5);
}
void InitConfiguration()
{
  /*
     must be run once during the installation
  */
  WriteByte(eepromVersionAddr, eepromVersion);
  WriteByte(eepromUnitIdAddr, unitGroup);
  WriteByte(eepromUnitIdAddr + 1, unitId);
  for (int i = 0; i < tempListSize; i++) {
    WriteByte(eepromTemperatureListAddr + i, temperatureList[i]);
  }
  for (int i = 0; i < scheduleSize; i++) {
    WriteByte(eepromDS1820Addr + i, ds1820Addr[i]);
  }
  WriteByte(eepromRegistersAddr + reactivityRegister, reactivity);
  WriteByte(eepromRegistersAddr + sizeAnticipationRegister, sizeAnticipation);
  WriteByte(eepromRegistersAddr + maximumTemperatureRegister, maximumTemperature);
  WriteByte(eepromRegistersAddr + absenceTemperatureReductionRegister, absenceTemperatureReduction);
  WriteByte(eepromRegistersAddr + KpPIDRegister, KpPID);
  WriteByte(eepromRegistersAddr + KiPIDRegister, KiPID);
  WriteByte(eepromRegistersAddr + KpdPIDRegister, KpdPID);
  WriteByte(eepromRegistersAddr + thresholdPIDRegister, thresholdPID);
  for (int i = 0; i < scheduleSize; i++) {
    WriteByte(eepromScheduleAddr + i, Schedule[i]);
  }
  PrintEeprom();
}
void PrintEeprom()
{
  for (int i = 0; i < 100; i++) {
    ReadByte(i);
  }
}

