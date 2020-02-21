#!/usr/bin/env python3

import paho.mqtt.client as mqtt
import sys

def on_connect(client, userdata, flags, rc):
	print("[+] Connection successful")
	client.subscribe('#', qos = 1) # Sub to all topics.
	client.subscribe('$SYS/#')

def on_message(client, userdata, msg):
	print("[+] Topic: {} - Message: {}".format(msg.topic, msg.payload))

if __name__ == "__main__":
	client = mqtt.Client(client_id= "MqttClient")
	client.on_connect = on_connect
	client.on_message = on_message

	client.connect('127.0.0.1', 1883, 60)
	client.loop_forever()

	#sys.exit(0)