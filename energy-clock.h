// 12 pixel EnergyClock
#include "esphome.h"

// Define the PRICE_CAT enum
enum PRICE_CAT
{
  VERY_CHEAP,
  CHEAP,
  NORMAL,
  EXPENSIVE,
  VERY_EXPENSIVE
};

// Define a struct to map price categories to colours and limits
struct PriceCat_t
{
  PRICE_CAT category;
  Color color;
  double lowLim;
};

// Prices category, colors and lower boundary
PriceCat_t priceCats[] = {
    // 100 % brightness
    {VERY_CHEAP, Color(0x00FF00), 0.00},    // Green
    {CHEAP, Color(0x55AA00), 0.10},         // Light Green
    {NORMAL, Color(0x698C00), 0.20},        // Yellowish Green
    {EXPENSIVE, Color(0xFFFF00), 0.30},     // Yellow
    {VERY_EXPENSIVE, Color(0xFF0000), 0.40} // Red
};

// Negative prices
Color colourNegative = Color(0x800080);

// Global variables
double currentPrice;
double minPrice;
double maxPrice;
String TodaysPrices;
String TomorrowsPrices;

// EnergyClock class:
class EnergyClock : public Component
{

public:

  // Set the values from Home Assistant state
  void SetCurrentPrice(double price)
  {
    if (!isnan(price))
    {
      currentPrice = price;
    }
  }

  void SetMinPrice(double price)
  {
    if (!isnan(price) || price == 0.0)
    {
      minPrice = price;
      // ESP_LOGD("SetMinPrice", "Min set to: %.2f", minPrice);
    }
  }

  void SetMaxPrice(double price)
  {
    if (!isnan(price) || price == 0.0)
    {
      maxPrice = price;
      // ESP_LOGD("SetMaxPrice", "Max set to: %.2f", maxPrice);
    }
  }

  void SetTodaysPrices(String prices)
  {
    if (prices != "")
    {
      TodaysPrices = prices;
    }
  }

  void SetTomorrowsPrices(String prices) { TomorrowsPrices = prices; }

  void testPixel(display::Display *buff, int y)
  {
    Color pixelColor;

    // Test rectangle with color matching price category VERY_EXPENSIVE (RED)
    pixelColor = getPriceColour(priceCats[VERY_EXPENSIVE].lowLim);
    buff->draw_pixel_at(0, y, pixelColor);
  }

  // 12 neopixel ring
  void drawPriceClock(display::Display *buff)
  {
    int currentHour = 0;
    Color pixelColor = COLOR_OFF;
    double price = 0;

    if (id(homeassistant_time).now().is_valid())
      currentHour = id(homeassistant_time).now().hour;
    int start = 0;

    if (currentHour >= 12) // Past noon
      start = 12;
      
    // Loop through 12 consecutive hours, starting from 0 or 12
    for (int i = 0; i < 12; i++)
    {
      int hour = start + i; // Actual hour in the day
      int y = i % 12;       // map y to 0-11

      price = priceArray[hour]; // Get price for thisHour

      ESP_LOGD("drawPriceRing", "i: %d, hour: %d, price: %.2f", i, hour, price);

      pixelColor = getPriceColour(price); // get price bar color
      pixelColor = getScaled(pixelColor); // scale with brightness
      // draw pixel
      buff->draw_pixel_at(i, y, pixelColor);
    }
  }

  // Deserialize the JSON string from ENTSO-E
  void SetPrices(String day)
  {
    String prices;
    double *targetArray;

    int i = 0;
    int startPos = 0;

    int objectStart, objectEnd;
    String objectStr;
    int pricePos;

    int valueStart;
    int valueEnd;

    String priceStr;

    // Select the appropriate data source and target array
    if (day == "tomorrow")
    {
      prices = TomorrowsPrices;
      targetArray = priceArrayTomorrow;
    }
    else
    {
      prices = TodaysPrices;
      targetArray = priceArray;
    }

    // Find each object in the array
    while (startPos < prices.length() && i < 50)
    {
      objectStart = prices.indexOf('{', startPos);
      if (objectStart == -1)
        break;

      objectEnd = prices.indexOf('}', objectStart);
      if (objectEnd == -1)
        break;

      // Extract the object string
      objectStr = prices.substring(objectStart, objectEnd + 1);

      // pos of price field within object
      pricePos = objectStr.indexOf("'price':");

      if (pricePos != -1)
      {
        // Find the start of the price value
        valueStart = pricePos + 8; // Length of "'price':" or "\"price\":"

        // Skip any whitespace
        while (valueStart < objectStr.length() &&
               (objectStr[valueStart] == ' ' || objectStr[valueStart] == ':'))
        {
          valueStart++;
        }

        // Find the end of the price value (either comma or closing brace)
        valueEnd = objectStr.indexOf('}', valueStart);

        if (valueEnd != -1)
        {
          // Extract and convert the price value
          priceStr = objectStr.substring(valueStart, valueEnd);
          targetArray[i] = priceStr.toFloat(); //
          // ESP_LOGD("SetPrices", "Extracted price[%d]: %.2f", i, targetArray[i]);
          i++;
        }
      }

      // Move to the next object
      startPos = objectEnd + 1;
    }

    // Fill remaining positions with 0 if any
    while (i < 50)
    {
      targetArray[i] = 0;
      i++;
    }
  }

private:
  display::Display *vbuff;

  double priceArray[50];
  double priceArrayTomorrow[50];
  String TomorrowsPrices;

  // Return Colour matching with prices
  Color getPriceColour(double price)
  {
    const int numOfCats = sizeof(priceCats) / sizeof(priceCats[0]);
    
    if (price < 0.0)
    {
      // negative prices
      return colourNegative;
    }

    for (int i = 0; i < numOfCats; i++)
    {
      // If it's the last category, check if the price is above the lower limit
      if (i == numOfCats - 1)
      {
        if (price >= priceCats[i].lowLim)
        {
          return priceCats[i].color;
        }
      }
      // For other categories, check if the price is within the range
      else if (price >= priceCats[i].lowLim && price < priceCats[i + 1].lowLim)
      {
        return priceCats[i].color;
      }
    }
    // Log a warning if no matching color is found
    ESP_LOGD("EnergyClock", "No color matching with price: %f", price);

    return COLOR_OFF; // LedClock Off if no match found
  }

  /// @brief scale color with brightness number
  /// @param col 
  /// @return 
  Color getScaled(Color col = COLOR_OFF)
  {
    Color scaledCol;
    float brightness = 1.0f;
  
    if (id(display_brightness).has_state())
    {
      brightness = id(display_brightness).state / 100.0f;
      if (brightness < 0.0f) brightness = 0.0f;
      if (brightness > 1.0f) brightness = 1.0f;
    }
  
    scaledCol.r = (uint8_t)(col.r * brightness);
    scaledCol.g = (uint8_t)(col.g * brightness);
    scaledCol.b = (uint8_t)(col.b * brightness);
    scaledCol.w = 0;

    return scaledCol;
  }
  
}; // class
