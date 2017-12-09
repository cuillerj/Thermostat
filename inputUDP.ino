#define frameNumberBytePosition 0
#define requestResponseBytePosition 2
#define lenBytePosition 4
#define commandBytePosition 7
#define firstDataBytePosition 8
#define ackByte 0x3f
#define cmdAckBitFlag 7
#define crcLen 2

void TraitInput(uint8_t cmdInput) {
  lastReceivedTime = millis();
  bitWrite(diagByte, diagServerConnexion, 0);
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
  Serial.print(" in crc:");
  Serial.print(inputCRC, HEX);
  Serial.print(" CRC:0x");
  Serial.println(computeCRC, HEX);
#endif
  if (inputCRC != computeCRC)
  {
#if defined(debugConnection)
    Serial.println("crc error");
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
#if defined(debugInput)
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
          bitWrite(diagByte, diagExtTemp, 0);
          extTempUpdatedTime = millis();
#if defined(debugInput)
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
      case temperatureListRequest:   // temperature instruction list request
        {
          temperatureList[GatewayLink.DataInSerial[1]] = GatewayLink.DataInSerial[2];
          SendTemperatureList();
          break;
        }
      case scheduleUpdateRequest:   // schedule update request
        {
          uint8_t scheduleID = GatewayLink.DataInSerial[1];  // translation station indicator position to schedule position
          Schedule[scheduleID] = GatewayLink.DataInSerial[2];
          SendUnitarySchedule(scheduleID );
          break;
        }

      case setModeRequest:   // set mode request
        {
          runningMode = GatewayLink.DataInSerial[firstDataBytePosition];
          SendStatus(toAckFrame);
          break;
        }

      case setInstruction:   // set instruction
        {
          if (runningMode != modeOff) {
            bitWrite(runningMode, manualAutoModeBit, 1);
            manualModeStartTime = millis();
            tempInstruction = min(float(GatewayLink.DataInSerial[firstDataBytePosition]) / 10, float(thermostatRegister[maximumTemperatureRegister]) / 10);
            tempInstruction = max(tempInstruction, float(minimumTemperature) / 10);
          }
          SendStatus(toAckFrame);
          break;
        }
      case tracePIDRequest:   // set mode request
        {
#if defined(debugInput)
          Serial.print("tracePIDRequest:");
          Serial.println(GatewayLink.DataInSerial[firstDataBytePosition], HEX);
#endif
          PIDRequest = GatewayLink.DataInSerial[firstDataBytePosition];
          break;
        }
      case setTemporarilyHoldRequest:   // set mode request
        {
          bitWrite(runningMode, temporarilyHoldModeBit, 1);
          previousInstruction = tempInstruction;
          unsigned long deltaTimemsec = (GatewayLink.DataInSerial[firstDataBytePosition]);
          endOfTemporarilyHoldTime = millis() + deltaTimemsec * 60000;
#if defined(debugInput)
          Serial.print("millis/1000:");
          Serial.print(millis() / 1000);
          Serial.print(" wait:");
          Serial.println(endOfTemporarilyHoldTime / 1000);
#endif
          SendStatus(toAckFrame);
          break;
        }
      case setInstructionRequest:        // set temprature request
        {
          bitWrite(runningMode, manualAutoModeBit, 1);
          manualModeStartTime = millis();
          tempInstruction = (float(GatewayLink.DataInSerial[firstDataBytePosition])) / 10;
          SendStatus(toAckFrame);
          break;
        }
      case setSecurityRequest:   // set security
        {
          if (GatewayLink.DataInSerial[firstDataBytePosition] == 0x00)
          {
            securityOn = false;
          }
          if (GatewayLink.DataInSerial[firstDataBytePosition] == 0x01)
          {
            securityOn = true;
          }
          SendStatus(toAckFrame);
          break;
        }
      case updateTemperatureRequest:   // set temperature instrcution
        {
#if defined(debugInput)
          Serial.print("updateTemperatureRequest:");
          Serial.print(GatewayLink.DataInSerial[firstDataBytePosition]);
          Serial.print(" >");
          Serial.println(GatewayLink.DataInSerial[firstDataBytePosition + 1]);
#endif
          if (GatewayLink.DataInSerial[firstDataBytePosition] >= 0 && GatewayLink.DataInSerial[firstDataBytePosition] < tempListSize)
          {
            if ((GatewayLink.DataInSerial[firstDataBytePosition + 1] <= thermostatRegister[maximumTemperatureRegister]) && (GatewayLink.DataInSerial[firstDataBytePosition + 1] >= minimumTemperature))
            {
              temperatureList[GatewayLink.DataInSerial[firstDataBytePosition]] = GatewayLink.DataInSerial[firstDataBytePosition + 1];
            }
          }
          SendTemperatureList();
          break;
        }
      case updateRegisterRequest:   // set register value
        {
#if defined(debugInput)
          Serial.print("updateRegisterRequest: id 0x");
          Serial.print(GatewayLink.DataInSerial[firstDataBytePosition], HEX);
          Serial.print(" value: 0x");
          Serial.println(GatewayLink.DataInSerial[firstDataBytePosition + 1], HEX);
#endif
          if (GatewayLink.DataInSerial[firstDataBytePosition] >= 0 && GatewayLink.DataInSerial[firstDataBytePosition] < registerSize)
          {
            thermostatRegister[GatewayLink.DataInSerial[firstDataBytePosition]] = GatewayLink.DataInSerial[firstDataBytePosition + 1];
          }
          SendRegisters();
          break;
        }
      case uploadScheduleRequest:   // set register value
        {
#if defined(debugInput)
          Serial.println("uploadScheduleRequest");
#endif
          uploadCurrentIdx = 0; // to start upload
          break;
        }
      case uploadTemperatures:   // set register value
        {
#if defined(debugInput)
          Serial.println("uploadTemperatures");
#endif
          SendTemperatureList();
          break;
        }
      case uploadRegisters:   // set register value
        {
#if defined(debugInput)
          Serial.println("uploadRegisters");
#endif
          SendRegisters();
          break;
        }
      case updateSchedulRequest:   // set schedul instrcution
        {
#if defined(debugInput)
          Serial.print("updateSchedulRequest: id 0x");
          Serial.print(GatewayLink.DataInSerial[firstDataBytePosition], HEX);
          Serial.print(" value: 0x");
          Serial.println(GatewayLink.DataInSerial[firstDataBytePosition + 1], HEX);
#endif
          if (GatewayLink.DataInSerial[firstDataBytePosition] >= 0 && GatewayLink.DataInSerial[firstDataBytePosition] < scheduleSize)
          {
            if (((GatewayLink.DataInSerial[firstDataBytePosition + 1] & 0x0f) < tempListSize - 1 ) && (((GatewayLink.DataInSerial[firstDataBytePosition + 1] >> 4) & 0x0f ) < tempListSize - 1))
            {
              Schedule[GatewayLink.DataInSerial[firstDataBytePosition]] = GatewayLink.DataInSerial[firstDataBytePosition + 1];
            }
          }
          SendUnitarySchedule(GatewayLink.DataInSerial[firstDataBytePosition]);
          break;
        }
      case writeEepromRequest:   // write eeprom
        {
#define saveCurrentModeBit 0
#define saveTemperaturesBit 1
#define saveSchedulBit 2
#define saveRegistersBit 3
#if defined(debugInput)
          Serial.print("writeEepromRequest: param 0x");
          Serial.println(GatewayLink.DataInSerial[firstDataBytePosition], HEX);
#endif
          if (bitRead(GatewayLink.DataInSerial[firstDataBytePosition], saveCurrentModeBit))
          {
#if defined(debugInput)
            Serial.println("writeEeprom currentMode");
#endif
            SaveCurrentMode();
          }
          if (bitRead(GatewayLink.DataInSerial[firstDataBytePosition], saveTemperaturesBit))
          {
#if defined(debugInput)
            Serial.println("writeEeprom temperatures");
#endif
            InitTemparatures();
          }
          if (bitRead(GatewayLink.DataInSerial[firstDataBytePosition], saveSchedulBit))
          {
#if defined(debugInput)
            Serial.println("writeEeprom schedule");
#endif
            SaveSchedul();
          }
          if (bitRead(GatewayLink.DataInSerial[firstDataBytePosition], saveRegistersBit))
          {
#if defined(debugInput)
            Serial.println("writeEeprom registers");
#endif
            SaveRegisters();
          }
          break;
        }
    }
  }
}
