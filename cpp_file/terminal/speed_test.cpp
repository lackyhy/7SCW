#include "../../h_file/terminal/speed_test.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <string>

using namespace std;

class SpeedTest {
private:
    chrono::high_resolution_clock::time_point start_time;

public:
    void start() {
        start_time = chrono::high_resolution_clock::now();
    }

    double stop() {
        auto end_time = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000000.0;
    }


    void downloadTest() {
        cout << "Testing DOWNLOAD speed...\n";
        SpeedTest test;
        test.start();

        // Simulate download by processing data
        const int dataSize = 100000000; // 100Mb
        vector<char> buffer(dataSize);

        // Fill with data
        for (int i = 0; i < dataSize; i++) {
            buffer[i] = static_cast<char>(i % 256);
        }

        // Process data (simulate download)
        volatile char temp;
        for (int i = 0; i < dataSize; i++) {
            temp = buffer[i];
        }

        double time = test.stop();
        double speed = (dataSize / time) / (1024 * 1024); // MB/s
        cout << "Download Speed: " << speed << " MB/s\n";
        cout << "Time: " << time << " seconds\n\n";
    }

    void uploadTest() {
        cout << "Testing UPLOAD speed...\n";
        SpeedTest test;
        test.start();

        // Simulate upload by generating and processing data
        const int dataSize = 50000000; // 50MB simulated
        vector<int> data(dataSize);

        // Generate upload data
        for (int i = 0; i < dataSize; i++) {
            data[i] = i * 2;
        }

        // Process upload data
        long long sum = 0;
        for (int i = 0; i < dataSize; i++) {
            sum += data[i];
        }

        double time = test.stop();
        double speed = (dataSize * sizeof(int) / time) / (1024 * 1024); // MB/s
        cout << "Upload Speed: " << speed << " MB/s\n";
        cout << "Time: " << time << " seconds\n";
        cout << "Data processed: " << sum << "\n\n";
    }

    void pingTest() {
       cout << "Testing PING...\n";

        const int attempts = 10;
        double totalTime = 0;
        double minPing = 1000;
        double maxPing = 0;

        for (int i = 0; i < attempts; i++) {
            SpeedTest test;
            test.start();

            // Simulate network request
            volatile int counter = 0;
            for (int j = 0; j < 1000; j++) {
                counter++;
            }

            double ping = test.stop() * 1000; // Convert to milliseconds
            totalTime += ping;

            if (ping < minPing) minPing = ping;
            if (ping > maxPing) maxPing = ping;

            std::cout << "Ping " << (i + 1) << ": " << ping << " ms\n";
        }

        cout << "Min Ping: " << minPing << " ms\n";
        cout << "Max Ping: " << maxPing << " ms\n";
        cout << "Avg Ping: " << (totalTime / attempts) << " ms\n\n";
    }

    void start_menu() {
        cout << "=== NETWORK SPEED TEST ===\n\n";

        pingTest();
        downloadTest();
        uploadTest();

        cout << "Speed test completed!\n";
    }
};

SpeedTest SpeedTest;

void speed_test() {
    SpeedTest.start_menu();
}