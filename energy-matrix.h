// to do
// get multiple values of home assistant and map to matrix like matrixShowTibber
// https://esphome.io/api/classesphome_1_1display_1_1_display.html#a41d1f952fd82df8dcd09b7fd9a92df66
// https://github.com/Till-83/Tibber_Price_Monitor/tree/02744018b3ae67c152b2b9c5a669c9e4ff36c838
#include "esphome.h"

#define TEST

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

// Prices category, colors and limit
PriceCat_t priceCats[] = {
    {VERY_CHEAP, Color(0x00FF00), 0},       // Green
    {CHEAP, Color(0x55AA00), 0.10},         // Light Green
    {NORMAL, Color(0x698C00), 0.20},        // Yellowish Green
    {EXPENSIVE, Color(0x808000), 0.30},     // Yellow
    {VERY_EXPENSIVE, Color(0xFF0000), 0.50} // Red
};

// Global variables. Needed to retain values after reboot
// double currentPower;
// double dailyEnergy;
double currentPrice;
double todayMaxPrice;
String TodaysPrices;

// EnergyMatrix class:
class EnergyMatrix : public Component
{

public:
  // void DisplayIcons(display::Display *buff, int x, int y)
  // {
  //   if (currentPower >= 0)
  //     buff->image(x, y, &id(grid_power));
  //   else
  //     buff->image(x, y, &id(solar_power));
  // }

  // Test to draw a rectangle
  void testRectangle(display::Display *buff, int x, int y, int width, int height)
  {
    Color color;
    
    color = Color(0xFF0000); // Test red
    if (1) // arbirary if statement
    {
      // Test rectangle with color matching price category EXPENSIVE 
      // Color color = getPriceColour(priceCats[EXPENSIVE].lowLim);
      buff->rectangle(x, y, width, height, color);
    }
  }

  #ifndef TEST
  // void CreateGraph(display::Display *buff, int x, int y, int width, int height,
  //                  Color color = COLOR_ON)
  // {
  //   setPos(x, y);
  //   setWidth(width);
  //   setHeigh(height);
  //   buff->rectangle(x, y, width, height, color);
  // }

  void SetGraphGrid(display::Display *buff, double xLow, double xInterval, double yLow,
                    double yInterval, Color color = COLOR_ON)
  {
    double xLabel = 0, yLabel = 0;
    double i2;
    Color labelColor = COLOR_CSS_WHITESMOKE;
    for (double i = (xPos + xLow * xFactor); i <= graphWidth + xPos; i += xInterval * xFactor)
    {
      //			ESP_LOGD("GraphGrid i: ", String(i).c_str());
      buff->line(i, yPos, i, yPos + graphHeight, color);
      buff->printf(i - 4, yPos + graphHeight + 10, &id(small_text), labelColor,
                   TextAlign::BASELINE_LEFT, "%.0f", xLabel);
      xLabel += xInterval;
      i2 += xInterval * xFactor;
    }
    // For the last hour...
    buff->printf(i2 + 8, yPos + graphHeight + 10, &id(small_text), labelColor,
                 TextAlign::BASELINE_LEFT, "%.0f", xLabel);

    for (double j = (yLow * yFactor); j < graphHeight; j += yInterval * yFactor)
    {
      //			ESP_LOGD("GraphGrid j: ", String(j).c_str());
      buff->line(xPos, yPos + graphHeight - j, xPos + graphWidth, yPos + graphHeight - j, color);
      buff->printf(xPos - 2, yPos + graphHeight - j, &id(small_text), labelColor,
                   TextAlign::BASELINE_RIGHT, "%.1f", yLabel);
      yLabel += yInterval;
    }
  }

  // Functions to set the values from Home Assistant

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

  void SetCurrentPrice(double price)
  {
    if (!isnan(price))
    {
      currentPrice = price;
    }
  }

