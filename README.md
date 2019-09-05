# Arduino 4-plant Soil Moisture Sensor
This is code for an Arduino Nano soil moisture sensor that I built in the summer of 2018

### Purpose
I built this soil-moisture sensor as a side-project in the Summer following my first three quarters in Oregon State University's post-baccalaureate computer science program. It was one of the first hands-on projects I undertook in which I could see the actual results of my code (and my arduino build) in action afterwards. Plus I was always forgetting to water my house plants. This helped save my poor neglected house plants from a slow and thirsty demise.  

### Lessons Learned
I went through several iterations of "prototypes" before arriving at this code and this build. Here are some of the key lessons I learned
+ **Use quality components:** I originally build a similar device with much cheaper resistance-bases soil moisture sensors. These sensors had two sides each of which was coated with metal plating. They worked by passing a weak current through both plates and measuring the resistance between them. The greater the resistance, the drier the soil. The problem with this technique is that over time the metal plates, being charged and in a wet environment, corrode and dissolve. The capacitive sensors I switched to don't have any exposed contacts and thus never corrode.
+ **Test wiring thoroughly before soldering:** I lost a couple of prototypes because my soldering of the LCD displays was faulty in one way or another. A more complete vetting of my wiring diagram and of the components used would have prevented loss of some of these components.
+ **Use sockets for components:** (instead of soldering the components directly into your build. By soldering in a socket, you can remove and replace components whenever necessary. If a component fails at some point and you've soldered it directly into your build, you might have to scrap the whole thing.

