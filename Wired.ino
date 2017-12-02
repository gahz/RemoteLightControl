#include <EtherCard.h>

#define STATIC 1 // 1 for static

#if STATIC
static byte myip[] = { 192,168,1,200 };
static byte gwip[] = { 192,168,1,1 };
#endif

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
byte Ethernet::buffer[500]; // tcp/ip send and receive buffer

BufferFiller bfill;

bool led1Status = false;
bool led2Status = false;
bool led3Status = false;

const int led1pin = 2;
const int led2pin = 3;
const int led3pin = 4;

const char http_OK[] PROGMEM =
   "HTTP/1.0 200 OK\r\n"
   "Content-Type: text/html\r\n"
   "Pragma: no-cache\r\n\r\n";

const char http_Found[] PROGMEM =
   "HTTP/1.0 302 Found\r\n"
   "Location: /\r\n\r\n";

const char http_Unauthorized[] PROGMEM =
   "HTTP/1.0 401 Unauthorized\r\n"
   "Content-Type: text/html\r\n\r\n"
   "<h1>401 Unauthorized</h1>";

void homePage()
{
   bfill.emit_p(PSTR("$F"
       "<meta http-equiv='refresh' content='5'/>"
       "<title>Sistema de iluminacion</title>"
       "<div style='font-size:100px;'>" 
       "Led 1: <a href=\"?led1=$F\">$F</a></br></br>"
       "Led 2: <a href=\"?led2=$F\">$F</a></br></br>"
       "Led 3: <a href=\"?led3=$F\">$F</a></br></br>"
       "</div>")
       ,http_OK,
       led1Status?PSTR("off"):PSTR("on"),led1Status?PSTR("ON"):PSTR("OFF"),
       led2Status?PSTR("off"):PSTR("on"),led2Status?PSTR("ON"):PSTR("OFF"),
       led3Status?PSTR("off"):PSTR("on"),led3Status?PSTR("ON"):PSTR("OFF"));
}

void setup(){
  
  //Turning off all lamps
  pinMode(led1pin, OUTPUT);
  pinMode(led2pin, OUTPUT);
  pinMode(led3pin, OUTPUT);
  
  digitalWrite(led1pin, LOW);
  digitalWrite(led2pin, LOW);
  digitalWrite(led2pin, LOW);
  
  Serial.begin(57600);
  Serial.println("\n[backSoon]");

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println( "Failed to access Ethernet controller");
#if STATIC
  ether.staticSetup(myip, gwip);
#else
  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");
#endif

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  
}

void loop(){
  // wait for an incoming TCP packet, but ignore its contents
  /*
  if (ether.packetLoop(ether.packetReceive())) {
    memcpy_P(ether.tcpOffset(), page, sizeof page);
    ether.httpServerReply(sizeof page - 1);
  }*/
  
   // wait for an incoming TCP packet, but ignore its contents
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len); 
  
   if (pos) {
     
     delay(1);
         bfill = ether.tcpOffset();
         char *data = (char *) Ethernet::buffer + pos;
         
         
     if (strncmp("GET /", data, 5) != 0) {
             // Unsupported HTTP request
             // 304 or 501 response would be more appropriate
             bfill.emit_p(http_Unauthorized);
         }
         else {
             data += 5;
             
             if (data[0] == ' ') {
                 // Return home page
                 homePage();
             }
             else if (strncmp("?led1=on ", data, 9) == 0) {
                 digitalWrite(led1pin, HIGH);
                 led1Status = true;
                 bfill.emit_p(http_Found);
             }
             else if (strncmp("?led1=off ", data, 10) == 0) {
                 digitalWrite(led1pin, LOW);
                 led1Status = false;
                 bfill.emit_p(http_Found);
             }
             else if (strncmp("?led2=on ", data, 9) == 0) {
                 digitalWrite(led2pin, HIGH);
                 led2Status = true;
                 bfill.emit_p(http_Found);
             }
             else if (strncmp("?led2=off ", data, 10) == 0) {
                 digitalWrite(led2pin, LOW);
                 led2Status = false;
                 bfill.emit_p(http_Found);
             }
             else if (strncmp("?led3=on ", data, 9) == 0) {
                 digitalWrite(led3pin, HIGH);
                 led3Status = true;
                 bfill.emit_p(http_Found);
             }
             else if (strncmp("?led3=off ", data, 10) == 0) {
                 digitalWrite(led3pin, LOW);
                 led3Status = false;
                 bfill.emit_p(http_Found);
             }
             else {
                 // Page not found
                 bfill.emit_p(http_Unauthorized);
             }
         }
         
         ether.httpServerReply(bfill.position());    // send http response
     }
  
}
