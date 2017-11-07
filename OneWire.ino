void ReadTemperature()
{
  /*
     read DS1820 sensor and store value in lastTemp array
  */
  temperatureLastReadTime = millis();
  uint8_t data[12];
  float celsius;
  ds.reset();
  ds.select(ds1820Addr);
  ds.write(0x44, 0);        // start conversion, with parasite power on at the end
  // delay(750);     // maybe 750ms is enough
  // we might do a ds.depower() here, but the reset will take care of it.

  ds.reset();
  ds.select(ds1820Addr);
  ds.write(0xBE);         // Read Scratchpad
  for ( int i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];

  byte cfg = (data[4] & 0x60);
  // at lower res, the low bits are undefined, so let's zero them
  if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
  else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
  else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  //// default is 12 bit resolution, 750 ms conversion time
  celsius = (float)raw / 16.0;
#if defined(debugOn)
  Serial.print("iT:");
  Serial.print(celsius);
    Serial.print(";");
#endif
  lastTemp[averageTempCount % (lastTempSize)] =  celsius ;
  averageTempCount++;
}

boolean SearchDS1820Addr()
{
  // uint8_t addr[8] = {0x28, 0xFF, 0x95, 0x71, 0xB5, 0x16, 0x03, 0x27};
  if ( !ds.search(ds1820Addr)) {
#if defined(debugOn)
    Serial.println("No more addresses.");
    Serial.println();
#endif
    ds.reset_search();
    delay(250);
    return;
  }

}

