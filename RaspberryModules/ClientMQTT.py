""" 
This is a Python MQTT client.

It connects to a broker, and reconnect when connection is lost.

The main code can be inserted in a loop.

Configuration file must be in the same directory.

Author: Alessandro Bortoletto
Creation: 09/12/2018
Last update: 11/12/2018

"""

import paho.mqtt.client as mqtt
import time
import json
import sys

#set the standard output to point to a log file 
# --> EVERY print() WILL PRINT IN THE LOG FILE!
# --> The file is opened in 'append' mode
sys.stdout = open("PrimoTestMQTT_log.txt", "a")


#Define and initialize the config variables
BROKER_HOST = '0'
BROKER_PORT = 0

#This section reads and check the configuration file
try:
    
    #Import the parameters from the configuration file (PrimoTestMQTT.config)
    configFile = open("PrimoTestMQTT.config","r")

    for line in configFile: 

        #check for the keywords and save locally
        if line.count("BROKER_HOST")>0 :
            #find the position of the first useful character
            firstUseful = line.find("=") + 1
            #get only the last (useful) part of the string, DELETE THE LAST LINE BREAK CHARACTER '\n'
            value = line[firstUseful:len(line)-1]
            #save the obtained value into the constant variable
            BROKER_HOST = value
            #print that the operation was successful
            print("Broker host address: ",value)

        #check for the keywords and save locally
        if line.count("BROKER_PORT")>0 :
            #find the position of the first useful character
            firstUseful = line.find("=") + 1
            #get only the last (useful) part of the string
            value = line[firstUseful:len(line)]
            #save the obtained value into the constant variable, PARSING TO INT THE VALUE
            BROKER_PORT = int(value)
            #print that the operation was successful
            print("Broker port address: ",value)
            
    configFile.close()
            
except Exception as exception:
    print("An error occoured while reading configuration from file.", exception)
    sys.exit()

#check that all the variables are correct (safe code)
uncorrectFlag = False
if len(BROKER_HOST)<3:
    uncorrectFlag = True
if BROKER_PORT < 10:
    uncorrectFlag = True
if uncorrectFlag:
    print("An error occoured while checking configuration variables.")
    sys.exit()

print("\n--> The configuration file has been successfully acquired. <--\n")


#What to do when you receive a message
def on_message(client, userdata, message):
    print("Message received:" , str(message.payload.decode("utf-8")))
    print("(on topic:" , message.topic , ")")
    print("message qos=",message.qos)
    print("message retain flag=",message.retain)

#What to do when the connection with broker is lost
def on_disconnect(client, userdata, message):
    client.is_connected = False
    
#This function tries to connect to a broker and repeat until successful
def try_connecting(client):
    while not client.is_connected:
        try:
            print("Connecting to broker @",BROKER_HOST)
            client.connect(BROKER_HOST,BROKER_PORT,60) #connect to broker
        except Exception as ex:
            print("Connection failed: ", ex)
            # Add a sleep to avoid reconnecting continuosly: try to connect every 2 seconds
            time.sleep(2)
            print("\nTrying again...")
        else:
            print("Connection success!\n")
            client.is_connected = True
            

#Here starts the connection flow...
#Create the client
client = mqtt.Client()
client.is_connected = False

#Assign the functions to call on events
client.on_message = on_message
client.on_disconnect = on_disconnect

#Start the client loop and connect to broker
client.loop_start()
try_connecting(client)            

#Subscribe to desired topics here!
client.subscribe("HomeRPI/commands")

#Here starts the loop (put here main code)...
try:
    while True:
        
        if client.is_connected:
            
            #Local log print
            print("[",int(time.time()),"] Client is connected")
            #Publish a raw string to say that you still connected
            client.publish("HomeRPI/status","CONNECTED")
            
            #Publish a JSON message to say that you still connected
            messaggioJSON = {"status": "Connected"} #{Keyword: Value, Keyword: Value, Keyword: Value}
            client.publish('HomeRPI/status',json.dumps(messaggioJSON),1)
            
            #
            # The main code goes here (for example sensors updating)
            #
            
            #The following delay sets the FREQUENCY OF THE OPERATIONS (how often sample from sensors, how often upate to broker, etc.)
            time.sleep(2)
        
        else:
            
            print("[",int(time.time()),"] Connection lost!")
            # Try to reconnect to borker
            try_connecting(client)
            time.sleep(2)

except KeyboardInterrupt:
    print("\nExiting...")

#On shut down disconnect from broker and stop the client loop
client.disconnect()
client.loop_stop()

print("Exit success!\n")
