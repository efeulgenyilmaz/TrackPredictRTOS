#include <opencv2/opencv.hpp>
#include <iostream>

bool isRedPixel(const cv::Vec3b& color) {
    return color[2] > 150 && color[2] > color[1] + 50 && color[2] > color[0] + 50;
}

int countRedNeighbors(const cv::Mat& image, int x, int y, int directionX, int directionY) {
    int count = 0;
    for (int i = 1; i <= 120; i++) {
        int nx = x + i * directionX;
        int ny = y + i * directionY;
        if (nx >= 0 && nx < image.cols && ny >= 0 && ny < image.rows && isRedPixel(image.at<cv::Vec3b>(ny, nx))) {
            count++;
        } else {
            break;
        }
    }
    return count;
}

int main() {
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) { 
        std::cerr << "Cannot open the camera" << std::endl;
        return -1;
    }

    cv::namedWindow("Red Filtered", cv::WINDOW_AUTOSIZE);

    while (true) {
        cv::Mat frame;
        cap >> frame;

        if (frame.empty()) {
            std::cerr << "Received empty frame" << std::endl;
            break;
        }

        cv::Mat filteredFrame = cv::Mat::zeros(frame.size(), frame.type());

        for (int i = 0; i < frame.rows; i++) {
            for (int j = 0; j < frame.cols; j++) {
                if (isRedPixel(frame.at<cv::Vec3b>(i, j))) {
                    // check for 20 red pixels in each direction
                    if (countRedNeighbors(frame, j, i, 1, 0) > 115 || // right
                        countRedNeighbors(frame, j, i, -1, 0) > 115 || // left
                        countRedNeighbors(frame, j, i, 0, 1) > 115 || // down
                        countRedNeighbors(frame, j, i, 0, -1) > 115) { // up
                        filteredFrame.at<cv::Vec3b>(i, j) = frame.at<cv::Vec3b>(i, j);
                    }
                }
            }
        }

        cv::imshow("Red Filtered", filteredFrame);

        if (cv::waitKey(30) >= 0) break;
    }

    cap.release();
    cv::destroyAllWindows(); 
    return 0;
}

