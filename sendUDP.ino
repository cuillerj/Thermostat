#define unitaryScheduleResponse 0x02
#define registerResponse 0x04
#define temperatureListResponse 0x03
#define requestFrame true
#define responseFrame false
#define toAckFrame true
#define noAckFrame false
/*
  #define request 0x01
  #define response 0x00
  #define toAck 0x01
  #define noAck 0x00
*/
void SendUnitarySchedule(uint8_t scheduleID, uint8_t value)
{
  GatewayLink.PendingReqSerial = PendingReqRefSerial;
  GatewayLink.PendingDataReqSerial[0] = unitId;
  //  GatewayLink.PendingDataReqSerial[2] = unitaryScheduleRequest;
  GatewayLink.PendingDataReqSerial[3] = firstScheduleIndicatorPosition + scheduleID; //  translation from schedule poisition to station indicator position
  GatewayLink.PendingDataReqSerial[4] = Schedule[scheduleID];
  GatewayLink.PendingDataLenSerial = 0x06;
}
void SendRegister(uint8_t registerId)
{
  GatewayLink.PendingReqSerial = PendingReqRefSerial;
  GatewayLink.PendingDataReqSerial[0] = unitId;
  GatewayLink.PendingDataReqSerial[2] = registerResponse;
  GatewayLink.PendingDataReqSerial[3] = registerId; //
  GatewayLink.PendingDataReqSerial[4] = thermostatRegister[GatewayLink.DataInSerial[registerId]];

  GatewayLink.PendingDataLenSerial = 0x06;
}
void SendTemperatureList()
{
  GatewayLink.PendingReqSerial = PendingReqRefSerial;
  GatewayLink.PendingDataReqSerial[0] = unitId;
  GatewayLink.PendingDataReqSerial[2] = temperatureListResponse;
  GatewayLink.PendingDataReqSerial[3] = temperatureList[0]; //
  GatewayLink.PendingDataReqSerial[4] = 0x00;
  GatewayLink.PendingDataReqSerial[5] = temperatureList[1]; //
  GatewayLink.PendingDataReqSerial[6] = temperatureList[2]; //
  GatewayLink.PendingDataReqSerial[7] = 0x00;
  GatewayLink.PendingDataReqSerial[8] = temperatureList[3]; //
  GatewayLink.PendingDataReqSerial[9] = temperatureList[4]; //
  GatewayLink.PendingDataLenSerial = 0x0a;
}
void SendStatus()
{

}
void SendTimeRequest()
{
  /*
     ask to the server for uptodate time
  */
  GatewayLink.PendingDataReqSerial[3] = timeUpdate;
  FormatFrame(requestFrame, noAckFrame, 0x04);
}
void SendExtTempRequest()
{
  /*
     ask to the server for uptodate time
  */
  GatewayLink.PendingDataReqSerial[3] = extTempUpdate;
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
  Serial.print("send CRC:0x");
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

