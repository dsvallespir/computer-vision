#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <cmath>

using namespace cv;
using namespace std;
using namespace std::chrono;

class PointerDetector {
private:
    VideoCapture cap;
    int cameraIndex;
    bool useDisplay;
    
public:
    PointerDetector(int index = 0, bool display = true) 
        : cameraIndex(index), useDisplay(display) {
        initCamera();
    }
    
    bool initCamera() {
        // Try different backends
        //cap = VideoCapture(cameraIndex, CAP_V4L2);  // CAP_ANY for auto
        cap = VideoCapture(0);  // CAP_ANY for auto
        
        if (!cap.isOpened()) {
            cerr << "Error: Could not open camera " << cameraIndex << endl;
            return false;
        }
        
        // Set camera properties
        //cap.set(CAP_PROP_FRAME_WIDTH, 640);
        //cap.set(CAP_PROP_FRAME_HEIGHT, 480);
        //cap.set(CAP_PROP_FPS, 30);
        //cap.set(CAP_PROP_BUFFERSIZE, 1);
        //cap.set(CAP_PROP_AUTOFOCUS, 0);
        //cap.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M','J','P','G'));
        
        cout << "Camera initialized: " 
             << cap.get(CAP_PROP_FRAME_WIDTH) << "x" 
             << cap.get(CAP_PROP_FRAME_HEIGHT) << " @ " 
             << cap.get(CAP_PROP_FPS) << " fps" << endl;
        
        return true;
    }
    
