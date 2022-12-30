#include <ESP8266WiFi.h>
#include "ThingSpeak.h"
#include <ESP_Mail_Client.h>

/* Email Göndermek İçin SMTP sunucu ayarları */
#define SMTP_server "smtp.gmail.com"
#define SMTP_PORT 465  
#define AUTHOR_EMAIL "zardunio@gmail.com" //göndericinin emaili
#define AUTHOR_PASSWORD "ifnkfrgayzletzvd" //göndericinin şifresi 
#define RECIPIENT_EMAIL "zeyzey6102@gmail.com" //alıcı mail adresi

/*Pinlerin tanimlanmasi*/
#define trig1 D7
#define echo1 D8
#define trig2 D1
#define echo2 D2
#define led D3
#define buzzer D5

/*Wifi baglantisi*/
const char* WLAN_SSID = "Eseny";
const char* WLAN_PASSWORD = "Yasin1234";
WiFiClient client;

SMTPSession smtp; //mail için

/*thingSpeak için baglanti tanimlamalari*/
unsigned long channelID =1996245; // Thingspeak channel ID 
unsigned int field_no=1; 
const char* writeAPIKey = "T5JXJ1U78DE22CQN"; // Thingspeak write API Key 
const char* thingSpeakHost = "api.thingspeak.com";

/*Global degiskenler*/
int maxMesafe= 5; 
int kisiSayisi=0;
int maxKisiSayisi=5;
int kritikSeviye=3;

//Wifi kurulum
void wifiSetup()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Kablosuz Agina Baglaniyor");
    WiFi.begin(WLAN_SSID, WLAN_PASSWORD);
    // WiFi durum kontrolü
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("Kablosuz Agina baglandi");
  }
}

//Mail gonder
void sendEmail(){
    //wifiSetup();
    smtp.debug(1);
    ESP_Mail_Session session;
   
    session.server.host_name = SMTP_server ;
    session.server.port = SMTP_PORT;
    session.login.email = AUTHOR_EMAIL;
    session.login.password = AUTHOR_PASSWORD;
    session.login.user_domain = "";
 
    SMTP_Message message;
    message.sender.name = "ESP8266";
    message.sender.email = AUTHOR_EMAIL;
    message.subject = "Müşteri sayısı kritik seviyede. Garson sayısı artırılmalı!!"; //konu
    message.addRecipient("Zeynep",RECIPIENT_EMAIL);
    if (!smtp.connect(&session))
        return;
    if (!MailClient.sendMail(&smtp, &message))
        Serial.println("Error sending Email, " + smtp.errorReason());
}

//Thingspeakteki veriyi guncelle
void upload()
{
  int x;
  wifiSetup();
  /* ThingSpeak Field Yazma İşlemi */
  x=ThingSpeak.writeField(channelID,field_no,kisiSayisi,writeAPIKey); //değeri gönder 
  if(x==200){
    Serial.println("Veri güncellendi..");
  }
  /*else{
    Serial.println("Veri güncelleme başarısız, tekrar deneniyor.");
    delay(1500);
    upload();
  }*/
}

//Mesafe sensorunun uzaklik hesaplamasi
int mesafe(int maxMesafe,int trig,int echo){
  int sure,uzaklik;
  
  digitalWrite(trig,LOW);
  delayMicroseconds(5);
  digitalWrite(trig,HIGH);
  delayMicroseconds(15);
  digitalWrite(trig,LOW);

  sure=pulseIn(echo,HIGH);
  uzaklik= (sure/2)/29.1;
  delay(20);
  
  if(uzaklik>=maxMesafe){
    return 0;
  }
  return uzaklik;
}

void setup() {
  Serial.begin(9600);
  wifiSetup();
  ThingSpeak.begin(client);
  pinMode(trig1,OUTPUT);
  pinMode(echo1,INPUT);
  pinMode(trig2,OUTPUT);
  pinMode(echo2,INPUT);
  pinMode(led,OUTPUT);
  pinMode(buzzer,OUTPUT);
}

void loop() {
 int uzaklikGiris=mesafe(maxMesafe,trig1,echo1);
 int uzaklikCikis=mesafe(maxMesafe,trig2,echo2);
 
 if(uzaklikGiris > 0){
     kisiSayisi++;
     Serial.print("Uzaklik(cm): ");
     Serial.print(uzaklikGiris);
     Serial.print(" Kisi Sayisi: ");
     Serial.println(kisiSayisi);
     digitalWrite(buzzer,HIGH);
     delay(10);
     digitalWrite(buzzer,LOW);
  }

  if(uzaklikCikis > 0){
     kisiSayisi--;
     Serial.print("Uzaklik(cm): ");
     Serial.print(uzaklikCikis);
     Serial.print(" Kisi Sayisi: ");
     Serial.println(kisiSayisi);
  }

  if(kisiSayisi>=kritikSeviye){
    sendEmail(); 
    delay(10);
  }
    
  if(kisiSayisi>= maxKisiSayisi){
     Serial.println("Kafenin kapasitesi dolmustur!!!");
     digitalWrite(led,HIGH);
  }
  else{
     digitalWrite(led,LOW);
  }
  
  upload();
  delay(500);
}
