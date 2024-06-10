#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "dtst.h"
#include "camwork.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <signal.h>
#include <mutex>
#include <math.h>
#include <algorithm> // transforming lowercase
#include <fcntl.h> //
#include <termios.h> //
#include <cstdint> //for uint16_t

#define dataset_size 5

namespace resolution {
    const int x = 640; // 160 120
    const int y = 480;
}

node* head = nullptr;
int data_count = 0;
double confidence = 300;
bool quit = false;
bool t2_inwait = false;
int confidence_threshold = 80;
const char* uart = nullptr;
std::mutex mutex;

int uart0_filestream;

void set_pixel_color(sf::RenderWindow& window, int x, int y, int divide) {
    int color;
    if (divide >= 0 && divide <= 255) {
        color = 255 - (divide * 255 / dataset_size);
    } else if (divide == -1) {
        color = 0xFF0000; // red color
    } else {
        std::cout << "divide value is invalid!" << std::endl;
        return;
    }

    sf::Vertex pixel(sf::Vector2f(x, y), sf::Color(color >> 16 & 0xFF, color >> 8 & 0xFF, color & 0xFF));
    window.draw(&pixel, 1, sf::Points);
}

void SFML_out(sf::RenderWindow& window, sf::Font& font) {
    sf::Event event;
    while (!quit) {
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                quit = true;
            }
        }

        mutex.lock();
        node* temp = head->prev;
        window.clear(sf::Color::Black);

        if (data_count >= 5) {
            for (int i = 0; i < dataset_size && i < data_count; i++) {
                set_pixel_color(window, resolution::x - temp->x, temp->y, i);
                for (int k = temp->y - 5; k < temp->y + 5; k++) {
                    for (int j = temp->x - 5; j < temp->x + 5; j++) {
                        set_pixel_color(window, resolution::x - j, k, i);
                    }
                }
                temp = temp->prev;
                if (temp == head)
                    break;
            }
        }

        if (data_count > 2) {
            int dx = 2 * head->prev->x - head->prev->prev->x; // last + (last-first) = 2 * last - first
            int dy = 2 * head->prev->y - head->prev->prev->y;
            set_pixel_color(window, resolution::x - dx, dy, -1); // -1 red pixel flag
            for (int i = dy - 5; i < dy + 5; i++) {
                for (int j = dx - 5; j < dx + 5; j++) {
                    set_pixel_color(window, resolution::x - j, i, -1);
                }
            }
        }
        
        double percent = 100 - (confidence / 3);
        if(percent < 0){percent = 0;}
        std::string rawText = "%" + std::to_string(percent);
        sf::Text text(rawText, font);
        text.setCharacterSize(24);
        if(percent >= confidence_threshold){text.setFillColor(sf::Color::Green);}
        else if(percent < 20){text.setFillColor(sf::Color::Red);}
        else{text.setFillColor(sf::Color::White);}
        text.setPosition(10, 10); // position of the text in the window
        window.draw(text);

        window.display();
        
        mutex.unlock();
    }
}

