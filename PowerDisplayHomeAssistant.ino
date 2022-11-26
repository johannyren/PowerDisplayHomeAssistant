#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "SPI.h"
#include "TFT_eSPI.h"                  // https://github.com/Bodmer/TFT_eWidget
#include <TFT_eWidget.h>               // Widget library
#include "icons.h"
#include "settings.h"
#define BLANKTEXT "                               " // Text that will be printed on screen in any font
        
// Define colours used in graph etc.
#define DKGREY    0x4A49
#define LTGREY    0xE71C
#define LTBLUE    0xB6DF
#define LTTEAL    0xBF5F
#define LTGREEN   0xBFF7
#define LTCYAN    0xC7FF
#define LTRED     0xFD34
#define LTMAGENTA 0xFD5F
#define LTYELLOW  0xFFF8
#define LTORANGE  0xFE73
#define LTPINK    0xFDDF
#define LTPURPLE  0xCCFF
#define LTGREY    0xE71C
#define BLUE      0x001F
#define TEAL      0x0438
#define GREEN     0x07E0
#define CYAN      0x07FF
#define RED       0xF800
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define ORANGE    0xFC00
#define PINK      0xF81F
#define PURPLE    0x8010
#define GREY      0xC618
#define WHITE     0xFFFF
#define BLACK     0x0000

#define DKBLUE    0x000D
#define DKTEAL    0x020C
#define DKGREEN   0x03E0
#define DKCYAN    0x03EF
#define DKRED     0x6000
#define DKMAGENTA 0x8008
#define DKYELLOW  0x8400
#define DKORANGE  0x8200
#define DKPINK    0x9009
#define DKPURPLE  0x4010
#define DKGREY    0x4A49


// Display
/* Using library TFT_eSPI https://github.com/Bodmer/TFT_eWidget.
 *  Be sure to update the file User_Setup.h in the \libraries\TFT_eSPI folder to set pin numbers. 
 *  To match the pin numbers, use the following:
 *  // For NodeMCU - use pin numbers in the form PIN_Dx where Dx is the NodeMCU pin designation
    #define TFT_CS   PIN_D2  // Chip select control pin D8
    #define TFT_DC   PIN_D4  // Data Command control pin
    #define TFT_RST  PIN_D3  // Reset pin (could connect to NodeMCU RST, see next line)

*/    

// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();
unsigned long drawTime = 0;
WiFiUDP ntpUDP;

 
void reconnect();

String message = "";
bool messageReady = false;

// Handle delays
unsigned long powerStarted, priceStarted, graphStarted, nordpoolStarted = 0;
const long powerInterval = 10000;
const long priceInterval = 20000;
const long graphInterval = 120000;

// Price level
String price_kr, currentPrice_kr;
String currentPriceLevel = "NORMAL";
String price = "Normalt";
String priceLevel;
String currentPower;
String dailyEnergy;
float accumulatedCost, prevDailyEnergy;

// Set up graph widget
GraphWidget gr = GraphWidget(&tft);    // Graph widget gr instance with pointer to tft
TraceWidget tr = TraceWidget(&gr);     // Graph trace tr with pointer to gr
TraceWidget tr2 = TraceWidget(&gr);     // Graph trace tr with pointer to gr
uint32_t textColour = TFT_WHITE;
uint32_t textHighlightColour = TFT_ORANGE;
uint32_t bgColour = TFT_BLACK;
bool graphDrawn = false;

const float gxLow  = 0.0;
const float gxHigh = 24.0; // Number of hours
const float gyLow  = 0.0;
const float gyHigh = 11.00; // Max price
static float gx = 0.0, gy = 0.0;

  
void setup(void) {

// For HTTPS requests only. Keep commented otherwise! 
// If you don't need to check the fingerprint
     client.setInsecure();
// If you want to check the fingerprint
    // client.setFingerprint(HA_HOST_FINGERPRINT);
  
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  Serial.begin(115200);

// TFT setup
  tft.begin();
  tft.setRotation(2);
  tft.setTextFont(2);
  tft.fillScreen(DKGREY);            // Clear screen with Grey background
  
  // Connect to the WiFI
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  tft.println();
  tft.println();
  tft.print("Connecting Wifi: ");
  tft.println(ssid);

  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  IPAddress ip = WiFi.localIP();
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(ip);

  tft.println("");
  tft.println("WiFi connected");
  tft.println("IP address: ");
  tft.println(ip);
  delay(2000);
  tft.fillScreen(bgColour);            // Clear screen

}