    pair<bool, Mat> getFrame(int timeoutMs = 2000) {
        auto start = high_resolution_clock::now();
        Mat frame;
        
        while (duration_cast<milliseconds>(high_resolution_clock::now() - start).count() < timeoutMs) {
            if (cap.read(frame) && !frame.empty()) {
                return make_pair(true, frame);
            }
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        
        return make_pair(false, Mat());
    }
    
    pair<Point, double> detectPointerSimple(const Mat& frame) {
        Mat hsv, mask, mask1, mask2;
        
        // Convert to HSV
        cvtColor(frame, hsv, COLOR_BGR2HSV);
        
        // Define color ranges for pointer (adjust as needed)
        // For red pointer/laser
        Scalar lowerRed1(0, 120, 70);
        Scalar upperRed1(10, 255, 255);
        Scalar lowerRed2(170, 120, 70);
        Scalar upperRed2(180, 255, 255);
        
        // Create masks
        inRange(hsv, lowerRed1, upperRed1, mask1);
        inRange(hsv, lowerRed2, upperRed2, mask2);
        bitwise_or(mask1, mask2, mask);
        
        // Apply morphological operations
        Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
        morphologyEx(mask, mask, MORPH_OPEN, kernel);
        morphologyEx(mask, mask, MORPH_CLOSE, kernel);
        
        // Find contours
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(mask, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        
        if (!contours.empty()) {
            // Find largest contour
            int largestIdx = 0;
            double maxArea = 0;
            
            for (size_t i = 0; i < contours.size(); i++) {
                double area = contourArea(contours[i]);
                if (area > maxArea) {
                    maxArea = area;
                    largestIdx = i;
                }
            }
            
            if (maxArea > 50) {  // Minimum area threshold
                Moments m = moments(contours[largestIdx]);
                if (m.m00 != 0) {
                    int cx = static_cast<int>(m.m10 / m.m00);
                    int cy = static_cast<int>(m.m01 / m.m00);
                    return make_pair(Point(cx, cy), maxArea);
                }
            }
        }
        
        return make_pair(Point(-1, -1), 0);
    }
    
    void processFrame(Mat& frame, const Point& pointerPos, double area) {
        // Flip horizontally for mirror view
        flip(frame, frame, 1);
        
        // Draw pointer if detected
        if (pointerPos.x >= 0 && pointerPos.y >= 0) {
            // Adjust pointer position for flipped frame
            Point adjustedPos(frame.cols - pointerPos.x, pointerPos.y);
            
            // Draw circle at pointer
            circle(frame, adjustedPos, 10, Scalar(0, 255, 0), FILLED);
            
            // Draw crosshair
            line(frame, Point(adjustedPos.x - 15, adjustedPos.y), 
                 Point(adjustedPos.x + 15, adjustedPos.y), Scalar(0, 255, 255), 2);
            line(frame, Point(adjustedPos.x, adjustedPos.y - 15), 
                 Point(adjustedPos.x, adjustedPos.y + 15), Scalar(0, 255, 255), 2);
            
            // Display coordinates
            string coordText = "Pointer: (" + to_string(adjustedPos.x) + 
                              ", " + to_string(adjustedPos.y) + ")";
            putText(frame, coordText, Point(adjustedPos.x + 20, adjustedPos.y - 20),
                    FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
            
            // Display area
            string areaText = "Area: " + to_string(static_cast<int>(area));
            putText(frame, areaText, Point(adjustedPos.x + 20, adjustedPos.y + 20),
                    FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
        } else {
            // No pointer detected
            putText(frame, "NO POINTER DETECTED", Point(50, 50),
                    FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
        }
        
        // Draw coordinate system
        drawCoordinateSystem(frame);
    }
    
    void drawCoordinateSystem(Mat& frame) {
        int w = frame.cols;
        int h = frame.rows;
        
        // Center lines
        line(frame, Point(w/2, 0), Point(w/2, h), Scalar(100, 100, 100), 1);
        line(frame, Point(0, h/2), Point(w, h/2), Scalar(100, 100, 100), 1);
        
        // Center mark
        circle(frame, Point(w/2, h/2), 5, Scalar(200, 200, 200), 1);
        
        // Display resolution
        string resText = "Resolution: " + to_string(w) + "x" + to_string(h);
        putText(frame, resText, Point(10, 30), FONT_HERSHEY_SIMPLEX, 
                0.5, Scalar(200, 200, 200), 1);
    }
    
    void run() {
        if (!cap.isOpened()) {
            cerr << "Camera not opened!" << endl;
            return;
        }
        
        cout << "Starting pointer detection. Press:" << endl;
        cout << "  'q' to quit" << endl;
        cout << "  's' to save current frame" << endl;
        cout << "  'c' to calibrate color" << endl;
        
        Mat frame;
        int frameCount = 0;
        auto lastSuccessTime = high_resolution_clock::now();
        
        while (true) {
            auto result = getFrame();
            
            if (!result.first || result.second.empty()) {
                cerr << "Frame timeout or empty. Reinitializing camera..." << endl;
                this_thread::sleep_for(chrono::milliseconds(500));
                initCamera();
                continue;
            }
            
            frame = result.second;
            frameCount++;
            
            // Detect pointer
            auto detection = detectPointerSimple(frame);
            Point pointerPos = detection.first;
            double area = detection.second;
            
            // Process and display frame
            processFrame(frame, pointerPos, area);
            
            // Display frame if enabled
            if (useDisplay) {
                imshow("Pointer Detection", frame);
                
                // Handle keyboard input
                int key = waitKey(1);
                if (key == 'q' || key == 27) {  // 'q' or ESC
                    break;
                } else if (key == 's') {
                    string filename = "screenshot_" + to_string(time(nullptr)) + ".jpg";
                    imwrite(filename, frame);
                    cout << "Saved: " << filename << endl;
                } else if (key == 'c') {
                    calibrateColor(frame);
                }
            } else {
                // Headless mode - save occasional frames
                if (frameCount % 30 == 0) {
                    string filename = "frame_" + to_string(frameCount) + ".jpg";
                    imwrite(filename, frame);
                    cout << "Saved: " << filename << endl;
                }
                
                // Print pointer position
                if (pointerPos.x >= 0 && pointerPos.y >= 0) {
                    cout << "Frame " << frameCount << ": Pointer at (" 
                         << frame.cols - pointerPos.x << ", " 
                         << pointerPos.y << "), Area: " << area << endl;
                }
                
                // Check for console input (non-blocking)
                if (kbhit()) {
                    char ch = getchar();
                    if (ch == 'q' || ch == 27) {
                        break;
                    }
                }
            }
            
            // Small delay to prevent CPU overload
            this_thread::sleep_for(chrono::milliseconds(10));
        }
    }
    
    void calibrateColor(const Mat& frame) {
        cout << "\n=== Color Calibration ===" << endl;
        cout << "Click on your pointer color in the window." << endl;
        cout << "Press any key in the window when done." << endl;
        
        Mat display = frame.clone();
        flip(display, display, 1);
        imshow("Calibration - Click on pointer color", display);
        
        setMouseCallback("Calibration - Click on pointer color", 
                        [](int event, int x, int y, int flags, void* userdata) {
            if (event == EVENT_LBUTTONDOWN) {
                Mat* framePtr = static_cast<Mat*>(userdata);
                Mat hsv;
                cvtColor(*framePtr, hsv, COLOR_BGR2HSV);
                Vec3b hsvVal = hsv.at<Vec3b>(y, x);
                
                cout << "Clicked at (" << x << ", " << y << ")" << endl;
                cout << "HSV: " << static_cast<int>(hsvVal[0]) << ", " 
                     << static_cast<int>(hsvVal[1]) << ", " 
                     << static_cast<int>(hsvVal[2]) << endl;
                
                // Suggested ranges
                int h = hsvVal[0];
                cout << "\nSuggested HSV ranges:" << endl;
                cout << "Lower: [" << max(0, h-10) << ", 100, 100]" << endl;
                cout << "Upper: [" << min(180, h+10) << ", 255, 255]" << endl;
            }
        }, &display);
        
        waitKey(0);
        destroyWindow("Calibration - Click on pointer color");
    }
    
    // Cross-platform kbhit equivalent
    bool kbhit() {
        #ifdef _WIN32
            return _kbhit();
        #else
            struct timeval tv = {0L, 0L};
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(0, &fds);
            return select(1, &fds, NULL, NULL, &tv) > 0;
        #endif
    }
    
    ~PointerDetector() {
        if (cap.isOpened()) {
            cap.release();
        }
        destroyAllWindows();
        cout << "Pointer detector shutdown." << endl;
    }
};

int main(int argc, char** argv) {
    cout << "OpenCV Pointer Detection System" << endl;
    cout << "================================" << endl;
    
    // Parse command line arguments
    int cameraIndex = 0;
    bool useDisplay = true;
    
    if (argc > 1) {
        cameraIndex = atoi(argv[1]);
    }
    
    if (argc > 2) {
        string arg = argv[2];
        if (arg == "--headless" || arg == "-h") {
            useDisplay = false;
        }
    }
    
    try {
        PointerDetector detector(cameraIndex, useDisplay);
        detector.run();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}