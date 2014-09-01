#include <Wire.h>
#include <dht11.h>
#include <VirtualWire.h>


#define DS1307_ADDRESS 0x68
#define TX_PIN 8
#define DHT11_PIN 7


// Time
int second;
int minute;
int hour;
int weekDay; //0-6 -> sunday - Saturday
int monthDay;
int month;
int year;

//Data
char temperature;
char humidity;

//hw
dht11 DHT11;

void setup(){ 
  Serial.begin(9600);
  Serial.println( "Weather station outdoor by JPL");

  Serial.println("Init RTC");
  initRtc();
  Serial.println("Init RTC OK");

  Serial.println("Init DHT11");
  initDht11();
  Serial.println("DHT11 OK");

  Serial.println("Init VirtualWire");
  initVirtualWire();
  Serial.println("VirtualWire OK");
}

void loop(){
  updateTime();
  //sendTime();

  updateData();  
  //  sendData();
  //  delay(5000);
  delay(1000);

send();

}



void initRtc(){
  Wire.begin();
}

void initDht11(){
  DHT11.attach(DHT11_PIN);
}

void initVirtualWire(){
  // Initialise the IO and ISR
  vw_set_tx_pin(TX_PIN);
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(1000);	 // Bits per sec
}


void updateData(){
  int chk = DHT11.read();

  Serial.print("Read sensor: ");
  switch (chk)
  {
  case 0: 
    Serial.println("sensor read OK"); 

    temperature = DHT11.temperature;
    humidity = DHT11.humidity;

    char   buffer[30] ;
    sprintf(buffer, "Temperature = %d, Humidity =%d%", temperature, humidity);
    Serial.println(buffer);
    break;
  case -1: 
    Serial.println("Checksum error"); 
    break;
  case -2: 
    Serial.println("Time out error"); 
    break;
  default: 
    Serial.println("Unknown error"); 
    break;
  }

}


byte bcdToDec(byte val)  {
  // Convert binary coded decimal to normal decimal numbers
  return  (val/16*10) + (val%16) ;
}

void updateTime(){

  // Reset the register pointer
  Wire.beginTransmission(DS1307_ADDRESS);

  byte zero = 0x00;
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  second = bcdToDec(Wire.read());
  minute = bcdToDec(Wire.read());
  hour = (char)bcdToDec(Wire.read() & 0b111111); //24 hour time
  weekDay = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  monthDay = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());

  applyCorrection();

  //print the date EG   3/1/11 23:59:59
  Serial.print(month);
  Serial.print("/");
  Serial.print(monthDay);
  Serial.print("/");
  Serial.print(year);
  Serial.print(" ");
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(second);


}


void applyCorrection(){
  year = year + 2000;
  
  //applique la correction liée au changement d'heure
  boolean shouldUpdate = false;
  //Le dimanche 27 octobre 2013 : à 3 heures du matin il sera 2 heures
  if(monthDay >= 27 && month >= 10 && year>=2013){

    shouldUpdate =true;
  }

  //Le dimanche 30 mars 2014 : à 2 heures du matin il sera 3 heures
  if(monthDay >= 30 && month >= 3 && year>=2014){
    
    shouldUpdate =false;
  }

  //Le dimanche 26 octobre 2014 : à 3 heures du matin il sera 2 heures
  if(monthDay >= 26 && month >= 10 && year>=2014){
       
    shouldUpdate =true;
  }

  //Le dimanche 29 mars 2015 : à 2 heures du matin il sera 3 heures
  if(monthDay >= 29 && month >= 3 && year>=2015){
      
    shouldUpdate =false;
  }

  //Le dimanche 25 octobre 2015 : à 3 heures du matin il sera 2 heures*/
  if(monthDay >= 25 && month >= 10 && year>=2015){
      
    shouldUpdate =true;
  }


  if(shouldUpdate){
       
    hour = (hour+23)%24;
  }

}

void send(){
  
char buffer[10];
  
  
 sprintf(buffer, "N%02dM%02dT%02dH%02d", hour,minute,temperature,humidity);
  Serial.print("send : ");
  Serial.println(buffer);
  vw_send((uint8_t *)buffer, strlen(buffer));
  vw_wait_tx(); // Wait until the whole message is gone
  delay(1000);
  
  
}



