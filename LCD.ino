void LCDRefresh() {

  LCDLoopValue++;
  DateTime now = RTC.now();
  lcd.clear();
  lcd.noBlink();
  lcd.print(now.hour());
  lcd.print(":");
  lcd.print(now.minute());
  lcd.setCursor(6, 1);
  lcd.print(AverageTemp(), 1);
  lcd.setCursor(0, 1);
  if (runningMode != modeOff) {
    lcd.print(tempInstruction, 1);
    if (bitRead(runningMode, temporarilyHoldModeBit) || bitRead(runningMode, manualAutoModeBit)) { // hold mode
      if (bitRead(runningMode, temporarilyHoldModeBit)) {  // forced mode
        lcd.print("h");
      }
      else {
        lcd.print("f");
      }
    }
    else {
      if (securityOn) {
        lcd.print("s");
      }
      else {
        lcd.print("!");
      }
    }

  }
  else {
    //  lcd.print("Off ");
  }
  lcd.setCursor(13, 1);
  if (extTemp >= 0)
  {
    lcd.print("+");
  }
  else {
    lcd.print("-");
  }
  if (extTemp != noTempAvailable)
  {
    lcd.print(abs(extTemp)); // meteo
  }

  lcd.setCursor(6, 0);

  switch (LCDLoopValue % 3) {
    case 0:
      if (diagByte != 0x00) {
        lcd.blink();
        lcd.print(" err:0x");
        lcd.print(diagByte, HEX);
      }
      else {
        lcd.print(now.day());
        lcd.print("/");
        lcd.print (now.month());
        lcd.print("/");
        lcd.print (now.year());
      }
      break;
    case 1:
      lcd.print(runningModeKeyWord[runningMode & 0x07]);
      /*
        switch (runningMode)
        {
          case 0x00:
            lcd.print(outservString);
            break;
          case 0x01:
            lcd.print(calendarString);
            break;
          case 0x02:
            lcd.print(holidayString);
            break;
          case 0x03:
            lcd.print(notFreezingString);
            break;
        }
      */
      break;

    case 2:
      lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
      lcd.print(" ");
      lcd.print(now.day());
      lcd.print("/");
      lcd.print(now.month());
      return;
      /*
        lcd.print(" Ex:");
        if (bitRead(Diag,0) == 0){    // meteo a jour
          lcd.print(char(MeteoSign));
          lcd.print(char(MeteoTempC1));
          lcd.print(char(MeteoTempC2));
          lcd.print(" ");
          lcd.print(char(MeteoHumC1));
          lcd.print(char(MeteoHumC2));
        }
        lcd.print("%");
        }
        lcd.setCursor(0, 1);
        lcd.print (TempLCDFloat,1);
        lcd.print(" ");

        //  lcd.print(weekday(now.day()));
        lcd.print(ModeConsigne);
        if (IndForce==0){
        lcd.print("C");
        }
        else {
        lcd.print("F");
        }
        lcd.print(TempConsigne,1);
      */
  }
}
void  ResetLcd(uint8_t loopPosition)
{
#define lcdDelay 200
  LCDLoopValue = loopPosition;
  LCDLastRefreshTime = millis() - LCDRefreshCycle + lcdDelay;
}
