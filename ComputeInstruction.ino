void ComputeInstruction() {
  /*
     compute temperature request
  */

  DateTime now = RTC.now();
  if (bitRead(runningMode, temporarilyHoldModeBit))
  {
    tempInstruction = float(minimumTemperature) / 10;
    if (bitRead(runningMode, manualAutoModeBit) )
    {
      return;
    }
    //  digitalWrite(RelayPIN, 0);
    return;
  }
  if (bitRead(runningMode, manualAutoModeBit) )
  {
    return;
  }

  switch (runningMode)
  {
    case modeOff:    // mode off
      {
        digitalWrite(RelayPIN, HeatingPowerOff);
        relayPinStatus = false;
        tempInstruction = 0;
        break;
      }
    case modeDay1:
      {
        schedulIndex = (7 * 24 + now.hour());
#if defined(debugOn)
        Serial.print("day1 idx:");
        Serial.print(Schedule[schedulIndex], HEX);
        Serial.print(" flag:");
        Serial.print((now.minute() >= 30));
        Serial.print(" instr:");
        Serial.println(AnticipationConsigne(schedulIndex, (now.minute() >= 30)));
#endif
        tempInstruction = AnticipationConsigne(schedulIndex, (now.minute() >= 30)) ;
        if (securityOn && !bitRead(runningMode, manualAutoModeBit))
        {
          tempInstruction = tempInstruction - float(outHomeTemperatureDecrease) / 10;
        }
        break;
      }
    case modeDay2:
      {
        schedulIndex = (8 * 24 + now.hour());
#if defined(debugOn)
        Serial.print("day2 idx:");
        Serial.print(Schedule[schedulIndex], HEX);
        Serial.print(" flag:");
        Serial.print((now.minute() >= 30));
        Serial.print(" instr:");
        Serial.println(AnticipationConsigne(schedulIndex, (now.minute() >= 30)));
#endif
        tempInstruction = AnticipationConsigne(schedulIndex, (now.minute() >= 30)) ;
        if (securityOn && !bitRead(runningMode, manualAutoModeBit))
        {
          tempInstruction = tempInstruction - float(outHomeTemperatureDecrease) / 10;
        }
        break;
      }
    case modeWeek:
      {
        schedulIndex = ((now.dayOfTheWeek()) * 24 + now.hour());
#if defined(debugOn)
        Serial.print("week idx:");
        Serial.print(Schedule[schedulIndex], HEX);
        Serial.print(" flag:");
        Serial.print((now.minute() >= 30));
        Serial.print(" instr:");
        Serial.println(AnticipationConsigne(schedulIndex, (now.minute() >= 30)));
#endif
        tempInstruction = AnticipationConsigne(schedulIndex, (now.minute() >= 30)) ;
        if (securityOn && !bitRead(runningMode, manualAutoModeBit))
        {
          tempInstruction = tempInstruction - float(outHomeTemperatureDecrease) / 10;
        }
        break;
      }
    case modeNotFreezing:
      {
        tempInstruction = temperatureList[notFreezeIdx] / 10;
        break;
      }
  }
}
float AnticipationConsigne(int schedulIndex, boolean halfHour) {
#define defaultExtTemp 10
#define minReactivity 0.1
#define adjustCoef 0.9
  int WorkMeteo = defaultExtTemp;
  uint8_t anticipWork[7];
  float SchedTemp;
  float WorkReacChauff;
  if (halfHour) {
    SchedTemp = temperatureList[Schedule[schedulIndex] & 0x0F];
    anticipWork[0] = temperatureList[Schedule[schedulIndex] & 0x0F];
    anticipWork[1] = temperatureList[Schedule[schedulIndex + 1] % (scheduleSize - 1) >> 4 & 0x0F];
    anticipWork[2] = temperatureList[Schedule[schedulIndex + 1] % (scheduleSize - 1) & 0x0F];
    anticipWork[3] = temperatureList[Schedule[schedulIndex + 2] % (scheduleSize - 1) >> 4 & 0x0F];
    anticipWork[4] = temperatureList[Schedule[schedulIndex + 2] % (scheduleSize - 1) & 0x0F];
    anticipWork[5] = temperatureList[Schedule[schedulIndex + 3] % (scheduleSize - 1) >> 4 & 0x0F];
    anticipWork[6] = temperatureList[Schedule[schedulIndex + 3] % (scheduleSize - 1) & 0x0F];
  }
  else {
    SchedTemp = temperatureList[Schedule[schedulIndex] >> 4 & 0x0F];
    anticipWork[0] = temperatureList[Schedule[schedulIndex] >> 4 & 0x0F];
    anticipWork[1] = temperatureList[Schedule[schedulIndex] & 0x0F];
    anticipWork[2] = temperatureList[Schedule[schedulIndex + 1] % (scheduleSize - 1) >> 4 & 0x0F];
    anticipWork[3] = temperatureList[Schedule[schedulIndex + 1] % (scheduleSize - 1) & 0x0F];
    anticipWork[4] = temperatureList[Schedule[schedulIndex + 2] % (scheduleSize - 1) >> 4 & 0x0F];
    anticipWork[5] = temperatureList[Schedule[schedulIndex + 2] % (scheduleSize - 1) & 0x0F];
    anticipWork[6] = temperatureList[Schedule[schedulIndex + 3] % (scheduleSize - 1) >> 4 & 0x0F];
  }
#if defined(debugOn)
  Serial.print("work:");
  for (int i = 0; i < sizeAnticipation; i++)
  {
    Serial.print(anticipWork[i]);
    Serial.print("-");
  }
  Serial.println();
#endif
  /*
    if (SchedTemp>ruptTemp && ruptToSend==0x00)
    {
    deltaTemp=SchedTemp-ruptTemp;
    ruptTemp=SchedTemp;
    ruptToSend=0x03;
    }
    if (ruptToSend==0x00)
    {
    ruptTemp=SchedTemp;
    }
  */

  if (!bitRead(diagByte, diagExtTemp)) { // meteo uptodate
    WorkMeteo = extTemp;
  }

  float deltaTemp = (WorkMeteo - defaultExtTemp);
  int signdeltaTemp = (deltaTemp / abs(deltaTemp));
  float newReac = thermostatRegister[reactivityRegister];
  newReac = newReac / 100;
  float workCoef = adjustCoef;
  if (deltaTemp > 0)
  {
    workCoef = 1 / adjustCoef;
  }
  WorkReacChauff = max(newReac + (newReac * deltaTemp / defaultExtTemp) * workCoef, minReactivity);
#if defined(debugOn)
  Serial.print(" wr:");
  Serial.print(WorkReacChauff, 4);
  Serial.print(" ");
  Serial.print(deltaTemp);
  Serial.print(" ");
#endif
  DateTime now = RTC.now();
  int min30 = now.minute() % 30; // pour calcul du shift dans la tranche de 30 minutes
  int actualTemp = AverageTemp() * 10;
  for (int i = 1; i < sizeAnticipation; i++) {
    if ((anticipWork[i] - actualTemp) > (WorkReacChauff * (i - 1) * 10 + WorkReacChauff * 10 * (30 - min30) / 30) ) { //
      int prevSchedTemp = SchedTemp;
      SchedTemp = max(SchedTemp, (actualTemp + (anticipWork[i] - actualTemp) * 0.9) - 1); //
      if (abs(SchedTemp - prevSchedTemp) < 2)
      {
        SchedTemp = prevSchedTemp;
      }
      return (SchedTemp / 10);
    }
  }

  return (SchedTemp / 10);
}
