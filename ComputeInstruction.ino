void ComputeInstruction() {
  /*
     compute temperature request
  */
  int schedulIndex = 0;
  int f = 0;
  DateTime now = RTC.now();
  uint8_t DemiH = 0x00;
  if (bitRead(runningMode, manualAutoModeBit))
  {
    return;
  }
  switch (runningMode)
  {
    case modeOff:    // mode off
      {
        digitalWrite(RelaisPIN, 0);
        break;
      }
    case modeDay1:
      {
        schedulIndex = (7 * 24 + now.hour());
        Serial.println(Schedule[schedulIndex], HEX);
        Serial.println((now.minute() >= 30));
        break;
      }
    case modeDay2:
      {
        schedulIndex = (8 * 24 + now.hour());
        break;
      }
    case modeNotFreezing:
      {
        tempInstruction = temperatureList[notFreezeIdx] / 10;
        break;
      }
  }
}
