import requests

files = {'file': open('/git/SmartHome-StairsLight/.pio/build/esp32doit-devkit-v1/firmware.bin', 'rb')}
requests.post("http://192.168.0.180/update", files=files)