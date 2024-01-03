import subprocess
import requests
import logging

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s %(levelname)s %(message)s'
)

logging.info("Compiling...")

# process = subprocess.Popen(
#     ["platformio", "run", "--environment", "esp32doit-devkit-v1"],
#     stdout=subprocess.PIPE)

# while True:
#     output = process.stdout.readline()
#     if output == '' and process.poll() is not None:
#         break
#     if output:
#         print(output.decode('utf-8').strip())


logging.info("Updating the device...")
files = {'file': open('/git/SmartHome-StairsLight/.pio/build/esp32doit-devkit-v1/firmware.bin', 'rb')}
requests.post("http://192.168.0.180/update", files=files)
logging.info("Done!")