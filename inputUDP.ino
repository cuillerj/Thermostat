#define frameNumberBytePosition 0
#define requestResponseBytePosition 2
#define lenBytePosition 4
#define commandBytePosition 7
#define firstDataBytePosition 8
#define ackByte 0x3f
#define cmdAckBitFlag 7
#define crcLen 2
#define timeUpdateResponse 0x01
#define extTempUpdateResponse 0x02
#define temperatureUpdateRequest 0x01
#define scheduleUpdateRequest 0x02
#define updateRegisterRequest 0x03

void TraitInput(uint8_t cmdInput) {
  if (GatewayLink.DataInSerial[requestResponseBytePosition] == ackByte)   // is it an aknoledgment ?
  {
    lastAckTrameNumber = GatewayLink.DataInSerial[frameNumberBytePosition];
    if (lastAckTrameNumber == trameNumber)
    {
      pendingAckSerial = 0x00;
#if defined(debugConnection)
      Serial.println("no ack pending");
#endif
    }
    return;
  }
  uint8_t inputCRC = GatewayLink.DataInSerial[GatewayLink.DataInSerial[lenBytePosition] - 1];
  uint8_t computeCRC = CRC8(&GatewayLink.DataInSerial[commandBytePosition] , GatewayLink.DataInSerial[lenBytePosition] - commandBytePosition - crcLen);
#if defined(debugConnection)
  Serial.print("frame len:");
  Serial.print(GatewayLink.DataInSerial[lenBytePosition], HEX);
  Serial.print(" crc:");
  Serial.println(inputCRC, HEX);
  Serial.print("CRC:0x");
  Serial.println(computeCRC, HEX);
#endif
  if (inputCRC != computeCRC)
  {
#if defined(debugConnection)
    Serial.print("crc error");
#endif
    return;
  }

  if (bitRead(GatewayLink.DataInSerial[requestResponseBytePosition], cmdAckBitFlag)) // does this frame to be acknoledge ?
  {
    SendAckFrame();
  }
  if (!bitRead(GatewayLink.DataInSerial[requestResponseBytePosition], commandBytePosition)) // this is a response ?
  {
#if defined(debugConnection)
    Serial.print("response: 0x");
    Serial.println(GatewayLink.DataInSerial[commandBytePosition], HEX);
#endif
    switch (GatewayLink.DataInSerial[commandBytePosition]) {
      case timeUpdateResponse:
        {
          /*
            convert byte input to char date and time to setup clock
            input[0] id j number, [1] month number, [2] year number minus 2000, [3] hour number, [4]minute number, [5] second number
          */
          char DateToInit[15] = "xxx xx 20xx";
          char TimeToInit[9] = "xx: xx: xx";
          if (GatewayLink.DataInSerial[firstDataBytePosition + 1] > 0 && GatewayLink.DataInSerial[firstDataBytePosition + 1] < 13)
          {
            DateToInit[0] = MonthList[3 * (GatewayLink.DataInSerial[firstDataBytePosition + 1] - 1)];
            DateToInit[1] = MonthList[3 * (GatewayLink.DataInSerial[firstDataBytePosition + 1] - 1) + 1];
            DateToInit[2] = MonthList[3 * (GatewayLink.DataInSerial[firstDataBytePosition + 1] - 1) + 2];
            DateToInit[4] = uint8_t(GatewayLink.DataInSerial[firstDataBytePosition ] / 10 + 48);
            DateToInit[5] = uint8_t((GatewayLink.DataInSerial[firstDataBytePosition ] - (GatewayLink.DataInSerial[firstDataBytePosition ] / 10) * 10) + 48);
            DateToInit[9] = uint8_t(GatewayLink.DataInSerial[firstDataBytePosition + 3] / 10 + 48); //
            DateToInit[10] = uint8_t((GatewayLink.DataInSerial[firstDataBytePosition + 3] - (GatewayLink.DataInSerial[firstDataBytePosition + 3] / 10) * 10) + 48);
            TimeToInit[0] = uint8_t(GatewayLink.DataInSerial[firstDataBytePosition + 4] / 10 + 48);
            TimeToInit[1] = uint8_t((GatewayLink.DataInSerial[firstDataBytePosition + 4] - (GatewayLink.DataInSerial[firstDataBytePosition + 4] / 10) * 10) + 48);
            TimeToInit[3] = uint8_t(GatewayLink.DataInSerial[firstDataBytePosition + 6] / 10 + 48);
            TimeToInit[4] = uint8_t((GatewayLink.DataInSerial[firstDataBytePosition + 6] - (GatewayLink.DataInSerial[firstDataBytePosition + 6] / 10) * 10) + 48);
            TimeToInit[6] = uint8_t(GatewayLink.DataInSerial[firstDataBytePosition + 7] / 10 + 48);
            TimeToInit[7] = uint8_t((GatewayLink.DataInSerial[firstDataBytePosition + 7] - (GatewayLink.DataInSerial[firstDataBytePosition + 7] / 10) * 10) + 48);
            RTC.adjust(DateTime(DateToInit, TimeToInit));
            lastUpdateClock = millis();
            bitWrite(diagByte, diagTimeUpToDate, 0);
#if defined(debugOn)
            Serial.print("set time ");
            DateTime now = RTC.now();
            Serial.print(now.year(), DEC);
            Serial.print('/');
            Serial.print(now.month(), DEC);
            Serial.print('/');
            Serial.print(now.day(), DEC);
            Serial.print(' ');
            Serial.print(now.hour(), DEC);
            Serial.print(':');
            Serial.print(now.minute(), DEC);
            Serial.print(':');
            Serial.println(now.second(), DEC);
#endif
          }
          break;
        }
      case extTempUpdateResponse:
        {
          extTemp = GatewayLink.DataInSerial[firstDataBytePosition + 1] & 0x7f;
          if (GatewayLink.DataInSerial[firstDataBytePosition] == 0x2d)
          {
            extTemp = -extTemp;
          }
#if defined(debugOn)
          Serial.print("ext temp: ");
          Serial.println(extTemp);
#endif
          break;
        }
    }
  }
  else  // this is a command
  {
#if defined(debugConnection)
    Serial.print("request: 0x");
    Serial.println(GatewayLink.DataInSerial[commandBytePosition], HEX);
#endif
    switch (GatewayLink.DataInSerial[commandBytePosition]) {
      case temperatureUpdateRequest:   // temperature instruction update request
        {
          temperatureList[GatewayLink.DataInSerial[1]] = GatewayLink.DataInSerial[2];
          SendTemperatureList();
          break;
        }
      case scheduleUpdateRequest:   // schedule update request
        {
          uint8_t scheduleID = GatewayLink.DataInSerial[1] - firstScheduleIndicatorPosition;  // translation station indicator position to schedule position
          Schedule[scheduleID] = GatewayLink.DataInSerial[2];
          SendUnitarySchedule(scheduleID + 50, Schedule[scheduleID]);
          break;
        }
      case updateRegisterRequest:   // update register request
        {
          thermostatRegister[GatewayLink.DataInSerial[1]] = GatewayLink.DataInSerial[2];
          SendRegister(GatewayLink.DataInSerial[1]);
          break;
        }

    }
  }
}
