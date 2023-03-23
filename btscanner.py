import sys
import argparse
import time
import datetime
from bluetooth import discover_devices
from fpdf import FPDF

def parse_duration(duration_str):
    value = int(duration_str[:-1])
    unit = duration_str[-1]

    if unit == 's':
        return value
    elif unit == 'm':
        return value * 60
    elif unit == 'h':
        return value * 3600
    else:
        raise ValueError("Invalid duration format")

parser = argparse.ArgumentParser(description='Scan Bluetooth devices and generate a PDF report.')
parser.add_argument('-d', '--duration', required=True, help="Duration of the scan (e.g., '1m', '3h')")
parser.add_argument('-o', '--output', required=True, help='Output PDF file')
args = parser.parse_args()

duration = parse_duration(args.duration)
start_time = time.time()
devices = {}

try:
    while time.time() - start_time < duration:
        nearby_devices = discover_devices(lookup_names=True)
        for address, name in nearby_devices:
            now = datetime.datetime.now()
            print(f"Found device: {name} ({address}) at {now}")

            if address not in devices:
                devices[address] = {'name': name, 'seen': [(now, now)]}
            else:
                last_seen = devices[address]['seen'][-1][1]
                if (now - last_seen).total_seconds() > 60:
                    devices[address]['seen'].append((now, now))
                else:
                    devices[address]['seen'][-1] = (devices[address]['seen'][-1][0], now)

        time.sleep(5)
except KeyboardInterrupt:
    pass

pdf = FPDF()
pdf.add_page()
pdf.set_font("Arial", size=12)

pdf.cell(200, 10, txt="Bluetooth Devices Report", ln=1, align="C")

for address, device_info in devices.items():
    pdf.cell(0, 10, txt=f"Device Name: {device_info['name']}", ln=1)
    pdf.cell(0, 10, txt=f"MAC Address: {address}", ln=1)

    for start, end in device_info['seen']:
        pdf.cell(0, 10, txt=f"Online from {start} to {end}", ln=1)

    pdf.cell(0, 10, txt="", ln=1)

pdf.output(args.output)
