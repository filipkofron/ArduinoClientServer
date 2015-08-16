#include <SPI.h>
#include <RH_RF22.h>

RH_RF22 rf22_driver;

PROGMEM const char * const initFailed = "[Init]: Failed.";
PROGMEM const char * const initSuccess = "[Init]: Success.";
PROGMEM const char * const recvErr = "[Recv]: Error.";
PROGMEM const char * const separ = " ";
PROGMEM const char * const separVal = ":";
PROGMEM const char * const preRssi = " RSSI: ";
PROGMEM const char * const preRemote = " REMOTE:";
PROGMEM const char * const preId = " id: ";
PROGMEM const char * const preMyLastId = " myLastId: ";
PROGMEM const char * const preDeltaT = " deltaT: ";
PROGMEM const char * const preText = " text: \"";
PROGMEM const char * const afterText = "\"";
PROGMEM const char * const sent = ": Sent.";
PROGMEM const char * const text = "This is server!";

#define BUF_LEN RH_RF22_MAX_MESSAGE_LEN

unsigned long lastRemoteTime = 0;
unsigned long lastInBlink = 0;
unsigned long blinkOff = 0;
int8_t lastRssi = 0;
unsigned long count = 0;

void setup() 
{
  Serial.begin(9600);
  Serial.setTimeout(2);
  pinMode(7, OUTPUT);

  if (rf22_driver.init())
  {
    Serial.println(initSuccess);
    rf22_driver.setModemConfig(RH_RF22::GFSK_Rb2_4Fd36);
    rf22_driver.setTxPower(RH_RF22_TXPOW_17DBM);
  }
  else
  {
    Serial.println(initFailed);  
  }
}

struct Message
{
  unsigned long remoteTime;
  unsigned long id;
  unsigned long yourId;
  int8_t rssi;
  char text[BUF_LEN - 2 * sizeof(unsigned long) - sizeof(int8_t)];
} __attribute__ ((packed));

void blink(bool success)
{
  if(success)
  {
    blinkOff = 50;
    lastInBlink = millis();
    digitalWrite(7, HIGH);
  }
  if(blinkOff)
  {
    unsigned long diff = lastInBlink - millis();
    if(diff > blinkOff)
      blinkOff = 0;
    else
      blinkOff -= diff;
  }
  else
  {
    digitalWrite(7, LOW);
  }
  lastInBlink = millis();
}

void loop()
{
  uint8_t buf[BUF_LEN];
  Message *msg = (Message *) buf;
  uint8_t len = sizeof(buf);
  unsigned long timeMillis = millis();

  memset(buf,0,BUF_LEN);
  
  if (rf22_driver.waitAvailableTimeout(100))
  {
    bool recvOk = rf22_driver.recv(buf, &len);
    buf[len - 1] = '\0';

    if (recvOk)
    {
      lastRssi = rf22_driver.lastRssi();
      unsigned long deltaT = msg->remoteTime - lastRemoteTime;
      lastRemoteTime = msg->remoteTime;

      Serial.print(timeMillis); Serial.print(separVal);
      Serial.print(preRssi); Serial.print(lastRssi, DEC);
      Serial.print(preRemote);
      Serial.print(preId); Serial.print(msg->id, DEC);
      Serial.print(preDeltaT); Serial.print(deltaT, DEC);
      Serial.print(preMyLastId); Serial.print(msg->yourId, DEC);
      Serial.print(preRssi); Serial.print(msg->rssi, DEC);
      Serial.print(preText); Serial.println(msg->text);

      blink(true);

      memset(buf,0,BUF_LEN);
    
      msg->remoteTime = timeMillis;
      msg->rssi = lastRssi;
      msg->id = count;
      msg->yourId = msg->id;
      memcpy(msg->text, text, strlen(text) + 1);
      rf22_driver.send(buf, BUF_LEN);
      rf22_driver.waitPacketSent();

      count++;
    }
    else
    {
      Serial.println(recvErr);
    }
  }

  blink(false);  
}


