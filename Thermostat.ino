
#define Version "Th"
#define ver 0x01 // version

#define debugConnection true     // can only be set with ATmega2560 or ATmega1280
#define debugOn true     // can only be set with ATmega2560 or ATmega1280
#define debugDS1820 true
#define debugEeprom true
/*
   V0 refonte en cours de pilotage chaudiere (   remplacement de la communication RF par WIFI gateway ESP8266,   suppression du module DHT, ajout d'une fonction ecriture eeprom,
   prevoir traiter info event externes (alarme, ouverture porte & fenetre...)...

*/
#include <EEPROM.h>
#include <OneWire.h>
#include <RTClib.h>
#include <TimeLib.h>
#include <LiquidCrystal_I2C.h>
#include <C:\Users\jean\Documents\Arduino\libraries\ArduinoIRremotemaster\IRremote.h>
#include <C:\Users\jean\Documents\Arduino\libraries\ArduinoIRremotemaster\IRremoteInt.h>
#include <C:\Users\jean\Documents\Arduino\libraries\ArduinoIRremotemaster\IRremote.cpp>
/*
  communication parameters used for exchanges with ESP8266 Wifi Gateway
*/
#include <SerialLink.h>
#define receiveFlagPosition 0
#define PendingReqRefSerial  0x01; // 0x01 request to the gateway means route (ready to eventualy add some more action in the gateway)
#define toAckFrame true
#define noAckFrame false
//uint8_t PendingSecReqRefSerial = 0x01; // 0x01 request to the gateway means route
byte cycleRetrySendUnit = 0; // cycle retry check unitary command - used in case of acknowledgement needed from the Linux server
uint8_t trameNumber = 0;     // frame number to send
uint8_t lastAckTrameNumber = 0;  // last frame number acknowledged by the server
uint8_t pendingAckSerial = 0;    // flag waiting for acknowledged
int retryCount = 0;            // number of retry for sending
uint8_t pendingRequest;
uint8_t gatewayStatus = 0x00;    // 0x00 not ready  0x01 ready GPIO on wait boot completed 0x02 ready
boolean securityOn = false;

#include <thermostatDefines.h> // contient les parametres de config initiale
#include <thermostatInstructions.h> // contient les parametres de config initiale
#include <thermostatSchedule.h> // contient les parametres de schedull initiale
SerialLink GatewayLink(gatewayLinkSpeed);   // define the object link to the gateway
/*
    pending command description
*/
#define increaseTemp 0x01
#define decreaseTemp 0x02

uint8_t pendingCommand = 0x00;

/*
   registers
*/
uint8_t thermostatRegister[registerSize] = {reactivity, sizeAnticipation, maximumTemperature, outHomeTemperatureDecrease, KpPID, KiPID, KpdPID, thresholdPID};

/*
    diagnostics
*/
uint8_t diagByte = diagInitMask;
#define bootStepsNumber 3
uint8_t bootMode = bootStepsNumber;
uint8_t prevDiagByte = diagByte;
boolean relayPinStatus = false;

int uploadCurrentIdx = -1;

/*
    commands
*/
#define updateModeBit 7
#define updateTemperatureBit 6
/*
   temperature variables
*/
#define manualAutoModeBit 7
#define temporarilyHoldModeBit 6
#define noTempAvailable -100
uint8_t runningMode = modeOff; //
float tempInstruction = temperatureList[modeNotFreezing] / 10; // init to no freezze threshold
float previousInstruction = tempInstruction;
float refTempRelayOn;
int prevTarget = 0;
int extTemp = noTempAvailable;

/*
   temperature sensor
*/
OneWire  ds(oneWirePIN);  // on pin 8 (a 4.7K resistor is necessary)
uint8_t ds1820Addr[8];


/*
    LCD
*/
uint8_t LCDLoopValue = 0x00;

uint8_t WorkAnticip[7];  // zone de travail anticipation consigne

/*
     PID
*/
#define PIDDataSize 5
int dataPID[PIDDataSize] = {0, 0, 0, 0, 0}; // temperature * 10
uint8_t PIDCycle = 0x00;
int sigmaPrec = 99999; //
int sigmaE = 0;
int windowSize = 0;
uint8_t statPID = 0x00; // statut resultat du PID
uint8_t switchPID = 0x00;
/*
   timers
*/

#define pendingTimeout 60000
#define updateClockCycle 120000
#define delayBetweenInfo 5000   // delay before sending new data to the server  
#define temperatureReadCycle 30000
#define PIDComputeCycle 240000
#define LCDRefreshCycle 5000
#define instructionRefreshCycle 15000
#define extTempRefreshCycle 300000
#define manualMaxDuration 7200000    //2h
#define extTempDuration 10800000     //3h
#define autoSaveModeDuration 43200000    // 24h
#define hysteresisDelay 120000
#define PIDCyclDuration 120000
#define sendStatusCycle 300000
#define completeBootDuration 180000
#define uploadIntervalDuration 10000
#define connexionTimeout 1800000  // 30mn

