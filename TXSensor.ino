#include <MANCHESTER.h>
#include <dht11.h>
#include <ATTinyWatchdog.h>
#include <avr/power.h>
#include <avr/interrupt.h>


#define DHT11PIN          2
#define TXPin             1
#define LedPin            0
#define PowerPin          3


dht11 DHT11;

#define SensorNumber      1
#define SensorType        1    // 1 Temp/Hum, 2 Current



void setup() 
{  
  MANCHESTER.SetTxPin(TXPin);
//  UpdateMillis=0;
    // Setup watchdog to notify us every 4 seconds
  ATTINYWATCHDOG.setup(8);
  // Turn off subsystems which we aren't using  
  power_timer0_disable();
  // timer1 used by MANCHESTER
  power_usi_disable();
  // ADC used for reading a sensor
  // ATTINYWATCHDOG turns off ADC before sleep and
  // restores it when we wake up
  pinMode(DHT11PIN, INPUT);
  pinMode(PowerPin, OUTPUT);
  pinMode(LedPin, OUTPUT);
  pinMode(TXPin, OUTPUT);
}



void loop()
{
  WakeUp();
  delay(10);
  SendMessage();
  Sleep();
  DeepSleep(70); // 4 minutes, 40 seconds
}


void SendMessage()
{
  for (int x=0; x<2; x++)
  {
    float t,h;
    digitalWrite(PowerPin, HIGH);
    delay(50);
    int chk = DHT11.read(DHT11PIN);
    if (chk==DHTLIB_OK)
    { 
      t = DHT11.temperature;
      h = DHT11.humidity;
    }
    unsigned int data = (int)t;
    if (data != 0)
    {
      digitalWrite(LedPin, HIGH);
      //                    eg 2*1000=2000 + 10000 for temp + 25c = 12025
      MANCHESTER.Transmit((SensorNumber * 1000) + (SensorType * 10000) + data);
      digitalWrite(LedPin, LOW);
      delay(50);
    }
    data = (int)h;
    if (data != 0)
    {      
      digitalWrite(LedPin, HIGH);
      //                    eg 2*1000=2000 + 1+1 (for hum) * 10000 = 20000 for hum + 44% = 22044
      MANCHESTER.Transmit((SensorNumber*1000) + ((SensorType+1) * 10000) + data);
      digitalWrite(LedPin, LOW);
    }
    delay(50);
    digitalWrite(PowerPin, LOW);
  }  
}


void Sleep()
{
  pinMode(LedPin, INPUT);
  pinMode(DHT11PIN, INPUT);
  pinMode(TXPin, INPUT);
  pinMode(PowerPin, INPUT);
}

void WakeUp()
{
  pinMode(TXPin, OUTPUT);
  pinMode(LedPin, OUTPUT);
  pinMode(PowerPin, OUTPUT);  
}

void DeepSleep(unsigned int multiple)
{  
  Sleep();
  // deep sleep for multiple * 4 seconds
  ATTINYWATCHDOG.sleep(multiple);
  WakeUp();
}
