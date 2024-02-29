# Arduino Dashboard

!Arduino Logo

This project allows you to work with time using the RTC DS3231 module, and displaying information to an OLED screen. An IR sensor, PIR motion detector and smoke detector is also used for home security. An RGB LED module is also connected as a light alert indicator and low-power light source at night. The interface is split into 4 separate tabs, toggled with touch sensors. The LED and OLED modules are also toggleable for better longevity.

## Features

There are 4 tabs:
- Time: displaying time and date, a counter for seconds that have passed since a particular event, and conversion to date.
- Sensors: displaying readings from all sensors.
- Sentry Mode: when toggled On, Sentry mode will kick in after 20 seconds then scan the room for motion. The Blue LED will shine bright whenever motion is detected. An optional buzzer can be connected as well for audio alert.
- Quotes: this mode will cycle between a set of inserted quotes, then display it to the OLED screen. 

Switch description are available in the code.

## Getting Started

1. Download the source code onto your local machine
2. Open Arduino IDE, install the required libraries (RTClib, Adafruit_SSD1306)
3. Mount and connect all components together, either on the breadboard or PCB
4. Add your own customizations to the code, including quotes, date calculations, etc.
5. Upload your code to your Arduino.


## Contributing

Contributions are welcome! If you find any issues or have suggestions, feel free to open an issue or submit a pull request.

## License

This project is licensed under the MIT License.

---

Happy tinkering! 🚀
