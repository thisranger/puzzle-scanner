/**
 * @file puzzle.c
 * @author thisranger
 * @date 18/10/2026
 * @last_update 18/10/2026
 */

/* ============================================================= */
/*                         INCLUDES                              */
/* ============================================================= */
#include <iostream>
#include <opencv2/opencv.hpp>
#include "templates.h"

using namespace std;
using namespace cv;

/* ============================================================= */
/*                          DEFINES                              */
/* ============================================================= */
typedef struct
{
    Point center;
    vector<Point> contour;
} box_t;

/* ============================================================= */
/*                        GLOBAL VARIABLES                       */
/* ============================================================= */

/* ============================================================= */
/*                        LOCAL VARIABLES                        */
/* ============================================================= */

/* ============================================================= */
/*                     FUNCTION PROTOTYPES                       */
/* ============================================================= */
void FindPuzzle(Mat grey, Mat& puzzle);
void CreateGridMask(Mat puzzle, Mat& mask);
void CreateHLineMask(Mat binary, Mat& mask);
void WarpBox(box_t& box, Mat puzzle, Mat& matBox);

/* ============================================================= */
/*                          FUNCTIONS                            */
/* ============================================================= */


int main()
{
    Templates templates("C:/Users/Thijs/Projects/EmbeddedVision/puzzle_scanner/numbers");
    
    
    Mat src;

    src = imread("C:/Users/Thijs/Projects/EmbeddedVision/puzzle_scanner/images/img_14.png", IMREAD_GRAYSCALE);
    if (!src.data)
    {
        cout << "Could not open image!";
        return 1;
    }

    Mat puzzle;
    FindPuzzle(src, puzzle);

    namedWindow("Puzzle", WINDOW_NORMAL);
    imshow("Puzzle", puzzle);

    Mat gridMask;
    CreateGridMask(puzzle, gridMask);

    namedWindow("Grid", WINDOW_NORMAL);
    imshow("Grid", gridMask);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(gridMask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

    double maxArea = 0;
    int maxAreaIndex = 0;

    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        if (area > maxArea)
        {
            maxArea = area;
            maxAreaIndex = i;
        }
    }

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

    childIndex = hierarchy[maxAreaIndex][2];
    vector<box_t> boxes;

    int averageBoxHeight = 0;

    Mat dst = Mat::zeros(puzzle.size(), puzzle.type());
    while (childIndex != -1)
    {
        vector<Point> contour = contours[childIndex];

        if (contourArea(contour) > averageArea * 0.6)
        {
            double peri = arcLength(contour, true);
            approxPolyDP(contour, contour, 0.02 * peri, true);

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

    std::sort(boxes.begin(), boxes.end(), [](const box_t& a, const box_t& b)
    {
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
                  [](const box_t& a, const box_t& b)
                  {
                      return a.center.x < b.center.x;
                  });

        for (int x = 0; x < gridWidth; x++)
        {
            grid[x][y] = boxes[y * gridWidth + x];

            Mat box;
            WarpBox(grid[x][y], puzzle, box);

            putText(dst, templates.Match(box, 0.7), grid[x][y].center, FONT_HERSHEY_SIMPLEX, 1, Scalar(128), 2,
                    LINE_AA);



            cout << templates.Match(box, 0.8)[0];

            // imshow("Box", box);
            // waitKey(0);
        }

    }


    namedWindow("Scan", WINDOW_NORMAL);
    imshow("Scan", dst);
    waitKey(0);

    return 0;
}



void WarpBox(box_t& box, Mat puzzle, Mat& matBox)
{
    vector<Point> approx;
    double peri = arcLength(box.contour, true);
    approxPolyDP(box.contour, approx, 0.02 * peri, true);

    vector<Point> hull;
    convexHull(approx, hull);

    vector<Point2f> corners;
    for (const auto& p : hull)
        corners.emplace_back(p.x, p.y);

    // Order corners: top-left, top-right,
    // bottom-right, bottom-left.
    sort(corners.begin(), corners.end(),
         [](const Point2f& a, const Point2f& b)
         {
             return a.y < b.y;
         });

    vector<Point2f> top = {corners[0], corners[1]};
    vector<Point2f> bottom = {corners[2], corners[3]};

    sort(top.begin(), top.end(),
         [](const Point2f& a, const Point2f& b)
         {
             return a.x < b.x;
         });

    sort(bottom.begin(), bottom.end(),
         [](const Point2f& a, const Point2f& b)
         {
             return a.x < b.x;
         });

    // Determine output dimensions from detected corners.
    double width = max(
        norm(top[1] - top[0]),
        norm(bottom[0] - bottom[1]));
    float marginW = 0;
    width += marginW * 2;

    double height = max(
        norm(bottom[0] - top[0]),
        norm(bottom[1] - top[1]));
    float marginH = 0;
    height += marginH * 2;

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
        Point2f(25, 0),
        Point2f(25, 25),
        Point2f(0, 25)
    };

    Mat H = getPerspectiveTransform(srcCord, dstCord);

    warpPerspective(puzzle, matBox, H, Size(25, 25));
}



void FindPuzzle(Mat grey, Mat& puzzle)
{
    Mat dst(grey.rows, grey.cols, CV_8UC1);

    GaussianBlur(grey, dst, Size(5, 5), 0);

    adaptiveThreshold(dst, dst, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 21, 2);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(dst, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);

    double maxArea = 0;
    int maxAreaIndex = 0;

    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        if (area > maxArea)
        {
            maxArea = area;
            maxAreaIndex = i;
        }
    }
    vector<Point> contour = contours[maxAreaIndex];

    Mat mask = Mat::zeros(grey.size(), grey.type());
    drawContours(mask, contours, maxAreaIndex, Scalar(255), -1);
    drawContours(mask, contours, maxAreaIndex, Scalar(0), 1);
    bitwise_and(grey, mask, grey);

    vector<Point> approx;
    double peri = arcLength(contour, true);
    approxPolyDP(contour, approx, 0.02 * peri, true);

    vector<Point> hull;
    convexHull(approx, hull);

    vector<Point2f> corners;
    for (const auto& p : hull)
        corners.emplace_back(p.x, p.y);

    // Order corners: top-left, top-right,
    // bottom-right, bottom-left.
    sort(corners.begin(), corners.end(),
         [](const Point2f& a, const Point2f& b)
         {
             return a.y < b.y;
         });

    vector<Point2f> top = {corners[0], corners[1]};
    vector<Point2f> bottom = {corners[2], corners[3]};

    sort(top.begin(), top.end(),
         [](const Point2f& a, const Point2f& b)
         {
             return a.x < b.x;
         });

    sort(bottom.begin(), bottom.end(),
         [](const Point2f& a, const Point2f& b)
         {
             return a.x < b.x;
         });

    // Determine output dimensions from detected corners.
    double width = max(
        norm(top[1] - top[0]),
        norm(bottom[0] - bottom[1]));
    float marginW = width * 0.04;
    width += marginW * 2;

    double height = max(
        norm(bottom[0] - top[0]),
        norm(bottom[1] - top[1]));
    float marginH = height * 0.04;
    height += marginH * 2;

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



void CreateGridMask(Mat puzzle, Mat& mask)
{
    Mat binary;
    adaptiveThreshold(
        puzzle, binary, 255,
        ADAPTIVE_THRESH_GAUSSIAN_C,
        THRESH_BINARY_INV,
        51, 5);


    Mat binary90;
    rotate(binary, binary90, ROTATE_90_CLOCKWISE);

    Mat row;
    CreateHLineMask(binary, row);

    Mat col;
    CreateHLineMask(binary90, col);
    rotate(col, col, ROTATE_90_COUNTERCLOCKWISE);

    bitwise_or(row, col, mask);
}


void CreateHLineMask(Mat binary, Mat& mask)
{
    erode(binary, binary, getStructuringElement(cv::MORPH_RECT, cv::Size(3, 1)), Point(-1, -1), 10);
    morphologyEx(binary, binary, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(5, 5)));

    // dilate(row, row, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 5)), Point(-1, -1), 2);
    dilate(binary, binary, getStructuringElement(cv::MORPH_RECT, cv::Size(5, 1)), Point(-1, -1), 20);

    vector<vector<Point>> contours;
    findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    double maxArea = 0;
    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        if (area > maxArea)
        {
            maxArea = area;
        }
    }


    mask = Mat::zeros(binary.size(), binary.type());
    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        if (area > maxArea * 0.2)
        {
            Mat contour = Mat::zeros(binary.size(), binary.type());
            drawContours(mask, contours, i, Scalar(255), -1);
        }
    }
}