  void SetTodayMaxPrice(double price)
  {
    if (!isnan(price) || price == 0)
    {
      todayMaxPrice = price;
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

  // Display current power usage
  void WritePowerText(display::Display *buff, int x, int y)
  {
    // if (isnan(currentPower) || currentPower == 0)
    //   currentPower = LoadValueFromNvm("CurrentPower");
    // if (isnan(currentPrice) || currentPrice == 0)
    //   currentPrice = LoadValueFromNvm("CurrentPrice");
    buff->printf(x, y, &id(large_text), PriceColour(currentPrice), TextAlign::BASELINE_CENTER,
                 "%.0f W", currentPower);
  }
  // Display current price and the price level
  void WritePriceText(display::Display *buff, int x, int y)
  {

    // if (isnan(currentPrice) || currentPrice == 0)
    //   currentPrice = LoadValueFromNvm("CurrentPrice");

    // print price
    buff->printf(120, 257, &id(price_text), COLOR_CSS_WHITESMOKE, TextAlign::BASELINE_CENTER,
                 "%.2f kr/kWh", currentPrice);

    String price;
    if (inRange(currentPrice, EXTREMELY_EXPENSIVE, 100))
    {
      price = EXTREMELY_EXPENSIVE_TEXT;
    }
    else if (inRange(currentPrice, VERY_EXPENSIVE, EXTREMELY_EXPENSIVE))
    {
      price = VERY_EXPENSIVE_TEXT;
    }
    else if (inRange(currentPrice, EXPENSIVE, VERY_EXPENSIVE))
    {
      price = EXPENSIVE_TEXT;
    }
    else if (inRange(currentPrice, NORMAL, EXPENSIVE))
    {
      price = NORMAL_TEXT;
    }
    else if (inRange(currentPrice, CHEAP, NORMAL))
    {
      price = CHEAP_TEXT;
    }
    else if (inRange(currentPrice, VERY_CHEAP, CHEAP))
    {
      price = VERY_CHEAP_TEXT;
    }
    else if (inRange(currentPrice, -100, VERY_CHEAP))
    {
      price = BELOW_VERY_CHEAP_TEXT;
    }

    buff->printf(x, y, &id(large_text), PriceColour(currentPrice), TextAlign::BASELINE_CENTER, "%s",
                 price.c_str());
  }
  // Write the timeline on the graph
  void writeTimeAxis(display::Display *buff, double hour, double minute, Color color = COLOR_ON)
  {
    double timeLineVal = hour + (minute / 60);
    buff->line(xPos + timeLineVal * xFactor, yPos, xPos + timeLineVal * xFactor, yPos + graphHeight,
               color);
  }

  // Write energy consumed so far today
  // void WriteDailyAmount(display::Display *buff, int x, int y, Color color = COLOR_ON)
  // {
  //   if (isnan(dailyEnergy) || dailyEnergy == 0)
  //   {
  //     dailyEnergy = LoadValueFromNvm("DailyEnergy");
  //   }
  //   buff->printf(x, y, &id(energy_text), color, TextAlign::BASELINE_CENTER, "Idag: %.1f kWh",
  //                dailyEnergy);
  //   buff->printf(x, y + 23, &id(energy_text), color, TextAlign::BASELINE_CENTER, "Kostnad: %.2f
  //   kr",
  //                CalculateAccumulatedCost(currentPrice, dailyEnergy));
  // }

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

  // display 8x8 prices visual
  void drawPrice(display::Display *buff)
  {
    int lastheight = 0;
    int height;

    for (int i = 0; i < 8; i++)
    {
      if (!PRICES.price[i].isNull)
      {
        // Calculate the height of the bar
        if (PRICES.maximumPrice == PRICES.minimumPrice)
        {
          height = 1; // Avoid division by zero
        }
        else
        {
          height = (int)(8 * (priceArray[i] - PRICES.minimumPrice) /
                         (PRICES.maximumPrice - PRICES.minimumPrice));
          height++;
        }
  
        // Constrain height to matrix bounds
        if (height > 8)
        {
          height = 8;
        }
  
        // // Debugging output
        // Serial.println(PRICES.price[i].price);
        // Serial.println(PRICES.price[i].level);
        // Serial.println(i);
        // Serial.println(height);
  
        if (i != 0 || firstBlink) // Draw if ready for display
        {
          drawMatrixLine(buff, i, height, colors[PRICES.price[i].level]);
        }
      }
    }
  }

  // Deserialize the JSON string from NordPool
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
    #endif // TEST

private:
  display::Display *vbuff;

  int xPos, yPos;
  float xFactor, yFactor;

  double priceArray[50];
  double priceArrayTomorrow[50];
  double prevDailyEnergy, accumulatedCost;
  String TomorrowsPrices;

  void setPos(int x, int y)
  {
    xPos = x;
    yPos = y;
  }

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

  #ifndef TEST
  double AddPrice(display::Display *buff, int hour, double price, int lastHour, double lastPrice)
  {
    if (lastHour < 0)
      lastHour = 0;
    if (lastPrice == 0)
      lastPrice = price;
    DrawGraphLine(buff, lastHour, hour, lastPrice, price, getPriceColour(lastPrice)); // matrix line

    return price;
  }
    #endif // TEST

  // Return Colour matching with prices
  Color getPriceColour(double nNewPrice) {
      const int numOfCats = sizeof(priceCats) / sizeof(priceCats[0]);
  
      for (int i = 0; i < numOfCats; i++) { // Iterate through all categories
          PRICE_CAT category = priceCats[i].category;
  
          if (category == VERY_CHEAP) { // Lowest price category
              if (inRange(currentPrice, -10, priceCats[i].lowLim))
                  return priceCats[i].color;
          } else if (category == VERY_EXPENSIVE) { // Highest price category
              if (inRange(currentPrice, priceCats[i].lowLim, 10))
                  return priceCats[i].color;
          } else if (inRange(nNewPrice, priceCats[i].lowLim, priceCats[i + 1].lowLim)) {
              return priceCats[i].color;
          }
      }
  
      // Log a warning if no matching color is found
      ESP_LOGD("EnergyMatrix", "No color matching with price: %f", nNewPrice);
  
      // Default return value if no match is found
      return COLOR_OFF; // Replace with a suitable default color
  }

  bool inRange(float val, float minimum, float maximum)
  {
    return ((minimum <= val) && (val <= maximum));
  }

}; // class
