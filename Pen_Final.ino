#include <WiFi.h>
#include <WebServer.h>
//#include <esp_system.h>
#include <Arduino.h>
#include <stdint.h>
#include "SCMD.h"
#include "SCMD_config.h" //Contains #defines for common SCMD register names and values
#include "Wire.h"
#define LEFT_MOTOR 1 // B1
#define RIGHT_MOTOR 0 // A1 
const char* ssid = "ESP32";  // Enter SSID here
const char* password = "12345678";  //Enter Password here
int DELAY=5;
int len=500;
String ch="q";
String mod="q";
#define back_limit 6 // Yellow
#define front_limit 4 // Green 
#define left_limit 8 // Red
#define right_limit 10 // Blue
#define estop 12 // EStop
bool stop_process=false;
bool automode=false;
SCMD myMotorDriver; 

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
WebServer server(80);
void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  pinMode(back_limit,INPUT);
  pinMode(front_limit,INPUT);
  pinMode(right_limit,INPUT);
  pinMode(left_limit,INPUT);
  pinMode(estop,INPUT);
  Serial.begin(9600);
  Serial.println("Begining of program");
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  server.on("/", handle_OnConnect);
  server.on("/mtrfwd", handle_fwd);
  server.on("/mtrrev", handle_rev);
  server.on("/mtrright",handle_right);
  server.on("/mtrleft", handle_left);
  server.on("/halt", handle_halt);
  server.on("/start", handle_start);
  server.on("/manual", handle_manual);
  server.on("/home", handle_gohome);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
  
  myMotorDriver.settings.commInterface = I2C_MODE;
  myMotorDriver.settings.I2CAddress = 0x5D;
  myMotorDriver.settings.chipSelectPin = 10;
  myMotorDriver.begin();
  Serial.println(myMotorDriver.begin());
  while ( myMotorDriver.begin() != 0xA9 ) //Wait until a valid ID word is returned
  {
    Serial.println( "ID mismatch, trying again" );
    delay(1000);
  }
  Serial.println( "ID matches 0xA9" );
  Serial.println(myMotorDriver.ready());
  //  Check to make sure the driver is done looking for peripherals before beginning
  Serial.println("Waiting for enumeration...");
  while ( myMotorDriver.ready() == false );
  Serial.println("Done.");
//  Serial.println();

  while ( myMotorDriver.busy() );
  myMotorDriver.enable(); //Enables the output driver hardware
  Serial.println( "not busy" ); 
  Serial.print("Automode:");
  Serial.println(automode); 
  halt();
}
void handle_start(){
  Serial.println("Process Start in Auto Mode");
  server.send(200, "text/html", SendHTML());
  automode=true;
  autostart();
}
void handle_manual(){
  Serial.println("Going Manual");
  server.send(200, "text/html", SendHTML());
  automode=false;
}
void handle_gohome(){
  Serial.println("Go Home");
  server.send(200, "text/html", SendHTML());
}
void handle_fwd(){
  server.send(200, "text/html", SendHTML());
  if(automode==false){
    fwd(len);}
}
void handle_rev(){
  // To move PenHolder REVERSE
  server.send(200, "text/html", SendHTML());
  if(automode==false){
   rev(len);}
}
void handle_right(){
  server.send(200, "text/html", SendHTML());
  if(automode==false){
    right(len);}
}
void handle_left(){
  server.send(200, "text/html", SendHTML());
  if(automode==false){
   left(len);}
}
void handle_halt(){
  server.send(200, "text/html", SendHTML());
  halt();
 }
