#include <EEPROM.h>
#include <WiFiEspClient.h>
#include <WiFiEsp.h>
#include <PubSubClient.h>
#include <EmonLib.h>
#include "DHT.h"
#include "SoftwareSerial.h"
//#define dht_pin_out A2
#define dht_pin_in A3
#define door_pin_a 2
#define door_pin_b 3
#define door_pin_c 4
#define dhttype DHT22   // DHT 22  (AM2302), AM2321
#define led_g 13
#define led_b 6
#define led_r 5
#define buzzer 8
int aa;

WiFiEspClient espClient;
PubSubClient client(espClient);
int status = WL_IDLE_STATUS;
SoftwareSerial Serial1(11,10);

float ampin = A1;
int value = 24;  // 24 untuk 10A, 20 untuk 20A
double irms = 0;
EnergyMonitor amp; 
DHT dht_in(dht_pin_in, dhttype);
float t_in=28.00;
float h_in=90.00;
String s1="28.00";
int door_state_a = 0;
int door_state_b = 0;
int door_state_c = 0;
int x = 0;
boolean a,b,c,d;
const int port = 1883;
int sttconnect,sttb;
long rssi_signal;
int quality;
int qlty=50;
void(* resetFunc) (void) = 0;
int timer_int=0;




//==============================eeprom============================
const int EEPROM_MIN_ADDR = 0;
const int EEPROM_MAX_ADDR = 511;
boolean eeprom_is_addr_ok(int addr) {
  return ((addr >= EEPROM_MIN_ADDR) && (addr <= EEPROM_MAX_ADDR));
}
boolean eeprom_write_bytes(int startAddr, const byte* array, int numBytes) {
// counter
  int i;
  if (!eeprom_is_addr_ok(startAddr) || !eeprom_is_addr_ok(startAddr + numBytes)) {
    return false;
  }
  for (i = 0; i < numBytes; i++) {
    EEPROM.write(startAddr + i, array[i]);
  }
  return true;
}

boolean eeprom_write_string(int addr, const char* string) {
  int numBytes; 
  numBytes = strlen(string) + 1;
  return eeprom_write_bytes(addr, (const byte*)string, numBytes);
}

boolean eeprom_read_string(int addr, char* buffer, int bufSize) {
  byte ch;
  int bytesRead;
  if (!eeprom_is_addr_ok(addr)) {
    return false;
  }
  
  if (bufSize == 0) { 
    return false;
  }
  
  if (bufSize == 1) {
    buffer[0] = 0;
    return true;
  }
  bytesRead = 0;
  ch = EEPROM.read(addr + bytesRead);
  buffer[bytesRead] = ch;
  bytesRead++;
  while ( (ch != 0x00) && (bytesRead < bufSize) && ((addr + bytesRead) <= EEPROM_MAX_ADDR) ) {  
    ch = EEPROM.read(addr + bytesRead);
    buffer[bytesRead] = ch;
    bytesRead++;
  }
  
  if ((ch != 0x00) && (bytesRead >= 1)) {
    buffer[bytesRead - 1] = 0;
  }
  return true;
}
//const int BUFSIZE_DATA = 2;
//const int BUFSIZE_ID = 8;
//const int BUFSIZE_HOST = 16;
//const int BUFSIZE_SSID = 15;
//const int BUFSIZE_PASS = 11;
//const int BUFSIZE_VALUE = 3;
//const int BUFSIZE_TIMER = 4;
//const int BUFSIZE_DOOR = 5;
//const int BUFSIZE_ST = 2;
char edata[2];
char id[8];
char host[16];
char ssid[15];
char pass[11];
char datact[3];
char timer_jln[4];
char v_door[5];
char st[2];
//===================================================================================
int i,j,k,l,m,n,o,p,q,r,s,t;
String merk,data_2,data_value,str_id,str_host,str_ssid,str_pass,str_value, str_timer, str_door, str_st;
String test,data;
unsigned long lastSend;
//float TI = 25.00;
//float H = 99.00;
//int S = 50;

