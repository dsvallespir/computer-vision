#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using namespace cv;
using namespace std;

class DroidCam {
private:
    VideoCapture cap;
    string ip;
    int port;
    
public:
    DroidCam(const string& ipAddr = "192.168.1.35", int camPort = 4747) 
        : ip(ipAddr), port(camPort) {}
    
    bool connect() {
        // Format: "protocol://ip:port/video"
        string url = "http://" + ip + ":" + to_string(port) + "/video";
        
        cout << "Connecting to: " << url << endl;
        
        // Try different protocols
        vector<string> urlsToTry = {
            "http://" + ip + ":" + to_string(port) + "/video",
            "http://" + ip + ":" + to_string(port) + "/mjpeg",
            "http://" + ip + ":" + to_string(port),
            "http://" + ip + ":" + to_string(port) + "/video?type=some.mjpeg",
            "rtsp://" + ip + ":" + to_string(port) + "/h264_pcm.sdp"
        };
        
        for (const auto& url : urlsToTry) {
            cout << "Trying: " << url << endl;
            cap = VideoCapture(url);
            
            if (cap.isOpened()) {
                cout << "Connected successfully!" << endl;
                printCameraInfo();
                return true;
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        
        cerr << "Failed to connect to DroidCam" << endl;
        return false;
    }
    
    void printCameraInfo() {
        if (cap.isOpened()) {
            cout << "Camera Info:" << endl;
            cout << "  Width: " << cap.get(CAP_PROP_FRAME_WIDTH) << endl;
            cout << "  Height: " << cap.get(CAP_PROP_FRAME_HEIGHT) << endl;
            cout << "  FPS: " << cap.get(CAP_PROP_FPS) << endl;
            cout << "  Format: " << cap.get(CAP_PROP_FORMAT) << endl;
        }
    }
    
    void setResolution(int width, int height) {
        if (cap.isOpened()) {
            cap.set(CAP_PROP_FRAME_WIDTH, width);
            cap.set(CAP_PROP_FRAME_HEIGHT, height);
            cout << "Set resolution to: " << width << "x" << height << endl;
        }
    }
    
    bool readFrame(Mat& frame, int timeoutMs = 5000) {
        auto start = chrono::steady_clock::now();
        
        while (chrono::duration_cast<chrono::milliseconds>(
               chrono::steady_clock::now() - start).count() < timeoutMs) {
            
            if (cap.read(frame) && !frame.empty()) {
                return true;
            }
            
            // If connection drops, try to reconnect
            if (!cap.isOpened()) {
                cout << "Connection lost. Reconnecting..." << endl;
                connect();
            }
            
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        
        return false;
    }
    
    void runPointerDetection() {
        if (!cap.isOpened()) {
            cerr << "Camera not connected!" << endl;
            return;
        }
        
        cout << "Starting pointer detection..." << endl;
        cout << "Controls: 'q'=quit, 's'=save, 'c'=calibrate" << endl;
        
        Mat frame, hsv, mask;
        int frameCount = 0;
        
        // Color range for pointer (adjust these values)
        Scalar lowerColor(93, 34, 187);
        Scalar upperColor(123, 134, 255);
        
        namedWindow("DroidCam Pointer", WINDOW_NORMAL);
        resizeWindow("DroidCam Pointer", 800, 600);
        
        while (true) {
            if (!readFrame(frame)) {
                cerr << "Failed to read frame" << endl;
                break;
            }
            
            // Process frame
            cvtColor(frame, hsv, COLOR_BGR2HSV);
            inRange(hsv, lowerColor, upperColor, mask);
            
            // Apply morphology to clean up
            Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
            morphologyEx(mask, mask, MORPH_OPEN, kernel);
            morphologyEx(mask, mask, MORPH_CLOSE, kernel);
            
            // Find contours
            vector<vector<Point>> contours;
            findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
            
            // Draw detected pointers
            for (const auto& contour : contours) {
                double area = contourArea(contour);
                if (area > 100) {  // Filter small noise
                    Moments m = moments(contour);
                    if (m.m00 != 0) {
                        int cx = static_cast<int>(m.m10 / m.m00);
                        int cy = static_cast<int>(m.m01 / m.m00);
                        
                        // Draw pointer
                        circle(frame, Point(cx, cy), 15, Scalar(0, 255, 0), FILLED);
                        
                        // Draw crosshair
                        line(frame, Point(cx - 20, cy), 
                             Point(cx + 20, cy), Scalar(0, 255, 255), 2);
                        line(frame, Point(cx, cy - 20), 
                             Point(cx, cy + 20), Scalar(0, 255, 255), 2);
                        
                        // Display coordinates
                        string text = "(" + to_string(cx) + ", " + to_string(cy) + ")";
                        putText(frame, text, Point(cx + 25, cy - 10),
                                FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 2);
                        
                        cout << "Pointer at: " << text << endl;
                    }
                }
            }
            
            // Display FPS
            frameCount++;
            if (frameCount % 30 == 0) {
                double fps = cap.get(CAP_PROP_FPS);
                string fpsText = "FPS: " + to_string(static_cast<int>(fps));
                putText(frame, fpsText, Point(10, 30),
                        FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
            }
            
            // Show frames
            imshow("DroidCam Pointer", frame);
            imshow("Mask", mask);
            
            // Handle keyboard
            int key = waitKey(1);
            if (key == 'q' || key == 27) {
                break;
            } else if (key == 's') {
                string filename = "droidcam_capture_" + 
                                 to_string(time(nullptr)) + ".jpg";
                imwrite(filename, frame);
                cout << "Saved: " << filename << endl;
            } else if (key == 'c') {
                calibrateColor(hsv);
            }
        }
        
        destroyAllWindows();
    }
    
    void calibrateColor(const Mat& hsvFrame) {
        cout << "\n=== Color Calibration ===" << endl;
        cout << "Click on pointer color in the window." << endl;
        
        Mat display;
        cvtColor(hsvFrame, display, COLOR_HSV2BGR);
        
        namedWindow("Calibration", WINDOW_NORMAL);
        imshow("Calibration", display);
        
        setMouseCallback("Calibration", [](int event, int x, int y, int flags, void* data) {
            if (event == EVENT_LBUTTONDOWN) {
                Mat* hsvPtr = static_cast<Mat*>(data);
                Vec3b hsvVal = hsvPtr->at<Vec3b>(y, x);
                
                cout << "\nHSV at (" << x << ", " << y << "):" << endl;
                cout << "  H: " << static_cast<int>(hsvVal[0]) << endl;
                cout << "  S: " << static_cast<int>(hsvVal[1]) << endl;
                cout << "  V: " << static_cast<int>(hsvVal[2]) << endl;
                
                // Calculate suggested ranges
                int h = hsvVal[0];
                int s = hsvVal[1];
                int v = hsvVal[2];
                
                cout << "\nSuggested HSV ranges:" << endl;
                cout << "Lower: [" << max(0, h-15) << ", " 
                     << max(0, s-50) << ", " << max(0, v-50) << "]" << endl;
                cout << "Upper: [" << min(180, h+15) << ", " 
                     << min(255, s+50) << ", " << min(255, v+50) << "]" << endl;
            }
        }, (void*) &hsvFrame);
        
        waitKey(0);
        destroyWindow("Calibration");
    }
    
    ~DroidCam() {
        if (cap.isOpened()) {
            cap.release();
        }
        cout << "DroidCam disconnected." << endl;
    }
};

int main(int argc, char** argv) {
    // Default values
    string ip = "192.168.1.35";
    int port = 4747;
    
    // Parse command line arguments
    if (argc >= 2) {
        ip = argv[1];
    }
    if (argc >= 3) {
        port = stoi(argv[2]);
    }
    
    cout << "DroidCam OpenCV Connector" << endl;
    cout << "=========================" << endl;
    cout << "Usage: " << argv[0] << " [IP] [PORT]" << endl;
    cout << "Default: " << ip << ":" << port << endl;
    cout << "\nMake sure:" << endl;
    cout << "1. Phone and PC are on same WiFi" << endl;
    cout << "2. DroidCam app is running on phone" << endl;
    cout << "3. Firewall allows connection on port " << port << endl;
    
    DroidCam droidcam(ip, port);
    
    if (droidcam.connect()) {
        droidcam.setResolution(1280, 720);  // Optional: set resolution
        droidcam.runPointerDetection();
    } else {
        cerr << "Failed to connect to DroidCam" << endl;
        cerr << "Check:" << endl;
        cerr << "1. Correct IP address" << endl;
        cerr << "2. DroidCam is running on phone" << endl;
        cerr << "3. Port " << port << " is open" << endl;
        return 1;
    }
    
    return 0;
}