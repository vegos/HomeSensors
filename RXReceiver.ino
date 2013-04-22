#include "MANCHESTER.h"
#include <SPI.h>
#include <Ethernet.h>


byte mac[] = {  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
byte ip[] = { 192, 168, 0, 50 };
byte subnet[] = { 255, 255, 255, 0 };
byte gateway[] = { 192, 168, 0, 1 };

EthernetServer server(80);



boolean FirstTimeTemp=true;
boolean FirstTimeHum=true;
boolean FirstTimeCur=true;
char* Description[10] = { "", "Sensor 1", "Sensor 2", "Sensor 3", "Sensor 4", "Sensor 5", "Sensor 6", "Sensor 7", "Sensor 8", "Sensor 9" };
volatile int SensorType[10], CurrentTemp[10], CurrentHum[10], MaxTemp[10], MaxHum[10], MinTemp[10], MinHum[10], CurrentCurrent[10], MaxCurrent[10], MinCurrent[10], AverageTemp, AverageHum, AverageCurrent;

int TotalRXTemp, TotalRXHum = 0;
int TotalClients = 0;
 
 
 
 
void setup()
{
  Serial.begin(9600);
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
  Serial.println("");
  Serial.println("Ready!");
  Serial.println("");
}
  
  
  
  
void loop()
{
  if (MANRX_ReceiveComplete()) 
  {
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
      AverageTemp=(AverageTemp+CurrentTemp[Sensor])/2;
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
      AverageHum=(AverageHum+CurrentHum[Sensor])/2;
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
    if ((FirstTimeTemp) && (CurrentTemp[Sensor] != 0))
    {
      MaxTemp[Sensor] = CurrentTemp[Sensor];
      MinTemp[Sensor] = CurrentTemp[Sensor];
      AverageTemp = CurrentTemp[Sensor];
      FirstTimeTemp = false;
    }
    if ((FirstTimeHum) && (CurrentHum[Sensor] != 0))
    {
      MaxHum[Sensor] = CurrentHum[Sensor];
      MinHum[Sensor] = CurrentHum[Sensor];
      AverageHum = CurrentHum[Sensor];
      FirstTimeHum = false;
    }
    if ((FirstTimeCur) && (CurrentCurrent[Sensor] != 0))
    {
      MaxCurrent[Sensor] = CurrentCurrent[Sensor];
      MinCurrent[Sensor] = CurrentCurrent[Sensor];
      AverageCurrent = CurrentCurrent[Sensor];
      FirstTimeCur = false;
    }
    if (CurrentTemp[Sensor]>MaxTemp[Sensor])
      MaxTemp[Sensor] = CurrentTemp[Sensor];
      
    if (CurrentHum[Sensor]>MaxHum[Sensor])
      MaxHum[Sensor] = CurrentHum[Sensor];
      
    if (CurrentTemp[Sensor]<MinTemp[Sensor])
      MinTemp[Sensor] = CurrentTemp[Sensor];
      
    if (CurrentHum[Sensor]<MinHum[Sensor])
      MinHum[Sensor] = CurrentHum[Sensor];
      
    if (CurrentCurrent[Sensor]>MaxCurrent[Sensor])
      MaxCurrent[Sensor] = CurrentCurrent[Sensor];
      
    if (CurrentCurrent[Sensor]<MinCurrent[Sensor])
      MinCurrent[Sensor] = CurrentCurrent[Sensor];
  }
  
 ListenForEthernetClients();
}




// Lister for ethernet client -- Display all data from sensors

void ListenForEthernetClients()
{
  EthernetClient client = server.available();
  if (client)
  {
    Serial.println("- Ethernet client connected!");
    Serial.println("  Sending web page...");
    while (client.connected())
    {
      boolean currentLineIsBlank = true;
      if (client.available())
      {
        char c = client.read();
        if (c == '\n' && currentLineIsBlank) 
        {
          TotalClients += 1;
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connnection: close");
          client.println();
          client.println("<html>");
          client.println("<font face=\"Tahoma, Arial, Helvetica\" size=\"4\">");
          client.println("<BODY>");
          client.println(":: <b>Wireless Sensors Monitoring</b> ::<BR><BR><BR>");
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
                client.print("Current Temperature: <font color=red><b>");
                client.print(CurrentTemp[x]);
                client.println("</b>&deg;C</font><br>");
                client.print("Max Temp: ");
                client.print(CurrentTemp[x]);
                client.println("&deg;C<br>");
                client.print("Min Temp: ");
                client.print(MinTemp[x]);
                client.println("&deg;C<br><br>");
                client.print("Current Humidity: <font color=red><b>");
                client.print(CurrentHum[x]);
                client.println("</b>%</font><br>");
                client.print("Max Hum: ");
                client.print(MaxHum[x]);
                client.println("%<br>");
                client.print("Min Hum: ");
                client.print(MinHum[x]);
                client.println("%<br><br>");
                break;
              case 2:  // Current
                client.print("Current Current: <font color=red><b>");
                client.print(CurrentCurrent[x]);
                client.println("</b>mA</font><br>");
                client.print("Max Current: ");
                client.print(CurrentCurrent[x]);
                client.println("mA<br>");
                client.print("Min Current: ");
                client.print(MinCurrent[x]);
                client.println("mA<br><br>");
                break;                
            }
          }            
          client.println("<BR><BR>");
          client.print("<i>Messages Received: ");
          client.print(TotalRXTemp+TotalRXHum);
          client.println("<BR>");
          client.print("Total Web Clients: ");
          client.print(TotalClients);
          client.println("<BR>");
          client.print("Average Temp: ");
          client.print(AverageTemp);
          client.println("&deg;C<br>");
          client.print("Average Hum: ");
          client.print(AverageHum);
          client.println("%</i><br><br>");
          client.println("</body>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
    Serial.println("- Client disconnected.");
  }
}  
