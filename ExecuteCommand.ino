void ExecuteCommand()
{
#if defined(debugOn)
  Serial.print("command:");
  Serial.print(pendingCommand, HEX);
#endif

  if bitRead(pendingCommand, updateModeBit)
  {
    bitWrite(runningMode, manualAutoModeBit, 0);
    runningMode = pendingCommand & 0x07;
    if (runningMode == modeOff)
    {
      //  digitalWrite(RelayPIN, HeatingPowerOff);
    }
    else {
      //    digitalWrite(RelayPIN, HeatingPowerOn);
    }
  }
  if bitRead(pendingCommand, updateTemperatureBit)
  {
    bitWrite(runningMode, manualAutoModeBit, 1);
    manualModeStartTime = millis();
    if ((pendingCommand & 0x3f) == increaseTemp)
    {
      tempInstruction = min(tempInstruction + .5, float(thermostatRegister[maximumTemperatureRegister]) / 10);
    }
    if ((pendingCommand & 0x3f) == decreaseTemp)
    {
      tempInstruction = max(tempInstruction - .5, float(minimumTemperature) / 10);
    }
  }
  pendingCommand = 0x00;
  ResetLcd(0);
#if defined(debugOn)
  Serial.print(" ");
  Serial.print(runningMode, HEX);
  Serial.print(">");
  Serial.print(tempInstruction);
#endif
}

