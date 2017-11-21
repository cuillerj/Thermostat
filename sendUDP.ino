#define requestFrame true
#define responseFrame false
void SendUnitarySchedule(uint8_t scheduleID)
{

  GatewayLink.PendingDataReqSerial[3] = unitaryScheduleResponse;

  GatewayLink.PendingDataReqSerial[4] = firstScheduleIndicatorPosition + scheduleID; //  translation from schedule poisition to station indicator position
  GatewayLink.PendingDataReqSerial[5] = Schedule[scheduleID];
  FormatFrame(responseFrame, toAckFrame, 0x06);
}
void SendRegister(uint8_t registerId)
{
  GatewayLink.PendingDataReqSerial[3] = registerResponse;
  GatewayLink.PendingDataReqSerial[4] = registerId; //
  GatewayLink.PendingDataReqSerial[5] = thermostatRegister[GatewayLink.DataInSerial[registerId]];
  FormatFrame(responseFrame, toAckFrame, 0x06);
}
void SendRegisters()
{
  GatewayLink.PendingDataReqSerial[3] = registersResponse;
  GatewayLink.PendingDataReqSerial[4] = thermostatRegister[0]; //
  GatewayLink.PendingDataReqSerial[5] = 0x00;
  GatewayLink.PendingDataReqSerial[6] = thermostatRegister[1]; //
  GatewayLink.PendingDataReqSerial[7] = thermostatRegister[2]; //
  GatewayLink.PendingDataReqSerial[8] = 0x00;
  GatewayLink.PendingDataReqSerial[9] = thermostatRegister[3]; //
  GatewayLink.PendingDataReqSerial[10] = thermostatRegister[4]; //
  GatewayLink.PendingDataReqSerial[11] = 0x00;
  GatewayLink.PendingDataReqSerial[12] = thermostatRegister[5]; //
  GatewayLink.PendingDataReqSerial[13] = thermostatRegister[6]; //
  GatewayLink.PendingDataReqSerial[14] = 0x00;
  GatewayLink.PendingDataReqSerial[15] = thermostatRegister[7]; //
  FormatFrame(responseFrame, toAckFrame, 0x10);
}
void SendTemperatureList()
{
  GatewayLink.PendingDataReqSerial[3] = temperatureListResponse;
  GatewayLink.PendingDataReqSerial[4] = temperatureList[0]; //
  GatewayLink.PendingDataReqSerial[5] = 0x00;
  GatewayLink.PendingDataReqSerial[6] = temperatureList[1]; //
  GatewayLink.PendingDataReqSerial[7] = temperatureList[2]; //
  GatewayLink.PendingDataReqSerial[8] = 0x00;
  GatewayLink.PendingDataReqSerial[9] = temperatureList[3]; //
  GatewayLink.PendingDataReqSerial[10] = temperatureList[4]; //
  FormatFrame(responseFrame, toAckFrame, 0x0b);
}
void SendStatus(boolean ack)
{
  GatewayLink.PendingDataReqSerial[3] = statusResponse;
  GatewayLink.PendingDataReqSerial[4] = relayPinStatus;
  GatewayLink.PendingDataReqSerial[5] = 0x00;
  GatewayLink.PendingDataReqSerial[6] = runningMode;
  GatewayLink.PendingDataReqSerial[7] = diagByte;
  GatewayLink.PendingDataReqSerial[8] = 0x00;
  GatewayLink.PendingDataReqSerial[9] = uint8_t(tempInstruction * 10);
  GatewayLink.PendingDataReqSerial[10] = uint8_t(AverageTemp() * 10);
  GatewayLink.PendingDataReqSerial[11] = 0x00;
  GatewayLink.PendingDataReqSerial[12] = ver;
  GatewayLink.PendingDataReqSerial[13] = securityOn;
  FormatFrame(responseFrame, ack, 0x0e);
}

void SendTimeRequest()
{
  /*
     ask to the server for uptodate time
  */
  GatewayLink.PendingDataReqSerial[3] = timeUpdateRequest;
  FormatFrame(requestFrame, noAckFrame, 0x04);
}
void SendExtTempRequest()
{
  /*
     ask to the server for uptodate time
  */
  GatewayLink.PendingDataReqSerial[3] = extTempUpdateRequest;
  FormatFrame(requestFrame, noAckFrame, 0x04);
}
void FormatFrame(boolean request, boolean toAck, uint8_t frameLen)
{
  /*
     format frame according to server protocol
  */
  trameNumber++;
  GatewayLink.PendingDataLenSerial = frameLen + 2;
  GatewayLink.frameSwitch = uint8_t(trameNumber % 256);; //
  GatewayLink.PendingReqSerial = PendingReqRefSerial;
  if (request) {
    bitWrite(GatewayLink.PendingReqSerial, requestFrameBit, 1);
    if (toAck) {
      bitWrite(GatewayLink.PendingReqSerial, toAcknoledgeBit, 1);
    }
  }
  else {
    if (toAck) {
      bitWrite(GatewayLink.PendingReqSerial, ackBit, 1);
    }
  }
  GatewayLink.PendingDataReqSerial[0] = unitGroup;
  GatewayLink.PendingDataReqSerial[1] = unitId;
  GatewayLink.PendingDataReqSerial[2] = 0x00;
  /*
     add 0x00 and CRC8 at the end of the frame
  */
  GatewayLink.PendingDataReqSerial[frameLen] = 0x00;
  GatewayLink.PendingDataReqSerial[frameLen + 1] = CRC8(GatewayLink.PendingDataReqSerial , frameLen);
#if defined(debugConnection)
  Serial.print("format CRC:0x");
  Serial.println(CRC8(GatewayLink.PendingDataReqSerial , frameLen), HEX);
#endif
}
void SendAckFrame()
{
  GatewayLink.frameSwitch = uint8_t(trameNumber % 256);; //
  GatewayLink.PendingReqSerial = PendingReqRefSerial;
  GatewayLink.PendingDataReqSerial[0] = unitGroup;
  GatewayLink.PendingDataReqSerial[1] = unitId;
  GatewayLink.PendingDataReqSerial[3] = GatewayLink.DataInSerial[firstDataBytePosition];
  GatewayLink.PendingDataLenSerial = 0x06;
}

