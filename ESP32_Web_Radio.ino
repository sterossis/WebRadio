/////////////////////////////////////////////////////////////////
//         ESP32 Internet Radio Project     v1.00              //
//       Get the latest version of the code here:              //
//          http://educ8s.tv/esp32-internet-radio              //
//                                                             //
//     Modified by S. Ross to add channel selector buttons     //
//     to Nextion display.                                     //
/////////////////////////////////////////////////////////////////

#include <VS1053.h>  //https://github.com/baldram/ESP_VS1053_Library
//#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_wifi.h>
#include <Nextion.h>

#define VS1053_CS    32
#define VS1053_DCS   33
#define VS1053_DREQ  35

#define VOLUME  95 // volume level 0-100

Nextion myNextion(nextion, 9600); //create a Nextion object named myNextion using the nextion serial port @ 9600bps

int radioStation = 0;
int previousRadioStation = -1;

char ssid[] = "Your WiFi SSID";       // your network SSID (name)
char pass[] = "Your WiFi password";   // your network password

char *host[25] = {"wpr-ice.streamguys.net",
                  "listen.livestreamingservice.com",
                  "listen.livestreamingservice.com",
                  "listen.livestreamingservice.com",
                  "listen.livestreamingservice.com",
                  "listen.livestreamingservice.com",
                  "listen.livestreamingservice.com",
                  "listen.livestreamingservice.com",
                  "listen.livestreamingservice.com",
                  "listen.livestreamingservice.com",
                  "listen.livestreamingservice.com",
                  "listen.livestreamingservice.com",
                  "listen.livestreamingservice.com",
                  "listen.livestreamingservice.com",
                  "listen.livestreamingservice.com",
                  "listen.livestreamingservice.com",
                  "listen.livestreamingservice.com",
                  "airspectrum.cdnstream1.com",
                  "airspectrum.cdnstream1.com",
                  "192.99.35.215",
                  "kclustream.callutheran.edu",
                  "18803.live.streamtheworld.com",
                  "s6.voscast.com",
                  "198.27.70.42",
                  "5.135.154.69"
                 };
char *path[25] = {"/wpr-music-mp3-96",
                  "/181-classicaljazz_128k.mp3",
                  "/181-breeze_128k.mp3",
                  "/181-eagle_128k.mp3",
                  "/181-classical_128k.mp3",
                  "/181-rock40_128k.mp3",
                  "/181-blues_128k.mp3",
                  "/181-beatles_128k.mp3",
                  "/181-70s_128k.mp3",
                  "/181-goodtime_128k.mp3",
                  "/181-mellow_128k.mp3",
                  "/181-greatoldies_128k.mp3",
                  "/181-yachtrock_128k.mp3",
                  "/181-uktop40_128k.mp3",
                  "/181-classicalguitar_128k.mp3",
                  "/181-lite80s_128k.mp3",
                  "/181-soul_128k.mp3",
                  "/1648_128",
                  "/1261_192",
                  "/stream",
                  "/kclump3",
                  "/KUSCMP128_SC",
                  "/",
                  "/stream",
                  "/"
                 };
int   port[25] = {80, 80, 8004, 8030, 80, 8028, 80, 8062, 8066, 8046, 8060, 8132, 80, 8070, 8020, 8040, 8058,
                  8114,8000,5098,8092,80,10922,10042,14864};

int status = WL_IDLE_STATUS;
WiFiClient  client;
uint8_t mp3buff[32];   // vs1053 likes 32 bytes at a time

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);

void setup () {

  Serial.begin(115200);
  Serial2.begin(9600); //Initialize Nextion serial port
  delay(500);
  SPI.begin();

  initMP3Decoder();
  String command = "p1.picc=1";
  myNextion.sendCommand(command.c_str()); //Connecting...
  connectToWIFI();

}