void loop() {
  //const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 1024;
  DynamicJsonDocument document(6144);
  float currentPowerConsumption = 55, currentPowerProduction = 0;
  
  unsigned long currentMillis = millis();
  // Execute every 10 seconds
  if (currentMillis - powerStarted >= powerInterval) {
    powerStarted = currentMillis;    
    document = makeGETRequest(SENSOR_CURRENT_CONSUMPTION);    
    currentPowerConsumption = ExtractJSONAttribute(document, "state").toFloat(); 
       
    document = makeGETRequest(SENSOR_CONSUMPTION_TODAY);    
    dailyEnergy = ExtractJSONAttribute(document, "state");
    if (dailyEnergy.toFloat() < 0.2) {
      accumulatedCost = 0;
      prevDailyEnergy = 0;
    }
    if (SOLAR_PANELS)
    {
      document = makeGETRequest(SENSOR_CURRENT_PRODUCTION);    
      currentPowerProduction = ExtractJSONAttribute(document, "state").toFloat();
      WriteCurrentPower(130, 15, ((currentPowerConsumption - currentPowerProduction) * SENSOR_POWER_MULTIPLIER));
    }
    else
      WriteCurrentPower(130, 15, ((currentPowerConsumption) * SENSOR_POWER_MULTIPLIER));
               
    WriteTotal(120, 70);
  }
  
  // Execute every 20 seconds
  if (currentMillis - priceStarted >= priceInterval) {
    priceStarted = currentMillis;
    document = makeGETRequest(SENSOR_ELECTRICITY_PRICE);  
      
    // Check price level
    price_kr = ExtractJSONAttribute(document,  "state");  
    priceLevel = ExtractJSONAttribute(document, "attributes", "price_level");  

    document = makeGETRequest(SENSOR_CURRENT_CONSUMPTION);    
    currentPowerConsumption = ExtractJSONAttribute(document, "state").toFloat();
    
    if (SOLAR_PANELS)
    {
      document = makeGETRequest(SENSOR_CURRENT_PRODUCTION);    
      currentPowerProduction = ExtractJSONAttribute(document, "state").toFloat();         
      WriteCurrentPower(130, 15, ((currentPowerConsumption - currentPowerProduction) * SENSOR_POWER_MULTIPLIER));
    }
    else
      WriteCurrentPower(130, 15, ((currentPowerConsumption) * SENSOR_POWER_MULTIPLIER));

    WriteTotal(120, 70);
    WritePriceText(120, 237);
    }

  // Execute every 120 seconds
  if (currentMillis - graphStarted >= graphInterval or !graphDrawn) {
    graphStarted = currentMillis;
    graphDrawn = true;
    document = makeGETRequest(SENSOR_NORDPOOL);  
    PlotGraph(document);  
  }
}

void CreateGraph (int xPos, int yPos, float maxPriceToday ) 
{
  // Clear scale in y axis
  tft.fillRect(xPos-10, yPos, 20, 90, TFT_BLACK);
  tft.setFreeFont();                 // Select the font
  tft.setTextSize(1);
  // Graph area is 220 pixels wide, 80 pixels high, dark grey background
  gr.createGraph(220, 80, tft.color565(5, 5, 5));
  gr.setGraphScale(gxLow, gxHigh, gyLow, maxPriceToday);

  // X grid starts at 0 with lines every 2 x-scale units
  // Y grid starts at 0 with lines every 2 y-scale units
  // Dark grey grid
  gr.setGraphGrid(gxLow, 2.0, gyLow, 1.0, DKGREY);

  // Draw empty graph, top left corner at pixel coordinate 40,10 on TFT
  gr.drawGraph(xPos, yPos);

  // Draw the x axis scale
  tft.setTextDatum(TC_DATUM); // Top centre text datum
  for (int x=2; x<=24; x+=2){
    tft.drawNumber(x, gr.getPointX(x), gr.getPointY(0.0) + 3);  
  }

  // Draw the y axis scale
  tft.setTextDatum(MR_DATUM); // Middle right text datum
  for (float y=0; y<=maxPriceToday; y+=2){
    tft.drawNumber(y, gr.getPointX(0.0), gr.getPointY(y));  
  }   
}

