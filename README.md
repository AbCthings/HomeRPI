# The HomeRPI Project
<i>Domotics made simple</i>

Reference website for documentation and roadmap: https://rpi.home.blog/

This project aims at exploring the power of domotics along its whole chain, from field sensors to front-end and data analytics. The project adopts a <i>microservice</i> architecture to empower scalability and durability. Each service is developed as an independent module which 
can be run autonomously, enabling the integration in any other open-source application. Our main goal is to make things simple for both users and developer. We believe domotics shall make life easier and not harder :) Feel free to suggest/contribute/comment. Enjoy!

# Server modules: Raspberry Pi oriented
- Common communication bus
- MQTT wathcdog for platform logging
- Database handler for timeseries and latest values storage
- Web-server for edge devices OTA updates
- Web-server for User Interface (web responsive app)
- Analytics module
- Communication adapters
- Specific third-party products adapters

# Front-end: a responsive website to control everything
- Single page web responsive app: control, configuration and monitoring

# Edge elements: field sensors and actuation
Features for NodeMCU-ESP8266:
- Sensor acquisitions
- MQTT publisher and subscriber
- GPIO piloting
- OTA Updates
- NTP time synchronization
- EEPROM interface for data persistence

# Mobile app development: Android and iOS 
Features to develop for Android:
- Android
- iOS
