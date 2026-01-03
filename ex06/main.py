import cv2
import mediapipe as mp

mp_hands = mp.solutions.hands
hands = mp_hands.Hands()
mp_draw = mp.solutions.drawing_utils

cap = cv2.VideoCapture(0)

while cap.isOpened():
    success, image = cap.read()
    if not success:
        continue
    
    # Process with MediaPipe
    image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    results = hands.process(image_rgb)
    
    if results.multi_hand_landmarks:
        for hand_landmarks in results.multi_hand_landmarks:
            # Get index finger tip (landmark 8)
            index_tip = hand_landmarks.landmark[8]
            h, w, c = image.shape
            cx, cy = int(index_tip.x * w), int(index_tip.y * h)
            
            cv2.circle(image, (cx, cy), 10, (0, 255, 0), -1)
    
    cv2.imshow('Finger Pointer', image)
    if cv2.waitKey(5) & 0xFF == 27:
        break

cap.release()
cv2.destroyAllWindows()