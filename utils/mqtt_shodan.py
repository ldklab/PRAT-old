#!/usr/bin/env python3

import paho.mqtt.client as mqtt

import argparse
import json
import sys

def on_connect(client, userdata, flags, rc):
	print("[+] Connection successful")
	client.subscribe('#', qos = 1) # Sub to all topics.

	if status:
		client.subscribe('$SYS/#') # Broker status.
	else:
		print("[-] Ignoring $SYS messages")

def on_message(client, userdata, msg):
	print("[+] Topic: {}".format(msg.topic))
	try:
		parsed = json.loads(msg.payload)
		print("[+] Message: \n{}".format(json.dumps(parsed, indent=4, sort_keys=True)))
	except ValueError:
		#print("[-] {} could not be parsed as JSON. Getting str.".format(msg.payload))
		try:
			print("[+] Message: \n{}".format(msg.payload.decode('utf-16')))
		except UnicodeDecodeError:
			print("[+] Message: \n{}".format(msg.payload))

def main():
	try:
		my_parser = argparse.ArgumentParser(description="Specify host IP to connect to")

		my_parser.add_argument('Host',
								metavar='host',
								type=str,
								help='The host to connect to')
		my_parser.add_argument('--status',
								dest='status',
								action='store_true',
								help='Enable broker status output')
		my_parser.add_argument('--no-status',
								dest='status',
								action='store_false',
								help='Disable broker status output')

		my_parser.set_defaults(status=False)
		args = my_parser.parse_args()

		host = args.Host

		client = mqtt.Client(client_id= "MqttClient")
		client.on_connect = on_connect
		client.on_message = on_message

		client.connect(host, 1883, 60)
		client.loop_forever()
	except KeyboardInterrupt:
		print("\n[-] Exiting.")
		client.disconnect()
		sys.exit(0)

if __name__ == "__main__":
	main()