unsigned long durationSincegatewayReady;  //
unsigned long lastUpdateClock;
unsigned long pendingRequestClock;
unsigned long timeSendSecSerial;  // used to check for acknowledgment of secured frames
unsigned long timeReceiveSerial;  // used to regularly check for received message
unsigned long timeSendInfo;     // to regurarly send information to th server
unsigned long temperatureLastReadTime;
unsigned long PIDLastComputedTime;
unsigned long LCDLastRefreshTime;
unsigned long instructionRefreshTime;
unsigned long extTempRefreshTime;
unsigned long extTempUpdatedTime;
unsigned long manualModeStartTime = 0;
unsigned long changeModeTime;
unsigned long timeSwitchOnOff;
unsigned long PIDCycleTime;
unsigned long sendStatusTime;
unsigned long uploadlastTime;
unsigned long lastReceivedTime;
unsigned long endOfTemporarilyHoldTime;
#define MonthList "JanFebMarAprMayJunJulAugSepOctNovDec"  // do not change can not be localized
RTC_DS1307 RTC;
LiquidCrystal_I2C lcd(LCDAddr, LCDChars, LCDLines); // set the LCD address to 0x27 for a 16 chars and 2 line display
IRrecv irrecv(InfraRedPIN);
decode_results results;
/*

*/
#define lastTempSize 5
float lastTemp[lastTempSize];
uint8_t averageTempCount = 0;

