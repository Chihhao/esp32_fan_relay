#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

#include "index.h" 

#define VERSION "2022-09-08a"

#define RELAY1 16
#define RELAY2 17
#define RELAY3 18
#define RELAY4 19

#define BUTTON1 33
#define BUTTON2 32
#define BUTTON3 26
#define BUTTON4 25
#define BUTTON_PRESSED LOW


#define RELAY_ON LOW
#define RELAY_OFF HIGH

const char* ssid = "xxxxx";
const char* password = "xxxxx";

WebServer server(80);

String hostname = "ESP32_Fan_Control";

int nowStatus = 0;

void setup() {
  Serial.begin(115200);
  delay(20);  
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT); 
  digitalWrite(RELAY1, RELAY_OFF);   
  digitalWrite(RELAY2, RELAY_OFF);   
  digitalWrite(RELAY3, RELAY_OFF);   
  digitalWrite(RELAY4, RELAY_OFF);   

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  pinMode(BUTTON4, INPUT_PULLUP);   


  xTaskCreatePinnedToCore(
    Task_KeepWifi, //本任務實際對應的Function
    "Task_KeepWifi", //任務名稱（自行設定）
    8192, //所需堆疊空間（常用10000）
    NULL, //輸入值
    0, //優先序：0為最低，數字越高代表越優先
    NULL, //對應的任務handle變數
    0); //指定執行核心編號（0、1或tskNO_AFFINITY：系統指定）
  
}

void webServerInit(){
    server.stop();
    server.on("/", handleRoot);
  
    server.on("/readLevel", []() {
      server.send(200, "text/plain", statusStr());
    });
  
    server.on("/readIP", []() {
      server.send(200, "text/plain", WiFi.localIP().toString());
    });
  
    server.onNotFound(handleNotFound);
  
  /*handling uploading firmware file */
    server.on("/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    }, []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        /* flashing firmware to ESP*/
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
      }
    });
    server.begin();
    Serial.println("HTTP server started");
}

void Task_KeepWifi(void * pvParameters){

    WiFi.mode(WIFI_STA);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(hostname.c_str());  //define hostname
    WiFi.begin(ssid, password);
    unsigned long timeStamp_wifi = millis();

    bool MDNS_need_to_reset = true;
    
    if(WiFi.status() == WL_CONNECTED){
        Serial.print("Connected to ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());           
    }   
    
    while(true){
        delayMicroseconds(1);
        
        if (WiFi.status() == WL_CONNECTED) { 
            if(MDNS_need_to_reset){
                webServerInit();
                if (MDNS.begin("ESP32_Fan_Control")) {
                  Serial.println("MDNS responder started");
                }
                MDNS_need_to_reset = false;
            }          
            continue; 
        }
        else{ 
          
          MDNS_need_to_reset = true;          
          if (millis() - timeStamp_wifi >= 15000) {
              Serial.println("Reconnecting to WiFi...");
              WiFi.disconnect();
              WiFi.reconnect();
              timeStamp_wifi = millis();                    
          }
          
        }

    }
}


void loop() {  
  if (WiFi.status() == WL_CONNECTED) {
      server.handleClient();
  }
  checkButton();  
  delay(1);
}

void handleRoot() {
  //Serial.println("==[start]====================");
  String message;
  message += "server.args()=";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";  }
  //Serial.print(message);
  //Serial.println("---");
  if(server.hasArg("level")){
    Serial.print("--->get level=");
    Serial.println(server.arg("level"));
    int _relayNo = (server.arg("level")).toInt();

    SwitchRelay(_relayNo);

  }
  //Serial.println("==[end]======================");

  String s = MAIN_page;  
  s.replace("__VERSION__", VERSION);
  s.replace("__IP__", WiFi.localIP().toString());
  server.send(200, "text/html", s);
  delay(1); 
  
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void SwitchRelay(int _RELAY){
    if(_RELAY == nowStatus) return ;
    
    nowStatus = _RELAY;
    
    digitalWrite(RELAY1, RELAY_OFF);   
    digitalWrite(RELAY2, RELAY_OFF);   
    digitalWrite(RELAY3, RELAY_OFF);   
    digitalWrite(RELAY4, RELAY_OFF);  
     
    if(_RELAY==0) {return;}    
    switch(_RELAY){
      case 1: digitalWrite(RELAY1, RELAY_ON); break;
      case 2: digitalWrite(RELAY2, RELAY_ON); break;
      case 3: digitalWrite(RELAY3, RELAY_ON); break;
      case 4: digitalWrite(RELAY4, RELAY_ON); break;
    }    
}

String statusStr(){  
    switch(nowStatus){
      case 0: return "停";
      case 1: return "弱";
      case 2: return "中";
      case 3: return "強";
    }  
}

void checkButton(){
  if(digitalRead(BUTTON1) == BUTTON_PRESSED){
      Serial.println("1");
      SwitchRelay(0); 
      delay(500);
  }
  else if(digitalRead(BUTTON2) == BUTTON_PRESSED){
      Serial.println("2");
      SwitchRelay(1); 
      delay(500);
  }
  else if(digitalRead(BUTTON3) == BUTTON_PRESSED){
      Serial.println("3");
      SwitchRelay(2); 
      delay(500);
  }
  else if(digitalRead(BUTTON4) == BUTTON_PRESSED){
      Serial.println("4");
      SwitchRelay(3); 
      delay(500);
  }

  
}
