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
      digitalWrite(GatewayRelayPin, 0);
    }
    else{
      digitalWrite(GatewayRelayPin, 1);      
    }
  }
  if bitRead(pendingCommand, updateTemperatureBit)
  {
    bitWrite(runningMode, manualAutoModeBit, 1);
    manualModeStartTime=millis();
    if ((pendingCommand & 0x3f) == increaseTemp)
    {
      tempInstruction = min(tempInstruction + .5, maximumTemperature);
    }
    if ((pendingCommand & 0x3f) == decreaseTemp)
    {
      tempInstruction = max(tempInstruction - .5, 0);
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

