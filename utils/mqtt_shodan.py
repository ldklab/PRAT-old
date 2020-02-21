#!/usr/bin/env python3

import paho.mqtt.client as mqtt

import argparse
import sys

def on_connect(client, userdata, flags, rc):
	print("[+] Connection successful")
	client.subscribe('#', qos = 1) # Sub to all topics.
	client.subscribe('$SYS/#')

def on_message(client, userdata, msg):
	print("[+] Topic: {} - Message: {}".format(msg.topic, msg.payload))

if __name__ == "__main__":
	try:
		my_parser = argparse.ArgumentParser(description="Specify host IP to connect to")

		my_parser.add_argument('Host',
								metavar='host',
								type=str,
								help='The host to connect to')

		args = my_parser.parse_args()

		host = args.Host

		client = mqtt.Client(client_id= "MqttClient")
		client.on_connect = on_connect
		client.on_message = on_message

		client.connect(host, 1883, 60)
		client.loop_forever()
	except KeyboardInterrupt:
		print("\n[-] Exiting.")
		sys.exit()
