# Bluetooth Stable Scanner
==================

This Python script scans for nearby Bluetooth devices for a specified duration and generates a PDF report with the details of the detected devices.

Requirements:
-------------
- Python 3
- PyBluez
- ReportLab

To install the required dependencies, run the following command:

pip3 install PyBluez reportlab

Usage:
------
Run the script using the following command:

python3 bt_scanner.py DURATION -o OUTPUT

Where:
- DURATION is the duration of the scan, specified in the format Nh, Nm, or Ns (N is an integer). For example: 1h (1 hour), 30m (30 minutes), 45s (45 seconds).
- OUTPUT is the name of the output PDF file that will contain the report on the detected devices. If not specified, the output file will be saved as "output.pdf".

Examples:
---------
Run a scan for 1 hour and save the report to the file "scan_report.pdf":

python3 bt_scanner.py 1h -o scan_report.pdf

Run a scan for 15 minutes and save the report to the file "output.pdf":

python3 bt_scanner.py 15m

Run a scan for 30 seconds and save the report to the file "short_scan.pdf":

python3 bt_scanner.py 30s -o short_scan.pdf
