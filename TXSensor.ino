#include <MANCHESTER.h>
#include <dht11.h>

#define DHT11PIN          2
#define TXPin             1
#define LedPin            0
#define PowerPin          3

dht11 DHT11;

#define SensorNumber      1
#define SensorType        1	 // 1 Temp/Hum, 2 Current

long UpdateMillis;
const long UpdateRate = 300000;  // Every 5 Minutes



void setup() 
{  
  pinMode(DHT11PIN, INPUT);
  pinMode(PowerPin, OUTPUT);
  pinMode(LedPin, OUTPUT);
  pinMode(TXPin, OUTPUT);
  MANCHESTER.SetTxPin(TXPin);
  UpdateMillis=0; 		 // For TX right after booting
}



void loop()
{
  if (millis()-UpdateMillis>UpdateRate)
  {
    for (int x=0; x<2; x++)
    {
      float t,h;
      digitalWrite(PowerPin, HIGH);
      delay(250);
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
        MANCHESTER.Transmit((SensorNumber*1000) + (SensorType*10000) + data);
        digitalWrite(LedPin, LOW);
        delay(200);
      }
      data = (int)h;
      if (data != 0)
      {
        digitalWrite(LedPin, HIGH);
        MANCHESTER.Transmit((SensorNumber*1000) + ((SensorType*10000)+10000) + data);
        digitalWrite(LedPin, LOW);
      }
      delay(200);
      digitalWrite(PowerPin, LOW);
      UpdateMillis=millis();
    }
  }
}




// Will be continued....


// Testing for sleep. Input mode drains less current

void sleep()
{
  pinMode(LedPin, INPUT);
  pinMode(DHT11PIN, INPUT);
  pinMode(TXPin, INPUT);
  pinMode(PowerPin, INPUT);
}




// Return pins on their required state

void wakeup()
{
  pinMode(TXPin, OUTPUT);
  pinMode(LedPin, OUTPUT);
  pinMode(PowerPin, OUTPUT);  
}
