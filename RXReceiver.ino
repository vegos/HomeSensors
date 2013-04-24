#include "MANCHESTER.h"
#include <SPI.h>
#include <Ethernet.h>
#include <MemoryFree.h>

byte mac[] = {  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
byte ip[] = { 192, 168, 0, 105 };
byte subnet[] = { 255, 255, 255, 0 };
byte gateway[] = { 192, 168, 0, 1 };

EthernetServer server(80);



boolean FirstTimeTemp=true;
boolean FirstTimeHum=true;
boolean FirstTimeCur=true;
char* Description[10] = { "", "Sensor 1", "Sensor 2", "Sensor 3", "Sensor 4", "Sensor 5", "Sensor 6", "Sensor 7", "Sensor 8", "Sensor 9" };
volatile int SensorType[10], CurrentTemp[10], CurrentHum[10], CurrentCurrent[10];

int TotalRXTemp, TotalRXHum = 0;
int TotalWebClients = 0;
 
long LastTransmission;

 
 
void setup()
{
  Serial.begin(9600);
  Serial.print("- Initializing variables...");
  for (int x=0; x<10; x++)
  {
    SensorType[x]=0;
    CurrentTemp[x]=0;
    CurrentHum[x]=0;
    CurrentCurrent[x]=0;
  }
  Serial.println(" [OK]");  
  Serial.print("- Starting Network...");
  SPI.begin();
  Ethernet.begin(mac, ip, gateway, subnet);
  Serial.println(" [OK]");
  Serial.print("- Starting Server...");
  server.begin();
  Serial.println(" [OK]");
  Serial.print("  Server running @ ");
  Serial.println(Ethernet.localIP());  
  Serial.print("- Starting RX...");
  MANRX_SetRxPin(A5);
  MANRX_SetupReceive();
  Serial.println(" [OK]");
  MANRX_BeginReceive();  
  Serial.print("Free Memory: ");
  Serial.println(freeMemory());
  Serial.println("");
  Serial.println("Ready!");
  Serial.println("");
}
  
  
  
  
void loop()
{
  if (MANRX_ReceiveComplete()) 
  {
Serial.print("Transmission difference in millis: ");
Serial.println(millis()-LastTransmission);
    LastTransmission = millis();
    Serial.print("- Incomming transmission (#");
    Serial.print(TotalRXTemp+TotalRXHum+1);
    Serial.println(")");
    unsigned int data = MANRX_GetMessage();
    MANRX_BeginReceive();
    Serial.print("  Raw Data: ");
    Serial.println(data);
    // decode data
    int Sensor = (data % 10000) / 1000;        // XYZZZ where X: 1 Temp, 2 Hum, Y: Sensor (1..9), ZZZ = Data (000..999)
    int Type = (data / 10000);
    int RawData = data % 1000;
    TotalRXTemp += 1;
    Serial.print("  Sensor: ");
    Serial.println(Sensor);
    Serial.print("  Type: ");
    Serial.println(Type);
    if (Type == 1)  // Temperature
    {
      SensorType[Sensor]=1;
      CurrentTemp[Sensor]=RawData;
      Serial.print("  ");
      Serial.print(Description[Sensor]);
      Serial.print(" -- Temperature: ");
      Serial.print(CurrentTemp[Sensor]);
      Serial.println("C");
    }
    if (Type == 2)  // Humidity
    {
      SensorType[Sensor]=1;
      CurrentHum[Sensor]=RawData;
      Serial.print("  Sensor #");
      Serial.print(Sensor);
      Serial.print(" / ");
      Serial.print(Description[Sensor]);
      Serial.print(" -- Humidity: ");
      Serial.print(CurrentHum[Sensor]);
      Serial.println("%");
    }
    if (Type == 3)  // Current
    {
      SensorType[Sensor]=2;
      CurrentCurrent[Sensor]=RawData;
      Serial.print("  Sensor #");
      Serial.print(Sensor);
      Serial.print(" / ");
      Serial.print(Description[Sensor]);
      Serial.print(" -- Current: ");
      Serial.print(CurrentCurrent[Sensor]);
      Serial.println("mA");
    }

    Serial.println("- End of transmission.");
  }
  
 ListenForEthernetClients();
}




// Lister for ethernet client -- Display all data from sensors

void ListenForEthernetClients()
{
  EthernetClient client = server.available();
  if (client)
  {
    boolean currentLineIsBlank = true;
    Serial.println("- Ethernet client connected!");
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        if ((c == '\n') && currentLineIsBlank)
        {
          Serial.println("  Sending web page...");          
          TotalWebClients += 1;
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connnection: close");
          client.println();

	  // Create a remark line. It's used by script running on Linux Server for parsing the data to cacti...          
          client.print("<!-- DATA,");
          for (int x=1; x<10; x++)
          {
            client.print(x);
            client.print(",");
            client.print(Description[x]);
            client.print(",");
            client.print(SensorType[x]);
            client.print(",");
            client.print(CurrentTemp[x]);
            client.print(",");
            client.print(CurrentHum[x]);
            client.print(",");
            client.print(CurrentCurrent[x]);
            client.print(",");
          }
          client.println("END -->");
          client.println("<html>");
          client.println("<font face=\"Tahoma, Arial, Helvetica\" size=\"4\">");
          client.println("<BODY>");
          client.println(":: <b>Magla Home Monitoring</b> ::<BR><BR><BR>");
          for (int x=1; x<10; x++)
          {
            client.print("<b><font color=blue>Sensor #");
            client.print(x);
            client.print(" -- ");
            client.print(Description[x]);
            client.println("</font></b><br><br>");
            switch (SensorType[x])
            {
              case 1:  // Temp or Hum
                client.print("Temperature: <font color=red><b>");
                client.print(CurrentTemp[x]);
                client.println("</b>&deg;C</font><br>");
                client.print("Humidity: <font color=red><b>");
                client.print(CurrentHum[x]);
                client.println("</b>%</font><br><br>");
                break;
              case 2:  // Current
                client.print("Current: <font color=red><b>");
                client.print(CurrentCurrent[x]);
                client.println("</b>mA</font><br><br>");
                break;                
            } // Case Closed
          }
          client.println("<BR><BR>");
          client.print("<i>Messages Received: ");
          client.print(TotalRXTemp+TotalRXHum);
          client.println("<BR>");
          client.print("Last Transmission was ");
          int secs = ((millis()-LastTransmission)/1000) % 60;
          int mins = ((millis()-LastTransmission)/60000) % 60;
          client.print(mins);
          client.print(" minutes and ");
          client.print(secs);
          client.println(" seconds ago.<BR>");
          client.print("Total Web Clients: ");
          client.print(TotalWebClients);
          client.println("<BR><BR>");
          client.println("</body>");
          client.println("</html>");
          break;
        } // If closed
        if (c == '\n')
          currentLineIsBlank = true;
        else 
          if (c != '\r')
            currentLineIsBlank = false;
      }
    }
    delay(1);
    client.stop();
    Serial.println("- Client disconnected.");
  }
}  
