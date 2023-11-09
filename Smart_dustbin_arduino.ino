#include <Servo.h>   //servo library
#include <SoftwareSerial.h>       //it is for exp module                   
SoftwareSerial esp8266(2,3);                   
#define serialCommunicationSpeed 9600               
#define DEBUG true 
Servo servo;            //initializing the pins with pin number of aurdino
int trigPin = 5;        //this pin is for ultrasonic sensor which contains the trig pin for input
int echoPin = 6;       //this pin is for ultrasonic sensor which contains the echopin for o/p
int IR = 4;             //this is the IR sensor pin for whether the object is nearer to the dustbin
int servoPin = 9;       //this is the servometer [pin]
int Rled= 13;          //redled pin
int Gled= 12;                 //greenled pin
long duration, dist, average;   
long aver[3];   //array for average
int IR_state = 1;           
String sts = "EMPTY";


void setup() {           //This method is for setting up the pins    
    //Serial.begin(9600);
    pinMode(Rled,OUTPUT);      //This is for setting up the pins redled is for o/p for the purpose to display dustbin is full      
    pinMode(IR,INPUT);         //This is for setting up the pins irpin is for o/p like for sensing the nearer object
    pinMode(Gled,OUTPUT);      //This is for setting up the pins redled is for o/p for the purpose to display dustbin is halffull and also empty   
    servo.attach(servoPin);     //this is for attaching the servopin to getopen the lid
    pinMode(trigPin, OUTPUT);    //this is for ultrasonic sensors trigpin to detect the o/p
    pinMode(echoPin, INPUT);      //this is for ultrasonic sensors trigpin to detect the i/p
    servo.write(0);         //close cap on power on
    delay(100);
    servo.detach();
     
    digitalWrite(Rled,LOW);    //initial state of redled is low means off state
    digitalWrite(Gled,LOW);      //initial state of greenled is low means off state
    delay(1000);
    Serial.begin(serialCommunicationSpeed);         //this 2 lines of code to handle espmodules  
    esp8266.begin(serialCommunicationSpeed);     
    InitWifiModule();
    delay(1000);
} 

void measure() {                   //this method is to deal with measuring the distance using ultrasonic sensor
 digitalWrite(10,HIGH);
digitalWrite(trigPin, LOW);
delayMicroseconds(5);
digitalWrite(trigPin, HIGH);
delayMicroseconds(15);
digitalWrite(trigPin, LOW);
pinMode(echoPin, INPUT);
duration = pulseIn(echoPin, HIGH);
dist = (duration/2) / 29.1;    //obtain distance
}
void loop() { 
  IR_state = digitalRead(IR);
  for (int i=0;i<=2;i++) {   //average distance
    measure();               
   aver[i]=dist;            
    delay(10);              //delay between measurements
  }
 dist=(aver[0]+aver[1]+aver[2])/3;    

if (( IR_state==LOW )&& (dist>10)) {
//Change distance as per your need
 servo.attach(servoPin);
  delay(1);
 servo.write(0);  
 delay(3000);       
 servo.write(150);    
 delay(1000);
 servo.detach();
 delay(1000);
 IR_state=HIGH;    
}

if ( dist<10 ) {
  //Serial.println(dist);
  //Serial.println("Dust Bin Full");
  sts = "FULL";       //setting the condition for displaying the status of the dustbin in the mobile also in the form of led if ultrasonic sensor detects the length <10 means full
  digitalWrite(Rled,HIGH);
  digitalWrite(Gled,LOW);
  //sendFinal();
  delay(1000);
}
if (( dist>10 )&& (dist <50)) {
  //Serial.println(dist);
  //Serial.println("Dust Half Empty");    //if ultrasonic sensor detects the length >10 and <50 means halffull
  sts = "HALF EMPTY";
  digitalWrite(Rled,LOW);
  digitalWrite(Gled,HIGH);
  delay(1000);
  //sendFinal();
}
//else {
//  //Serial.println(dist);
//  //Serial.println("Dust Bin Empty");
//  sts = "EMPTY";
//  digitalWrite(Rled,LOW);
//  digitalWrite(Gled,HIGH);
//  delay(1000);
//  //sendFinal();
//}

//Serial.println(dist);
    if(esp8266.available())      //esp module checks whether the data is avalilable to send                                   
 {    
    if(esp8266.find("+IPD,"))      //it finds the ip data packet
    {
     delay(1000);
 
     int connectionId = esp8266.read()-48;       //This value is typically used to identify the connection ID converts to ascii value                                          
     String webpage = "<h1>DUST BIN STATUS: " + String(sts) + "</h1>";  //this line having the sentence to display by taking the dynamic sts vaue whether the dustbin is full or ...
     //webpage = webpage + "                   " + "<h2>" + rate + "</h2>";
     String cipSend = "AT+CIPSEND=";   //these are the setups like length,conn..id
     cipSend += connectionId;
     cipSend += ",";
     cipSend +=webpage.length();
     cipSend +="\r\n";
     
     sendData(cipSend,1000,DEBUG);     //sends the data to the esp module
     sendData(webpage,1000,DEBUG);
  
     String closeCommand = "AT+CIPCLOSE=";    //this lines of code to close the connection
     closeCommand+=connectionId; // append connection id
     closeCommand+="\r\n";    
     sendData(closeCommand,3000,DEBUG);  //closes after 3s
     
     sts = "EMPTY";
    }
  }

}

String sendData(String command, const int timeout, boolean debug)  //this function is used to send the data or response from the espmodule
{
    String response = "";                                             
    esp8266.print(command);                                          
    long int time = millis();                                      
    while( (time+timeout) > millis())                                 
    {      
      while(esp8266.available())                                      
      {
        char c = esp8266.read();                                     
        response+=c;                                                  
      }  
    }    
    if(debug)                                                        
    {
      Serial.print(response);
    }    
    return response;                                                  
}

void InitWifiModule()
{
  sendData("AT+RST\r\n", 2000, DEBUG);  
  delay (5000); 
  sendData("AT+CWMODE=1\r\n", 1500, DEBUG);   
  delay (1500);                                            
  sendData("AT+CWJAP=\"IOTPROJECT\",\"12345678\"\r\n", 9000, DEBUG);     //network name and password for the wifi these are all the servers    
  delay (9000);                                           
  sendData("AT+CIPMUX=1\r\n", 1500, DEBUG);                                             
  delay (1500);
  sendData("AT+CIPSERVER=1,80\r\n", 1500, DEBUG); 
  delay (1500);
  sendData("AT+CIFSR\r\n", 1500, DEBUG);   
  delay (1500);                                    

}

void sendFinal()    //this is the method or function for sending the status to the esp module
{
    if(esp8266.available())                                           
 {    
    if(esp8266.find("+IPD,"))
    {
     delay(1000);
 
     int connectionId = esp8266.read()-48;                                                
     String webpage = "<h1>DUST BIN STATUS: " + String(sts) + "</h1>";
     //webpage = webpage + "                   " + "<h2>" + rate + "</h2>";
     String cipSend = "AT+CIPSEND=";
     cipSend += connectionId;
     cipSend += ",";
     cipSend +=webpage.length();
     cipSend +="\r\n";
     
     sendData(cipSend,1000,DEBUG);
     sendData(webpage,1000,DEBUG);
 
     String closeCommand = "AT+CIPCLOSE="; 
     closeCommand+=connectionId; // append connection id
     closeCommand+="\r\n";    
     sendData(closeCommand,3000,DEBUG);
     
     sts = "";
    }
  }
}
