# PowerDisplayHomeAssistant

This is a small display that shows the current electricity consumption, together with a graph of the today's electricity price, using either NordPool or Tibber. The software pulls the data from a Home Assistant instance, so all sources must be available there.
<p align="center">
  <img src="(https://github.com/johannyren/PowerDisplayHomeAssistant/blob/main/Images/Display1.jpg?raw=true)" width="350" title="Display">
</p>

![alt text](https://github.com/johannyren/PowerDisplayHomeAssistant/blob/main/Images/Display1.jpg?raw=true)

The hardware consists of a Wemos D1 Mini and a ILI9341 display.

## Wiring of the ILI9341:

```
ILI9341   -> WEMOS D1
VCC       -> 3.3V
GND       -> GND
CS        -> D2 (GPIO4)
RESET     -> D3 (GPIO0)
D/C       -> D4 (GPIO2)
SDI(MOSI) -> D7 (GPIO13)
SCK       -> D5 (GPIO14)
LED       -> 3.3V
```
<p align="left">
  <img src="(https://github.com/johannyren/PowerDisplayHomeAssistant/blob/main/Images/Wiring_ILI9341.jpg?raw=true)" title="ILI9341">
</p>

![alt text](https://github.com/johannyren/PowerDisplayHomeAssistant/blob/main/Images/Wiring_ILI9341.jpg?raw=true)


![alt text](https://github.com/johannyren/PowerDisplayHomeAssistant/blob/main/Images/Wiring__WemosD1.jpg?raw=true)

## Libraries
This sketch id using the library TFT_eSPI (https://github.com/Bodmer/TFT_eWidget)
Be sure to update the file User_Setup.h in the \libraries\TFT_eSPI folder to set pin numbers. 

To match the pin numbers for the ILI9341 wiring above, use the following:
```
// For NodeMCU - use pin numbers in the form PIN_Dx where Dx is the NodeMCU pin designation
    #define TFT_CS   PIN_D2  // Chip select control pin
    #define TFT_DC   PIN_D4  // Data Command control pin
    #define TFT_RST  PIN_D3  // Reset pin (could connect to NodeMCU RST, see next line)
```
## Datasources and connection to Home Assistant
Data sources from Home Assistant are defined in settings.h, together with WiFi details and bearer token for accessing Home Assistant.

Icons for Grid and Solar symbols are defined in icons.h. A few variants are available - choose the ones you prefer.

## Casing
STL files are available for 3D printing a casing for the display. Two variants - one straight for putting on a wall, and one tilted, optimal for a desktop display.

    
