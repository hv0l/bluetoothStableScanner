package main

import (
	"errors"
	"flag"
	"fmt"
	"os"
	"strconv"
	"time"

	"github.com/jung-kurt/gofpdf"
	"github.com/paulrosania/go-bluetooth"
)

func main() {
	durationStr := flag.String("d", "", "Duration of the scan, e.g. 1h, 30m, 45s")
	outputFile := flag.String("o", "output.pdf", "Name of the output PDF file")
	flag.Parse()

	if *durationStr == "" {
		fmt.Println("Error: Missing duration parameter")
		flag.Usage()
		os.Exit(1)
	}

	duration, err := time.ParseDuration(*durationStr)
	if err != nil {
		fmt.Println("Error: Invalid duration format")
		flag.Usage()
		os.Exit(1)
	}

	devices, err := scanBluetoothDevices(duration)
	if err != nil {
		fmt.Printf("Error: %v\n", err)
		os.Exit(1)
	}

	err = generatePDFReport(devices, *outputFile)
	if err != nil {
		fmt.Printf("Error: %v\n", err)
		os.Exit(1)
	}

	fmt.Printf("Report generated: %s\n", *outputFile)
}

func scanBluetoothDevices(duration time.Duration) (map[string]*bluetooth.Device, error) {
	bt := bluetooth.New()
	err := bt.Start()
	if err != nil {
		return nil, err
	}
	defer bt.Stop()

	devices := make(map[string]*bluetooth.Device)

	start := time.Now()
	for time.Since(start) < duration {
		device, err := bt.Scan()
		if err != nil {
			return nil, err
		}
		if device != nil {
			devices[device.Address] = device
		}
	}
	return devices, nil
}

func generatePDFReport(devices map[string]*bluetooth.Device, outputFile string) error {
	pdf := gofpdf.New("P", "mm", "A4", "")
	pdf.AddPage()
	pdf.SetFont("Arial", "", 12)

	pdf.Cell(0, 10, "Bluetooth Devices Report")
	pdf.Ln(20)

	pdf.SetFont("Arial", "B", 12)
	pdf.Cell(60, 10, "Address")
	pdf.Cell(60, 10, "Name")
	pdf.Cell(60, 10, "RSSI")
	pdf.Ln(10)

	pdf.SetFont("Arial", "", 12)
	for _, device := range devices {
		pdf.Cell(60, 10, device.Address)
		pdf.Cell(60, 10, device.Name)
		pdf.Cell(60, 10, strconv.Itoa(device.RSSI))
		pdf.Ln(10)
	}

	err := pdf.OutputFileAndClose(outputFile)
	if err != nil {
		return errors.New("Failed to generate PDF report")
	}
	return nil
}
