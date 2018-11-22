#!python3

import paho.mqtt.client as mqtt
import time

#modules
########################################
def on_message(client, userdata, message):
    print("Message received:" , str(message.payload.decode("utf-8")))
    print("(on topic:" , message.topic , ")")
    #print("message qos=",message.qos)
    #print("message retain flag=",message.retain)
########################################

#constants
########################################
BROKER_ADDRESS = "192.168.1.127"
#broker_address="iot.eclipse.org"
########################################

#program
########################################
print("Creating new instance of the MQTT Listener Module")

client = mqtt.Client("MQTTClient") #create new instance

client.on_message = on_message #attach function to callback

#print("connecting to broker")
#client.connect(BROKER_ADDRESS) #connect to broker

connected = False
while not connected:
    try:
        print("Connecting to broker @",BROKER_ADDRESS)
        client.connect(BROKER_ADDRESS,1883,60) #connect to broker
    except Exception as ex:
        print("Connection failed: ", ex)
        print("Trying again...")
    else:
        print("Connection success!")
        connected = True

client.loop_start() #start the loop

print("Subscribing to topic","RpiHome/standardTopic")
client.subscribe("RpiHome/standardTopic")

#print("Publishing message to topic","house/bulbs/bulb1")
#client.publish("house/bulbs/bulb1","OFF")

#main code here
try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("Exiting")
    
client.loop_stop() #stop the loop
client.disconnect()
########################################
