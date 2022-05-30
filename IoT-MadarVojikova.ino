//   pro komunikaci se serverem
#include <ThingerESP8266.h>
#include <ESP8266WiFi.h>

//Knihovny na teploměr
#include <OneWire.h>
#include <DallasTemperature.h> 

//Knihovny na pH senzor
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Komunikace se serverem
#define USERNAME "Robnin"
#define DEVICE_ID "1"
#define DEVICE_CREDENTIAL "ANVbx7k2Yo$r002s"
#define SSID "Internet_Madar"
#define SSID_PASSWORD "********"

int hladina = 14; //Senzor hladiny vody
int docasna = 0;

long aktualniTeplota = 0; 

OneWire oneWire(12); //Teplotní senzor
DallasTemperature sensors(&oneWire); //Předávám informace knihovně

//ph
float calibracni_hodnota = 29.34 - 0.7;
int phval = 0; 
unsigned long int avgval; 
int buffer_arr[10],temp;
 
float ph_act;

ThingerESP8266 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

void setup() {
  Serial.begin(115200);
  thing.add_wifi(SSID, SSID_PASSWORD);
  sensors.begin(); //aktivace senzoru
}

void loop() {



  
  Serial.println("--------------------------------- "); //Dělící linka

  //------------Měření pH------------
  for(int i=0;i<10;i++) 
  { 
  buffer_arr[i]=analogRead(A0);
  delay(30);
  }
  for(int i=0;i<9;i++)
  {
  for(int j=i+1;j<10;j++)
  {
  if(buffer_arr[i]>buffer_arr[j])
  {
  temp=buffer_arr[i];
  buffer_arr[i]=buffer_arr[j];
  buffer_arr[j]=temp;
  }
  }
  }
  avgval=0;
  for(int i=2;i<8;i++)
  avgval+=buffer_arr[i];
  float volt=(float)avgval*5.0/1024/6; 
  ph_act = -5.70 * volt + calibracni_hodnota;
  
  Serial.print("Hodnota pH: "); //Vypsání pH
  Serial.println(ph_act); //Getování poslední hodnoty ph_act
  delay(500);
 
  //-----------hladina vody------------
  docasna = analogRead(hladina);
   
  if (docasna<=200){ Serial.println("Hladina vody: Prázdno");
  } else if (docasna>100 && docasna<=500){ Serial.println("Hladina vody: Minimum");
  } else if (docasna>500 && docasna<=550){ Serial.println("Hladina vody: Střední");
  } else if (docasna>600){ 
    Serial.println("Hladina vody: Maximální"); 
  }
  delay(500); 

  //------------teploměr------------
  sensors.requestTemperatures();
 
  aktualniTeplota = sensors.getTempCByIndex(0); 
  Serial.print("Teplota vody je: ");
  Serial.println(aktualniTeplota);  
  delay(500);

  //odeslání na server
  thing["data"] >> [](pson& out){
  out["Teplota"] = aktualniTeplota;
  out["Hladina"]= docasna;
  out["PH"]= ph_act;
  };
  thing.handle();
  thing.stream(thing["data"]);
}