void setup() {
  // put your setup code here, to run once:
  sttconnect=0;
  sttb=0;
  aa=0;
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(door_pin_a, INPUT);
  pinMode(door_pin_b, INPUT);
  pinMode(door_pin_c, INPUT);
  pinMode(led_g, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(led_b, OUTPUT);
  pinMode(led_r, OUTPUT);
  dht_in.begin();
  eeprom_read_string(0, edata, 2);
  test=edata;
  int test_a = atoi(edata);
  if(test_a != 3){ 
    test="3";
    str_id = "CH0004";
    str_host="192.168.50.31";
    str_ssid="djsoda";
    str_pass="12345678";
    str_value="24";
    str_timer="30";
    str_door="0";
    str_st="0";
    test.toCharArray(edata, 2);
    eeprom_write_string(0, edata);     
    str_id.toCharArray(id, 8);
    eeprom_write_string(2, id);
    delay(20);    
    str_host.toCharArray(host, 16);
    eeprom_write_string(10, host);
    delay(20);
    str_ssid.toCharArray(ssid, 15);
    eeprom_write_string(26, ssid);
    delay(20);
    str_pass.toCharArray(pass, 11);
    eeprom_write_string(41, pass);
    delay(20);
    str_value.toCharArray(datact, 3);
    eeprom_write_string(52, datact);
    delay(20);
    str_timer.toCharArray(timer_jln, 4);
    eeprom_write_string(55, timer_jln);
    delay(20);
    str_door.toCharArray(v_door, 5);
    eeprom_write_string(60, v_door);
    delay(20);
    str_st.toCharArray(st, 2);
    eeprom_write_string(62, st);
    delay(20);
    
    
  }

  eeprom_read_string(2, id, 8);
  eeprom_read_string(10, host, 16);  
  eeprom_read_string(26, ssid, 15);  
  eeprom_read_string(41, pass, 11);  
  eeprom_read_string(52, datact, 3);
  eeprom_read_string(55, timer_jln, 4);
  eeprom_read_string(60, v_door, 5);
  eeprom_read_string(62, st, 2);
  
  test=datact;
  str_timer=timer_jln;
  value= test.toInt(); 
  timer_int=(str_timer.toInt()-2);
  if(timer_int<0){
    timer_int=0;
  }
  Serial.print("ID : ");
  Serial.println(id);
  Serial.print("Host : ");
  Serial.println(host);
  Serial.print("Ssid : ");
  Serial.println(ssid);
  Serial.print("Pass : ");
  Serial.println(pass);
  Serial.print("CT : "); 
  Serial.println(value);
  Serial.print("Timer : ");
  Serial.println(str_timer);
  Serial.print("Value Door : ");
  Serial.println(v_door);
  Serial.print("Status Door : ");
  Serial.println(st);
  delay(1000); 
  amp.current(ampin, value);
  wifi_initial();
  a=true;
  b=false;
  c=false;
  client.setCallback(callback);
  lastSend=0;
  digitalWrite(led_g,LOW);
  digitalWrite(led_b,LOW);
 
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()>0){
    merk=Serial.readString();
  }
  delay(10);
  if(merk.length()>0){
    pecahbelah(merk);
    merk="";
  }
  if (!client.connected()) 
  {
    digitalWrite(led_g,LOW);
    digitalWrite(led_r, HIGH);
    //status = WL_IDLE_STATUS;
    //if(status != WL_CONNECTED){
    //    wifi_initial();
    //}  
    reconnect();
    sttconnect=0;
    sttb=0;
    aa=0;
  }
  else if (client.connected())
  { 
    digitalWrite(led_g,HIGH);
    digitalWrite(led_r, LOW); 
    for(int j=0; j<5;j++)
    {
      client.loop();
    }
    door_function();
    if ( ((millis() - lastSend)/1000) > timer_int )
    {
        data_sensor();
        //digitalWrite(led,HIGH);
        //delay(100);
        lastSend = millis();
        
    }
    //digitalWrite(led,LOW);
  }

}

void callback(const char* topic, byte* payload, unsigned int length) 
{
    data="";
    for (int i=0;i<length;i++)
    {
      data += (char)payload[i];
      delay(50);
    }
    Serial.println(data);
    if(data.length()>0)
    {
      pecahbelah(data);
    }
    
}

