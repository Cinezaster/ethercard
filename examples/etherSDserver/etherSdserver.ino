// This is a basic demo that combines the ether card library with the SD card library 
// This example works with a standard arduino and the ENC28J60 Ethernet shield with POE & SD slot - IE shield v1.1 from iteadstudio

// To make this work:
// in the ethercard library you have to change enc28j60.cpp file on line 243 to 2 and put the jumper on the shield to CSn & D10.
// use the readwrite example from the standard SD library to make a test.txt file on your SD card (use pin 9 for the Select SD (!SD.begin(9)) )

// the limitations:
// - There s not much free ram, be carefull with the buffer size.
// - At this point there is no way to send more then one ethernet packet (1518 bytes)
// - I'm not succeeded to get data from the SD card to the ethernet buffer. 

// examples are a mix from the examples on the ethercard library on git by jcw and from the arduino examples

+

#include <EtherCard.h>
#include <SD.h>

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte myip[] = { 192,168,0,203 };

byte Ethernet::buffer[500];
BufferFiller bfill;

File myFile;

void setup () {
  Serial.begin(9600);
  pinMode(10, OUTPUT);  
  digitalWrite(10,HIGH);
  if (!SD.begin(9)) {
    Serial.println("init SD failed!");
    return;
  }
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println( "Failed Ethernet");
  ether.staticSetup(myip);
  Serial.println("init done."); 
}


void loop () {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  
  if (pos) { // check if valid tcp data is received
     bfill = ether.tcpOffset();
        char* data = (char *) Ethernet::buffer + pos;

        Serial.println(data);
        // receive buf hasn't been clobbered by reply yet
        if (strncmp("GET / ", data, 6) == 0){
            File dataFile = SD.open("test.txt"); // if the file is available, read the file en send it serial:
            if (dataFile) {
              while (dataFile.available()) {
                Serial.write(dataFile.read());
              }
              dataFile.close();
              bfill.emit_p(PSTR(
                "HTTP/1.0 401 Unauthorized\r\n"
                "Content-Type: text/html\r\n"
                "\r\n"
                "<h1>You just opened text.txt</h1><p>please watch your serial monitor</p>"));
            }
            // if the file isn't open, pop up an error:
            else {
              Serial.println("error opening datalog.txt");
              bfill.emit_p(PSTR(
                "HTTP/1.0 401 Unauthorized\r\n"
                "Content-Type: text/html\r\n"
                "\r\n"
                "<h1>There was a error opening text.txt</h1>"));
            }
        }
        else
            bfill.emit_p(PSTR(
                "HTTP/1.0 401 Unauthorized\r\n"
                "Content-Type: text/html\r\n"
                "\r\n"
                "<h1>401 Unauthorized</h1>"));
        ether.httpServerReply(bfill.position()); // send web page data
  }
}
