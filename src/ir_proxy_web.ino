#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
const uint16_t kRecvPin = 14;

const char *essid = "";
const char *pass = "";

IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.
IRrecv irrecv(kRecvPin);

decode_results results;

ESP8266WebServer server(80);

const char INDEX_HTML[] =
"<!DOCTYPE HTML>"
"<html>"
"<head>"
"<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
"<title>ESP8266 IR Proxy</title>"
"<style>"
"\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\""
"</style>"
"</head>"
"<body>"
"<h1>ESP8266 IR remote proxy</h1>"
"<FORM action=\"/\" method=\"post\">"
"<ul>"
"<li><button type=\"submit\" name=\"code\" value=\"0xD0C\">Off</button></li>"
"<li><button type=\"submit\" name=\"code\" value=\"0x47F\">Tuner</button></li>"
"<li><button type=\"submit\" name=\"code\" value=\"0xD7F\">AUX</button></li>"
"<li><button type=\"submit\" name=\"code\" value=\"0xC10\">Vol+</button></li>"
"<li><button type=\"submit\" name=\"code\" value=\"0xC11\">Vol-</button></li>"
"<li><button type=\"submit\" name=\"code\" value=\"0xC60\">Prog+</button></li>"
"<li><button type=\"submit\" name=\"code\" value=\"0xC61\">Prog-</button></li>"
"</ul>"
"</FORM>"
"</body>"
"</html>";

void handleRoot() {
  if (server.hasArg("code")) {
    handleSubmit();
  }
  else {
    server.send(200, "text/html", INDEX_HTML);
  }
}

void returnFail(String msg) {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(500, "text/plain", msg + "\r\n");
}

void handleSubmit() {
  String code;

  if (!server.hasArg("code")) return returnFail("BAD ARGS");
  code = server.arg("code");

  // send code
  long unsigned int c = strtoul(code.c_str(), 0, 16);

  Serial.print("Code: ");
  Serial.print(code);
  Serial.print(" int: ");
  Serial.println(c);

  irsend.sendRC5(c, 12);

  server.send(200, "text/html", INDEX_HTML);
}

void returnOK()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "OK\r\n");
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup() {
  Serial.begin(115200);
  irsend.begin();
  irrecv.enableIRIn();  // Start the receiver

  WiFi.begin(essid, pass);
  Serial.println("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("Connection status: %d\n", WiFi.status());

  if (!MDNS.begin("ir")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);


  server.begin();
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}

void loop() {
  server.handleClient();
}