void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void handle_OnConnect() {
  Serial.println("Client Connected");
  server.send(200, "text/html", SendHTML()); 
}
void fwd(int d){
  // To move PenHolder FORWARD
  Serial.println("FWD");
  myMotorDriver.setDrive( LEFT_MOTOR, 1, 130); 
  delay(1);
  myMotorDriver.setDrive( RIGHT_MOTOR, 0, 130); 
 for(int i =0;i<d;i++){
    
    if(digitalRead(front_limit)==LOW or digitalRead(estop)==LOW)
    {
      Serial.println("Limit Reached or ESTOP ");
      break;}
    delay(1);  
  }
  myMotorDriver.setDrive( LEFT_MOTOR, 0, 0); 
  delay(1);
  myMotorDriver.setDrive( RIGHT_MOTOR, 0, 0); 
}

void rev(int d){
  // To move PenHolder REVERSE
  Serial.println("REV");
   myMotorDriver.setDrive( LEFT_MOTOR, 0, 130); 
   delay(1);
  myMotorDriver.setDrive( RIGHT_MOTOR, 1, 130); 
 for(int i =0;i<d;i++)
 { 
    if(digitalRead(back_limit)==LOW or digitalRead(estop)==LOW)
    {
      Serial.println("Limit Reached");
      break;}
    delay(1);  
  }
  myMotorDriver.setDrive( LEFT_MOTOR, 0, 0); 
  delay(1);
  myMotorDriver.setDrive( RIGHT_MOTOR, 0, 0); 
  
}
void right(int d){
  // To move PenHolder RIGHT
  Serial.println("RIGHT");
  myMotorDriver.setDrive( LEFT_MOTOR, 1, 130); 
  delay(1);
  myMotorDriver.setDrive( RIGHT_MOTOR, 1, 127); 
  for(int i =0;i<d;i++)
  {  
    if(digitalRead(right_limit)!=LOW or digitalRead(estop)==LOW)
    {
      Serial.println("Limit Reached");
      break;}
    delay(1);  
  }
  myMotorDriver.setDrive( LEFT_MOTOR, 0, 0); 
  delay(1);
  myMotorDriver.setDrive( RIGHT_MOTOR, 0, 0); 
  
}
void left(int d){
  // To move PenHolder LEFT
  Serial.println("LEFT");
  Serial.println(digitalRead(estop));
  myMotorDriver.setDrive( LEFT_MOTOR, 0, 127); 
  delay(2);
  myMotorDriver.setDrive( RIGHT_MOTOR, 0, 130); 
  for(int i =0;i<d;i++)
  {  
    if(digitalRead(left_limit)==LOW or digitalRead(estop)==LOW){
      Serial.println("Limit Reached");
      
      break;}
    delay(1);  
  }
  myMotorDriver.setDrive( LEFT_MOTOR, 0, 0); 
  delay(1);
  myMotorDriver.setDrive( RIGHT_MOTOR, 0, 0); 
  
}
void halt(){
  // To HALT PenHolder 
  Serial.println("HALT");
  myMotorDriver.setDrive( LEFT_MOTOR, 0, 0); 
  delay(5);
  myMotorDriver.setDrive( RIGHT_MOTOR, 0, 0); 
  delay(5);
  
}

void rightfwd(int d){
  // To move PenHolder Diagonally Right Forward
  Serial.println("RIGHT FWD");
  myMotorDriver.setDrive( LEFT_MOTOR, 1, 130); 
  delay(5);
  myMotorDriver.setDrive( RIGHT_MOTOR, 0, 0); 
 for(int i =0;i<d;i++){
    
    if(digitalRead(right_limit)!=LOW or digitalRead(front_limit)==LOW or digitalRead(estop)==LOW){
      Serial.println("Limit Reached");
      break;}
    delay(1);  
  }
  myMotorDriver.setDrive( LEFT_MOTOR, 0, 0); 
  delay(1);
  myMotorDriver.setDrive( RIGHT_MOTOR, 0, 0); 
}

