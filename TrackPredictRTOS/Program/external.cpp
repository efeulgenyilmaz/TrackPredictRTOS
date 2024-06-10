#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <thread>
#include "dtst.h"

int time_mode = 0;
int camera_status = 0;

int width = 640; //resolution
int height = 480;

cv::VideoCapture cap(0);

// structure to store RGB pixel values
typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} RGBPixel;
clock_t cap_time;
clock_t prev_cap_time;

cv::Mat capture_frame(cv::VideoCapture& cap){

    clock_t cptr_start = clock();
    
    
    cv::Mat frame;
    cap >> frame;
    prev_cap_time = cap_time;
    cap_time = clock();
    
    clock_t cptr_end = clock();
    double capture_time = ((double) (cptr_end - cptr_start)) / CLOCKS_PER_SEC;
    if(time_mode){printf("capture time: %f\n", capture_time);}

    if (frame.empty()) {
        std::cerr << "Error: Failed to capture frame" << std::endl;
        return cv::Mat();
    }
    
    return frame;
}


RGBPixel* malloc_and_format(){
    clock_t malloc_start = clock();
    // convert OpenCV Mat to RGBPixel array
    
    cv::Mat frame = capture_frame(cap);
    
    int width = frame.cols;
    int height = frame.rows;
    RGBPixel* image = (RGBPixel*)malloc(width * height * sizeof(RGBPixel));
    if (!image) {
        std::cerr << "Error: Memory allocation failed" << std::endl;
        return NULL;
    }
    
    
    for (int y = 0; y < height; y++) { //threadlenebilir!
        for (int x = 0; x < width; x++) {
            cv::Vec3b pixel = frame.at<cv::Vec3b>(y, x);
            image[y * width + x] = { pixel[2], pixel[1], pixel[0] }; // openCV uses BGR ordering
        }
    }
    
    clock_t malloc_end = clock();
    double malloc_time = ((double) (malloc_end - malloc_start)) / CLOCKS_PER_SEC;
    if(time_mode){printf("malloc time: %f\n", malloc_time);}
    
    
    
    return image;
}

// function to check if a pixel is red based on RGB values
int isRedPixel(RGBPixel pixel) {
    return (pixel.red > 150 && pixel.red > pixel.green + 50 && pixel.red > pixel.blue + 50); //150 100 100
}

int countRedNeighbors(RGBPixel* image, int x, int y, int width, int height, int directionX, int directionY) {
    int count = 0;
    for (int i = 1; i <= 20; i++) {
        int nx = x + i * directionX;
        int ny = y + i * directionY;
        if (nx >= 0 && nx < width && ny >= 0 && ny < height && isRedPixel(image[ny * width + nx])) {
            count++;
        } else {
            break;
        }
    }
    return count;
}

// function to calculate centroid of red pixels in the image
node * calculateRedCentroid(/*RGBPixel* image, */int width, int height, node* head, bool* no_object) {
    clock_t calc_start = clock();
    
    RGBPixel* image = malloc_and_format();
    
    
    int totalRedPixels = 0;
    int sumX = 0, sumY = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width + x;
            if (isRedPixel(image[index])) {
                if(countRedNeighbors(image, x, y, width, height, 1, 0) == 20 ||  // right
                   countRedNeighbors(image, x, y, width, height, -1, 0) == 20 || // left
                   countRedNeighbors(image, x, y, width, height, 0, 1) == 20 ||  // down
                   countRedNeighbors(image, x, y, width, height, 0, -1) == 20){ // up
                        sumX += x;
                        sumY += y;
                        totalRedPixels++;
                }
            }
        }
    }
    
    int centroidX = 0;
    
    int centroidY = 0;
    if (totalRedPixels > 0) {
        centroidX = sumX / totalRedPixels;
        centroidY = sumY / totalRedPixels;
    }
    
    free(image);
    
    if(centroidX){
        double time_dif = ((double) (cap_time - prev_cap_time)) / CLOCKS_PER_SEC;
        
        head = add(head, centroidX, centroidY, time_dif);
        printf("\nX: %d, Y: %d ADDED\n", centroidX, centroidY);
        *no_object = false;
        show(head);
    }
    else{
        *no_object = true;
        printf("\nNo Object Identified\n");
        cap_time = prev_cap_time;
        prev_cap_time = cap_time - head->prev->time;
    }
    
    
    clock_t calc_end = clock();
    
    double calc_center_time = ((double) (calc_end - calc_start)) / CLOCKS_PER_SEC;
    if(time_mode){printf("center calculating function time: %f\n", calc_center_time);}
    
    return head;
}

int check_args(int argc, char *argv[]) {

    
    if (argc > 1) {
        if (strcmp(argv[1], "-t") == 0 || strcmp(argv[1], "--time") == 0) {
            time_mode = 1;
            return 2;
        } 
        else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            FILE *file;
            file = fopen("help.txt", "r");
            if (file == NULL) {
		printf("Error opening the file.\n");
		return 1;
	    }
	    char buffer[100];
	    while (fgets(buffer, sizeof(buffer), file) != NULL) {
		printf("%s", buffer);
	    }
	    fclose(file);
            return 1;
        }
        else {
            printf("Invalid argument: %s\n", argv[1]);
            printf("Usage: [-t | --time] [-h | --help]\n");
	    return 1;
        }
    }
    return 0;
}


void keep_the_camera_open(){

    clock_t cam_open_start = clock();
    
    
    if (!cap.isOpened()) {
        std::cerr << "Error: Failed to open camera" << std::endl;
        camera_status = 0;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        cv::VideoCapture cap(0);
        keep_the_camera_open();
    }
    
    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    
    camera_status = 1;
    clock_t cam_open_end = clock();
    double cam_open_time = ((double) (cam_open_end - cam_open_start)) / CLOCKS_PER_SEC;
    if(time_mode){printf("camera open time: %f\n", cam_open_time);}
}






void test(){
    printf("\nheader works!\n");
}

/*int main(int argc, char *argv[]) {
    clock_t program_start = clock();
    
    int args_result = check_args(argc, argv);
    
    keep_the_camera_open(); //hep acik
    
    RGBPixel* image = malloc_and_format(); //0.1 sn de bir
    
    
    int centroidX, centroidY;
    calculateRedCentroid(image, width, height, &centroidX, &centroidY);
    

    //display avg x and y
    printf("%d %d", centroidX, centroidY);
    
    
    free(image);

    clock_t program_end = clock();
    
    double overall_time = ((double) (program_end - program_start)) / CLOCKS_PER_SEC;
    if(time_mode){printf("\nOverall time: %f\n", overall_time);}
    
    return 0;
}*/

