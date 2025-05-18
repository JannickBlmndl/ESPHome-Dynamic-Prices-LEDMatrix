// get multiple values of home assistant and map to matrix like matrixShowTibber
// https://esphome.io/api/classesphome_1_1display_1_1_display.html#a41d1f952fd82df8dcd09b7fd9a92df66
// https://github.com/Till-83/Tibber_Price_Monitor/tree/02744018b3ae67c152b2b9c5a669c9e4ff36c838
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
    // {VERY_CHEAP, Color(0x00FF00), 0.00},    // Green
    // {CHEAP, Color(0x55AA00), 0.10},         // Light Green
    // {NORMAL, Color(0x698C00), 0.20},        // Yellowish Green
    // {EXPENSIVE, Color(0x808000), 0.30},     // Yellow
    // {VERY_EXPENSIVE, Color(0xFF0000), 0.40} // Red
    // 50% brightness
    {VERY_CHEAP, Color(0x007F00), 0.00},    // Green (50% brightness)
    {CHEAP, Color(0x2B5500), 0.10},         // Light Green (50% brightness)
    {NORMAL, Color(0x344600), 0.20},        // Yellowish Green (50% brightness)
    {EXPENSIVE, Color(0x404000), 0.30},     // Yellow (50% brightness)
    {VERY_EXPENSIVE, Color(0x7F0000), 0.40} // Red (50% brightness)
};

// Global variables
double currentPrice;
double minPrice;
double maxPrice;
String TodaysPrices;
String TomorrowsPrices;

// EnergyMatrix class:
class EnergyMatrix : public Component
{

public:
  // Test to draw a rectangle
  void testRectangle(display::Display *buff, int x, int y, int width, int height)
  {
    Color color;

    // Test rectangle with color matching price category VERY_EXPENSIVE (RED)
    color = getPriceColour(priceCats[VERY_EXPENSIVE].lowLim);
    buff->rectangle(x, y, width, height, color);
  }

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
      ESP_LOGD("SetMinPrice", "Min set to: %.2f", minPrice);
    }
  }

  void SetMaxPrice(double price)
  {
    if (!isnan(price) || price == 0.0)
    {
      maxPrice = price;
      ESP_LOGD("SetMaxPrice", "Max set to: %.2f", maxPrice);
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

  // display 8x8 prices visual
  void drawPriceMatrix(display::Display *buff)
  {
    int height;
    int currentHour = 0;
    int tmpHour;
    bool dayFlag = false; // include data of tommorow

    // Get current hour from Home Assistant time
    if (id(homeassistant_time).now().is_valid())
    {
      currentHour = id(homeassistant_time).now().hour;
      ESP_LOGD("drawPrice", "currentHour: %d", currentHour);
    }

    for (int i = 0; i < 8; i++) // loop over 8 hours
    {
      tmpHour = currentHour + i;
      double price = 0;

      if (tmpHour > 23)
      {
        tmpHour = tmpHour - 24;
        dayFlag = true;
      }

      price = dayFlag == 0 ? priceArray[tmpHour] : priceArrayTomorrow[tmpHour];

      if (isnan(price))
      {
        ESP_LOGD("drawPrice", "hour: %d, price is NUll", tmpHour);
      }
      else // There's a valid price
      {
        // Calculate the height of the bar
        if (maxPrice == minPrice)
        {
          height = 1; // Avoid division by zero
        }
        else
        {
          height = (int)(8 * (price - minPrice) / (maxPrice - minPrice));
          height++;
        }

        if (height > 8) // Constrain height to matrix bounds
          height = 8;

        ESP_LOGD("drawPrice", "i: %d, hour: %d, price: %.2f, Height: %d, dayFlag: %d", i,
                 tmpHour % 24, price, height, dayFlag);
        // Draw
        drawMatrixLine(buff, i, height, getPriceColour(price));
      }
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
          ESP_LOGD("SetPrices", "Extracted price[%d]: %.2f", i, targetArray[i]);
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

  int xPos, yPos;
  float xFactor, yFactor;

  double priceArray[50];
  double priceArrayTomorrow[50];
  String TomorrowsPrices;

  // Draw line on matrix
  void drawMatrixLine(display::Display *buff, int column, int height, Color color = COLOR_ON)
  {
    int y2 = height-1; // y2, height starts from 0

    buff->line(column, 0, column, y2, color);
  }

  // Return Colour matching with prices
  Color getPriceColour(double price)
  {
    const int numOfCats = sizeof(priceCats) / sizeof(priceCats[0]);

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
    ESP_LOGD("EnergyMatrix", "No color matching with price: %f", price);
    
    return COLOR_OFF; // LEdMatrix Off if no match found
  }

}; // class
