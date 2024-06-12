# Predictive Object Tracking and Automated Response System

![image](https://github.com/efeulgenyilmaz/TrackPredictRTOS/assets/172309112/b1433825-4e0c-4c0d-94e1-7bd6a9ca315d)

## Overview

This project, **Predictive Object Tracking and Automated Response System**, demonstrates a sophisticated approach to real-time object tracking and automatic response using advanced algorithms and efficient hardware communication. It aims to enhance accuracy and efficiency in automated targeting applications.

![turet_reduced](https://github.com/efeulgenyilmaz/TrackPredictRTOS/assets/172309112/7827ad6e-fb9d-4bea-a321-50b39237b1f7)

## Table of Contents

1. [Introduction](#introduction)
2. [Features](#features)
3. [System Architecture](#system-architecture)
4. [Installation](#installation)
5. [Contributing](#contributing)
6. [License](#license)
7. [Acknowledgments](#acknowledgments)

## Introduction

In modern automation, real-time object tracking and precise engagement are critical. This project integrates automated electronics, predictive algorithms, and robust software architecture to enhance defense and surveillance systems.

### Project Purpose

The goal is to design an algorithm capable of identifying, tracking, and predicting the next location of an object using efficient, low-level language algorithms.

### Project Scope

- Advanced data management with a circular data structure.
- Unique image processing algorithms.
- Real-time data communication via UART with shared memory protection.
- Concurrent task execution using threads.

### Project Outcomes

- Process image frames and predict object locations.
- Visualize object tracking data and control hardware.
- Lock and engage targets with high confidence based on prediction.

## Features

- **Real-time Object Tracking**: Capture and process image frames to determine the object's position.
- **Predictive Algorithms**: Estimate the next location based on historical data.
- **Automated Response**: Engage targets automatically when confidence levels are met.
- **Graphical User Interface (GUI)**: Real-time visualization of tracking data.
- **Efficient Hardware Communication**: Use of UART for data transfer and PWM for servo control.

## System Architecture

### Hardware

- **Camera**: Captures frames at regular intervals.
- **Microcontroller (Raspberry Pi Pico)**: Receives processed data and controls hardware.
- **Processing Unit**: Performs image processing and predictions.
- **Monitor (Optional)**: Displays real-time tracking information.

### Software

- **Frame Processing**: Detects object position and removes outliers.
- **Data Structure**: Uses a circular linked list for efficient cyclic operations.
- **Firmware**: Microcontroller firmware handles real-time data processing and hardware control.

## Installation

### Prerequisites

- **Operating System**: Linux
- **Dependencies**: CMake, SFML library, OpenCV, Raspberry Pi Pico SDK

### Steps

1. **Clone the Repository**

   ```bash
   git clone https://github.com/efeulgenyilmaz/trackerRTOS.git
   cd trackerRTOS
   
2. **Run the Program**

   ```bash
   cd Program
   gcc compile.c -o compile
   ./compile
   ./main
   ./main (--help --time)

3. **Upload Firmware to Microcontroller (Optional)**
   
   Follow the Raspberry Pi Pico SDK documentation to upload the firmware.  
   "https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf"
   
   build the pico code:
   
   cd Pico
   mkdir build
   cd build
   cmake ..
   make
   
   upload the .uf2 file into a raspberry pi pico.
   connect uart and hardware to related pins and run the program again.
   
4. **Check the 'More' Folder for Testing Image Filter (Optional)**

   Follow the "Instructions.md" to compile and test.
   
## Contributing

We welcome contributions to enhance this project. Please follow these steps:

1. Fork the repository.
2. Create a new branch (feature/your-feature).
3. Commit your changes.
4. Push the branch to your fork.
5. Open a Pull Request.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.  
   
## Acknowledgments

Advisor: Dr. Mert Yağcıoğlu  
Supervisor: Dr. Aziz Kubilay Ovacıklı  
Contributors: Efe Ülgen Yılmaz  