void data_sensor()
{
  //===================Suhu ruang=======================
  int suhu_out= A2;
  float suhu1,data_vs,data_left,data_left2,data_right,nilai_hambatan,nilai_a1,nilai_a2;
  char temp[6];
  
  suhu1=analogRead(suhu_out);
  data_left=suhu1/1023;
  data_left2 = 2.2*data_left*1000;
  data_right = 1 - data_left;
  nilai_hambatan = data_left2/data_right;
  nilai_a1 = 1/nilai_hambatan;  
  nilai_a2 = (nilai_a1*1000000/4.96) + 4.96;
  dtostrf(nilai_a2,4,2,temp);
  s1 = String(temp);
  
  //===================dht==============================
  delay(1000);
  t_in  = dht_in.readTemperature();
  h_in  = dht_in.readHumidity();
  //float t_out = dht_out.readTemperature();
  //float h_out = dht_out.readHumidity();
  if (isnan(h_in) || isnan(t_in)) {
    Serial.println("dht error");
    t_in = 0.00;
    h_in = 0.00;
    //return;
  }
  

  //===================ampere===========================

  irms = amp.calcIrms(1480);  // Calculate Irms only
  irms=irms-0.25;
  if(irms < 0.20)
  {
    irms = 0.00;
  }

  //======kirim sinyal ESP8266
  rssi_signal =WiFi.RSSI();
  quality = rssi_signal + 100;
  qlty = 2*quality;
  if(rssi_signal >= -50)
  {
    qlty = 100;
  }
  else if(rssi_signal <= -100)
  {
    qlty = 0;
  }
  
  
  String p= "{";
  p +="\"KD\":"; p +="\""; p += id; p +="\""; p+=",";
  p +="\"TI\":"; p += t_in; p+=",";
  p +="\"TO\":"; p += s1; p+=",";
  p +="\"HM\":"; p += h_in; p+=",";
  p +="\"AR\":"; p += irms;p +=",";
  p +="\"SG\":"; p += qlty ;p += ",";
  p +="\"DR\":"; p += st ;p += ",";
  p +="\"DC\":"; p += v_door;p +="}";
  
  char kirim_sensor[100];
  p.toCharArray( kirim_sensor, 100 );
  String topic = id;
  topic += "_CH";
  client.publish( topic.c_str(), kirim_sensor );
  Serial.println( kirim_sensor );
  
}