void setup() {
  /*
     initialize serial link
     arduino to gateway serial link will be serial with Uno and serial2 with Mega
  */

#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)   // with mega serial remains available 
  Serial.begin(38400);
#endif
  LoadParameters();
  pinMode(GatewayReadyPin, INPUT);
  pinMode(InfraRedPIN, INPUT);
  pinMode(configPIN, INPUT);
//  pinMode(configWifiPIN, INPUT);
  pinMode(RelayPIN, OUTPUT);
  pinMode(GatewayRelayPin, OUTPUT);
 // pinMode(GatewayConfigPin, OUTPUT);
  Wire.begin();
  RTC.begin();
  DateTime now = RTC.now();
  lcd.init();                      // initialize the lcd
  lcd.backlight();
  lcd.print(now.hour());
  lcd.print(":");
  lcd.print(now.minute());
  lcd.setCursor(0, 1);
#if defined(debugOn)
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.println(now.minute(), DEC);
#endif

  digitalWrite(GatewayRelayPin, 0);   // power on gateway NC
  /*
     digitalWrite(GatewayConfigPin, !digitalRead(configWifiPIN));   // power on gateway NC relay
    if (digitalRead(configWifiPIN))
    {
      Serial.begin(38400);
      Serial.println("ConfigWifi Mode");
      ConfigWifi();
      Serial.end();
    }
    else {
  */
  GatewayLink.SerialBegin();
  // }
  uint8_t retry = 0;
  bitWrite(diagByte, diagDS1820, 1);
  while (retry < 5 && bitRead(diagByte, diagDS1820))
  {
    SearchDS1820Addr();
    if (ds1820Addr != 0x0000000000000000)
    {
      bitWrite(diagByte, diagDS1820, 0);
    }
    retry++;
  }
  if (  bitRead(diagByte, diagDS1820))
  {
    runningMode = modeOff;
  }
#if defined(debugDS1820)
  Serial.print("addr =");
  for (int i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(ds1820Addr[i], HEX);
  }
  Serial.println();
#endif
  if (digitalRead(configPIN))
  {
    Serial.print("InitConfiguration");
    InitConfiguration();
  }

  delay(20000);
  ReadTemperature();                // init the temperature table
  
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  delay(10);
  ReceiveIR();
  if (bootMode > 0 && (millis() > completeBootDuration) && !  bitRead(diagByte, diagServerConnexion)) {
    if (bootMode == 3) {
#if defined(debugOn)
      Serial.println("send temp");
#endif
      bootMode--;
      SendTemperatureList();
    }
    if (bootMode == 2 && millis() > completeBootDuration + 30000) {
#if defined(debugOn)
      Serial.println("send reg");
#endif
      bootMode--;
      SendRegisters();
    }
    if (bootMode == 1 && millis() > completeBootDuration + 60000) {
      bootMode--;
      uploadCurrentIdx = 0; // to start upload
#if defined(debugOn)
      Serial.println("start upload sched");
#endif
    }
  }
  if (millis() - lastReceivedTime > connexionTimeout) {
    bitWrite(diagByte, diagServerConnexion, 1);
  }
  if (millis() - LCDLastRefreshTime > LCDRefreshCycle)
  {
    LCDRefresh();
    LCDLastRefreshTime = millis();
  }
  if (pendingCommand != 0x00)
  {
    ExecuteCommand();
  }
  /*
       send and receive loop
  */
  if (digitalRead(GatewayReadyPin) == 1) {
    // ***  keep in touch with the server
    int getSerial = GatewayLink.Serial_have_message();  // check if we have received a message
    if (getSerial > 0)                                  // we got a message
    {
#if defined(debugConnection)
      Serial.println("receive something");
      for (int i = 0; i <= 15; i++)
      {
        Serial.print("0x");
        Serial.print(GatewayLink.DataInSerial[i], HEX);
        Serial.print("-");
      }
      Serial.println();
#endif
      TraitInput(GatewayLink.DataInSerial[receiveFlagPosition]);          // analyze the message
    }
    if (GatewayLink.PendingReqSerial != 0x00)           // check if we have a message to send (or a message currently sending)
    {
      GatewayLink.DataToSendSerial();                    // send message on the serial link
      timeSendSecSerial = millis();                      // reset timer
    }
    if ( millis() - timeSendSecSerial >= 5000 && pendingAckSerial != 0x00 && retryCount < 5) {
      GatewayLink.ReSendSecuSerial();                    // re-send data that was not already ack by the server
#if defined(debugConnection)
      Serial.println("retry");
#endif
      timeSendSecSerial = millis();         // clear timer
      retryCount = retryCount + 1;          // update rerty count
    }
    if (retryCount >= 5)
    {
      retryCount = 0;
      pendingAckSerial = 0x00;
    }
  }
  if (digitalRead(GatewayReadyPin) == 1 && pendingRequest == 0x00 && ((millis() - lastUpdateClock > updateClockCycle ) || bitRead(diagByte, diagTimeUpToDate)))
  {
    SendTimeRequest();
    pendingRequest = timeUpdateRequest;
    pendingRequestClock = millis();
  }
  if (millis() - pendingRequestClock > pendingTimeout)
  {
    pendingRequest = 0x00;
  }
  if (millis() - temperatureLastReadTime > temperatureReadCycle)
  {
    ReadTemperature();
#if defined(debugOn)
    Serial.print("aT:");
    Serial.println(AverageTemp());
#endif
  }
  if (millis() - PIDLastComputedTime > PIDComputeCycle)
  {
    PIDLastComputedTime = millis();
    //   PIDCycle = (PIDCycle + 1) ;
    // sigmaPrec = sigmaE;
    SwitchPID();
  }
  if (millis() - instructionRefreshTime > instructionRefreshCycle)
  {
    ComputeInstruction();
    Executeinstruction();
    instructionRefreshTime = millis();
  }
  if ((millis() - extTempRefreshTime > extTempRefreshCycle) || (extTempRefreshTime == 0 && digitalRead(GatewayReadyPin) == 1 && pendingRequest == 0x00))
  {
    SendExtTempRequest();
    extTempRefreshTime = millis();
  }
  if (millis() - extTempUpdatedTime > extTempDuration)
  {
    bitWrite(diagByte, diagExtTemp, 1);
  }

  if (manualModeStartTime != 0 && millis() - manualModeStartTime > autoSaveModeDuration)
  {
    SaveCurrentMode();                // save the last running mode as default
    manualModeStartTime = 0;
  }

  if (bitRead(runningMode, manualAutoModeBit) && millis() - manualModeStartTime > manualMaxDuration) {
    bitWrite(runningMode, manualAutoModeBit, 0);
    SendStatus(toAckFrame);
    sendStatusTime = millis();
  }
  if (bitRead(runningMode, temporarilyHoldModeBit) && millis() > endOfTemporarilyHoldTime) {
    bitWrite(runningMode, temporarilyHoldModeBit, 0);
    if (bitRead(runningMode, manualAutoModeBit))
    {
      tempInstruction = previousInstruction;
    }
    SendStatus(toAckFrame);
    sendStatusTime = millis();
  }
  if (millis() - sendStatusTime > sendStatusCycle)
  {
    SendStatus(noAckFrame);
    sendStatusTime = millis();
  }
  if ((uploadCurrentIdx >= 0 && uploadCurrentIdx <  scheduleSize) && (millis() - uploadlastTime > uploadIntervalDuration))
  {
#if defined(debugOn)
    Serial.print(" upload sched:");
    Serial.println(uploadCurrentIdx);
#endif
    UploadSchedul();
  }
}

float AverageTemp()
{
  /*
     mitigate temperature variations by computing the last temperatures average
  */
  uint8_t idx = 0;
  float averageTemp = 0;
  while (idx < lastTempSize)
  {
    averageTemp = averageTemp + lastTemp[idx];
    if (lastTemp[idx] == 0)
    {
      break;
    }
    idx++;
  }
  if (idx != 0)
  {
    return averageTemp / (idx);
    bitWrite(diagByte, diagDS1820, 0);
  }
  else {
    bitWrite(diagByte, diagDS1820, 1);
    return -noTempAvailable;
  }
}

void UploadSchedul() {
  uploadlastTime = millis();
  SendUnitarySchedule(uploadCurrentIdx);
  uploadCurrentIdx++;
}


































































































































































































































