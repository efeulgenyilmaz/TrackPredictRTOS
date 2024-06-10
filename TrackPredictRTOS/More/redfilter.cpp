#include <opencv2/opencv.hpp>
#include <iostream>

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
                cv::Vec3b color = frame.at<cv::Vec3b>(i, j);
                if (color[2] > 150 && color[2] > color[1] + 50 && color[2] > color[0] + 50) {
                    filteredFrame.at<cv::Vec3b>(i, j)[2] = color[2];
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

