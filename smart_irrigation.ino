#include <LiquidCrystal.h>
const int rs=13,en=12,d4=14,d5=27,d6=26,d7=25;
LiquidCrystal lcd(rs,en,d4,d5,d6,d7);


#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>


const char* ssid = "Irrigation123";
const char* password = "123456789";


#include<dht.h>  
#define dht_dpin 15

int IR=4;

int Relay1=18;
int Relay2=19;

int Fence=5;

int Buzzer=23;


dht DHT;


WiFiClient  client;

// Telegram Bot Token (from BotFather)
#define BOT_TOKEN "8357130564:AAEhuAOPzyZ4us20lVkhZ6Ip1l9iOFhEMbU"

// Your Telegram Chat ID
#define CHAT_ID "5604302392"

// Initialize Telegram BOT
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

unsigned long lastTime;
unsigned long botRequestDelay = 1000; // Check for new messages every second

int str_len;
char textmessage[20];
void Connect_wifi();


float tempc,Humidity;
int M_val,R_val,L_val;

void setup() 
{
  pinMode(IR,INPUT);
  pinMode(Buzzer,OUTPUT);

  pinMode(Relay1,OUTPUT);
  pinMode(Relay2,OUTPUT);
  pinMode(Fence,OUTPUT);

  digitalWrite(Relay1,LOW);
   digitalWrite(Relay2,LOW);
    digitalWrite(Fence,LOW);
   
   Serial.begin(9600);
 
    lcd.begin(16, 2);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" Solar Smart ");
    lcd.setCursor(0,1);
    lcd.print("Irrigatipn S/m");
    delay(2000);

  
    WiFi.mode(WIFI_STA);   
 
  Connect_wifi();


      // Initialize secure client
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Required for HTTPS
  // If certificate issues, use:
  // secured_client.setInsecure();

  // Send a startup message to Telegram
  bot.sendMessage(CHAT_ID, "Solar Based Smart Irrigation System...");

}
void Connect_wifi()
{



  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
}

void loop() 
{
   Humidity_Check();
   Moisture_Check();
   Rain_Check();
   LDR_Check();
   IR_Check();

    if (millis() - lastTime > botRequestDelay) 
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      for (int i=0; i<numNewMessages; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;

        Serial.println("Message received: " + text);

        // Command handling
        if (text == "/hello") {
          bot.sendMessage(chat_id, "Hello! ESP32 is online.", "");
        }
        else if (text == "/status") 
        {
          String status = "Moisture:"+String(M_val)+'\n'+"Rain:"+String(R_val)+'\n'+"Temperature:"+String(tempc)+'\n'+"Humidity:"+String(Humidity)+'\n'+"LDR:"+String(L_val);
          bot.sendMessage(chat_id, status, "");
        }

        else {
          bot.sendMessage(chat_id, "Unknown command. Available commands:\n/hello\n/status", "");
        }
      }
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTime = millis();
  }
   
     
}
void Humidity_Check(void)
{
     DHT.read11(dht_dpin);
      Humidity=DHT.humidity;
      tempc=DHT.temperature;
      Serial.print("$Humidity: ");
      Serial.print(DHT.humidity);   // printing Humidity on LCD
      Serial.print(" %");
      Serial.print(" # ");
  
      Serial.print("$Temperature:");
      Serial.print(DHT.temperature);   // Printing temperature on LCD
      Serial.print(" C");
      Serial.println(" # ");

      lcd.clear();
      lcd.print("Humidity:"+String(DHT.humidity));
      lcd.setCursor(0,1);
      lcd.print("Temp:"+String(DHT.temperature));
      
      delay(1000);
IR_Check();
 }
void Moisture_Check()
{
 M_val=analogRead(36);
 M_val=4095-M_val;
 Serial.println("Moisture="+String(M_val));
  lcd.clear();
  lcd.print("Moisture:"+String(M_val));
  delay(1000);
  if(M_val<200)
  {
     lcd.clear();
    lcd.print("Low Moisture");
    lcd.setCursor(0,1);
    lcd.print("Pump On...");
    Serial.println("Low Moisture Pump On...");
     digitalWrite(Relay1,HIGH);
     bot.sendMessage(CHAT_ID, "Low Moisture Pump On...");
    
     delay(1000);   
  }
  if(M_val>300)
  {
     digitalWrite(Relay1,LOW);
     delay(1000);   
  }
  IR_Check();
}
void Rain_Check()
{
 R_val=analogRead(39);
  R_val=4095-R_val;
 Serial.println("Rain="+String(R_val));
  lcd.clear();
  lcd.print("Rain:"+String(R_val));
  delay(1000);
  if(R_val>1000)
  {
     lcd.clear();
    lcd.print("Rain Detected");
    lcd.setCursor(0,1);
    lcd.print("Pump Off...");
    Serial.println("Rain Detected...");
     digitalWrite(Relay1,LOW);
     bot.sendMessage(CHAT_ID, "Rain Detected");
    
     delay(1000);   
  }
IR_Check();
}
void LDR_Check()
{
 L_val=analogRead(34);
 L_val=4095-L_val;
 Serial.println("LDR="+String(L_val));
  lcd.clear();
  lcd.print("LDR:"+String(L_val));
  delay(1000);
  if(L_val<1000)
  {
     lcd.clear();
    lcd.print("Darkness Detected");
    lcd.setCursor(0,1);
    lcd.print("Light On...");
    Serial.println("Darkness Detected...");
     digitalWrite(Relay2,HIGH);
     bot.sendMessage(CHAT_ID, "Darkness Detected");
    
     delay(1000);   
  }
  if(L_val>2000)
  {
      digitalWrite(Relay2,LOW);
  }
  IR_Check();
}

void IR_Check()
{
  
  if(digitalRead(IR)==LOW)
  {
    lcd.clear();
    lcd.print("Animal Detected..");
    Serial.println("Animal Detected");
    digitalWrite(Buzzer, HIGH);
    digitalWrite(Fence, HIGH);
    bot.sendMessage(CHAT_ID, "Animal Detected");
    delay(1000);
    digitalWrite(Buzzer, LOW);
    digitalWrite(Fence, LOW);
  }

}