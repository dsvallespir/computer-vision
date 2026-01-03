import cv2
import numpy as np

cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()
    if not ret:
        break
    
    # Convert to HSV for better color detection
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    
    # Define color range for your pointer (adjust these values)
    lower_color = np.array([0, 120, 70])
    upper_color = np.array([10, 255, 255])
    
    # Create mask
    mask = cv2.inRange(hsv, lower_color, upper_color)
    
    # Find contours
    contours, _ = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    
    if contours:
        # Get largest contour (your pointer)
        largest_contour = max(contours, key=cv2.contourArea)
        
        # Get tip point (simplified - using bounding rectangle)
        x, y, w, h = cv2.boundingRect(largest_contour)
        tip_point = (x + w//2, y)  # Assuming tip is at top
        
        cv2.circle(frame, tip_point, 10, (0, 255, 0), -1)
    
    cv2.imshow('Pointer Detection', frame)
    
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()