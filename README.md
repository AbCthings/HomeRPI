# The HomeRPI Project
<i>Domotics made simple</i>

Reference website for documentation and roadmap: https://rpi.home.blog/

This project aims at exploring the power of domotics along its whole chain, from field sensors to front-end and data analytics. The project adopts a <i>microservice</i> architecture to empower scalability and durability. Each service is developed as an independent module which 
can be run autonomously. Our main goal is to make things simple for both users and developer. We believe domotics shall make life easier and not harder :) Feel free to suggest/contribute/comment. Enjoy!

# Server modules: Raspberry Pi oriented
Features to develop for Raspberry:
- Listener MQTT (DONE)
- Publisher MQTT
- MariaDb interfacing
- Other integrations for IoT (zwave, google assistant, etc.)
- Web server for OTA to NodeMCU

# Front-end: a single website to control everything
Front-end development:
- Node-red dashboard for control and monitor

# Edge elements: field sensors and actuation
Features to develop for NodeMCU:
- Sensor readings
- MQTT publisher and subscriber
- GPIO piloting
- OTA Updates
- NTP time synchronization

# Mobile app development: Android and iOS 
Features to develop for Android:
- MQTT publisher and subscriber
- GUI updater (update varables from MQTT messages and send commands)
