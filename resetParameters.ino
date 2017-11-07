void setDefaultValues()
{
#if defined(debugOn)
  Serial.println("setDefaultValues");
#endif
  updateEeprom(eepromVersionAddr, eepromVersion);
  updateEeprom(eepromUnitIdAddr, unitId);
}
void updateEeprom(int addr, uint8_t value)
{
  if ( EEPROM.read(addr) != value)
  {
    EEPROM.write(addr, value);
    delay(10);
  }
}

