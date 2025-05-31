## ESPHome Dynamic Prices LEDMatrix
<img src="https://raw.githubusercontent.com/JannickBlmndl/ESPHome-Dynamic-Prices-LEDMatrix/main/imgs/ESPHome_LedMatrix.png" width="200" />
ESPHome project for dynamic hourly energy prices from [HACS Entso-e integration](https://github.com/JaccoR/hass-entso-e)on a simple 8x8 WS2812B LED matrix.

Prerequirements:
- Home Assistant + ESPHome installation
- HACS Entso-e integration [tutorial (dutch)](https://doe-duurzaam.nl/artikel/dynamische-energieprijzen-inlezen-met-home-assistant-zo-doe-je-dat/) 

## Hardware
- Wemos D1 Mini or similar ESP board with WiFi
- Neopixel / WS2812 compatible LED matrix (8x8) CJMCU-64 8x8 (D-out could be d-in!)
- 5V USB power suply
- 90degree USB-C cable
- 3D printed stand 
  - [3D print by Hatrick3D](https://www.printables.com/model/908102-8x8-led-matrix-display-as-energy-price-indicator-f)
  - [another by Rutger Middendorp](https://www.printables.com/model/1146867-dynamic-energy-price-indicator)

### Pinout

| Wemos D1 mini | LED matrix CJMCU-64         |Comment|
| ----------- | --------------|--------------|
|GPIO13 (MOSI) | D-IN |Silkscreen label could be DOUt
|VBUS (5V)| +5V||
|GND |GND||

## Bonus 
Lovelace config for an [Apexcharts](https://github.com/RomRider/apexcharts-card) daily price dashboard card by [Marcel](https://doe-duurzaam.nl/2023/01/02/dynamische-energieprijzen-inlezen-met-home-assistant-zo-doe-je-dat/#comment-3417).

<img src="https://raw.githubusercontent.com/JannickBlmndl/ESPHome-Dynamic-Prices-LEDMatrix/main/imgs/apexchart_preview.png" width="400" />

```
type: custom:apexcharts-card
apex_config:
  chart:
    height: 300px
graph_span: 24h
span:
  start: day
now:
  show: true
  label: Now
header:
  show: true
  title: Energy price today (€/kWh)
experimental:
  color_threshold: true
series:
# SSET ENTSO-e entity_id here
  - entity: sensor.elektriciteit_gemiddelde_prijs_vandaag 
    stroke_width: 2
    float_precision: 3
    type: column
    opacity: 1
    color_threshold:
      - value: -0.1
        color: "#800080"
      - value: 0.1
        color: "#00ff00"
      - value: 0.2
        color: "#ffff00"
      - value: 0.3
        color: "#ff2a00"
    data_generator: |
      return entity.attributes.prices.map((record, index) => {
        return [record.time, record.price];
      });
yaxis:
  - id: Price
    decimals: 2
    min: 0
```

__Advanced__\
To automate the display's brightness based on presence, you can link the `number.esphome_wemos_energy_matrix_brightness` entity to a PIR sensor, such as the [Everything Presence Lite](https://shop.everythingsmart.io/products/everything-presence-lite), using a Home Assistant automation.

To adjust the display brightness, use the action below. The change will take effect after the configured [update_interval](https://github.com/JannickBlmndl/ESPHome-Dynamic-Prices-LEDMatrix/blob/1d1374a6d2a879d34fcea5a68a5b5627f7bdad60/esphome-wemos-energy-matrix.yaml#L6) seconds.
```
action: number.set_value
metadata: {}
data:
  value: "50.0"
target:
  entity_id: number.esphome_wemos_energy_matrix_brightness
```

## Credits
https://github.com/Till-83/Tibber_Price_Monitor/tree/main @Till-83
https://github.com/johannyren/PowerDisplayESPHome/blob/main/power-display-esphome.yaml @johannyren

Inspiration from
https://www.bright.nl/nieuws/1263042/diy-zo-3d-print-je-een-led-matrix-scherm-voor-dynamische-energietarieven.html

### Project status
Running stably on 1 device