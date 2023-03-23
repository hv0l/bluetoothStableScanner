#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <PDFWriter/PDFWriter.h>
#include <PDFWriter/PDFPage.h>
#include <PDFWriter/PageContentContext.h>
#include <PDFWriter/PDFTextString.h>

struct Device {
    std::string name;
    std::vector<std::pair<std::chrono::system_clock::time_point, std::chrono::system_clock::time_point>> seen;
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <duration_in_seconds> <output_pdf>" << std::endl;
        return 1;
    }

    int duration = std::stoi(argv[1]);
    std::string output_file = argv[2];

    auto start_time = std::chrono::system_clock::now();
    std::map<std::string, Device> devices;

    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start_time).count() < duration) {
        inquiry_info *ii = nullptr;
        int max_rsp = 255;
        int num_rsp;
        int dev_id, sock, len, flags;
        char addr[19] = {0};
        char name[248] = {0};

        dev_id = hci_get_route(nullptr);
        sock = hci_open_dev(dev_id);
        if (dev_id < 0 || sock < 0) {
            std::cerr << "Error opening socket" << std::endl;
            return 1;
        }

        len = 8;
        flags = IREQ_CACHE_FLUSH;
        ii = (inquiry_info *)malloc(max_rsp * sizeof(inquiry_info));
        num_rsp = hci_inquiry(dev_id, len, max_rsp, nullptr, &ii, flags);
        if (num_rsp < 0) {
            std::cerr << "Error during HCI inquiry" << std::endl;
            return 1;
        }

        for (int i = 0; i < num_rsp; i++) {
            ba2str(&(ii + i)->bdaddr, addr);
            memset(name, 0, sizeof(name));
            if (hci_read_remote_name(sock, &(ii + i)->bdaddr, sizeof(name), name, 0) < 0) {
                strcpy(name, "[unknown]");
            }
            std::string address(addr);

            auto now = std::chrono::system_clock::now();
            std::cout << "Found device: " << name << " (" << address << ") at " << std::chrono::system_clock::to_time_t(now) << std::endl;

            if (devices.find(address) == devices.end()) {
                devices[address] = {name, {std::make_pair(now, now)}};
            } else {
                auto &last_seen = devices[address].seen.back().second;
                if (std::chrono::duration_cast<std::chrono::seconds>(now - last_seen).count() > 60)
                devices[address].seen.push_back(std::make_pair(now, now));
            } else {
                last_seen = now;
            }
        }
    }

    free(ii);
    close(sock);
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// Generate PDF report
PDFWriter pdfWriter;
pdfWriter.StartPDF(output_file, ePDFVersion14);

PDFPage page;
page.SetMediaBox(PDFRectangle(0, 0, 595, 842));
auto contentContext = pdfWriter.StartPageContentContext(page);

contentContext->BT();
contentContext->k(0, 0, 0, 1);
contentContext->Tf(pdfWriter.GetFontForFile("Helvetica"), 12);
contentContext->Tm(1, 0, 0, 1, 50, 792);
contentContext->Tj("Devices found:");
contentContext->ET();

int y_offset = 30;
for (const auto &entry : devices) {
    contentContext->BT();
    contentContext->k(0, 0, 0, 1);
    contentContext->Tf(pdfWriter.GetFontForFile("Helvetica"), 12);
    contentContext->Tm(1, 0, 0, 1, 50, 792 - y_offset);
    contentContext->Tj(entry.second.name + " (" + entry.first + ")");
    contentContext->ET();

    y_offset += 20;

    for (const auto &times : entry.second.seen) {
        std::time_t start = std::chrono::system_clock::to_time_t(times.first);
        std::time_t end = std::chrono::system_clock::to_time_t(times.second);
        std::stringstream time_range;
        time_range << std::put_time(std::localtime(&start), "%Y-%m-%d %H:%M:%S") << " - " << std::put_time(std::localtime(&end), "%Y-%m-%d %H:%M:%S");

        contentContext->BT();
        contentContext->k(0, 0, 0, 1);
        contentContext->Tf(pdfWriter.GetFontForFile("Helvetica"), 12);
        contentContext->Tm(1, 0, 0, 1, 80, 792 - y_offset);
        contentContext->Tj(time_range.str());
        contentContext->ET();

        y_offset += 20;
    }
}

pdfWriter.EndPageContentContext(contentContext);
pdfWriter.WritePageAndRelease(page);
pdfWriter.EndPDF();

return 0;