void PlotGraph (DynamicJsonDocument nordPoolDocument)
{
  double lastprice = 0;
  double price;
  float maxPricetoday = nordPoolDocument["attributes"]["max"];
  Serial.println("***************************");
  Serial.print("Max price today: ");
  Serial.println(maxPricetoday);
  CreateGraph (10, 128, maxPricetoday);
  PlotTimeline(maxPricetoday);
  
  for (int priceCount=0;priceCount<24;priceCount++)
  {
    price = nordPoolDocument["attributes"]["today"][priceCount];
    lastprice = AddPrice(priceCount, price,  priceCount-1,  lastprice);
  }
  price = nordPoolDocument["attributes"]["tomorrow"][0];
  if (price > 0)
    lastprice = AddPrice(24, price,  23,  lastprice);
}

void PlotTimeline(float maxPriceToday)
{
  // Get current time, to draw a timeline
  NTPClient timeClient(ntpUDP, "se.pool.ntp.org", 3600);
  
  timeClient.update();
  Serial.println("Timeline value: ");
  double hours = timeClient.getHours();
  double minutes = timeClient.getMinutes();
  double timeLineVal = hours + (minutes/60);
  tr2.startTrace(LTGREY);
  tr2.addPoint(timeLineVal, 0);
  tr2.addPoint(timeLineVal, maxPriceToday);
}

double AddPrice(int hour, double price, int lastHour, double lastPrice)
{
  if(lastHour<0)
    lastHour=0;
  
  if(lastPrice==0)
    lastPrice = price;
    
  tr.startTrace(PriceColour(lastPrice));
  tr.addPoint(lastHour, lastPrice); 
  tr.addPoint(hour, price);
  return price;
}

uint32_t PriceColour (float nNewPrice)
{
    uint32_t colour;
    //nNewPrice = nNewPrice;
    if (inRange(nNewPrice, EXTREMELY_EXPENSIVE, 100)) {colour = TFT_MAROON;}
    else if (inRange(nNewPrice, VERY_EXPENSIVE, EXTREMELY_EXPENSIVE)){colour =  TFT_RED;}
    else if (inRange(nNewPrice, EXPENSIVE, VERY_EXPENSIVE)){colour =  TFT_ORANGE;}
    else if (inRange(nNewPrice, NORMAL, EXPENSIVE)){colour = TFT_GREENYELLOW;}
    else if (inRange(nNewPrice, CHEAP, NORMAL)){colour = TFT_GREEN;}
    else if (inRange(nNewPrice, VERY_CHEAP, CHEAP)){colour = TFT_DARKGREEN;}
    else if (inRange(nNewPrice, 0, VERY_CHEAP)){colour = TFT_DARKGREEN;}     
    return colour; 
}

void WriteCurrentPower(int xPos, int yPos, int nCurrentPower)
{
  tft.setSwapBytes(true);
  currentPower = String(nCurrentPower);
  tft.setFreeFont(FF23);                 // Select the font
  tft.setTextColor(textHighlightColour, bgColour); 
  tft.drawCentreString(BLANKTEXT, xPos, yPos, GFXFF);// Clear the line  
  tft.drawCentreString(currentPower + " W", xPos, yPos, GFXFF);  
  if (nCurrentPower > 0)
     tft.pushImage(xPos-115, yPos-5, 32, 32, electric_pole);
  else
     tft.pushImage(xPos-115, yPos-5, 32, 32, sunny_solar32);
}

void WriteTotal(int xPos, int yPos)
{
    tft.setTextColor(textColour, bgColour);      
    tft.setFreeFont(FF17);                 // Select the font
    tft.drawCentreString(BLANKTEXT, xPos, yPos, GFXFF);// Clear the line  
    tft.drawCentreString("Idag: " + dailyEnergy + " kWh", xPos, yPos, GFXFF);
    tft.drawCentreString(BLANKTEXT, xPos, yPos+25, GFXFF);// Clear the line  
    tft.drawCentreString("Kostnad: " + CalculateAccumulatedCost(price_kr, dailyEnergy) + " kr", xPos, yPos+25, GFXFF); 
}