void leftfwd(int d){
   // To move PenHolder Diagonally Left Forward
   Serial.println("LEFT FWD");
  myMotorDriver.setDrive( LEFT_MOTOR, 0, 0); 
  delay(5);
  myMotorDriver.setDrive( RIGHT_MOTOR, 0, 130); 
    for(int i =0;i<d;i++)
    {
    if(digitalRead(left_limit)==LOW or digitalRead(front_limit)==LOW or digitalRead(estop)==LOW)
    {
      Serial.println("Limit Reached");
      break;}
    delay(1);  
  }
  myMotorDriver.setDrive( LEFT_MOTOR, 0, 0); 
  delay(1);
  myMotorDriver.setDrive( RIGHT_MOTOR, 0, 0); 
}
void rightrev(int d){
   // To move PenHolder Diagonally Right Reverse
   Serial.println("RIGHT REV");
  myMotorDriver.setDrive( LEFT_MOTOR, 0, 0); 
  delay(5);
  myMotorDriver.setDrive( RIGHT_MOTOR, 1, 130); 
  for(int i =0;i<d;i++)
  {
   if(digitalRead(right_limit)!=LOW or digitalRead(back_limit)==LOW or digitalRead(estop)==LOW)
   {
      Serial.println("Limit Reached");
      break;}
    delay(1);}
  myMotorDriver.setDrive( LEFT_MOTOR, 0, 0); 
  delay(1);
  myMotorDriver.setDrive( RIGHT_MOTOR, 0, 0); 
}
void leftrev(int d){
   // To move PenHolder Diagonally Left Reverse
   Serial.println("LEFT REV");
  myMotorDriver.setDrive( LEFT_MOTOR, 0, 130); 
  delay(5);
  myMotorDriver.setDrive( RIGHT_MOTOR, 0, 0); 
   for(int i =0;i<d;i++){
    if(digitalRead(left_limit)==LOW or digitalRead(back_limit)==LOW or digitalRead(estop)==LOW)
    {
      Serial.println("Limit Reached");
      break;}
    delay(1);  }
  myMotorDriver.setDrive( LEFT_MOTOR, 0, 0); 
  delay(1);
  myMotorDriver.setDrive( RIGHT_MOTOR, 0, 0); 
}
void pause(int d)
{ 
  halt();
  delay(d);
}
bool autofaultpresent(){
  if(automode ==false or digitalRead(left_limit)==LOW or digitalRead(right_limit)!=LOW or digitalRead(back_limit)==LOW or digitalRead(front_limit)==LOW or digitalRead(estop)==LOW)
  {
    return true;
  }
  else 
  {
    return false;
  }
}
void autostart(){
  Serial.println("Start in Auto Mode");
      halt();
      delay(DELAY);
      if(!autofaultpresent()){
         right(1667);}
      if(!autofaultpresent()){  
           pause(1000);}
      if(!autofaultpresent()){
       fwd(1026);}
      if(!autofaultpresent()){
       pause(1000);}
      if(!autofaultpresent()){
        left(1739);}
      if(!autofaultpresent()){
        pause(1000);}
      if(!autofaultpresent()){
        rightfwd(2091);}
     if(!autofaultpresent()){
        pause(1000);}
     if(!autofaultpresent()){
       rightrev(1315);}
     if(!autofaultpresent()){
       pause(1000);}
     if(!autofaultpresent()){
       leftrev(3055);}
      if(!autofaultpresent()){
       pause(1000);}
      if(!autofaultpresent()){ 
       fwd(974);}
      if(!autofaultpresent()){
       pause(1000);}
     if(!autofaultpresent()){
        rightrev(2499);}
      if(!autofaultpresent()){
        pause(1000);}
      if(!autofaultpresent()){
        left(1609);}
      if(!autofaultpresent()){
        Serial.println("Cycle Finish Going to Manaual Mode");automode =false;}
        
      else{
        Serial.println("Cycle Fault Going Manual Mode");
          automode =false;
        }
}
void loop() {
  server.handleClient();
  // put your main code here, to run repeatedly:
  if(stop_process==false and digitalRead(estop)==LOW){
      Serial.println("ESTOP Pressed");
      stop_process=true;
      halt();
      } 
  if(Serial.available())
  {
    ch= Serial.readStringUntil('\n');
    if(ch=="auto")
    {
      automode=true;
      Serial.println("Mode: Auto");
    }
    if(ch=="man")
    {
      automode=false;
      Serial.println("Mode: Manual");
    }
    if(ch=="w" and automode == false and stop_process==false){
      fwd(len);
      delay(5);
    }
    else if(ch=="s" and automode== false and stop_process==false){
      rev(len);
      delay(5);
    }
    else if(ch=="a" and automode== false  and stop_process==false){
      left(len);
      delay(5);
    }
    else if(ch=="d" and automode== false and stop_process==false){
      right(len);
      delay(5);
    }
    else if(ch=="q" and automode == false and stop_process==false){
      leftfwd(len);
      delay(5);
    }
    else if(ch=="e" and automode == false and stop_process==false){
      rightfwd(len);
      delay(5);
    }
    else if(ch=="z" and automode == false and stop_process==false){
      leftrev(len);
      delay(5);
    }
    else if(ch=="x" and automode == false and stop_process==false){
      rightrev(len);
      delay(5);
    }
    else if(ch=="start" and automode == true and !autofaultpresent())
    {
      autostart();
      }
    }
    else if(ch=="reset"){
      Serial.println("RST Pressed");
      stop_process=false;
      delay(5);
      }
    else if (ch=="r")
    {
      halt();
      Serial.println("HALT in else");
      ch="p";
      delay(5);
    }
}

