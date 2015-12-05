/*
 * Women Safety Wearable 
 * Author : Geeve George
 * Instrubtables : http://www.instructables.com/member/Geeve+George/
 */

 


#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiServer.h>
#include <LWiFiClient.h>

#define WIFI_AP "Your Wifi SSID"
#define WIFI_PASSWORD "Your Wifi Password"
#define WIFI_AUTH LWIFI_WPA  

int serverPort = 80;
LWiFiServer server(serverPort);
int LED = 13;

#include <Wire.h>

#include <rgb_lcd.h>

#include <LGPS.h>

gpsSentenceInfoStruct info;
char buff[256];

rgb_lcd lcd;


int ButtonPin = 7;
int BuzzerPin = 2;

double latitude;
double longitude;

int buttonState;


static unsigned char getComma(unsigned char num,const char *str)
{
  unsigned char i,j = 0;
  int len=strlen(str);
  for(i = 0;i < len;i ++)
  {
     if(str[i] == ',')
      j++;
     if(j == num)
      return i + 1; 
  }
  return 0; 
}

static double getDoubleNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atof(buf);
  return rev; 
}

static double getIntNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atoi(buf);
  return rev; 
}

void parseGPGGA(const char* GPGGAstr)
{
  
  double latitude;
  double longitude;
  int tmp, hour, minute, second, num ;
  if(GPGGAstr[0] == '$')
  {
    tmp = getComma(1, GPGGAstr);
    hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
    minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
    second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');
    
    sprintf(buff, "UTC timer %2d-%2d-%2d", hour, minute, second);
    Serial.println(buff);
    
    tmp = getComma(2, GPGGAstr);
    latitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(4, GPGGAstr);
    longitude = getDoubleNumber(&GPGGAstr[tmp]);
    sprintf(buff, "latitude = %10.4f, longitude = %10.4f", latitude, longitude);
    Serial.println(buff); 
    
    tmp = getComma(7, GPGGAstr);
    num = getIntNumber(&GPGGAstr[tmp]);    
    sprintf(buff, "satellites number = %d", num);
    Serial.println(buff); 
  }
  else
  {
    Serial.println("Not get data"); 
  }
}






void setup()
{

  Serial.begin(9600);
  lcd.begin(16,2);

  LGPS.powerOn();
  Serial.println("LGPS Power on, and waiting ..."); 
  delay(3000);
  
 
  pinMode(ButtonPin,INPUT);
  pinMode(BuzzerPin,OUTPUT);
  
  pinMode(LED, OUTPUT);
  LWiFi.begin();

  // keep retrying until connected to AP
  Serial.println("Connecting to AP");
  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  {
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(100);
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(600);

  }
  digitalWrite(LED, HIGH);
  printWifiStatus();
  Serial.println("Start Server");
  server.begin();
  Serial.println("Server Started");
  digitalWrite(LED, LOW);
}






int loopCount = 0;

void loop()
{

  
  // put your main code here, to run repeatedly:
  String str = "";
  String url = "";
  int i;
  delay(500);
  loopCount++;
  LWiFiClient client = server.available();
  if (client)
  {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected())
    {
      if (client.available())
      {
        // we basically ignores client request, but wait for HTTP request end
        char c = client.read();
        Serial.print(c);
        if(c != '\n')
          str += c;
        if(c == '\n')
        {
          //Serial.println(str);
          if(str.startsWith("GET "))
          {
            url = str.substring(4, str.lastIndexOf(" "));
            Serial.print("URL:");
            Serial.print(url);
            Serial.println(":");
          }
          str = "";
        }

        if (c == '\n' && currentLineIsBlank)
        {
          Serial.println("send response");
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          if(url != String("favicon.ico"))
          {
            client.println("<!DOCTYPE HTML>");
            client.println("<html>\n<head>\n<title>LED Control</title>\n</head>");
            IPAddress ip = LWiFi.localIP();
            client.println("<body><center>");
            client.println("<form action='");
            client.println("' method='GET'>Tell your device what to do!<br><br><input type='radio' name='q' value='on'>Turn the LED on.<br><input type='radio' name='q' value='off'>Turn the LED off.<br><br><input type='submit' value='Do it!'></form>");
            //i = digitalRead(LED);
            url.toLowerCase();
            
            
            if(url == String("/?q=on"))
            {

              lcd.clear();
              lcd.println("Lat:");
              lcd.println(latitude);
              
              lcd.setCursor(0,1);
              lcd.println("Lon:");
              lcd.println(longitude);
     
              Serial.println("http://www.latlong.net/c/?lat=");
              Serial.println(latitude);
              Serial.println("&long=");
              Serial.println(longitude);
      
          
 
              digitalWrite(BuzzerPin,HIGH);
              delay(2000);
              digitalWrite(BuzzerPin,LOW);
              lcd.clear();

    
              digitalWrite(LED, HIGH);
              client.println("LED on<br>");
            }
            else if(url == String("/?q=off"))
            {
              digitalWrite(LED, LOW);
              client.println("LED off<br>");

              digitalWrite(BuzzerPin,LOW);
              lcd.clear();
            }
            else
            {
              client.println("Doing nothing<br>");

              digitalWrite(BuzzerPin,LOW);
              lcd.clear();
            }
            
            
            client.println("</center></body>\n</html>");
            client.println();
            break;
          }
        }
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(50);

    // close the connection:
    Serial.println("close connection");
    client.stop();
    Serial.println("client disconnected");
  }


  buttonState = digitalRead(ButtonPin);

  if(buttonState == HIGH){
  
              lcd.clear();
              lcd.println("Lat:");
              lcd.println(latitude);
              
              lcd.setCursor(0,1);
              
              lcd.println("Lon:");
              lcd.println(longitude);
     
              Serial.println("LINK : http://www.latlong.net/c/?lat=");
              Serial.println(latitude);
              Serial.println("&long=");
              Serial.println(longitude);
    
          
 
    digitalWrite(BuzzerPin,HIGH);
    delay(2000);
    digitalWrite(BuzzerPin,LOW);
    lcd.clear();

     
    
  }

  else 
  {
    digitalWrite(BuzzerPin,LOW);
    lcd.clear();
    
  }


  
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(LWiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = LWiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.print("subnet mask: ");
  Serial.println(LWiFi.subnetMask());

  Serial.print("gateway IP: ");
  Serial.println(LWiFi.gatewayIP());

  // print the received signal strength:
  long rssi = LWiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