void WritePriceText(int xPos, int yPos)
{    

  if (!price_kr.equals(currentPrice_kr)){
    Serial.println("Current price: " + currentPrice_kr);
    Serial.println("New price: " + price_kr);
    currentPrice_kr = price_kr;
    float nNewPrice = price_kr.toFloat();
    bgColour = TFT_BLACK;
    //price = PriceText(nNewPrice);
    
    if (inRange(nNewPrice, EXTREMELY_EXPENSIVE, 100)) {price = "Extremt dyrt";}
    else if (inRange(nNewPrice, VERY_EXPENSIVE, EXTREMELY_EXPENSIVE)){price = "Mycket dyrt";}
    else if (inRange(nNewPrice, EXPENSIVE, VERY_EXPENSIVE)){price = "Dyrt";}
    else if (inRange(nNewPrice, NORMAL, EXPENSIVE)){price = "Normalt";}
    else if (inRange(nNewPrice, CHEAP, NORMAL)){price = "Billigt";}
    else if (inRange(nNewPrice, VERY_CHEAP, CHEAP)){price = "Mycket Billigt";}
    else if (inRange(nNewPrice, 0, VERY_CHEAP)){price = "Mycket billigt";} 

    
    textHighlightColour = PriceColour(nNewPrice);
    tft.setFreeFont(FF18);                 // Select the font
    tft.drawCentreString(BLANKTEXT, xPos, yPos, GFXFF);// Clear the line  
    tft.drawCentreString(price_kr + " kr/kWh", xPos, yPos, GFXFF);  
    tft.setFreeFont(FF23);                 // Select the font
    tft.setTextColor(textHighlightColour, bgColour); 
    tft.drawCentreString(BLANKTEXT, xPos, yPos+30, GFXFF);// Clear the line  
    tft.drawCentreString("" + price,xPos, yPos+30,GFXFF);  
  }
}

String CalculateAccumulatedCost(String currentPrice, String dailyEnergy) {
  float nCurrentPrice = currentPrice.toFloat();
  if (nCurrentPrice == 0)
    return "0";
  float nDailyEnergy = dailyEnergy.toFloat();
  float nEnergyDelta = nDailyEnergy - prevDailyEnergy; 
  prevDailyEnergy = nDailyEnergy;
  accumulatedCost += (nEnergyDelta * nCurrentPrice);

  Serial.println("Price: " + String(nCurrentPrice));
  Serial.println("DailyEnergy: " + String(nDailyEnergy));
  Serial.println("Delta: " + String(nEnergyDelta));
  Serial.println("AccumulatedCost: " + String(accumulatedCost));
  return String(accumulatedCost);
}

DynamicJsonDocument makeGETRequest(String entity_id) 
{  
  if (entity_id == "")
  {
    Serial.println("Error: Entity id is null");
  }

  // Opening connection to server (Use 8123 as port if HTTP)
  if (SECURE_SERVER)  
  {
    if (!client.connect(HA_HOST, 443))
    {
      Serial.println("Connection to " + String(HA_HOST) + " failed using port 443");
    }
  }
  else
  {
    if (!client.connect(HA_HOST, 8123))
    {
      Serial.println("Connection to " + String(HA_HOST) + " failed failed using port 8123");
    }    
  }

  // Send HTTP request
  client.print(F("GET "));
  // This is the second half of a request (everything that comes after the base URL)
  Serial.print("header: ");
  Serial.println ("/api/states/" + entity_id);
  client.print("/api/states/" + entity_id);
  client.println(F(" HTTP/1.1"));

  //Headers
  client.print(F("Host: "));
  client.println(HA_HOST);
  client.println("Authorization: Bearer " + bearerToken); 
  client.println();
// Message JSON
  if (client.println() == 0)
  {
    Serial.println(F("Failed to send request"));
    if (WiFi.status() != WL_CONNECTED) 
    {
      Serial.println(F("Network Connection lost. Trying to reconnect"));
      reconnect();
    } 
  }
  
  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
    Serial.print(F("Response: "));
    Serial.println(status);

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
  }

  // Allocate the JSON document
  // Use arduinojson.org/v6/assistant to compute the capacity.
  DynamicJsonDocument doc(6144);

  // Parse JSON object
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    //return error.f_str();
  }
  return doc;
}

