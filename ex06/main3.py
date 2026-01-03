import cv2
import numpy as np

def detect_colored_pointer():
    cap = cv2.VideoCapture(0)
    
    # Adjust these HSV values for your pointer color
    # For green pointer/laser
    lower_green = np.array([40, 50, 50])
    upper_green = np.array([80, 255, 255])
    
    # For red pointer/laser
    lower_red1 = np.array([0, 120, 70])
    upper_red1 = np.array([10, 255, 255])
    lower_red2 = np.array([170, 120, 70])
    upper_red2 = np.array([180, 255, 255])
    
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        
        # Flip horizontally for mirror view
        frame = cv2.flip(frame, 1)
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        
        # Create mask for pointer color
        mask1 = cv2.inRange(hsv, lower_red1, upper_red1)
        mask2 = cv2.inRange(hsv, lower_red2, upper_red2)
        mask = cv2.bitwise_or(mask1, mask2)
        
        # Clean up mask
        kernel = np.ones((5,5), np.uint8)
        mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
        mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)
        
        # Find contours
        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        
        if contours:
            # Get largest contour
            largest_contour = max(contours, key=cv2.contourArea)
            
            # Get moments to find center
            M = cv2.moments(largest_contour)
            if M["m00"] != 0:
                cx = int(M["m10"] / M["m00"])
                cy = int(M["m01"] / M["m00"])
                
                # Draw pointer
                cv2.circle(frame, (cx, cy), 10, (0, 255, 0), -1)
                cv2.putText(frame, f'({cx}, {cy})', (cx+15, cy-15), 
                           cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)
        
        # Show both original and mask
        cv2.imshow('Pointer Detection', frame)
        cv2.imshow('Mask', mask)
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    detect_colored_pointer()