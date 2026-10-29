#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Preferences.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <math.h>
#include "dashboard.h"

constexpr int GREEN=26,YELLOW=27,RED=25;
constexpr uint32_t MAGIC=0x4D505531;
constexpr size_t QUEUE_SIZE=128,HISTORY_SIZE=512;
constexpr uint16_t CAL_SAMPLES=300;
#include "config.h"
struct __attribute__((packed)) Packet {uint32_t magic,seq,us;float ax,ay,az,gx,gy,gz,temp;uint8_t valid;};
struct Incoming {Packet p;uint32_t id,rxMs;int rssi;};
struct Record {
  Incoming in;uint64_t sourceUs;uint32_t nodeSession,calId;
  float ax,ay,az,gx,gy,gz,norm,tilt,deviation;
  uint8_t state;
};
Incoming queue[QUEUE_SIZE];Record *history=nullptr;
portMUX_TYPE mux=portMUX_INITIALIZER_UNLOCKED;
uint32_t rxHead=0,processed=0,queueLost=0,historyCount=0,sourceLost=0;
uint32_t nodeSession=1,lastSeq=0,lastSourceUs=0,bootId=0,alarmUntil=0;
uint64_t sourceWrap=0;bool haveSource=false;
float packetHz=0;uint32_t rateAt=0,rateCount=0,lastLcd=0,lastLog=0;
WebServer web(80);Preferences prefs;
struct Calibration {uint32_t version;float ref[3],bias[3],norm;};
Calibration cal={};bool calReady=false,calPending=true,calSaved=false;
uint32_t calId=0;uint16_t calCount=0;
double sum[6]={},square[6]={};float previous[6]={};
uint32_t calFirstUs=0,calLastUs=0;float calNoiseG=0,calNoiseDps=0;