DynamicJsonDocument makeGETRequestForTime(String entity_id) 
{  
  String hostURL = "http://worldtimeapi.org";

  // Opening connection to server (Use 80 as port if HTTP)
  if (!client.connect(hostURL, 80))
  {
    Serial.println(F("Connection failed"));
  }

  // Send HTTP request
  client.print(F("GET "));
  // This is the second half of a request (everything that comes after the base URL)
  Serial.print("header: ");
  Serial.println ("/api/ip" + entity_id);
  client.print("/api/ip" + entity_id);
  client.println(F(" HTTP/1.1"));

  //Headers
  client.print(F("Host: "));
  client.println(hostURL);
//  client.println("Authorization: Bearer " + bearerToken); 
//  client.println();
// Message JSON
  if (client.println() == 0)
  {
    Serial.println(F("Failed to send request"));
    if (WiFi.status() != WL_CONNECTED) 
    {
      Serial.println(F("Network Connection lost. Trying to reconnect"));
      reconnect();
    } 
  }
  
  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
    Serial.print(F("Response: "));
    Serial.println(status);

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
  }

  // Allocate the JSON document
  // Use arduinojson.org/v6/assistant to compute the capacity.
  DynamicJsonDocument doc(6144);

  // Parse JSON object
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    //return error.f_str();
  }
  return doc;
}


bool inRange(float val, float minimum, float maximum)
{
  return ((minimum <= val) && (val <= maximum));
}

void reconnect() 
{  
    int wifitries = 0;
    Serial.print("Reconnecting");
    WiFi.mode(WIFI_STA);  
    WiFi.begin(ssid, password);  
    while (WiFi.status() != WL_CONNECTED) { 
        wifitries++; 
        delay(1000);  
        Serial.print(".");
        if (wifitries == 10)
        {
          Serial.print("Reconnect failed. Waiting 1 min before retrying.");
          delay(60000);
          return;
        }
    }  
    Serial.println("Connected!");
} 

String ExtractJSONAttribute(DynamicJsonDocument doc, String attribute)
{
  String value = doc[attribute].as<char*>();
  Serial.println("Attribute: " + attribute + "  Value: " + value);
  return value;
}

String ExtractJSONAttribute(DynamicJsonDocument doc, String attribute1, String attribute2)
{
  String value = doc[attribute1][attribute2].as<char*>();
  Serial.println("Attribute: " + attribute1 + "/" + attribute2 + "  Value: " + value);
  return value;
}

double ExtractJSONAttributeFloat(DynamicJsonDocument doc, String attribute1, String attribute2)
{
  double value = doc[attribute1][attribute2];
  Serial.print("Attribute: " + attribute1 + "/" + attribute2 + "  Value: ");
  Serial.println (value, 2);
  return value;
}

String ExtractJSONAttribute(DynamicJsonDocument doc, String attribute1, String attribute2, String attribute3)
{
  String value = doc[attribute1][attribute2][attribute3].as<char*>();
  Serial.println("Attribute: " + attribute1 + "/" + attribute2 + "/" + attribute3 + "  Value: " + value);
  return value;
}


// There follows a crude way of flagging that this example sketch needs fonts which
// have not been enbabled in the User_Setup.h file inside the TFT_HX8357 library.
//
// These lines produce errors during compile time if settings in User_Setup are not correct
//
// The error will be "does not name a type" but ignore this and read the text between ''
// it will indicate which font or feature needs to be enabled
//
// Either delete all the following lines if you do not want warnings, or change the lines
// to suit your sketch modifications.

#ifndef LOAD_GLCD
//ERROR_Please_enable_LOAD_GLCD_in_User_Setup
#endif

#ifndef LOAD_GFXFF
ERROR_Please_enable_LOAD_GFXFF_in_User_Setup!
#endif
