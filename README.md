# GreenLineTicker
Small IoT display for showing real-time predictions for when the next Green Line train will arrive

![example animation](tick.gif)

## Parts
* [Adafruit ESP8266 Feather](https://www.adafruit.com/product/2821)
* [CharliePlex LED Matrix](https://www.adafruit.com/product/3136)

## Configuration
Change lines 10 and 11 for your network configuration, and line 93 to select what stop the ticker will reflect predictions for.
Reference the MBTA API for help: https://api-v3.mbta.com/docs/swagger/index.html#/
Stops can be listed via: https://api-v3.mbta.com/stops

**Note**: This doesn't automatically adjust for DST, yet :)
