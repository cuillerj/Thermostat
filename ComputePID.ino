void ComputePid()
{
  if (dataPID[0] == 0 && AverageTemp() != noTempAvailable) // first time init dataPID
  {
#if defined(debugOn)
    Serial.println("init PID Data");
#endif
    for (int i = 0; i < PIDDataSize; i++)
    {
      dataPID[i] = int(AverageTemp() * 10); // init
    }
  }
  sigmaE = 0;
  if (AverageTemp() != noTempAvailable)
  {
    dataPID[PIDCycle % PIDDataSize] = int(AverageTemp() * 10);
    int target = int(tempInstruction * 10);
#if defined(debugOn)
    Serial.print("PID cycle:");
    Serial.print(PIDCycle % PIDDataSize);
    Serial.print(" data:");
    Serial.print(dataPID[PIDCycle % PIDDataSize]);
    Serial.print(" target:");
    Serial.print(target);
    Serial.print(" switch:");
    Serial.print(switchPID);
#endif
    int proportionalTerm = (thermostatRegister[KpPIDRegister] * (target - dataPID[PIDCycle]));
    for (int i = 0; i < PIDDataSize; i++) {
      sigmaE = sigmaE + target - dataPID[i];
    }
    if (prevTarget != int(tempInstruction * 10))
    {
      sigmaPrec = sigmaE;
      prevTarget = int(tempInstruction * 10);
    }
    int derivativeTerm = ((sigmaE - sigmaPrec) * thermostatRegister[KdPIDRegister]);
    int integralTerm = (sigmaE * KiPIDRegister) / 5;
    windowSize = (proportionalTerm + integralTerm + derivativeTerm) / 10;
#if defined(debugOn)
    Serial.print(" window:");
    Serial.println(windowSize);
#endif
  }
}

void SwitchPID ()
{
  switch (switchPID)
  {
    case 0x00:  // calcul integrale 1 cyle sur 2 soit environ toutes les 4 mn
      PIDCycle = (PIDCycle + 1) % 5;
      sigmaPrec = sigmaE;
      switchPID = 0x01;
      break;
    case 0x01:
      switchPID = 0x00;
      break;
  }
}
