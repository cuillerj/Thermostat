// not used
void ConfigWifi()
{
  String ssid = "";
  String psw = "";
  uint8_t toValidate = 0x00;
  while (digitalRead(configWifiPIN))
  {
    delay(10);
  //  Serial.println(toValidate, HEX);
    if ((toValidate & 0x0f) == 0x03)
    {
      Serial.println("validate: yes or no ");
      bitWrite(toValidate, 2, 1);
    }
    if (Serial.available() > 0)
    {
      // read the incoming string:
      String incomingString = Serial.readString();
      Serial.print("received: ");
      Serial.println(incomingString);

      if (incomingString.substring(0, 5) == "ssid=")
      {
        ssid = incomingString.substring(5);
        bitWrite(toValidate, 0, 1);
        Serial.print(ssid);
      }


      if (incomingString.substring(0, 4) == "psw=")
      {
        psw = incomingString.substring(4);
        bitWrite(toValidate, 1, 1);
        Serial.print(psw);
      }


      if ((toValidate & 0x0f) == 0x07 && incomingString.substring(0, 3) == "yes")
      {
        Serial.print("update ");
        bitWrite(toValidate, 3, 1);
      }

      if ((toValidate & 0x0f) == 0x07 && incomingString.substring(0, 2) == "no")
      {
        Serial.print("cancel ");
        psw = "";
        ssid = "";
        toValidate = 0x00;
      }
    }
  }
}

