// Load Wi-Fi library
#include <WiFi.h>
#include <cstdlib>
#include <SPI.h>

// Replace with your network credentials
const char *ssid = "PHILIP-LAPTOP";
const char *password = "Kellys1234";

const unsigned char TRANSFER_START = 0xaa;
const unsigned char TRANSFER_END = 0x33;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void printWebsite(WiFiClient &client);
void handlePostRequest(String &header);
void sendCmd(const char cmd, const unsigned int length, const char *data);

void setup()
{
  Serial.begin(115200);
  SPI.begin();

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
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

void loop()
{
  WiFiClient client = server.available(); // Listen for incoming clients

  if (client)
  { // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime)
    { // loop while the client's connected
      currentTime = millis();
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out the serial monitor
        header += c;
        if (c == '\n')
        { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            Serial.println();
            Serial.println("Header: ");
            Serial.println(header);

            // Get Request always sends the website, no matter the url or payload
            if (header.indexOf("GET") >= 0)
            {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: Closed");
              client.println();

              printWebsite(client);
            }
            // POST /send-cmd?cmd=01&data=0FF000 HTTP/1.1
            else if (header.indexOf("POST /send-cmd?") >= 0)
            {
              unsigned int index1 = header.indexOf("POST /send-cmd?") + 15;
              unsigned int index2 = header.indexOf(" HTTP/1.1");
              String url = header.substring(index1, index2); // extract the payload from the request

              Serial.println();
              Serial.print("Payload: ");
              // Serial.println(url);

              String cmd_str = url.substring(4, 6);
              String data_str = url.substring(12, url.length());
              Serial.print("cmd: ");
              Serial.println(cmd_str);
              Serial.print("data: ");
              Serial.println(data_str);

              // Command as an 8-bit number
              unsigned char cmd = strtoul(("0x" + cmd_str).c_str(), NULL, 16); // convert hex into decimal
              // Data stored in an array of 8-bit numbers
              unsigned char data[255]; // create an array for storing the data with maximum size available. This is just because I'm lazy and don't want to allocate memory.
              // Length of the Data sent as an 8-bit number
              const unsigned char length = data_str.length() / 2;
              for (size_t i = 0; i < length; i++)
              {
                Serial.println(data_str.substring(2 * i, 2 * (i + 1)));
                data[i] = (unsigned char)strtoul(("0x" + data_str.substring(2 * i, 2 * (i + 1))).c_str(), NULL, 16);
                Serial.println(data[i]);
              }
              sendCmd(cmd, length, data);
            }
            // Deny any other POST requests
            else if (header.indexOf("POST /") >= 0)
            {
              client.println("HTTP/1.1 400 BAD REQUEST");
              client.println("Connection: Closed");
            }
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          }
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
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