void calculateConfidence(std::mutex& dtst_mtx){
    while (!quit) {
        dtst_mtx.lock();
        if (data_count <= dataset_size) {
            dtst_mtx.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        node* temp = head->prev;
        int dif_array[dataset_size];
        for (int iter = 0; iter < dataset_size; iter++) {
            int dx, dy;
            double distance, velocity;
            dx = abs(temp->x - temp->prev->x);
            dy = abs(temp->y - temp->prev->y);
            distance = sqrt(pow(dx, 2) + pow(dy, 2));
            velocity = distance / temp->time;
            dif_array[iter] = velocity;
        }

        double sum = 0;
        for (int i = 0; i < dataset_size; ++i) {
            sum += dif_array[i];
        }
        double average = sum / dataset_size;

        confidence = average;

        dtst_mtx.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void ctrlc_handler(int signum) {
    std::cout << "\nCtrl+C detected. Exiting gracefully.\n";
    quit = true; // quit flag
    //exit(EXIT_SUCCESS);
}
int map_range(int value, int source_min, int source_max, int target_min, int target_max);
void shoot() {
    quit = true;
    usleep(100000); //10ms
    int dx = 2 * head->prev->x - head->prev->prev->x; // last + (last-first) = 2 * last - first
    int dy = 2 * head->prev->y - head->prev->prev->y;
    dx = resolution::x - dx;
    
    int angle1 = map_range(dx, 0, resolution::x, 25, 375);
    int angle2 = map_range(dy, 0, resolution::y, 50, 300);
    
    int conf = -1;

        uint16_t tx_buffer[3];
        tx_buffer[0] = (uint16_t)angle1;
        tx_buffer[1] = (uint16_t)angle2;
        tx_buffer[2] = (uint16_t)conf;
        

        uint8_t byte_buffer[6];
        byte_buffer[0] = (tx_buffer[0] >> 8) & 0xFF;
        byte_buffer[1] = tx_buffer[0] & 0xFF;
        byte_buffer[2] = (tx_buffer[1] >> 8) & 0xFF;
        byte_buffer[3] = tx_buffer[1] & 0xFF;
        byte_buffer[4] = (tx_buffer[2] >> 8) & 0xFF;
        byte_buffer[5] = tx_buffer[2] & 0xFF;

        if (uart0_filestream != -1) {
            ssize_t count = write(uart0_filestream, byte_buffer, sizeof(byte_buffer));
            if (count < 0) {
                printf("UART TX error while shooting\n");
            } else {
                printf("SHOOT AT: %d, %d, %hu\n", angle1, angle2, conf);
            }
        }
    
}
void check_confidence() {
    while (!quit) {
        int count = 0;
        while (count < 50) {
            if ((100 - (confidence / 3)) > confidence_threshold) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                count++;
                printf("\ncount: %d\n", count);
                //printf("\nconfidence: %f\n", confidence);
            } else {
                count = 0;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                break;
            }
        }
        if (count == 50) {
            quit = true;
            std::cout << "Condition met 50 times in a row. Shooting...\n";
            shoot();
        }
    }
}


int setup_uart(const char* device, int baud_rate) {
    uart0_filestream = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart0_filestream == -1) {
        printf("Error - Unable to open UART. Ensure it is not in use by another application\n");
        return -1;
    }

    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = baud_rate | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    return 0;
}

int map_range(int value, int source_min, int source_max, int target_min, int target_max) {
    int result = ((value - source_min) * (target_max - target_min)) / (source_max - source_min) + target_min;
    return target_max - result; //only result for reverse
}

void send_uart_data(std::mutex& dtst_mtx) {
    while (!quit) {
        dtst_mtx.lock();
        node* temp = head->prev;
        int angle1 = map_range(temp->x, 0, resolution::x, 25, 375);
        int angle2 = map_range(temp->y, 0, resolution::y, 50, 300);
        dtst_mtx.unlock();

        int conf = confidence;
        
        if(data_count == 2){ //to send -1 at the beginning
            conf = -1;
        }
        
        if (conf > 250) {
            conf = 250;
        }

        uint16_t tx_buffer[3];
        tx_buffer[0] = (uint16_t)angle1;
        tx_buffer[1] = (uint16_t)angle2;
        tx_buffer[2] = (uint16_t)conf;

        uint8_t byte_buffer[6];
        byte_buffer[0] = (tx_buffer[0] >> 8) & 0xFF;
        byte_buffer[1] = tx_buffer[0] & 0xFF;
        byte_buffer[2] = (tx_buffer[1] >> 8) & 0xFF;
        byte_buffer[3] = tx_buffer[1] & 0xFF;
        byte_buffer[4] = (tx_buffer[2] >> 8) & 0xFF;
        byte_buffer[5] = tx_buffer[2] & 0xFF;

        if (uart0_filestream != -1) {
            ssize_t count = write(uart0_filestream, byte_buffer, sizeof(byte_buffer));
            if (count < 0) {
                printf("UART TX error\n");
            } else {
                printf("Sent UART data: %d, %d, %d\n", angle1, angle2, conf);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    //one more time for -1
    
    dtst_mtx.lock();
    node* temp = head->prev;
    int angle1 = map_range(temp->x, 0, resolution::x, 25, 375);
    int angle2 = map_range(temp->y, 0, resolution::y, 50, 300);
    dtst_mtx.unlock();
    int conf = -1;
        
    uint16_t tx_buffer[3];
    tx_buffer[0] = (uint16_t)angle1;
    tx_buffer[1] = (uint16_t)angle2;
    tx_buffer[2] = (uint16_t)conf;

    uint8_t byte_buffer[6];
    byte_buffer[0] = (tx_buffer[0] >> 8) & 0xFF;
    byte_buffer[1] = tx_buffer[0] & 0xFF;
    byte_buffer[2] = (tx_buffer[1] >> 8) & 0xFF;
    byte_buffer[3] = tx_buffer[1] & 0xFF;
    byte_buffer[4] = (tx_buffer[2] >> 8) & 0xFF;
    byte_buffer[5] = tx_buffer[2] & 0xFF;

    if (uart0_filestream != -1) {
        ssize_t count = write(uart0_filestream, byte_buffer, sizeof(byte_buffer));
        if (count < 0) {
            printf("UART TX error\n");
        } else {
            printf("Sent UART data: %d, %d, %d\n", angle1, angle2, conf);
        }
    }
}

bool getYesNoInput(const std::string& prompt) {
    std::string response;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, response);

        std::transform(response.begin(), response.end(), response.begin(), ::tolower);

        if (response == "yes" || response == "y") {
            return true;
        } else if (response == "no" || response == "n") {
            return false;
        }

        std::cout << "\nInvalid input. Please enter 'yes' or 'no'.\n";
    }
}


int main(int argc, char* argv[]) {

    std::ifstream settingsFile("settings.txt");
    if (!settingsFile) {
        std::cerr << "Unable to open settings file. Using default settings.";
        uart = "/dev/ttyUSB0";
    }else{
        std::string line;
    
        if (std::getline(settingsFile, line)) {
            uart = new char[line.size() + 1];
            std::strcpy(const_cast<char*>(uart), line.c_str());
        }
        if (std::getline(settingsFile, line)) {
            confidence_threshold = std::stoi(line);
        }
    }
    settingsFile.close();

    bool no_object = true;

    std::mutex dtst_mtx;
    std::mutex renderer_mtx;

    test();

    sf::RenderWindow window(sf::VideoMode(resolution::x, resolution::y), "SFML Window");

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        // failed to load font
        std::cout << "Failed to load font!" << std::endl;
        return 0;
    }

    int args_result = check_args(argc, argv);
    if (args_result == 1) {
        return 0;
    } else if (args_result == 2) {
        time_mode = 1;
    } else if (args_result == 0) {
    } else {
        std::cout << "argument error!";
        return 0;
    }

    dtst_mtx.lock();
    head = add(head, 0, 0, 0);
    data_count++;
    show(head);
    dtst_mtx.unlock();

    signal(SIGINT, ctrlc_handler);

    keep_the_camera_open();
    
    if (setup_uart(uart, B115200) != 0) {
        if (getYesNoInput("\nNo hardware was found. Do you still want to run the program? [y,n]\n")) {
            std::cout << "\nContinuing...\n";
        } else {
            std::cout << "\nExiting...\n";
            return 1;
        }
        
    }
    
    
    std::thread thread2([&dtst_mtx, &no_object]() {
        while (!quit) {
            t2_inwait = false;
            dtst_mtx.lock();
            head = calculateRedCentroid(resolution::x, resolution::y, head, &no_object);
            if(!no_object){
                data_count++;
                }
            printf("\ndataCount: %d", data_count);
            dtst_mtx.unlock();
            //printf("confidence: %f", confidence);
            t2_inwait = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    std::thread thread3(calculateConfidence, std::ref(dtst_mtx));
    std::thread thread4(check_confidence);
    std::thread thread5(send_uart_data, std::ref(dtst_mtx));
    
    while (!quit) {
        if(!no_object) {
            if(t2_inwait) { // mutex yerine cozum flag
                SFML_out(window, font);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
    
    thread2.join();
    thread3.join();
    thread4.join();
    thread5.join();
    
    close(uart0_filestream);
    free_dtst(head);
    
    return 0;
}

