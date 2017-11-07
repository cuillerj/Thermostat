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
    Serial.println(target);
#endif
    int proportionalTerm = (KpPID * (target - dataPID[PIDCycle]));
    for (int i = 0; i < PIDDataSize; i++) {
      sigmaE = sigmaE + target - dataPID[i];
    }
    if (prevTarget != int(tempInstruction * 10))
    {
      sigmaPrec = sigmaE;
      prevTarget = int(tempInstruction * 10);
    }
    int derivativeTerm = ((sigmaE - sigmaPrec) * KpPID);
    int integralTerm = (sigmaE * KiPID) / 5;
    windowSize = (proportionalTerm + integralTerm + derivativeTerm) / 10;
#if defined(debugOn)
    Serial.println(windowSize);
#endif
  }
}
