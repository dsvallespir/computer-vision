#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main() {
    // Open camera
    VideoCapture cap(0);
    
    if (!cap.isOpened()) {
        cerr << "Error: Cannot open camera" << endl;
        return -1;
    }
    
    // Set camera properties
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);
    
    Mat frame, hsv, mask;
    
    // Color range for red pointer (adjust as needed)
    Scalar lowerRed(0, 120, 70);
    Scalar upperRed(10, 255, 255);
    
    cout << "Pointer Detection Started. Press 'q' to quit." << endl;
    
    while (true) {
        cap >> frame;
        
        if (frame.empty()) {
            cerr << "Error: Empty frame" << endl;
            break;
        }
        
        // Flip for mirror view
        flip(frame, frame, 1);
        
        // Convert to HSV
        cvtColor(frame, hsv, COLOR_BGR2HSV);
        
        // Create mask
        inRange(hsv, lowerRed, upperRed, mask);
        
        // Find contours
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        
        // Draw detected pointers
        for (const auto& contour : contours) {
            double area = contourArea(contour);
            if (area > 50) {
                Moments m = moments(contour);
                if (m.m00 != 0) {
                    int cx = static_cast<int>(m.m10 / m.m00);
                    int cy = static_cast<int>(m.m01 / m.m00);
                    
                    // Draw pointer
                    circle(frame, Point(cx, cy), 10, Scalar(0, 255, 0), FILLED);
                    putText(frame, 
                           to_string(cx) + "," + to_string(cy),
                           Point(cx + 15, cy - 15),
                           FONT_HERSHEY_SIMPLEX, 0.6, 
                           Scalar(255, 255, 255), 2);
                }
            }
        }
        
        // Show results
        imshow("Pointer Detection", frame);
        imshow("Mask", mask);
        
        // Exit on 'q'
        if (waitKey(1) == 'q') {
            break;
        }
    }
    
    cap.release();
    destroyAllWindows();
    
    return 0;
}