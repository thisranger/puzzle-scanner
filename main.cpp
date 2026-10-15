#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

typedef struct
{
    Point center;
    vector<Point> contour;
} box_t;


void FindPuzzle(Mat grey, Mat &puzzle)
{
    Mat dst(grey.rows, grey.cols, CV_8UC1);

    GaussianBlur(grey, dst, Size(5, 5), 0);

    // threshold(grey, dst, 100, 255, THRESH_BINARY_INV | THRESH_TRIANGLE);
    // imshow("OTSU", dst);

    adaptiveThreshold(dst, dst, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 21, 2);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours( dst, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);

    double maxArea = 0;
    int maxAreaIndex = 0;

    for( size_t i = 0; i< contours.size(); i++ )
    {
        double area = contourArea(contours[i]);
        if(area > maxArea)
        {
            maxArea = area;
            maxAreaIndex = i;
        }
    }
    vector<Point> contour = contours[maxAreaIndex];

    double peri = arcLength(contour, true);
    approxPolyDP(contour, contour, 0.02*peri, true);

    vector<Point> hull;
    convexHull(contour, hull);

    vector<Point2f> corners;
    for (const auto& p : hull)
        corners.emplace_back(p.x, p.y);

    // Order corners: top-left, top-right,
    // bottom-right, bottom-left.
    sort(corners.begin(), corners.end(),
         [](const Point2f& a, const Point2f& b) {
             return a.y < b.y;
         });

    vector<Point2f> top = {corners[0], corners[1]};
    vector<Point2f> bottom = {corners[2], corners[3]};

    sort(top.begin(), top.end(),
         [](const Point2f& a, const Point2f& b) {
             return a.x < b.x;
         });

    sort(bottom.begin(), bottom.end(),
         [](const Point2f& a, const Point2f& b) {
             return a.x < b.x;
         });

    // Determine output dimensions from detected corners.
    double width = max(
        norm(top[1] - top[0]),
        norm(bottom[0] - bottom[1]));
    float marginW = width * 0.04;
    width += marginW*2;

    double height = max(
        norm(bottom[0] - top[0]),
        norm(bottom[1] - top[1]));
    float marginH = height * 0.04;
    height += marginH*2;

    Point2f srcCord[4] = {
        Point2f(top[0].x - marginW, top[0].y - marginH),
        Point2f(top[1].x + marginW, top[1].y - marginH),
        Point2f(bottom[1].x + marginW, bottom[1].y + marginH),
        Point2f(bottom[0].x - marginW, bottom[0].y + marginH),
    };

    if (width < 1 || height < 1)
        return;

    Point2f dstCord[4] = {
        Point2f(0, 0),
        Point2f(width - 1, 0),
        Point2f(width - 1, height - 1),
        Point2f(0, height - 1)
    };

    Mat H = getPerspectiveTransform(srcCord, dstCord);

    warpPerspective(grey, puzzle, H, Size(width, height));
}




