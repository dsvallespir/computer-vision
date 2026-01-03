#include <opencv2/opencv.hpp>
#include <vector>

cv::Mat convolve3x3(const cv::Mat& src, const cv::Mat& kernel) {
    CV_Assert(src.channels() == 1); // Ensure single channel image
    CV_Assert(kernel.rows == 3 && kernel.cols == 3); // Ensure 3x3 kernel

    cv::Mat dst = src.clone();

    for (int y = 1; y < src.rows - 1; y++) {
        for (int x = 1; x < src.cols - 1; x++) {
            float acc = 0.0f;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    float pixel = src.at<uchar>(y + ky, x + kx);
                    float kval = kernel.at<float>(ky + 1, kx + 1);

                    acc += pixel * kval;
                }
            }

            acc = std::clamp(acc, 0.0f, 255.0f);
            dst.at<uchar>(y, x) = static_cast<uchar>(acc);
        }
    }

    return dst;
}

cv::Mat kernelSobelX() {
    return (cv::Mat_<float>(3,3) <<
        -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1
    );
}

int main() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera." << std::endl;
        return -1;
    }

    cv::Mat frame, gray;
    cv::Mat sobel_manual, sobel_cv;

    while(true){
        cap >> frame;
        if (frame.empty()) break;

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        sobel_manual = convolve3x3(gray, kernelSobelX());

        cv::Sobel(gray, sobel_cv, CV_32F, 1, 0, 3);
        cv::convertScaleAbs(sobel_cv, sobel_cv);

        cv::imshow("Original (gray)", gray);
        cv::imshow("Sobel X Manual", sobel_manual);
        cv::imshow("Sobel X OpenCV", sobel_cv);

        if (cv::waitKey(1) == 27) break; // Exit on ESC key
    }

    return 0;
}