void door_function()
{
  door_state_a = digitalRead(door_pin_a);
  door_state_b = digitalRead(door_pin_b);
  door_state_c = digitalRead(door_pin_c);
  if(door_state_a == LOW && door_state_b == LOW && door_state_c == LOW )
  { 
    c=false;
    aa=0;
    if(a == true)
    {
      /*
      //===================dht==============================
      delay(1000);
      t_in  = dht_in.readTemperature();
      h_in  = dht_in.readHumidity();
      if (isnan(h_in) || isnan(t_in)) {
        Serial.println("dht error");
        t_in = 0.00;
        h_in = 0.00;
        //return;
      }
    
      //===================ampere===========================
    
      irms = amp.calcIrms(1480);  // Calculate Irms only
      //irms=irms-0.20;
      if(irms < 0.10)
      {
        irms = 0.00;
      }
    
      //======kirim sinyal ESP8266
      rssi_signal =WiFi.RSSI();
      quality = rssi_signal + 100;
      qlty = 2*quality;
      if(rssi_signal >= -50)
      {
        qlty = 100;
      }
      else if(rssi_signal <= -100)
      {
        qlty = 0;
      }
      */
      str_st = "0";
      str_st.toCharArray(st, 2);
      eeprom_write_string(62, st);
      delay(50);
      /*
      String dc = "{";
      dc +="\"KD\":"; dc +="\""; dc += id; dc +="\""; dc+=",";
      dc +="\"TI\":"; dc += t_in; dc+=",";
      dc +="\"TO\":"; dc += s1; dc+=",";
      dc +="\"HM\":"; dc += h_in; dc+=",";
      dc +="\"AR\":"; dc += irms;dc +=",";
      dc +="\"SG\":"; dc += qlty ;dc += ",";
      dc +="\"DR\":"; dc += st ;dc += ",";
      dc +="\"DC\":"; dc += v_door;dc +="}";
      
      char door_c[100];
      dc.toCharArray( door_c, 100 );
      String topic_dc = id;
      topic_dc += "_CH";
      client.publish( topic_dc.c_str(), door_c );
      Serial.println( door_c );
      */
      data_sensor();
      b=true;
    }
    a=false;
  }
  else if(door_state_a == HIGH || door_state_b == HIGH || door_state_c == HIGH)
  {
    if(b == true)
    { 
      a=true;    
      x=atoi(v_door);  
      x++;
      str_door=String(x);
      str_door.toCharArray(v_door, 5);
      eeprom_write_string(60, v_door);
      delay(50);

      str_st = "1";
      str_st.toCharArray(st, 2);
      eeprom_write_string(62, st);
      delay(50);
      /*
      String dp = "{";
      dp +="\"KD\":"; dp +="\""; dp += id; dp +="\""; dp+=",";
      dp +="\"TI\":"; dp += t_in; dp+=",";
      dp +="\"TO\":"; dp += s1; dp+=",";
      dp +="\"HM\":"; dp += h_in; dp+=",";
      dp +="\"AR\":"; dp += irms;dp +=",";
      dp +="\"SG\":"; dp += qlty ;dp += ",";
      dp +="\"DR\":"; dp += st ;dp += ",";
      dp +="\"DC\":"; dp += v_door;dp +="}";
      
      char door_o[100];
      dp.toCharArray( door_o, 100 );
      String topic_do = id;
      topic_do += "_CH";
      client.publish( topic_do.c_str(), door_o );
      Serial.println( door_o );
      delay(100);
      */
      data_sensor();
      b=false;
      
    }
    
    aa=0;
    while(door_state_a != LOW || door_state_b != LOW || door_state_c != LOW )
    {
       door_state_a = digitalRead(door_pin_a);
       door_state_b = digitalRead(door_pin_b);
       door_state_c = digitalRead(door_pin_c);
       aa++;
       Serial.println(aa);
       delay(1000);
       if(aa > 9)
       {
          c=true;
          aa=0;
       }
       else if(door_state_a == LOW && door_state_b == LOW && door_state_c == LOW)
       {
          aa=0;
          c=false;
          break;
       }

       if(c == true)
        {
          while(door_state_a != LOW || door_state_b != LOW || door_state_c != LOW)
          {
            digitalWrite(led_g, LOW);
            digitalWrite(led_r, LOW);
            door_state_a = digitalRead(door_pin_a);
            door_state_b = digitalRead(door_pin_b);
            door_state_c = digitalRead(door_pin_c);
            buzzer_tone(1);
            digitalWrite(led_b, HIGH);
            delay(50);
            buzzer_tone(0);
            digitalWrite(led_b, LOW);
            delay(50);
            buzzer_tone(1);
            digitalWrite(led_b, HIGH);
            delay(50);
            buzzer_tone(0);
            digitalWrite(led_b, LOW);
            delay(500);
            if(door_state_a == LOW && door_state_b == LOW && door_state_c == LOW)
            {
              digitalWrite(led_b, LOW);
              digitalWrite(led_g, HIGH);
              digitalWrite(led_r, LOW);
              c=false;
              break;
            }
          }
        }
    }
  }
  
  
  
}

