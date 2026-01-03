#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

int main() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera." << std::endl;
        return -1;
    }

    cv::Mat frame;

    while (true){
        auto start = std::chrono::high_resolution_clock::now();

        cap >> frame;
        if (frame.empty()) break;

        cv::GaussianBlur(frame, frame, cv::Size(15, 15), 0);
        cv::putText(frame, "Hello CV!", cv::Point(30,40),
                    cv::FONT_HERSHEY_SIMPLEX, 1.0,
                    cv::Scalar(0,255,0), 2);

        auto end = std::chrono::high_resolution_clock::now();
        float fps = 2000.0f / std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        cv::putText(frame, "FPS: " + std::to_string((int)fps),
                    cv::Point(30,80), cv::FONT_HERSHEY_SIMPLEX,
                    1.0, cv::Scalar(255,0,0), 2);
        

        cv::imshow("Camera", frame);

        if (cv::waitKey(1) == 27) break; // Exit on ESC key

    }

    return 0;
}