void loop() {

  if (radioStation != previousRadioStation)
  {
    station_connect(radioStation);
    previousRadioStation = radioStation;
  }

  if (client.available() > 0)
  {
    uint8_t bytesread = client.read(mp3buff, 32);
    player.playChunk(mp3buff, bytesread);
  }

  String message = myNextion.listen(); //check for message to see if channel selector button was pushed
  if (message == "65 0 2 1 ff ff ff") {
    if (radioStation < 24) //Change value (24) to the number of stations to n-1. In my case there are 25 stations pre-programmed. 
      radioStation++;
    else
      radioStation = 0;
  }
  if (message == "65 0 3 1 ff ff ff") {
    if (radioStation > 0)
      radioStation--;
    else
      radioStation = 24; //Change value (24) to the number of stations to n-1. In my case there are 25 stations pre-programmed. 
  }
}

void station_connect (int station_no ) {
  if (client.connect(host[station_no], port[station_no]) ) Serial.println("Connected now");
  client.print(String("GET ") + path[station_no] + " HTTP/1.1\r\n" +
               "Host: " + host[station_no] + "\r\n" +
               "Connection: close\r\n\r\n");
  drawRadioStationName(station_no);
}

void connectToWIFI()
{
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void initMP3Decoder()
{
  player.begin();
  player.switchToMp3Mode(); // optional, some boards require this
  player.setVolume(VOLUME);
}

void drawRadioStationName(int id)
{

  String command;
  switch (id)
  {
    case 0: command = "p1.picc=2"; myNextion.sendCommand(command.c_str()); break; //WPR Wescosin Public Radio
    case 1: command = "p1.picc=3"; myNextion.sendCommand(command.c_str()); break; //181.FM Classical Jazz
    case 2: command = "p1.picc=4"; myNextion.sendCommand(command.c_str()); break; //181.FM The Breeze
    case 3: command = "p1.picc=5"; myNextion.sendCommand(command.c_str()); break; //181.FM The Eagle - Classic Rock
    case 4: command = "p1.picc=6"; myNextion.sendCommand(command.c_str()); break; //181.FM Classical Music
    case 5: command = "p1.picc=7"; myNextion.sendCommand(command.c_str()); break; //181.FM Rock 40
    case 6: command = "p1.picc=8"; myNextion.sendCommand(command.c_str()); break; //181.FM The Blues
    case 7: command = "p1.picc=9"; myNextion.sendCommand(command.c_str()); break; //181.FM The Beatles
    case 8: command = "p1.picc=10"; myNextion.sendCommand(command.c_str()); break; //181.FM Super 70's
    case 9: command = "p1.picc=11"; myNextion.sendCommand(command.c_str()); break; //181.FM Good Time Oldies
    case 10: command = "p1.picc=12"; myNextion.sendCommand(command.c_str()); break; //181.FM Mellow Gold
    case 11: command = "p1.picc=13"; myNextion.sendCommand(command.c_str()); break; //181.FM Classic Hits 181
    case 12: command = "p1.picc=14"; myNextion.sendCommand(command.c_str()); break; //181.FM Yacht Music
    case 13: command = "p1.picc=15"; myNextion.sendCommand(command.c_str()); break; //181.FM UK Top 40
    case 14: command = "p1.picc=16"; myNextion.sendCommand(command.c_str()); break; //181.FM Classical Guitar
    case 15: command = "p1.picc=17"; myNextion.sendCommand(command.c_str()); break; //181.FM Lite 80's
    case 16: command = "p1.picc=18"; myNextion.sendCommand(command.c_str()); break; //181.FM Soul
    case 17: command = "p1.picc=19"; myNextion.sendCommand(command.c_str()); break; //Easy Hits Florida
    case 18: command = "p1.picc=20"; myNextion.sendCommand(command.c_str()); break; //Magic Oldie Florida
    case 19: command = "p1.picc=21"; myNextion.sendCommand(command.c_str()); break; //Classic Rock Planet
    case 20: command = "p1.picc=22"; myNextion.sendCommand(command.c_str()); break; //KCLU
    case 21: command = "p1.picc=23"; myNextion.sendCommand(command.c_str()); break; //KUSC
    case 22: command = "p1.picc=24"; myNextion.sendCommand(command.c_str()); break; //San Francisco 70's Hits
    case 23: command = "p1.picc=25"; myNextion.sendCommand(command.c_str()); break; //Classic Rock 109
    case 24: command = "p1.picc=26"; myNextion.sendCommand(command.c_str()); break; //Music Reminders        
  }
}
