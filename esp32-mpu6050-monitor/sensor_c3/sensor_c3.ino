#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>
#include <esp_wifi.h>
// Confirm these pins against your actual wiring before changing.
constexpr int SDA_PIN=9, SCL_PIN=8;
constexpr uint32_t MAGIC=0x4D505531;
struct __attribute__((packed)) Packet { uint32_t magic,seq,us; float ax,ay,az,gx,gy,gz,temp; uint8_t valid; };
uint8_t peer[]={0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t addr=0; uint32_t seq=0, nextSample=0,lastProbe=0,errors=0;
bool regWrite(uint8_t r,uint8_t v){ Wire.beginTransmission(addr); Wire.write(r); Wire.write(v); return Wire.endTransmission()==0; }
bool readRegs(uint8_t r,uint8_t *b,int n){ Wire.beginTransmission(addr); Wire.write(r); if(Wire.endTransmission(false))return false; if(Wire.requestFrom(addr,(uint8_t)n)!=n)return false; for(int i=0;i<n;i++)b[i]=Wire.read(); return true; }
bool initSensor(){
 for(uint8_t a: {0x68,0x69}){ addr=a; uint8_t id=0; if(readRegs(0x75,&id,1)&&id==0x68) return regWrite(0x6B,1)&&regWrite(0x1A,3)&&regWrite(0x19,9)&&regWrite(0x1B,0)&&regWrite(0x1C,0); } addr=0;return false;
}
void setup(){
 Serial.begin(115200); Wire.begin(SDA_PIN,SCL_PIN); Wire.setClock(400000); Wire.setTimeOut(10);
 WiFi.mode(WIFI_STA); WiFi.setSleep(false); delay(100);WiFi.setTxPower(WIFI_POWER_8_5dBm);
 esp_err_t channelResult=esp_wifi_set_channel(6,WIFI_SECOND_CHAN_NONE);
 uint8_t primary=0;wifi_second_chan_t secondary;esp_wifi_get_channel(&primary,&secondary);
 Serial.printf("RADIO channel=%u result=%d\n",primary,channelResult);
 if(esp_now_init()!=ESP_OK){Serial.println("ESP-NOW init failed");while(true)delay(1000);}
 esp_now_peer_info_t p={}; memcpy(p.peer_addr,peer,6);p.channel=6;p.ifidx=WIFI_IF_STA;
 if(esp_now_add_peer(&p)!=ESP_OK){Serial.println("peer failed");while(true)delay(1000);}
 bool ok=initSensor(); Serial.printf("C3 READY SDA=%d SCL=%d MPU=%s MAC=%s\n",SDA_PIN,SCL_PIN,ok?"OK":"MISSING",WiFi.macAddress().c_str());
}
void loop(){
 uint32_t now=micros(); if((int32_t)(now-nextSample)<0){delay(1);return;}nextSample=now+10000;
 if(!addr && millis()-lastProbe>2000){lastProbe=millis();initSensor();}
 Packet p={}; p.magic=MAGIC;p.seq=seq++;p.us=now; uint8_t b[14];
 if(addr&&readRegs(0x3B,b,14)){int16_t v[7];for(int i=0;i<7;i++)v[i]=(int16_t)((b[2*i]<<8)|b[2*i+1]);p.ax=v[0]/16384.f;p.ay=v[1]/16384.f;p.az=v[2]/16384.f;p.temp=v[3]/340.f+36.53f;p.gx=v[4]/131.f;p.gy=v[5]/131.f;p.gz=v[6]/131.f;p.valid=1;}else{addr=0;}
 if(esp_now_send(peer,(uint8_t*)&p,sizeof(p))!=ESP_OK)errors++;
 static uint32_t logAt=0;if(millis()-logAt>2000&&Serial.availableForWrite()>100){logAt=millis();Serial.printf("seq=%lu sensor=%u a=%.3f,%.3f,%.3f send_errors=%lu\n",(unsigned long)p.seq,p.valid,p.ax,p.ay,p.az,(unsigned long)errors);}
}