int main()
{
    Mat src;

    src = imread("C:/Users/Thijs/Projects/EmbeddedVision/puzzle_scanner/images/20261011_142023.jpg",
                 IMREAD_GRAYSCALE);
    src = imread("C:/Users/Thijs/Projects/EmbeddedVision/puzzle_scanner/images/img_5.png", IMREAD_GRAYSCALE);
    if (!src.data)
    {
        cout << "Could not open image!";
        return 1;
    }

    namedWindow("Orginal", WINDOW_NORMAL);
    imshow("Orginal", src);

    Mat puzzle;

    FindPuzzle(src, puzzle);


    GaussianBlur(puzzle, puzzle, Size(13, 13), 0);
    adaptiveThreshold(puzzle, puzzle, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 41, 2);

    // morphologyEx(puzzle, puzzle, MORPH_OPEN, getStructuringElement(MORPH_RECT, Size(5, 5)));

    Mat left;
    Mat right;

    // morphologyEx(puzzle, left, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(15, 1)), Point(0, -1));
    // morphologyEx(puzzle, puzzle, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(15, 1)), Point(14, -1));

    // bitwise_and(left, right, puzzle);

    // morphologyEx(puzzle, puzzle, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(1, 15)));



    // vector<Vec4i> lines;
    //
    // double minLineLength = MIN(puzzle.rows, puzzle.cols)/40;
    //
    // HoughLinesP(puzzle, lines, 1, CV_PI/180, 40);
    //
    //
    // Mat linesImage = Mat::zeros(puzzle.size(), puzzle.type());
    //
    // for( size_t i = 0; i < lines.size(); i++ )
    // {
    //     Vec4i l = lines[i];
    //     line( puzzle, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255), 1, LINE_8);
    //     line( linesImage, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255), 1, LINE_8);
    // }
    //
    // namedWindow("Lines", WINDOW_NORMAL);
    // imshow("Lines", linesImage);


    namedWindow("Puzzle", WINDOW_NORMAL);
    imshow("Puzzle", puzzle);
    waitKey(0);

    Mat dst;
    bitwise_not(puzzle, dst);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours( puzzle, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

    double maxArea = 0;
    int maxAreaIndex = 0;

    for( size_t i = 0; i< contours.size(); i++ )
    {
        double area = contourArea(contours[i]);
        if(area > maxArea)
        {
            maxArea = area;
            maxAreaIndex = i;
        }
    }
    Mat mask = Mat::zeros(puzzle.size(), puzzle.type());
    drawContours(mask, contours, maxAreaIndex, Scalar(255), -1);
    bitwise_and(dst, mask, dst);
    drawContours(dst, contours, maxAreaIndex, Scalar(0), 50);

    int childIndex = hierarchy[maxAreaIndex][2];

    double averageArea = 0;

    int numChildren = 0;
    while (childIndex != -1)
    {
        vector<Point> contour = contours[childIndex];

        averageArea += contourArea(contour);

        childIndex = hierarchy[childIndex][0];

        numChildren++;
    }
    averageArea /= numChildren;

    int gridCount = 0;
    childIndex = hierarchy[maxAreaIndex][2];
    vector<box_t> boxes;

    int averageBoxHeight = 0;

    while (childIndex != -1)
    {
        vector<Point> contour = contours[childIndex];

        if (contourArea(contour) > averageArea * 0.6)
        {
            double peri = arcLength(contour, true);
            approxPolyDP(contour, contour, 0.02*peri, true);

            vector<Point> hull;
            convexHull(contour, hull);

            drawContours(dst, hull, 0, Scalar(255), -1);
            Rect rect = boundingRect(hull);

            Point center(rect.x + rect.width / 2, rect.y + rect.height / 2);
            averageBoxHeight += rect.height;

            boxes.emplace_back(box_t{center, hull});
        }

        childIndex = hierarchy[childIndex][0];
    }
    averageBoxHeight /= boxes.size();
    cout << "Grid count: " << boxes.size() << endl;

    std::sort(boxes.begin(), boxes.end(), [](const box_t &a, const box_t &b) {
        return a.center.y < b.center.y;
    });

    int gridWidth;
    int lowestY = boxes[0].center.y;
    for (gridWidth = 0; gridWidth < boxes.size(); gridWidth++)
    {
        if (boxes[gridWidth].center.y > lowestY + averageBoxHeight / 2)
            break;
    }
    cout << "Grid height: " << gridWidth << endl;

    int gridHeight = boxes.size() / gridWidth;
    vector<vector<box_t>> grid(gridWidth, vector<box_t>(gridHeight));
    for (int y = 0; y < gridHeight; y++)
    {
        std::sort(boxes.begin() + y * gridWidth, boxes.begin() + (y + 1) * gridWidth,
            [](const box_t &a, const box_t &b)
            {
                return a.center.x < b.center.x;
            });

        for (int x = 0; x < gridWidth; x++)
        {
            grid[x][y] = boxes[y * gridWidth + x];

            putText(dst, to_string(x+y*gridWidth), grid[x][y].center, FONT_HERSHEY_SIMPLEX, 1, Scalar(128), 2, LINE_AA);
        }
    }



    namedWindow("Grid", WINDOW_NORMAL);
    do {
        imshow("Grid", dst);

        Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
        // dilate(dst, dst, kernel);

    } while (waitKey(0)!=27);

    return 0;
}
