#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

#include "index.h" 

#define VERSION "2022-09-05a"

#define RELAY1 16
#define RELAY2 17
#define RELAY3 18
#define RELAY4 19

#define RELAY_ON LOW
#define RELAY_OFF HIGH

const char* ssid = "xxxxx";
const char* password = "xxxxx";

WebServer server(80);

String hostname = "ESP32_Fan_Control";

int nowStatus = 0;

void WIFI_Connect() {
  WiFi.disconnect();
  server.stop();
  
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str()); //define hostname
  
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("ESP32_Fan_Control")) {
    Serial.println("MDNS responder started");
  }

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
}


void loop() {
  if (WiFi.status() != WL_CONNECTED) { WIFI_Connect();}
  
  server.handleClient();
  delay(2);//allow the cpu to switch to other tasks  
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
