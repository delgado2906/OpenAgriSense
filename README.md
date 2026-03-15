# OpenAgriSense
Embedded sensor system for continuous monitoring of temperature, humidity, light, and rainfall, viewable via LCD or ESP-32 Wi-Fi server. Autonomously controls irrigation and Peltier modules, sends SMS alerts on threshold breaches, and runs entirely on a 50W solar panel and 12V battery.

Overview:
ESP32 Smart Agriculture System is an open-source IoT project designed to monitor environmental and water conditions in small-scale agricultural systems. The goal of this project is to provide farmers, students, and makers with an affordable and modular platform capable of collecting real-time data from different sensors and using it to make better decisions for crop management.

The system uses an ESP32 microcontroller as the central node, connected to multiple sensors that measure key variables affecting plant growth, such as soil moisture, sunlight intensity, and water quality. The collected data can then be transmitted via WiFi to a dashboard or server for monitoring and analysis.

This project aims to demonstrate how low-cost embedded systems can be used to build practical precision agriculture tools that improve crop monitoring while remaining accessible to students and hobbyists.

Project Goals:

The main objectives of this project are:

1. Provide an affordable open-source agriculture monitoring system.
2. Collect real-time environmental data from agricultural environments.
3. Enable remote monitoring using WiFi connectivity.
4. Create a modular architecture so additional sensors can easily be added.
5. Serve as an educational platform for IoT and smart agriculture systems.

Ultimately, the project is designed so that anyone with basic electronics knowledge can replicate or extend it.

Core Features:

1. Real-time sensor data collection
2. Wireless communication via ESP32 WiFi
3. Modular sensor architecture
4. Designed for low-cost agricultural environments
4. Expandable with additional sensors or automation systems

Hardware Components:
The system is built around commonly available components that allow easy replication.

Microcontroller:
ESP32 Development Board

The ESP32 acts as the central controller responsible for reading sensors, processing data, and transmitting information via WiFi.

Sensors:

The system currently integrates several sensors used for monitoring crop conditions:

1. Soil Moisture Sensor (FC-28)
Measures the moisture level in the soil to help determine irrigation needs.

2. Soil Temperature Sensor (MAX6675)
Provides additional soil data that can influence crop performance.

3. Ambient Temperature and Humidity (DHT-22)
Collects data regarding temparature and humidity of the surroundings.

4. Carbon Dioxide Sensor (MG811)
Tracks the amount of carbon dioxide present in the crop area to provide additional data that can affect the crops growth.

6. Light Intensity Sensor (Uv Uvm-30a)
Measures sunlight exposure, which is an important variable for plant growth.

7. Water Turbidity Sensor (TTS Sensor)
Used to monitor the quality of water used for irrigation.

8. Total Dissolved Solids Sensor (TDS Sensor)
Measures the concentration of disolved solids in water, helping to measure the quality of the water that feeds the crops.

9. pH Sensor (pH sensor + HW-828)
Measures the pH of the water used to irrigate the crops.

System Implementation:

Sensor Data Acquisition

Each sensor is connected to the ESP32 using either analog or digital input pins. The microcontroller periodically reads data from these sensors and processes the raw values into meaningful measurements.

Data Processing

The ESP32 collects sensor readings at regular intervals and formats them into structured data that can be transmitted or logged for further analysis.

Wireless Communication

Using its built-in WiFi capability, the ESP32 can transmit collected data to an external system such as:

1. a web dashboard
2. a remote server
3. an IoT platform

This enables real-time monitoring of crop conditions from any connected device.

Expandability:

The system is designed with modularity in mind. Additional components can easily be integrated, such as:

1. irrigation control systems
2. additional environmental sensors
3. automated decision systems

This allows the platform to evolve into a more advanced smart agriculture infrastructure.

Educational Value:

Beyond its agricultural use, this project also serves as an educational platform for learning about:

1. Internet of Things (IoT)
2. embedded systems
3. environmental sensing
4. real-time data monitoring
5. precision agriculture technologies

Because the entire system is open-source, it can be used by students, researchers, and hobbyists interested in building their own IoT-based agricultural tools.

Future Improvements:

Some planned improvements for future versions include:

1. Automated irrigation control
2. Cloud-based data storage
3. Advanced data visualization dashboards
4. AI-based crop condition analysis
5. Mobile monitoring applications
