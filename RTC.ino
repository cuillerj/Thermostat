char prevDate[12] = "xx xx 20xx";
char prevTime[8] = "xx:xx:xx";

boolean RTCAdjustime(char DateToInit[12], char TimeToInit[8]) {
  boolean retCode = true;
  DateTime now = RTC.now();
  RTC.adjust(DateTime(DateToInit, TimeToInit));
  if (IsCurrentDateTimeOk()) {
    for (int i = 0; i < 12; i++) {
      prevDate[i] = DateToInit[i];
    }
    for (int i = 0; i < 8; i++) {
      prevTime[i] = TimeToInit[i];
    }
  }
  else {
    RTC.adjust(DateTime(prevDate, prevTime));
    retCode = false;
  }
  return retCode;
}
boolean IsCurrentDateTimeOk() {
  boolean retCode = true;
  DateTime now = RTC.now();
  if (now.year() == 2165) {
    retCode = false;;
  }
  if (now.month() < 0 || now.month() > 12) {

  }
  if (now.day() < 0 || now.day() > 31) {
    retCode = false;;
  }
  if (now.hour() < 0 || now.hour() > 24) {
    retCode = false;;
  }
  if (now.minute() < 0 || now.minute() > 60) {
    retCode = false;;
  }
  if (now.second() < 0 || now.second() > 60) {
    retCode = false;;
  }
  return retCode;
}
