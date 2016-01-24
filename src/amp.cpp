#include "application.h"

#include "amp.h"
#include "xml.h"
#include "uhttp.h"

#include <stdint.h>
#include <math.h>

namespace {
    
TCPClient client;
uint8_t server[] = {192, 168, 1, 192}; // TODO: write setter
const int RESPONSE_BUF_LENGTH = 2048;
char response[RESPONSE_BUF_LENGTH];

bool sendAmp(const char* cmd, const char** body = NULL, int* length = NULL) {
  int size = strlen(cmd);
  if (client.connect(server, 80))
  {
    client.println("POST /YamahaRemoteControl/ctrl HTTP/1.1");
    client.println("Host: 192.168.1.192");
    client.print("Content-Length: "); client.println(size);
    client.println();
    client.println(cmd);

    memset(response, 0, RESPONSE_BUF_LENGTH);
    int pos = 0;
    while (client.connected()) {
        if (client.available()) {
            int c = client.read();
            if (c > -1) {
                response[pos++] = (char)c;
            }
        }
    }
    if (!client.connected()) {
        client.stop();
    }
    return uhttp::parseHeader(response, body, length) == 200;
  }
  return false;
}

} // end anonymous namespace

namespace amp {

int getVol() {
  const char *volGetCommand = "<YAMAHA_AV cmd=\"GET\"><Main_Zone><Volume><Lvl>GetParam</Lvl></Volume></Main_Zone></YAMAHA_AV>";
  //const char *resp = "<YAMAHA_AV rsp=\"GET\" RC=\"0\"><Main_Zone><Volume><Lvl><Val>-445</Val><Exp>1</Exp><Unit>dB</Unit></Lvl></Volume></Main_Zone></YAMAHA_AV>";
  
  int length = 0;
  const char *resp;
  Serial.println("A");
  sendAmp(volGetCommand, &resp, &length);
  Serial.println("B");
  if (length <= 0) {
    return -1000;
  }
  Serial.println("C");
  const char *pathVol[] = {"YAMAHA_AV", "Main_Zone", "Volume", "Lvl", "Val"};
  const char *pathUnit[] = {"YAMAHA_AV", "Main_Zone", "Volume", "Lvl", "Unit"};
  const char *pathExp[] = {"YAMAHA_AV", "Main_Zone", "Volume", "Lvl", "Exp"};
  const char* text = NULL;
  const char* textEnd = NULL;

  //char* unit = NULL;
  float vol = 0;
  if (getText(resp, pathVol, 5, &text, &textEnd)) {
    vol = strtola(text, textEnd);
  Serial.println("vol");
  }
  if (getText(resp, pathUnit, 5, &text, &textEnd)) {
    //unit = (char*)calloc(sizeof(char), 1+textEnd-text);
  Serial.println("unit");
  }
  if (getText(resp, pathExp, 5, &text, &textEnd)) {
    vol /= pow(10, strtola(text, textEnd));
  Serial.println("exp");
  }

  Serial.print("done: ");
  Serial.println(vol);
  return vol;
}
    

bool getPower() { return false; }
bool getMute() { return false; }
int getInput() { return false; }

// updateState();

//const char *volUpCmd = "<YAMAHA_AV cmd=\"PUT\"><Main_Zone><Volume><Lvl><Val>Up 1 dB</Val><Exp></Exp><Unit></Unit></Lvl></Volume></Main_Zone></YAMAHA_AV>";
//const char *volDownCmd = "<YAMAHA_AV cmd=\"PUT\"><Main_Zone><Volume><Lvl><Val>Down 1 dB</Val><Exp></Exp><Unit></Unit></Lvl></Volume></Main_Zone></YAMAHA_AV>";
bool setVol(int vol) {
    char cmd[150]; // 126 + x
    sprintf(cmd, "<YAMAHA_AV cmd=\"PUT\"><Main_Zone><Volume><Lvl><Val>%d</Val><Exp>1</Exp><Unit>dB</Unit></Lvl></Volume></Main_Zone></YAMAHA_AV>", vol);
    return sendAmp(cmd);
}

bool toggleMute() { return false; }

bool turnOff() {
    return sendAmp("<YAMAHA_AV cmd=\"PUT\"><System><Power_Control><Power>Standby</Power></Power_Control></System></YAMAHA_AV>");
}

bool turnOn() {
    return sendAmp("<YAMAHA_AV cmd=\"PUT\"><System><Power_Control><Power>On</Power></Power_Control></System></YAMAHA_AV>");
}

} // end anonymous namespace

