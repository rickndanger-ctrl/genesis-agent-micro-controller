#include <Arduino.h>
#include <USB.h>
#include <USBHID.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Preferences.h>
#include "Attention.h"
#include "Completion.h"
#include "MatrixAnimation.h"
#include "SlotNavigation.h"
#include "FrameDecoder.h"
#include "BoardConfig.h"
#include "Encoder.h"
#include "KnobGesture.h"
#include "CodexGesture.h"
#if ARDUINO_USB_MODE != 0
#error Select USB-OTG TinyUSB mode
#endif

USBHID hid;
Adafruit_NeoPixel pixel(1, PixelPin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel matrix(25, MatrixData, NEO_GRB + NEO_KHZ800);
Adafruit_ST7735 screen(DisplayCS, DisplayDC, DisplayReset);
static const uint8_t descriptor[] = {
  0x06,0x00,0xff,0x09,0x01,0xa1,0x01,0x85,0x06,0x09,0x02,
  0x15,0x00,0x26,0xff,0x00,0x75,0x08,0x95,0x3f,0x81,0x02,
  0x09,0x03,0x15,0x00,0x26,0xff,0x00,0x75,0x08,0x95,0x3f,0x91,0x02,0xc0
};
struct Packet { uint8_t bytes[63]; };
QueueHandle_t incoming;
volatile bool dropped = false;
FrameDecoder decoder;
bool claudeMode = false;
uint32_t lastClaude = 0;
bool claudeSeen = false;
uint8_t selected = 0;
uint8_t agentMenuIndex = 0; // Six agent slots, then the Actions menu.
uint32_t slotColor[6] = {};
uint32_t codexColor[6] = {};
float codexBrightness[6] = {1,1,1,1,1,1};
float slotBrightness[6] = {1,1,1,1,1,1};
uint32_t lastHost = 0;
bool hostSeen = false;
enum Mode { Agents, Actions, Dial };
Mode mode = Agents;
uint8_t actionIndex = 0;
const char *actionNames[] = {"Fast mode", "Approve", "Decline", "Fork chat", "Send prompt", "Hold to talk"};
const char *claudeNames[] = {"Send prompt", "Hold to talk"};
const char *claudeKeys[] = {"SEND", "DICTATE"};
int soundIndex() { return claudeMode?2:6; }
int testIndex() { return soundIndex()+1; }
int backIndex() { return soundIndex()+2; }
const char *actionKeys[] = {"ACT06", "ACT07", "ACT08", "ACT09", "ACT12", "ACT10"};
Preferences preferences;
AttentionState attention;
CompletionState completion;
MatrixAnimation matrixAnimation;
SlotNavigation codexNavigation;
uint32_t completionCount = 0;
uint32_t testCount=0, matrixFrameCount=0;
bool soundEnabled = true, buzzerReady = false;
bool talkAfterSelect = true;
uint32_t beepStarted = 0, testStarted = 0, alertCount = 0;
bool beeping = false, testing = false;
uint8_t attentionMask() {
  uint8_t mask = 0;
  for (int i=0;i<6;++i) if (slotBrightness[i]>0 && needsAttention(slotColor[i])) mask |= 1<<i;
  return mask;
}
void startBeep() {
  if(soundEnabled && buzzerReady) {beeping=true;beepStarted=millis();}
}
void updateBuzzer() {
  uint32_t elapsed=millis()-beepStarted;
  if(!soundEnabled || elapsed>=240) beeping=false;
  bool on=beeping && (elapsed<80 || elapsed>=160);
  if(testing && soundEnabled && buzzerReady) on=(millis()-testStarted)<3000 && (millis()-testStarted)%600<180;
  static bool wasOn=false;
  if(on!=wasOn) {ledcWriteTone(BuzzerPin,on?2700:0);wasOn=on;}
}
bool paintNeeded = true;
void startTest() {testing=true;testStarted=millis();++testCount;paintNeeded=true;matrixAnimation.wake(millis());}
volatile int encoderTicks = 0;
EncoderDecoder encoderDecoder;
int encoderDetents=1;
portMUX_TYPE encoderMux = portMUX_INITIALIZER_UNLOCKED;
void ARDUINO_ISR_ATTR encoderChanged() {
  uint8_t next = (digitalRead(EncoderA)<<1)|digitalRead(EncoderB);
  portENTER_CRITICAL_ISR(&encoderMux);
  encoderTicks += encoderDecoder.update(next);
  portEXIT_CRITICAL_ISR(&encoderMux);
}
struct Button {
  int pin; bool activeHigh=false; bool raw=false, stable=false; uint32_t changedAt=0;
  int poll() {
    bool down = digitalRead(pin)==(activeHigh ? HIGH : LOW);
    if(down!=raw) {raw=down;changedAt=millis();}
    if(raw!=stable && millis()-changedAt>=25) {stable=raw;return stable?1:-1;}
    return 0;
  }
};
// Live capture: the reseated encoder is HIGH during a press and LOW at rest.
Button encoderButton{EncoderButton, true}, userButton{UserButton};
KnobGesture knobGesture;
CodexGesture codexGesture;
char heldKey[12] = {};
int heldAgent = -1;

class GenesisHID : public USBHIDDevice {
public:
  GenesisHID() { hid.addDevice(this, sizeof(descriptor)); }
  uint16_t _onGetDescriptor(uint8_t *b) override { memcpy(b, descriptor, sizeof(descriptor)); return sizeof(descriptor); }
  void receive(uint8_t id, const uint8_t *data, uint16_t len) {
    if (id != 6 || !incoming) return;
    if (len == 64 && data[0] == 6) { ++data; --len; }
    if (len != 63) { dropped = true; return; }
    Packet packet; memcpy(packet.bytes, data, 63);
    if (xQueueSend(incoming, &packet, 0) != pdTRUE) dropped = true;
  }
  void _onOutput(uint8_t id, const uint8_t *b, uint16_t len) override { receive(id,b,len); }
  void _onSetFeature(uint8_t id, const uint8_t *b, uint16_t len) override { receive(id,b,len); }
} device;

bool sendJson(const JsonDocument &doc) {
  if (!hid.ready()) return false;
  String wire; serializeJson(doc, wire); wire += "\r\n";
  for (size_t offset = 0; offset < wire.length(); offset += 61) {
    uint8_t packet[63] = {2,0};
    packet[1] = min(size_t(61), wire.length() - offset);
    memcpy(packet+2, wire.c_str()+offset, packet[1]);
    if (!hid.SendReport(6, packet, sizeof(packet))) return false;
  }
  return true;
}
void key(const char *name, uint8_t action, int agent = -1) {
  JsonDocument doc; doc["m"] = claudeMode?"genesis.input":"v.oai.hid";
  doc["p"]["k"] = name; doc["p"]["act"] = action;
  if (agent >= 0) doc["p"]["ag"] = agent;
  sendJson(doc);
}
void switchControllerApp() {
  matrixAnimation.wake(millis());
  // Release the previous app's key before changing the event destination.
  if(*heldKey) {key(heldKey,0,heldAgent);*heldKey=0;}
  heldAgent=-1;knobGesture=KnobGesture{};codexGesture=CodexGesture{};
  mode=Agents;
claudeMode=!claudeMode;claudeSeen=false;hostSeen=false;lastHost=0;selected=0;agentMenuIndex=0;actionIndex=0;
memset(slotColor,0,sizeof(slotColor));attention=AttentionState{};completion=CompletionState{};beeping=false;testing=false;
if(!claudeMode) {memcpy(slotColor,codexColor,sizeof(slotColor));memcpy(slotBrightness,codexBrightness,sizeof(slotBrightness));}
JsonDocument notice;notice["m"]="genesis.input";notice["p"]["k"]="MODE";notice["p"]["act"]=1;notice["p"]["mode"]=claudeMode?"claude":"codex";sendJson(notice);paintNeeded=true;
}
void selectAgent() {
  char name[5]; snprintf(name,sizeof(name),"AG%02u",selected);
  key(name,1,selected); delay(10); key(name,0,selected);
}
void handleMessage(const char *bytes, size_t size) {
  JsonDocument request;
  if (deserializeJson(request, bytes, size) || !request.is<JsonObject>()) return;
  const char *method = request["m"] | request["method"] | "";
  JsonVariant params = request["p"].isNull() ? request["params"] : request["p"];
  if (!*method) return;
  lastHost = millis(); if(!hostSeen && !claudeMode) paintNeeded=true; hostSeen = true;
  JsonDocument reply; reply["id"] = request["id"];
  if (!strcmp(method,"device.status")) {
    auto result = reply["result"].to<JsonObject>();
    result["version"] = "v0.2.15-genesis";
    result["profile_index"] = 0; result["layer_index"] = 1;
    result["controller_mode"] = claudeMode?"claude":"codex";
    result["adapter"] = "Genesis Mini USB prototype";
    result["encoder_detents"] = encoderDetents;
    result["knob_down"] = encoderButton.stable;
    result["gesture_state"] = int(codexGesture.state);
    result["menu"] = int(mode);result["selection"] = mode==Agents?agentMenuIndex:actionIndex;
    result["sound_enabled"] = soundEnabled; result["buzzer_ready"] = buzzerReady;
    result["attention_mask"] = attentionMask(); result["alert_count"] = alertCount;
    result["completion_count"] = completionCount;
    result["codex_chat_mask"] = codexNavigation.chats;
    result["codex_slots_known"] = codexNavigation.known && uint32_t(millis()-codexNavigation.received)<6000;
    result["test_count"] = testCount;result["testing"] = testing;
    result["matrix_frames"] = matrixFrameCount;result["matrix_pin"] = MatrixData;
    result["buzzer_pin"] = BuzzerPin;
    uint8_t completionMask=0;
    for(int i=0;i<6;++i) if(completion.active(i,millis())) completionMask|=1<<i;
    result["completion_mask"] = completionMask;
    result["matrix_sleeping"] = matrixAnimation.asleep(millis()) && !completionMask && !attentionMask();
  } else if (!strcmp(method,"genesis.settings")) {
    if(params["codex_chat_mask"].is<int>()) {int mask=params["codex_chat_mask"].as<int>();if(mask>=0 && mask<=63)codexNavigation.update(mask,millis());}
    if(params["talk_after_select"].is<bool>()) {talkAfterSelect=params["talk_after_select"].as<bool>();preferences.putBool("talkJump",talkAfterSelect);}
    if((params["focus_dictate"]|false) && claudeMode) {mode=Actions;actionIndex=1;paintNeeded=true;}
    if(params["sound_enabled"].is<bool>()) {soundEnabled=params["sound_enabled"].as<bool>();preferences.putBool("sound",soundEnabled);paintNeeded=true;}
    if(params["encoder_detents"].is<int>()) {
      int value=params["encoder_detents"].as<int>();
      if(value==1 || value==2) {
        portENTER_CRITICAL(&encoderMux);encoderDetents=value;encoderTicks=0;
        encoderDecoder.reset((digitalRead(EncoderA)<<1)|digitalRead(EncoderB));portEXIT_CRITICAL(&encoderMux);
        preferences.putInt("detents",value);
      }
    }
    if(params["test_alert"]|false) startTest();
    reply["result"]["ok"]=1;
  } else if (!strcmp(method,"sys.version")) {
    reply["result"]["version"] = "v0.2.15-genesis";
  } else if (!strcmp(method,"v.oai.thstatus") || !strcmp(method,"genesis.claude.status")) {
    bool fromClaude=!strcmp(method,"genesis.claude.status");
    if(!fromClaude && params.is<JsonArray>()) {
      for(JsonObject item:params.as<JsonArray>()) {int id=item["id"]|-1;if(id>=0&&id<6) {
        if(!item["c"].isNull()) codexColor[id]=item["c"].as<uint32_t>();
        if(!item["b"].isNull()) codexBrightness[id]=constrain(item["b"].as<float>(),0.0f,1.0f);
      }}
    }
    if(fromClaude!=claudeMode) {reply["result"]["ok"]=1;sendJson(reply);return;}
    if(fromClaude) {lastClaude=millis();if(!claudeSeen)paintNeeded=true;claudeSeen=true;}
    if (!params.is<JsonArray>()) return;
    bool newAttention=false;
    for (JsonObject item : params.as<JsonArray>()) {
      int id = item["id"] | -1;
      if (id >= 0 && id < 6) {
        uint32_t oldColor=slotColor[id];float oldBrightness=slotBrightness[id];
        if (!item["c"].isNull()) slotColor[id] = item["c"].as<uint32_t>();
        if (!item["b"].isNull()) slotBrightness[id] = constrain(item["b"].as<float>(), 0.0f, 1.0f);
        if(oldColor!=slotColor[id] || oldBrightness!=slotBrightness[id])paintNeeded=true;
        if(attention.update(id,slotColor[id],slotBrightness[id])) newAttention=true;
        if(completion.update(id,slotColor[id],slotBrightness[id],millis())) ++completionCount;
      }
    }
    if(newAttention) {++alertCount;startBeep();}
    reply["result"]["ok"] = 1; reply["method"] = method;
  } else if (!strcmp(method,"v.oai.rgbcfg") || !strcmp(method,"host.focused_app")) {
    // Volatile acknowledgement only: no persistent configuration writes.
    reply["result"]["ok"] = 1; reply["method"] = method;
  } else {
    if (request["id"].isNull()) return;
    reply["error"]["code"] = 404;
    reply["error"]["message"] = "Unsupported by Genesis adapter";
  }
  sendJson(reply);
}
uint32_t slotRGB(uint8_t slot) {
  uint32_t c=slotColor[slot];float b=slotBrightness[slot];
  return pixel.Color(((c>>16)&255)*b,((c>>8)&255)*b,(c&255)*b);
}
#include "GenesisLogo.h"
static const uint32_t keyColors[6]={0x3478f6,0x9b5cf6,0xf0529c,0xff9933,0x30c778,0x29c4de};
void paint() {
  static GFXcanvas16 frame(160,80);
  frame.fillScreen(ST77XX_BLACK);
  frame.setTextSize(1); frame.setTextWrap(false);
  frame.setCursor(2,2);frame.setTextColor(0x7DFF);frame.print(claudeMode?"GENESIS CLAUDE":"GENESIS CODEX");
  frame.setCursor(102,2);
  bool linked=claudeMode?claudeSeen:hostSeen;
  frame.setTextColor(linked?ST77XX_GREEN:ST77XX_YELLOW);
  frame.print(hid.ready()?(linked?"LINKED":"WAIT"):"USB");
  frame.drawRGBBitmap(144,0,genesisLogo,16,16);
  if(mode==Agents && agentMenuIndex==7) {
    frame.setTextSize(2);frame.setCursor(4,28);frame.setTextColor(ST77XX_WHITE);frame.print(claudeMode?"App:Claude":"App:Codex");
    frame.setTextSize(1);frame.setCursor(2,64);frame.print(claudeMode?"Click:switch to Codex":"Click:switch to Claude");
  } else if(mode==Agents && agentMenuIndex==6) {
    frame.setTextSize(2);frame.setCursor(16,28);frame.setTextColor(ST77XX_WHITE);frame.print("Actions >");
    frame.setTextSize(1);frame.setCursor(2,64);frame.setTextColor(0xBDF7);frame.print("Click:open actions");
  } else if(mode==Agents) {
    for(int i=0;i<6;++i) {
      int x=2+(i%3)*53,y=14+(i/3)*23;
      frame.drawRoundRect(x,y,50,21,3,i==selected?ST77XX_WHITE:0x3186);
      uint32_t c=keyColors[i];
      frame.fillCircle(x+8,y+10,3,screen.color565(c>>16,c>>8,c));
      frame.setCursor(x+17,y+7);frame.setTextColor(screen.color565(c>>16,c>>8,c));
      frame.print(claudeMode?"C":"A");frame.print(i+1);
    }
    frame.setCursor(2,64);frame.setTextColor(0xBDF7);frame.print(claudeMode?"After C6: Actions / App":"After A6: Actions / App");
  } else if(mode==Actions) {
    int count=soundIndex();
    frame.setCursor(4,18);frame.setTextColor(0xBDF7);frame.print(claudeMode?"CLAUDE ACTION":"CODEX ACTION");
    const char *label=actionIndex==backIndex()?"< Back":actionIndex==testIndex()?"Test alert":actionIndex==soundIndex()?(soundEnabled?"Sound: ON":"Sound: OFF"):(claudeMode?claudeNames[actionIndex]:actionNames[actionIndex]);
    frame.setTextSize(2);frame.setCursor(4,35);frame.setTextColor(ST77XX_WHITE);frame.print(label);
    frame.setTextSize(1);frame.setCursor(2,64);frame.setTextColor(0xBDF7);frame.print(actionIndex>=count?"Click:select":(claudeMode?"Click:run Claude action":actionIndex==5?"Hold knob:talk":"Click:run host mapping"));
  } else {
    frame.setTextSize(2);frame.setCursor(12,24);frame.setTextColor(ST77XX_WHITE);frame.print("CODEX DIAL");
    frame.setTextSize(1);frame.setCursor(2,51);frame.setTextColor(0xBDF7);frame.print("Native turn + click");
    frame.setCursor(2,64);frame.print("Uses app dial setting");
  }
  frame.setCursor(2,72);frame.setTextColor(0x7DFF);frame.print("Double click: chats");
  int replySlot=completion.animationSlot(millis());
  if(replySlot>=0) {
    uint32_t rgb=keyColors[replySlot];
    uint16_t border=screen.color565((rgb>>16)&255,(rgb>>8)&255,rgb&255);
    if((millis()-completion.started[replySlot])%800<400) frame.drawRect(0,14,160,66,border);
  }
  if(testing) {
    uint32_t rgb=keyColors[selected%6];
    frame.fillRect(0,60,160,20,screen.color565(rgb>>16,rgb>>8,rgb));
    frame.setTextSize(1);frame.setCursor(12,66);frame.setTextColor(ST77XX_WHITE);frame.print("TEST ALERT RUNNING");
  }
  screen.drawRGBBitmap(0,0,frame.getBuffer(),160,80);
}
void setup() {
  incoming = xQueueCreate(32,sizeof(Packet));
  pinMode(UserButton, INPUT_PULLUP);pinMode(EncoderButton,INPUT_PULLUP);
  pinMode(EncoderA,INPUT_PULLUP);pinMode(EncoderB,INPUT_PULLUP);
  pinMode(7,INPUT);pinMode(5,INPUT); // Unused P2 buzzer signals.
  pinMode(1,INPUT);pinMode(18,INPUT); // Unused P4 matrix signals; DOUT stays input.
  pinMode(BuzzerPin,OUTPUT);digitalWrite(BuzzerPin,LOW);
  buzzerReady=ledcAttach(BuzzerPin,2700,8);
  if(buzzerReady) ledcWriteTone(BuzzerPin,0);
  preferences.begin("genesis",false);
  soundEnabled=preferences.getBool("sound",true);
  talkAfterSelect=preferences.getBool("talkJump",true);
  encoderDetents=constrain(preferences.getInt("detents",1),1,2);
  encoderDecoder.reset((digitalRead(EncoderA)<<1)|digitalRead(EncoderB));
  attachInterrupt(EncoderA,encoderChanged,CHANGE);attachInterrupt(EncoderB,encoderChanged,CHANGE);
  pixel.begin();pixel.setBrightness(24);
  matrix.begin();matrix.setBrightness(45);matrix.clear();matrix.show();
  SPI.begin(14,13,12,DisplayCS);
  screen.initR(INITR_MINI160x80);screen.setRotation(1);screen.setSPISpeed(10000000);
  // This panel uses BGR filters. Keep rotation 1 geometry, correct color order.
  uint8_t displayOrder=ST77XX_MADCTL_MY|ST77XX_MADCTL_MV|ST7735_MADCTL_BGR;
  screen.sendCommand(ST77XX_MADCTL,&displayOrder,1);
  USB.VID(0x303a); USB.PID(0x8360); USB.firmwareVersion(0x0100);
  USB.manufacturerName("Work Louder compatible - Genesis DIY");
  USB.productName("Codex Micro");static char serial[32];snprintf(serial,sizeof(serial),"GENESIS-%012llX",(unsigned long long)ESP.getEfuseMac());USB.serialNumber(serial);
  hid.begin();USB.begin();paint();
}
void loop() {
  if (dropped) { xQueueReset(incoming);decoder.reset();dropped=false; }
  Packet packet;
  while(xQueueReceive(incoming,&packet,0)==pdTRUE) decoder.receive(packet.bytes,63,handleMessage);
  portENTER_CRITICAL(&encoderMux);
  int steps=encoderTicks/encoderDetents;encoderTicks-=steps*encoderDetents;
  portEXIT_CRITICAL(&encoderMux);
  int enc=encoderButton.poll();
  if(enc || steps) matrixAnimation.wake(millis());
  // Claude uses click-to-toggle actions; reserve double-click for Back.
  bool claudeTap=false;
  bool codexTap=false;
  if(claudeMode) {
    auto gesture=codexGesture.update(enc,millis());
    if(steps) {auto turned=codexGesture.turn();if(gesture!=CodexGesture::Tap)gesture=turned;}
    enc=0;
    if(gesture==CodexGesture::Back) {mode=Agents;agentMenuIndex=selected;paintNeeded=true;}
    else if(gesture==CodexGesture::Tap || gesture==CodexGesture::Up) {enc=1;claudeTap=true;}
  }
  if(!claudeMode && (mode!=Agents || codexGesture.busy())) {
    auto gesture=codexGesture.update(enc,millis());
    if(steps) {auto turned=codexGesture.turn();if(gesture!=CodexGesture::Tap)gesture=turned;}
    enc=0;
    if(gesture==CodexGesture::Back) {mode=Agents;agentMenuIndex=selected;paintNeeded=true;}
    else if(gesture==CodexGesture::Down)enc=1;
    else if(gesture==CodexGesture::Up)enc=-1;
    else if(gesture==CodexGesture::Tap){enc=1;codexTap=true;}
  }
  if(enc==1) {
    if(mode==Agents && agentMenuIndex==7) {
      switchControllerApp();
    }
    else if(mode==Agents && agentMenuIndex==6) {mode=Actions;actionIndex=0;paintNeeded=true;}
    else if(mode==Actions && actionIndex==backIndex()) {mode=Agents;agentMenuIndex=selected;paintNeeded=true;}
    else if(mode==Actions && actionIndex==soundIndex()) {soundEnabled=!soundEnabled;preferences.putBool("sound",soundEnabled);if(soundEnabled)startBeep();paintNeeded=true;}
    else if(mode==Actions && actionIndex==testIndex()) {startTest();}
    else {
      if(mode==Agents) {snprintf(heldKey,sizeof(heldKey),"AG%02u",selected);heldAgent=selected;}
      else {strlcpy(heldKey,mode==Actions?(claudeMode?claudeKeys[actionIndex]:actionKeys[actionIndex]):"ENC_CLK",sizeof(heldKey));heldAgent=-1;}
      key(heldKey,1,heldAgent);
    }
  } else if(enc==-1 && *heldKey) {key(heldKey,0,heldAgent);*heldKey=0;
    if(!claudeMode && heldAgent>=0 && talkAfterSelect && codexNavigation.jump(heldAgent,millis())) {mode=Actions;actionIndex=5;paintNeeded=true;}
  }
  if((claudeTap || codexTap) && *heldKey) {key(heldKey,0,heldAgent);*heldKey=0;}
  if(steps) {
    int direction=steps>0?1:-1;
    for(int i=0;i<abs(steps);++i) {
      if(mode==Agents) {agentMenuIndex=(agentMenuIndex+8+direction)%8;if(agentMenuIndex<6)selected=agentMenuIndex;}
      else if(mode==Actions) actionIndex=(actionIndex+backIndex()+1+direction)%(backIndex()+1);
      else key(direction>0?"ENC_CW":"ENC_CC",2);
    }
    paintNeeded=true;
  }
  if(userButton.poll()==1) switchControllerApp();
  static uint32_t painted=0;
  static bool previousReady=false;
  bool ready=hid.ready();if(ready!=previousReady) {paintNeeded=true;previousReady=ready;if(!ready){hostSeen=false;completion=CompletionState{};}}
  if(claudeMode && claudeSeen && millis()-lastClaude>6000) {claudeSeen=false;memset(slotColor,0,sizeof(slotColor));completion=CompletionState{};paintNeeded=true;}
  if(!ready) beeping=false;
  updateBuzzer();
  if(testing && millis()-testStarted>=3000) {testing=false;paintNeeded=true;}
  if(millis()-painted>60) {
    painted=millis();
    static int previousReplySlot=-1;
    static bool previousReplyPhase=false;
    int replySlot=completion.animationSlot(millis());
    bool replyPhase=replySlot>=0 && (millis()-completion.started[replySlot])%800<400;
    if(paintNeeded || replySlot!=previousReplySlot || replyPhase!=previousReplyPhase) {paint();paintNeeded=false;}
    previousReplySlot=replySlot;previousReplyPhase=replyPhase;
    // Dedicated selection identity; the matrix continues showing live agent status.
    uint32_t c=keyColors[selected%6];
    if(!ready)c=0x200000;
    else if(!hostSeen)c=(millis()%1200<600)?0x201400:0;
    int completedSlot=completion.animationSlot(millis());
    if(ready && hostSeen && completedSlot>=0 && !testing) c=keyColors[completedSlot];
    pixel.setPixelColor(0,c);pixel.show();
    bool working=false;int workingSlot=-1;
    for(int i=0;i<6;++i) if(slotBrightness[i]>0 && slotColor[i]==0x304ffe) {working=true;workingSlot=i;}
    if(slotBrightness[selected%6]>0 && slotColor[selected%6]==0x304ffe) workingSlot=selected%6;
    if(working || completedSlot>=0 || attentionMask() || testing) matrixAnimation.wake(millis());
    matrix.setBrightness(testing?64:45);
    matrix.clear();
    if(testing) {
      for(int p=0;p<25;++p) matrix.setPixelColor(p,keyColors[selected%6]);
    } else if(completedSlot>=0) {
      uint32_t elapsed=millis()-completion.started[completedSlot];
      for(unsigned y=0;y<5;++y) for(unsigned x=0;x<5;++x)
        matrix.setPixelColor(y*5+x,MatrixAnimation::dim(keyColors[completedSlot],MatrixAnimation::replyLevel(x,y,elapsed)));
    } else if(working) {
      // A faster orbit identifies the working chat; attention still takes priority.
      for(unsigned y=0;y<5;++y) for(unsigned x=0;x<5;++x)
        matrix.setPixelColor(y*5+x,MatrixAnimation::dim(keyColors[workingSlot],MatrixAnimation::idleLevel(x,y,millis()*2)));
    } else if(!matrixAnimation.asleep(millis())) {
      uint32_t elapsed=millis()-matrixAnimation.lastActivity;
      for(unsigned y=0;y<5;++y) for(unsigned x=0;x<5;++x)
        matrix.setPixelColor(y*5+x,MatrixAnimation::dim(keyColors[selected%6],MatrixAnimation::idleLevel(x,y,elapsed)));

      for(int i=0;i<6 && (elapsed<5000 || working);++i) {
        int x=(i%3)*2,y=(i/3)*3;
        uint32_t color=MatrixAnimation::dim(keyColors[i],i==selected?160:35);
        matrix.setPixelColor(y*5+x,color);matrix.setPixelColor((y+1)*5+x,color);
      }
    }
    // Pending attention always overrides animation and sleep at its own marker.
    for(int i=0;i<6 && !testing;++i) if(slotBrightness[i]>0 && needsAttention(slotColor[i])) {
      int x=(i%3)*2,y=(i/3)*3;
      uint32_t color=millis()%800<400?keyColors[i]:0;
      matrix.setPixelColor(y*5+x,color);matrix.setPixelColor((y+1)*5+x,color);
    }
    matrix.show();++matrixFrameCount;
  }
  delay(1);
}
