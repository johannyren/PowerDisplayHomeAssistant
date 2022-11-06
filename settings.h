// Modify the settings in this file to suit your network, your Home Assistant installation 
// and your personal preferences:

// Define the price levels (SEK/kWh)
#define VERY_CHEAP 0.5
#define CHEAP 1.0
#define NORMAL 1.5
#define EXPENSIVE 3.0
#define VERY_EXPENSIVE 4.0
#define EXTREMELY_EXPENSIVE 5.0

// Enter just the base of the URL to Home Assistant (not http://)
#define HA_HOST "<my home assistant host name or ip>"

// Set to true if using https, otherwise set to false.
#define SECURE_SERVER true

// For HTTPS requests. Uncomment if using https.
  // WiFiClientSecure client;

// Important! If using https, also uncomment the line 
//     client.setInsecure();
// in the setup() function in PowerDisplayHomeAssist.ino


// For HTTPS requests only. OPTIONAL - The fingerprint of the site you want to connect to.
//#define HA_HOST_FINGERPRINT "BC 73 A5 9C 6E EE 38 43 A6 37 FC 32 CF 08 16 DC CF F1 5A 66"

// For HTTP requests. Comment if using https.
  WiFiClient client;

// Your Home Assistant bearer token. https://www.home-assistant.io/docs/authentication/
String bearerToken = "<my bearer token string goes here - check the link above to create one>";



// Set up your sensors, as they are named in your Home Assistant installation
//#define SENSOR_CURRENT_CONSUMPTION "sensor.house_power_consumption" // If using Home Assistant Glow pulse counter. https://klaasnicolaas.github.io/home-assistant-glow/
#define SENSOR_CURRENT_CONSUMPTION "sensor.momentary_active_import" // If using ESPHome HAN port reader  https://github.com/psvanstrom/esphome-p1reader
#define SENSOR_POWER_MULTIPLIER 1 // Convert between kW and W, if needed. If the consumption is in kW then change this to 1000.
#define SENSOR_CONSUMPTION_TODAY "sensor.forbrukning_huset_per_dag" // From Tibber
#define SENSOR_ELECTRICITY_PRICE "sensor.nordpool" // From NordPool, but you can also use Tibber
#define SENSOR_NORDPOOL "sensor.nordpool" // Expecting Home Assistant NordPool Integration. https://github.com/custom-components/nordpool

#define SOLAR_PANELS true // Change to true if using solar panels, and add the sensor below.
#define SENSOR_CURRENT_PRODUCTION "sensor.momentary_active_export" // If using ESPHome HAN port reader  https://github.com/psvanstrom/esphome-p1reader


// Home assistant
char ssid[] = "<my ssid>";       // your network SSID (name)
char password[] = "<my password>";  // your network key
