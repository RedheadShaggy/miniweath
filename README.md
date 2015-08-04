# miniweath
Simple c app for fetching current weather in city from OpenWeatherMap.org  
  
I develop it for logging weather using my OpenWRT router which has not so much memory for other packages.  

### Features
+ File size is about 10kb
+ Uses only standart c library
+ Uses sokets
+ Supports Json output format, may be useful for logging with crontab  
  
---
##### Compiling
Jsmn library downloads from its repo automatically on make. (Requires installed Mercurial).  
And for compiling just execute make.  

##### Usage
```
$ miniweath New-York
New York

    27°C

Wind        5 m/s to 230°
Cloudiness  Clouds (40%)
Pressure    1011 hpa
Humidity    61%
Sunrise     13:55
Sunset      04:07
```
  
Or if needed json output:
```
$ miniweath New-York -J
{"dt":1438712263, "name":"New York", "description":"Clouds", "cloudiness":40, "wind_speed":5, "wind_direction":230, "pressure":1011, "temp":27, "humidity":61, "sunrise":1438682157, "sunset":1438733255}
```
---
Jsmn it's a cool and small c library for parsing json.  
Check it out on http://zserge.com/jsmn.html
