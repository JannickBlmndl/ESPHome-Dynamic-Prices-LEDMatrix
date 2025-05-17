// to do
// get multiple values of home assistant and map to matrix like matrixShowTibber
// https://esphome.io/api/classesphome_1_1display_1_1_display.html#a41d1f952fd82df8dcd09b7fd9a92df66
// https://github.com/Till-83/Tibber_Price_Monitor/tree/02744018b3ae67c152b2b9c5a669c9e4ff36c838
#include "esphome.h"

#define DISABLED

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

// Global variables.
// double currentPower;
// double dailyEnergy;
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

  // Functions to set the values from Home Assistant

#ifndef DISABLED
  void SetCurrentPower(double power)
  {
    if (!isnan(power))
    {
      currentPower = power;
    }
  }
  void SetDailyEnergy(double energy)
  {
    if (!isnan(energy))
    {
      dailyEnergy = energy;
    }
  }
#endif // DISABLED

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

#ifndef DISABLED
  // Draw the graph
  // void DrawPriceGraph(display::Display *buff)
  // {
  //   double lastprice = 0;
  //   double price;

  //   for (int priceCount = 0; priceCount < 24; priceCount++)
  //   {
  //     price = priceArray[priceCount];
  //     lastprice = AddPrice(buff, priceCount, price, priceCount - 1, lastprice);
  //   }
  //   // Add last piece if we know the price at midninght tomorrow
  //   price = priceArrayTomorrow[0];
  //   if (price > 0)
  //     lastprice = AddPrice(buff, 24, price, 23, lastprice);
  // }
#endif // DISABLED

  // display 8x8 prices visual
  void drawPriceMatrix(display::Display *buff)
  {
    int height;

    for (int i = 0; i < 8; i++)
    {
      if (!isnan(priceArray[i]))
      {
        // Calculate the height of the bar
        if (maxPrice == minPrice)
        {
          height = 1; // Avoid division by zero
        }
        else
        {
          height = (int)(8 * (priceArray[i] - minPrice) /
                         (maxPrice - minPrice));
          height++;
        }

        // Constrain height to matrix bounds
        if (height > 8)
        {
          height = 8;
        }

        ESP_LOGD("drawPrice", "i: %d, price: %f, Height: %d", i, priceArray[i], height);
       
        // Draw
        drawMatrixLine(buff, i, height, getPriceColour(priceArray[i]));
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
          ESP_LOGD("SetPrices", "Extracted price[%d]: %.2f", i,targetArray[i]);
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

#ifndef DISABLED
  void SetPrices(String day)
  {
    String prices;
    String array[25];
    int r = 0, t = 0;

    // if (TodaysPrices == "")
    //   TodaysPrices = LoadStringFromNvm("TodaysPrices");

    if (day == "tomorrow")
      prices = TomorrowsPrices;
    else
      prices = TodaysPrices;

    // Remove brackets if present
    prices.replace("[", "");
    prices.replace("]", " ");

    for (int i = 0; i < prices.length(); i++)
    {
      if (prices[i] == ' ' || prices[i] == ',')
      {
        if (i - r > 1)
        {
          array[t] = prices.substring(r, i);
          t++;
        }
        r = (i + 1);
      }
    }

    for (int k = 0; k <= t; k++)
    {
      //		ESP_LOGD("SetPrices Prices: ", array[k].c_str());
      if (day == "tomorrow")
        priceArrayTomorrow[k] = array[k].toFloat();
      else
        priceArray[k] = array[k].toFloat();
    }
  }
#endif // DISABLED

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
    buff->line(column, 0, column, height, color);
  }

  // void DrawGraphLine(display::Display *buff, double x1, double x2, double y1, double y2,
  //                    Color color = COLOR_ON)
  // {
  //   buff->line(xPos + x1 * xFactor, yPos + graphHeight - (y1 * yFactor), xPos + x2 * xFactor,
  //              yPos + graphHeight - (y2 * yFactor), color);
  //   buff->line(xPos + x1 * xFactor, yPos + 1 + graphHeight - (y1 * yFactor), xPos + x2 * xFactor,
  //              yPos + 1 + graphHeight - (y2 * yFactor), color);
  //   buff->line(xPos + x1 * xFactor, yPos + 2 + graphHeight - (y1 * yFactor), xPos + x2 * xFactor,
  //              yPos + 2 + graphHeight - (y2 * yFactor), color);
  //   //		ESP_LOGD("GraphGrid x1: ", String(xPos + x1*xFactor).c_str());
  // }

#ifndef DISABLED
  double AddPrice(display::Display *buff, int hour, double price, int lastHour, double lastPrice)
  {
    if (lastHour < 0)
      lastHour = 0;
    if (lastPrice == 0)
      lastPrice = price;
    DrawGraphLine(buff, lastHour, hour, lastPrice, price, getPriceColour(lastPrice)); // matrix line

    return price;
  }
#endif // DISABLED

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
    // Matrix Off if no match found
    return COLOR_OFF;
  }

}; // class
