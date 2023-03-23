import sys
import time
import argparse
from datetime import timedelta
from collections import defaultdict

import bluetooth
from reportlab.lib.pagesizes import letter
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer
from reportlab.lib.styles import getSampleStyleSheet

def scan(duration):
    end_time = time.time() + duration
    devices = defaultdict(lambda: {'count': 0, 'rssi': []})

    while time.time() < end_time:
        nearby_devices = bluetooth.discover_devices(duration=8, lookup_names=True, lookup_class=True, device_id=-1, flush_cache=True)

        for addr, name, _ in nearby_devices:
            devices[addr]['name'] = name
            devices[addr]['count'] += 1

        time.sleep(1)

    return devices

def generate_pdf_report(devices, output_file):
    doc = SimpleDocTemplate(output_file, pagesize=letter)
    styles = getSampleStyleSheet()
    story = []

    for addr, device_info in devices.items():
        story.append(Paragraph(f"Device Address: {addr}", styles["Heading2"]))
        story.append(Paragraph(f"Device Name: {device_info['name']}", styles["BodyText"]))
        story.append(Paragraph(f"Times Detected: {device_info['count']}", styles["BodyText"]))
        story.append(Spacer(1, 12))

    doc.build(story)

def parse_duration(duration_str):
    if duration_str[-1] == "h":
        return int(duration_str[:-1]) * 3600
    elif duration_str[-1] == "m":
        return int(duration_str[:-1]) * 60
    elif duration_str[-1] == "s":
        return int(duration_str[:-1])
    else:
        raise ValueError("Invalid duration format.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("duration", help="Duration for scanning. Format: Nh, Nm or Ns (N is an integer)")
    parser.add_argument("-o", "--output", help="Output PDF file name", default="output.pdf")
    args = parser.parse_args()

    try:
        duration = parse_duration(args.duration)
    except ValueError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

    devices = scan(duration)
    generate_pdf_report(devices, args.output)