String SendHTML(){
  String ptr = R"(
 <!DOCTYPE html>
<html>
<head>
<title>Page Title</title>
<style>
.button {
  border: none;
  color: white;
  padding: 0px 10px 10px 10px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 80px;
  cursor: pointer;
  height:100px;
  width:100px;
}
.button2 {
  border: none;
  color: black;
  padding: 0px 10px 10px 10px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 40px;
  cursor: pointer;
  height:50px;
  width:200px;
  font-weight: bold;
  text-align: center;
}
.button3{
 color: black;
    text-align: center;
    font-size: 40px;
    height:100px;
    width:100px;
    border-radius: 50%;
    background-color: #FFA500;
}

.button1 {background-color: #808080;} 
table {
border-spacing: 20px;
width: 70%;
height: 70%;
}
.buttonstop{
background-color: #FF0000;
color: white;
width: 100%;
height: 100%;
}
.buttonstart{
background-color: #008000;
color: white;
width: 100%;
height: 100%;
}
.buttonhome{
background-color: #808080;
color: white;
width: 100%;
height: 100%;
}
div.wrap {
    width:1000px;
    margin:0 auto;
}
table {
  width: 70%;
}
</style>
</head>
<body>
<div class="wrap">
<center>
<table>
<tr><td>
<a href="/start"><button type="button" class="button2 buttonstart">START</button></a></td>
</tr>
<tr><td>
<a href="/manual"><button type="button" class="button2 buttonstop">Go Manual</button></a></td>
</tr>
<tr><td>
<a href="/home"><button type="button" class="button2 buttonhome">HOME</button></a></td>
</tr>
</table>
<table>
  <tr>
    <td></td>
    <td>
    <a href="/mtrfwd"><button type="button" class="button button1">&#8593</button></a></td>
    <td></td>
  </tr>
  <tr>
    <td><a href="/mtrleft"><button type="button" class="button button1">&#8592</button></a></td>
    <td><a href="/halt"><button type="button" class="button3">Halt</button></a></td>
    <td><a href="/mtrright"><button type="button" class="button button1">&#8594;</button></a></td>
  </tr>
  <tr>
  <td></td>
  <td><a href="/mtrrev"><button type="button" class="button button1">&#8595;</button></a></td>
  <td></td>
  </tr>
</table>
</center>
</div>
</body>
</html>

)";
  return ptr;
}
