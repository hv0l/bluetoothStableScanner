from setuptools import setup, find_packages

with open("requirements.txt") as f:
    required = f.read().splitlines()

setup(
    name="btscanner",
    version="1.0.0",
    description="Bluetooth Scanner: Scan for nearby Bluetooth devices and generate a PDF report",
    packages=find_packages(),
    install_requires=required,
    entry_points={
        "console_scripts": [
            "btscanner = bt_scanner:main",
        ],
    },
)