float magnitude(float x,float y,float z){return sqrtf(x*x+y*y+z*z);}
void clearWindow(){calCount=0;calFirstUs=calLastUs=0;memset(sum,0,sizeof(sum));memset(square,0,sizeof(square));}
void beginCalibration(){calReady=false;calPending=true;alarmUntil=0;clearWindow();Serial.println("CAL START: hold still for 3 seconds");}
bool validCalibration(const Calibration &c){
  if(c.version!=2||!isfinite(c.norm)||c.norm<0.8f||c.norm>1.2f)return false;
  for(int i=0;i<3;i++)if(!isfinite(c.ref[i])||!isfinite(c.bias[i])||fabsf(c.bias[i])>8)return false;
  return fabsf(magnitude(c.ref[0],c.ref[1],c.ref[2])-c.norm)<0.001f;
}
void calibrate(const Packet &p){
  if(!calPending)return;if(!p.valid){clearWindow();return;}
  float v[]={p.ax,p.ay,p.az,p.gx,p.gy,p.gz};
  float a=magnitude(p.ax,p.ay,p.az),g=magnitude(p.gx,p.gy,p.gz);
  if(a<0.8f||a>1.2f||g>8){clearWindow();return;}
  if(calCount){
    float da=magnitude(v[0]-previous[0],v[1]-previous[1],v[2]-previous[2]);
    float dg=magnitude(v[3]-previous[3],v[4]-previous[4],v[5]-previous[5]);
    if(da>0.035f||dg>1.5f||uint32_t(p.us-calLastUs)>100000)clearWindow();
  }
  if(!calCount)calFirstUs=p.us;calLastUs=p.us;
  for(int i=0;i<6;i++){sum[i]+=v[i];square[i]+=(double)v[i]*v[i];previous[i]=v[i];}calCount++;
  if(calCount<CAL_SAMPLES||uint32_t(p.us-calFirstUs)<2900000)return;
  float av=0,gv=0;
  for(int i=0;i<6;i++){double mean=sum[i]/calCount;float variance=fmax(0.0,square[i]/calCount-mean*mean);if(i<3)av+=variance;else gv+=variance;}
  calNoiseG=sqrtf(av);calNoiseDps=sqrtf(gv);
  if(calNoiseG>0.02f||calNoiseDps>0.8f){clearWindow();return;}
  Calibration candidate={};candidate.version=2;
  for(int i=0;i<3;i++){candidate.ref[i]=sum[i]/calCount;candidate.bias[i]=sum[i+3]/calCount;}
  candidate.norm=magnitude(candidate.ref[0],candidate.ref[1],candidate.ref[2]);
  if(!validCalibration(candidate)){clearWindow();return;}
  cal=candidate;calId++;calReady=true;calPending=false;
  calSaved=prefs.putBytes("reference",&cal,sizeof(cal))==sizeof(cal);
  Serial.printf("CAL OK id=%lu ref=%.5f,%.5f,%.5f bias=%.4f,%.4f,%.4f saved=%u\n",(unsigned long)calId,cal.ref[0],cal.ref[1],cal.ref[2],cal.bias[0],cal.bias[1],cal.bias[2],calSaved);
}
uint8_t lcdAddr=0;char lcdPrevious[2][17]={};
void lcdOut(uint8_t b){Wire.beginTransmission(lcdAddr);Wire.write(b|8);if(Wire.endTransmission()!=0)lcdAddr=0;}
void lcdNibble(uint8_t b){lcdOut(b);lcdOut(b|4);delayMicroseconds(2);lcdOut(b);delayMicroseconds(50);}
void lcdByte(uint8_t b,bool rs=false){lcdNibble((b&0xf0)|(rs?1:0));lcdNibble((b<<4)|(rs?1:0));}
void lcdLine(int row,const char *s){
  if(!lcdAddr)return;char line[17];memset(line,' ',16);line[16]=0;memcpy(line,s,strnlen(s,16));
  if(!memcmp(lcdPrevious[row],line,17))return;
  lcdByte(row?0xc0:0x80);for(int i=0;i<16;i++)lcdByte(line[i],true);memcpy(lcdPrevious[row],line,17);
}
void lcdInit(){
  Wire.begin(21,22);Wire.setTimeOut(10);for(uint8_t a:{0x27,0x3f}){Wire.beginTransmission(a);if(!Wire.endTransmission()){lcdAddr=a;break;}}
  if(!lcdAddr)return;delay(50);lcdNibble(0x30);delay(5);lcdNibble(0x30);delay(5);lcdNibble(0x30);lcdNibble(0x20);
  lcdByte(0x28);lcdByte(0x0c);lcdByte(1);delay(2);lcdByte(6);memset(lcdPrevious,0,sizeof(lcdPrevious));
}
void receive(const esp_now_recv_info_t *info,const uint8_t *data,int len){
  if(len!=sizeof(Packet)||memcmp(info->src_addr,SENSOR_MAC,6))return;Packet p;memcpy(&p,data,sizeof(p));
  if(p.magic!=MAGIC||p.valid>1||!isfinite(p.ax)||!isfinite(p.ay)||!isfinite(p.az)||!isfinite(p.gx)||!isfinite(p.gy)||!isfinite(p.gz)||!isfinite(p.temp))return;
  portENTER_CRITICAL(&mux);uint32_t id=++rxHead;queue[id%QUEUE_SIZE]={p,id,millis(),info->rx_ctrl->rssi};portEXIT_CRITICAL(&mux);
}
Record *latestRecord(){return historyCount?&history[(historyCount-1)%HISTORY_SIZE]:nullptr;}
void processSamples(){
  for(int work=0;work<96;work++){
    Incoming in;bool found=false;portENTER_CRITICAL(&mux);
    if(rxHead>processed){if(rxHead-processed>QUEUE_SIZE){queueLost+=rxHead-processed-QUEUE_SIZE;processed=rxHead-QUEUE_SIZE;}in=queue[(++processed)%QUEUE_SIZE];found=true;}
    portEXIT_CRITICAL(&mux);if(!found)break;Packet &p=in.p;
    if(haveSource){
      if(p.seq==lastSeq)continue;bool timeWrap=p.us<lastSourceUs&&lastSourceUs-p.us>0x80000000UL;
      bool reboot=p.seq<lastSeq&&p.us<lastSourceUs&&!timeWrap;
      if(reboot){nodeSession++;sourceWrap=0;beginCalibration();}
      else {if((int32_t)(p.seq-lastSeq)<=0)continue;if(timeWrap)sourceWrap+=(1ULL<<32);uint32_t diff=p.seq-lastSeq;if(diff>1&&diff<1000000)sourceLost+=diff-1;}
    }
    haveSource=true;lastSeq=p.seq;lastSourceUs=p.us;calibrate(p);
    Record r={};r.in=in;r.sourceUs=sourceWrap+p.us;r.nodeSession=nodeSession;r.calId=calReady?calId:0;r.state=!p.valid?4:!calReady?3:0;
    float scale=calReady?cal.norm:1;r.ax=p.ax/scale;r.ay=p.ay/scale;r.az=p.az/scale;
    r.gx=p.gx-(calReady?cal.bias[0]:0);r.gy=p.gy-(calReady?cal.bias[1]:0);r.gz=p.gz-(calReady?cal.bias[2]:0);
    r.norm=magnitude(r.ax,r.ay,r.az);r.deviation=fabsf(r.norm-1);
    if(calReady&&p.valid){float rawNorm=magnitude(p.ax,p.ay,p.az);float cosine=rawNorm>0.01f?(p.ax*cal.ref[0]+p.ay*cal.ref[1]+p.az*cal.ref[2])/(rawNorm*cal.norm):1;
      r.tilt=acosf(fmaxf(-1,fminf(1,cosine)))*180/PI;bool threshold=r.tilt>50||r.deviation>0.8f;
      if(threshold)alarmUntil=millis()+1000;r.state=threshold||(int32_t)(alarmUntil-millis())>0?2:(r.tilt>30||r.deviation>0.35f)?1:0;}
    history[historyCount%HISTORY_SIZE]=r;historyCount++;
  }
}
const char *stateCode(const Record *r){
  if(!r||millis()-r->in.rxMs>=500)return "offline";if(!r->in.p.valid)return "sensor_error";
  if(!calReady)return "calibrating";return r->state==2?"threshold":r->state==1?"warning":"normal";
}
void appendRecord(String &out,const Record &r){
  char b[640];const Packet &p=r.in.p;
  snprintf(b,sizeof(b),"[%lu,%lu,%lu,%llu,%lu,%u,%.5f,%.5f,%.5f,%.4f,%.4f,%.4f,%.2f,%.5f,%.5f,%.5f,%.4f,%.4f,%.4f,%.5f,%.3f,%.5f,%lu,%u]",(unsigned long)r.in.id,(unsigned long)r.nodeSession,(unsigned long)p.seq,(unsigned long long)r.sourceUs,(unsigned long)r.in.rxMs,p.valid,p.ax,p.ay,p.az,p.gx,p.gy,p.gz,p.temp,r.ax,r.ay,r.az,r.gx,r.gy,r.gz,r.norm,r.tilt,r.deviation,(unsigned long)r.calId,r.state);out+=b;
}
void api(){
  processSamples();Record *r=latestRecord();uint32_t after=web.hasArg("after")?strtoul(web.arg("after").c_str(),nullptr,10):0;
  uint32_t oldestIndex=historyCount>HISTORY_SIZE?historyCount-HISTORY_SIZE:0;
  uint32_t firstId=historyCount?history[oldestIndex%HISTORY_SIZE].in.id:0,newestId=r?r->in.id:0;
  if(!web.hasArg("after"))after=newestId;uint32_t lost=after&&firstId>after+1?firstId-after-1:0;
  char meta[1500];snprintf(meta,sizeof(meta),"{\"epoch\":%lu,\"state\":\"%s\",\"age_ms\":%lu,\"rssi_dbm\":%d,\"packet_hz\":%.1f,\"received\":%lu,\"radio_missing\":%lu,\"queue_missing\":%lu,\"history_gap\":%lu,\"lcd\":%u,\"cal\":{\"ready\":%s,\"progress\":%u,\"id\":%lu,\"saved\":%s,\"noise_g\":%.5f,\"noise_dps\":%.4f,\"reference_g\":[%.6f,%.6f,%.6f],\"gyro_bias_dps\":[%.5f,%.5f,%.5f],\"scale_g\":%.6f},\"latest\":",(unsigned long)bootId,stateCode(r),(unsigned long)(r?millis()-r->in.rxMs:0xffffffffUL),r?r->in.rssi:-127,packetHz,(unsigned long)(r?r->in.id:0),(unsigned long)sourceLost,(unsigned long)queueLost,(unsigned long)lost,lcdAddr,calReady?"true":"false",(unsigned)fminf(100,100.0f*calCount/CAL_SAMPLES),(unsigned long)calId,calSaved?"true":"false",calNoiseG,calNoiseDps,cal.ref[0],cal.ref[1],cal.ref[2],cal.bias[0],cal.bias[1],cal.bias[2],cal.norm);
  String response;response.reserve(20000);response=meta;if(r)appendRecord(response,*r);else response+="null";response+=",\"samples\":[";
  uint16_t sent=0;uint32_t cursor=after;
  for(uint32_t i=oldestIndex;i<historyCount&&sent<48;i++){const Record &s=history[i%HISTORY_SIZE];if(s.in.id<=after)continue;if(sent++)response+=',';appendRecord(response,s);cursor=s.in.id;}
  response+="],\"cursor\":";response+=cursor;response+=",\"more\":";response+=cursor<newestId?"true":"false";response+='}';
  web.sendHeader("Cache-Control","no-store");web.send(200,"application/json",response);
}
void setup(){
  Serial.begin(115200);history=new(std::nothrow) Record[HISTORY_SIZE];
  if(!history){Serial.println("HISTORY ALLOCATION FAILED");while(true)delay(1000);}
  bootId=esp_random();prefs.begin("mpu-reference",false);
  // A fresh stationary reference is always measured at boot. Stored values are diagnostic only.
  if(prefs.getBytesLength("reference")==sizeof(cal)){prefs.getBytes("reference",&cal,sizeof(cal));if(!validCalibration(cal))memset(&cal,0,sizeof(cal));}
  for(int pin:{GREEN,YELLOW,RED}){pinMode(pin,OUTPUT);digitalWrite(pin,LOW);}lcdInit();beginCalibration();
  WiFi.mode(WIFI_AP);WiFi.setSleep(false);if(!WiFi.softAP(AP_SSID,AP_PASSWORD,6)){Serial.println("AP FAILED");while(true)delay(1000);}
  delay(100);esp_wifi_set_channel(6,WIFI_SECOND_CHAN_NONE);
  if(esp_now_init()!=ESP_OK||esp_now_register_recv_cb(receive)!=ESP_OK){Serial.println("RADIO FAILED");while(true)delay(1000);}
  web.on("/",[]{web.sendHeader("Cache-Control","no-store");web.send_P(200,"text/html; charset=utf-8",DASHBOARD);});
  web.on("/api",HTTP_GET,api);web.on("/calibrate",HTTP_POST,[]{beginCalibration();web.send(202,"application/json","{\"ok\":true}");});
  web.onNotFound([]{web.send(404,"text/plain; charset=utf-8","Sahifa topilmadi");});web.begin();rateAt=millis();Serial.println("GATEWAY V2 READY http://192.168.4.1");
}
void loop(){
  processSamples();web.handleClient();uint32_t now=millis();Record *r=latestRecord();const char *state=stateCode(r);
  bool online=r&&now-r->in.rxMs<500&&r->in.p.valid;
  digitalWrite(GREEN,online&&calReady&&r->state==0);digitalWrite(YELLOW,!online||!calReady||(r&&r->state==1));digitalWrite(RED,online&&calReady&&r->state==2);
  if(!online&&calPending)clearWindow();
  if(now-rateAt>=1000){uint32_t count=r?r->in.id:0;packetHz=1000.0f*(count-rateCount)/(now-rateAt);rateCount=count;rateAt=now;}
  static uint32_t lcdRetry=0;if(!lcdAddr&&now-lcdRetry>2000){lcdRetry=now;lcdInit();}
  if(now-lastLcd>=500){lastLcd=now;char line[24];
    if(!online){lcdLine(0,"ALOQA UZILDI");lcdLine(1,"Sensorni tekshir");}
    else if(!calReady){lcdLine(0,"KALIBRLANMOQDA");snprintf(line,sizeof(line),"Qimirlatmang %u%%",(unsigned)(100*calCount/CAL_SAMPLES));lcdLine(1,line);}
    else {lcdLine(0,r->state==2?"CHEGARA OSHDI":r->state==1?"DIQQAT":"MEYORDA");snprintf(line,sizeof(line),"Q:%.1f A:%.2fg",r->tilt,r->norm);lcdLine(1,line);}}
  if(now-lastLog>=2000){lastLog=now;Serial.printf("V2 RX=%lu Hz=%.1f state=%s cal=%lu progress=%u tilt=%.2f gyro=%.3f LCD=0x%02x lost=%lu\n",(unsigned long)(r?r->in.id:0),packetHz,state,(unsigned long)calId,calCount,r?r->tilt:0,r?magnitude(r->gx,r->gy,r->gz):0,lcdAddr,(unsigned long)sourceLost);}
  delay(1);
}
