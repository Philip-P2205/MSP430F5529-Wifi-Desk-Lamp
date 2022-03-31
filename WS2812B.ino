// Load Wi-Fi library
#include <WiFi.h>
#include <cstdlib>
#include <SPI.h>

// Replace with your network credentials
const char* ssid = "PHILIP-LAPTOP";
const char* password = "Kellys1234";

const unsigned char TRANSFER_START = 0xaa;
const unsigned char TRANSFER_END = 0x33;

#define CMD_CLEAR 0x01
#define CMD_SINGLE 0x02
#define CMD_INDIVIDUAL 0x03
#define CMD_RAINBOW 0x04
#define CMD_PULSE 0x05
#define CMD_RANDOM 0x06
#define CMD_GRADIENT 0x07
#define CMD_FIRE 0x08
#define CMD_STARLIGHT 0x09
#define CMD_SPECTRUM 0x0A


// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

//Variable to store the color for the LEDs
String colorString;
unsigned int color;

// Auxiliar variables to store the current output state
String output26State = "off";
String output27State = "off";

// Assign output variables to GPIO pins
const int output26 = 26;
const int output27 = 27;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;


void printWebsite(WiFiClient &client);
void handleColorGetRequest(String &header);
void sendCmd(const char cmd, const unsigned int length, const char *data);


void setup() {
  Serial.begin(115200);
  SPI.begin();
  // Initialize the output variables as outputs
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: Closed");
            client.println();

            if (header.indexOf("GET /?color=%23") >= 0)
            handleColorGetRequest(header);

            printWebsite(client);
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void handleColorGetRequest(String &header)
{
  Serial.println(header); // Print header for debug reasons
  int index = header.indexOf("GET /?color=%23");
  colorString = "0x" + header.substring(index + 21, 15);
  Serial.println("ColorString: " + colorString);
  
  char *p;
  color = (unsigned int) strtoul(colorString.c_str(), &p, 16); // convert hex color into decimal (binary)
  unsigned int r = (color >> 16) & 0xFF;
  unsigned int g = (color >> 8) & 0xFF;
  unsigned int b = (color >> 0) & 0xFF;
  Serial.print("Color: ");
  Serial.println(color);
  Serial.print("RGB: ");
  Serial.print(r);
  Serial.print(", ");
  Serial.print(g);
  Serial.print(", ");
  Serial.println(b);

  unsigned char data[3] = {r, g, b};

  /*for (int i = 0; i < 80; i++)
  {
    data[3*i]=r;
    data[3*i+1]=g;
    data[3*i+2]=b;
  }*/

  sendCmd(CMD_SINGLE, 3, data);
}


void sendCmd(const unsigned char cmd, const unsigned char length, const unsigned char *data)
{
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE1)); // 100kHz is the highest the msp can handle at 8MHz

  SPI.transfer(TRANSFER_START);
  SPI.transfer(cmd);
  SPI.transfer(length);
  for (int i = 0; i < length; i++)
  {
    SPI.transfer(data[i]);
  }
  SPI.transfer(TRANSFER_END);
  SPI.endTransaction();
}


void printWebsite(WiFiClient &client)
{
  client.println("<!DOCTYPE html>");
  client.println("<html lang=\"en\">");
  client.println("<head>");
  client.println("<meta charset=\"UTF-8\">");
  client.println("<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
  client.println("<title>Document</title>");
  client.println("<style>");
  client.println("* {");
  client.println("font-family: \"Helvetica Neue\", Helvetica, Arial;");
  client.println("font-size: 72px;");
  client.println("margin: 0;");
  client.println("padding: 0;");
  client.println("}");
  client.println("body {");
  client.println("overflow: hidden;");
  client.println("}");
  client.println("#color {");
  client.println("position: absolute;");
  client.println("width: 75vw;");
  client.println("height: 50vh;");
  client.println("right: 0;");
  client.println("left: 50%;");
  client.println("border: transparent;");
  client.println("transform: translateX(-50%);");
  client.println("}");
  client.println("#submit {");
  client.println("position: absolute;");
  client.println("width: 50vw;");
  client.println("height: 10vh;");
  client.println("top: 82.5vh;");
  client.println("left: 50%;");
  client.println("transform: translateX(-50%);");
  client.println("background-color: crimson;");
  client.println("color: floralwhite;");
  client.println("border: none;");
  client.println("border-radius: 0.5rem;");
  client.println("}");
  client.println("#submit:hover {");
  client.println("cursor: pointer;");
  client.println("}");
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  client.println("<form action=\"\" method=\"get\">");
  client.println("<input type=\"color\" name=\"color\" id=\"color\">");
  client.println("<input type=\"submit\" id=\"submit\" value=\"Submit\">");
  client.println("</form>");
  client.println("</body>");
  client.println("</html>");
}