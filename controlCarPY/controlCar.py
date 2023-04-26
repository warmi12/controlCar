from pynput import keyboard
from paho.mqtt import client as mqtt_client
import sys

mqtt_message = {
  "Key.up": "f",
  "Key.down": "b",
  "Key.left": "l",
  "Key.right": "r"
}

key_state = {
  "Key.up": 0,
  "Key.down": 0,
  "Key.left": 0,
  "Key.right": 0
}

broker = '192.168.0.137'
port = 1883
topic = "/esp32/controlCar"
client_id = '2'
username = 'pi'
password = 'maciek'
client = None

def on_press(key):
    if key == keyboard.Key.esc:
        client.disconnect()
        sys.exit()   
    elif key_state[str(key)] == 0:
        publish(mqtt_message[str(key)])
        key_state[str(key)] = 1
        print('Pressed Key %s' % key)

def on_release(key):
    if key_state[str(key)] == 1:
        publish(mqtt_message[str(key)])
        key_state[str(key)] = 0
        print('Released Key %s' % key)
    
    
def connect_mqtt():
    global client
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client

def publish(msg):
    global client
    result = client.publish(topic, msg)
    status = result[0]
    if status == 0:
        print(f"Send `{msg}` to topic `{topic}`")
    else:
        print(f"Failed to send message to topic {topic}")

def run_mqtt():
    global client
    client = connect_mqtt()
    client.loop_start()
    
def run_keyvoard_listener():
    with keyboard.Listener(on_release = on_release, on_press = on_press) as listener:
        listener.join()

if __name__ == '__main__':
    run_mqtt()
    run_keyvoard_listener()