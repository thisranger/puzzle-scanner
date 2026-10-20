import cv2

gray = cv2.imread("C:/Users/Thijs/Projects/EmbeddedVision/puzzle_scanner/images/img_4.png", cv2.IMREAD_GRAYSCALE)

background = cv2.GaussianBlur(
    gray, (0, 0), 11
)

normalized = cv2.divide(
    gray, background,
    scale=255
)

normalized = cv2.equalizeHist(normalized)

ret,th1 = cv2.threshold(normalized,127,255,cv2.THRESH_BINARY | cv2.THRESH_OTSU)
th2 = cv2.adaptiveThreshold(normalized,255,cv2.ADAPTIVE_THRESH_MEAN_C, \
                           cv2.THRESH_BINARY,11,2)
th3 = cv2.adaptiveThreshold(normalized,255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C, \
                           cv2.THRESH_BINARY,11,2)


close = cv2.morphologyEx(th1, cv2.MORPH_CLOSE, cv2.getStructuringElement(cv2.MORPH_RECT, (3,3)))
cv2.imshow("close", close)

cv2.imshow("normalized", normalized)
cv2.imshow("th1", th1)
cv2.imshow("th2", th2)
cv2.imshow("th3", th3)

cv2.waitKey(0)