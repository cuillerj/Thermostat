void ReceiveIR() {
  boolean newCommand = false;

  if (irrecv.decode(&results)) {
#if defined(debugOn)
    Serial.println(results.value, HEX);
#endif
    irrecv.resume(); // Receive the next value
    newCommand = true;
  }
  if (results.value == IRIncreaseTemp && newCommand) { // "+"
#if defined(debugOn)
    Serial.println(" increase temp");
#endif
    pendingCommand = increaseTemp;
    newCommand = false;
    bitWrite(pendingCommand, updateTemperatureBit, 1);
    return;
  }
  if (results.value == IRDecreaseTemp && newCommand) { //"-"
#if defined(debugOn)
    Serial.println(" decreaseTemp");
#endif
    pendingCommand = decreaseTemp;
    newCommand = false;
    bitWrite(pendingCommand, updateTemperatureBit, 1);
    return;
  }
  if (results.value == IRSWitchOff && newCommand) {
#if defined(debugOn)
    Serial.println(" switchOff");
#endif
    pendingCommand = modeOff;
    newCommand = false;
    bitWrite(pendingCommand, updateModeBit, 1);
  }
  if (results.value == IRSelectModeWeek && newCommand) {
#if defined(debugOn)
    Serial.println(" select week mode");
#endif
    pendingCommand = modeWeek;
    newCommand = false;
    bitWrite(pendingCommand, updateModeBit, 1);
    manualModeStartTime = millis();
  }
  if (results.value == IRSelectModeDay1 && newCommand) { //"-"
#if defined(debugOn)
    Serial.println(" day1 mode");
#endif
    pendingCommand = modeDay1;
    newCommand = false;
    bitWrite(pendingCommand, updateModeBit, 1);
    manualModeStartTime = millis();
  }
  if (results.value == IRSelectModeDay2 && newCommand) { //"-"
#if defined(debugOn)
    Serial.println(" day2 mode");
#endif
    pendingCommand = modeDay2;
    newCommand = false;
    bitWrite(pendingCommand, updateModeBit, 1);
    manualModeStartTime = millis();
  }
  if (results.value == IRSelectModeNotFreezing && newCommand) { //"-"
#if defined(debugOn)
    Serial.println(" not freezing mode");
#endif
    pendingCommand = modeNotFreezing;
    newCommand = false;
    bitWrite(pendingCommand, updateModeBit, 1);
    manualModeStartTime = millis();
  }
}