void wifi_initial()
{
    sttb=0;
    WiFi.init(&Serial1); 
    if (WiFi.status() == WL_NO_SHIELD) 
      {
        digitalWrite(led_r, HIGH);
        Serial.println("Failed");
        wifi_initial();
      } 
      //Serial.println("Wait..");       
      while ( status != WL_CONNECTED) 
      {
          //Serial.print("conect to: ");
          digitalWrite(led_r, HIGH);
          digitalWrite(led_g, LOW);
          Serial.println(ssid);
          //WiFi.hostByName(id);
          status = WiFi.begin(ssid, pass);
          delay(500);
          sttconnect++;
          if(sttconnect > 9)
          {
            for(int zz=0; zz<60; zz++)
            {
              delay(1000);
              Serial.println(zz);
            }
            sttconnect=0;
          }
          
       } 
   digitalWrite(led_r, LOW);
   digitalWrite(led_g, HIGH);
   Serial.println("Connect");
   client.setServer(host, port);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    digitalWrite(led_r,HIGH);
    digitalWrite(led_g,LOW);
    status = WL_IDLE_STATUS;
    Serial.println("MQTT..");
    // Attempt to connect (clientId, username, password)
    if ( client.connect(id,"","")) 
    {
      sttb=0;
      Serial.println( "Connect" );
      digitalWrite(led_r, LOW);
      digitalWrite(led_g,HIGH);
      String topic = id;
      topic+="_R";     
      client.subscribe(topic.c_str(),0);
    }
    else if ( ! client.connect(id,"",""))
    {
          for(int a=0; a<5; a++)
          {
            Serial.println("5s");
            delay(5000);
          }
          sttb++;
          if(sttb > 40)
          {
            return wifi_initial();
            sttb=0;
          }
      }      
    }  
}

void pecahbelah(String edata){
  
  data_2,data_value,str_id,str_host,str_ssid,str_pass,str_value="";
  int scfv=edata.length()-1;
  char atr[50];
  if(edata.endsWith("#")){
      //===========save id to eeprom============
    if(edata.startsWith("_"))
        {          
          data_2 = edata.substring(1, scfv);
          data_2.toCharArray(id, 8);
          eeprom_write_string(2, id);
          delay(100);
          str_value="Save id";
        }else if(edata.startsWith("!"))
        {          
          data_2 = edata.substring(1, scfv);
          data_2.toCharArray(host, 16);
          eeprom_write_string(10, host); 
          delay(100);
          str_value="Save host";
        } else if(edata.startsWith("^"))
         {          
          data_2 = edata.substring(1, scfv);
          if(data_2.toInt()> -1){
            data_2.toCharArray(datact, 3);
            eeprom_write_string(52, datact);
            delay(100); 
            str_value="Save value CT";
          }       
        }else if(edata.startsWith("`"))
        {          
          data_2 = edata.substring(1, scfv);
          if(data_2.toInt()> -1){
            data_2.toCharArray(timer_jln, 4);
            eeprom_write_string(55, timer_jln);
            delay(100); 
            str_value="Save Timer";
          }  
        }else if(edata.startsWith("?"))
        {
          m='?';
          while(edata.charAt(m) !='?')
          m++;
          data_2 = edata.substring(m);
          n=0;
          while (data_2.charAt(n) !='@')
          n++;
          str_ssid = data_2.substring(1, n);
          str_ssid.toCharArray(ssid, 16);
          eeprom_write_string(26, ssid);
          delay(100);
          o='@';
          while(edata.charAt(o) !='@')
          o++;
          data_2 = edata.substring(o);
          p=0;
          while (data_2.charAt(p) !='#')
          p++;
          str_pass = data_2.substring(1, p);
          str_pass.toCharArray(pass, 15);
          eeprom_write_string(41, pass);
          delay(100);
          str_value="Save ssid & pass ";
          
        }
        else if(edata.startsWith("$"))
        {          
          data_2 = edata.substring(1, scfv);
          if(data_2.toInt()> -1){
            data_2.toCharArray(st, 2);
            eeprom_write_string(62, st);
            delay(100); 
            str_value="Save Status";
          }  
        }
        
        
                
        str_value.toCharArray( atr, 50 );
        str_id = id;
        str_id+="_C";
        client.publish( str_id.c_str(), atr );
        Serial.println(atr);
        delay(1000);
        resetFunc();
    }
    else
    {
        if(edata == "0")
        {
          
          //str_value="Reset Door";
          str_door="0";
          str_door.toCharArray(v_door, 5);
          eeprom_write_string(60, v_door);
          delay(20);
          /*
          str_value.toCharArray( atr, 50 );
          str_id = id;
          str_id+="_C";
          client.publish( str_id.c_str(), atr );
          Serial.println(atr);
          delay(200);
          */
          data_sensor();
          resetFunc();
        }
    }

}

void buzzer_tone(int triger)
{
  digitalWrite(buzzer, triger); 
}



