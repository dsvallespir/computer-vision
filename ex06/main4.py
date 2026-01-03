import cv2
import time
import numpy as np

class PointerDetector:
    def __init__(self, camera_index=0):
        self.camera_index = camera_index
        self.cap = None
        self.init_camera()
        
    def init_camera(self):
        """Initialize camera with proper settings"""
        if self.cap is not None:
            self.cap.release()
            
        self.cap = cv2.VideoCapture(self.camera_index)
        
        # Set camera properties
        #self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        #self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
        #self.cap.set(cv2.CAP_PROP_FPS, 4)
        #self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 5)  # Minimal buffer
        #self.cap.set(cv2.CAP_PROP_AUTOFOCUS, 0)   # Disable autofocus
        
        # Check if camera opened successfully
        if not self.cap.isOpened():
            print(f"Error: Could not open camera {self.camera_index}")
            # Try alternative camera indices
            for i in range(3):
                self.cap = cv2.VideoCapture(i)
                if self.cap.isOpened():
                    print(f"Using camera index {i}")
                    break
            else:
                raise Exception("No camera found")
    
    def get_frame(self, timeout=2.0):
        """Get frame with timeout handling"""
        start_time = time.time()
        
        while time.time() - start_time < timeout:
            ret, frame = self.cap.read()
            if ret:
                return True, frame
            else:
                # Camera might need reinitialization
                self.init_camera()
                time.sleep(0.1)
        
        return False, None
    
    def detect_pointer_simple(self, frame):
        """Simple color-based pointer detection"""
        # Convert to HSV
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        
        # Define color ranges (adjust based on your pointer)
        # Red color range
        lower_red1 = np.array([0, 120, 70])
        upper_red1 = np.array([10, 255, 255])
        lower_red2 = np.array([170, 120, 70])
        upper_red2 = np.array([180, 255, 255])
        
        # Create masks
        mask1 = cv2.inRange(hsv, lower_red1, upper_red1)
        mask2 = cv2.inRange(hsv, lower_red2, upper_red2)
        mask = cv2.bitwise_or(mask1, mask2)
        
        # Apply morphological operations
        kernel = np.ones((5, 5), np.uint8)
        mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
        mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)
        
        # Find contours
        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, 
                                      cv2.CHAIN_APPROX_SIMPLE)
        
        if contours:
            # Get largest contour
            largest = max(contours, key=cv2.contourArea)
            area = cv2.contourArea(largest)
            
            if area > 50:  # Minimum area threshold
                M = cv2.moments(largest)
                if M["m00"] != 0:
                    cx = int(M["m10"] / M["m00"])
                    cy = int(M["m01"] / M["m00"])
                    return (cx, cy), area
        
        return None, 0
    
    def run(self):
        """Main loop"""
        print("Starting pointer detection. Press 'q' to quit.")
        
        frame_count = 0
        last_success_time = time.time()
        
        try:
            while True:
                # Get frame with timeout
                success, frame = self.get_frame()
                
                if not success:
                    print("Warning: Frame timeout")
                    time.sleep(0.5)
                    continue
                
                # Flip frame horizontally for mirror view
                frame = cv2.flip(frame, 1)
                
                # Detect pointer
                pointer_pos, area = self.detect_pointer_simple(frame)
                
                # Draw results
                if pointer_pos:
                    cx, cy = pointer_pos
                    cv2.circle(frame, (cx, cy), 10, (0, 255, 0), -1)
                    cv2.putText(frame, f"Pointer: ({cx}, {cy})", 
                               (cx + 15, cy - 15), 
                               cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
                    print(f"Detected at ({cx}, {cy}) Area: {area:.1f}")
                    last_success_time = time.time()
                else:
                    # If no detection for 10 seconds, show warning
                    if time.time() - last_success_time > 10:
                        cv2.putText(frame, "NO POINTER DETECTED", 
                                   (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 
                                   1, (0, 0, 255), 2)
                
                # Display frame (if GUI available)
                try:
                    cv2.imshow('Pointer Detection', frame)
                    key = cv2.waitKey(1) & 0xFF
                    if key == ord('q'):
                        break
                except:
                    # Headless mode - save occasional frames
                    frame_count += 1
                    if frame_count % 30 == 0:  # Save every 30 frames
                        cv2.imwrite(f"frame_{frame_count}.jpg", frame)
                
                # Small delay to prevent CPU overload
                time.sleep(0.01)
                
        except KeyboardInterrupt:
            print("\nInterrupted by user")
        finally:
            self.cleanup()
    
    def cleanup(self):
        """Release resources"""
        if self.cap:
            self.cap.release()
        cv2.destroyAllWindows()
        print("Resources released")

# Run the detector
if __name__ == "__main__":
    detector = PointerDetector(camera_index=0)
    detector.run()