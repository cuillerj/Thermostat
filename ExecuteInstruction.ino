void Executeinstruction() {
#if defined(debugOn)
  Serial.print("Executeinstruction:");
  Serial.println(relayPinStatus);
#endif
  if (thermostatRegister[thresholdPIDRegister] != 255) {      // test PID usage
    ComputePid();
    if (AverageTemp() != 0) {
      unsigned long deltaTime = thermostatRegister[hysteresisDelayRegister];
      deltaTime = deltaTime * 6000;
      if (millis() - timeSwitchOnOff > deltaTime)    // temps mini entre 2 changements de relais on off pour eviter histerersis
      {
        if (windowSize > thresholdPID && !relayPinStatus)
        {
#if defined(debugOn)
          Serial.print("PID On:");
          Serial.println(windowSize);
#endif
          //     statPID = 0x01;
          timeSwitchOnOff = millis();
          digitalWrite(RelayPIN, HeatingPowerOn);
          relayPinStatus = true;
          refTempRelayOn = AverageTemp();         // save temperature to check increase
          SendStatus(toAckFrame);
          sendStatusTime = millis();
          return;
        }
        if (windowSize <= thresholdPID && relayPinStatus)
        {
#if defined(debugOn)
          Serial.print("PID Off:");
          Serial.println(windowSize);
#endif
          //       statPID = 0x00;
          timeSwitchOnOff = millis();
          digitalWrite(RelayPIN, HeatingPowerOff);
          relayPinStatus = false;
          SendStatus(toAckFrame);
          sendStatusTime = millis();
          return;
        }
      }
      else {
#if defined(debugOn)
        Serial.print("wait for hysteresis:");
        Serial.println(thermostatRegister[hysteresisDelayRegister]);
      }

#endif
    }
  }
  else  {
    if (AverageTemp() != 0) {
      if (AverageTemp() < tempInstruction - .2 && !relayPinStatus) {
#if defined(debugOn)
        Serial.println("1on");
#endif
        digitalWrite(RelayPIN, HeatingPowerOn);
        relayPinStatus = true;
        timeSwitchOnOff = millis();
        refTempRelayOn = AverageTemp();         // save temperature to check increase
        return;
      }
      if (AverageTemp() > tempInstruction + .1 && relayPinStatus) {
#if defined(debugOn)
        Serial.println("1of");
#endif
        digitalWrite(RelayPIN, HeatingPowerOff);
        relayPinStatus = false;

        //       tempRefRelaisOn = 0;

        return;
      }
    }
  }
}
