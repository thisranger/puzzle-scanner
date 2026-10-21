/**
 * @file main.cpp
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
    vector<result_t> content;
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
static void FindPuzzle(Mat grey, Mat& puzzle);

static void CreateGridMask(Mat puzzle, Mat& mask);
static void CreateHLineMask(Mat binary, Mat& mask);

static void GetBoxes(Mat gridMask, vector<box_t>& boxes, int& averageBoxHeight);

static void CreateAndScanGrid(vector<box_t>& boxes, vector<vector<box_t>>& grid, int averageBoxHeight, Mat puzzle, Templates& templates);
static void WarpBox(box_t& box, Mat puzzle, Mat& matBox);

/* ============================================================= */
/*                          FUNCTIONS                            */
/* ============================================================= */
int main()
{
    Templates templates("C:/Users/Thijs/Projects/EmbeddedVision/puzzle_scanner/numbers");

    Mat src;

    src = imread("C:/Users/Thijs/Projects/EmbeddedVision/puzzle_scanner/images/img_0.png", IMREAD_GRAYSCALE);
    if (!src.data)
    {
        cout << "Could not open image!";
        return 1;
    }

    Mat puzzle;

    //Finds the outer contour of the puzzle and uses that the transform the input
    FindPuzzle(src, puzzle);
    namedWindow("Transformed", WINDOW_NORMAL);
    imshow("Transformed", puzzle);

    Mat gridMask;

    //Creates a mask of the grid and returns it
    CreateGridMask(puzzle, gridMask);
    namedWindow("Grid Mask", WINDOW_NORMAL);
    imshow("Grid Mask", gridMask);

    vector<box_t> boxes;
    int averageBoxHeight;

    //Uses the mask the find the boxes and their average height
    GetBoxes(gridMask, boxes,averageBoxHeight);

    vector<vector<box_t>> grid;

    //Creates a grid of the boxes and scans it for the numbers
    CreateAndScanGrid(boxes, grid, averageBoxHeight, puzzle, templates);

    if (grid.empty())
        return 0;

    cout << "Grid width: " << grid.size() << " Grid height: " << grid[0].size()  << endl;

    Mat dst = Mat::zeros(puzzle.size(), puzzle.type());
    for (int y = 0; y < grid[0].size(); y++)
    {
        for (int x = 0; x < grid.size(); x++)
        {
            box_t box = grid[x][y];
            drawContours(dst, box.contour, 0, Scalar(255), -1);

            char number;
            double highestConfidence = 0;
            int highestIndex = 0;
            for (int i = 0; i < box.content.size(); i++)
            {
                if (box.content[i].confidence > highestConfidence)
                {
                    highestConfidence = box.content[i].confidence;
                    number = box.content[i].name[0];
                    highestIndex = i;
                }
            }

            if (highestConfidence < 0.75) {
                cout << ' ';
                continue;
            }
            cout << number;

            putText(dst, box.content[highestIndex].name, box.center, FONT_HERSHEY_SIMPLEX, 1, Scalar(128), 2,
                    LINE_AA);
        }

        cout << endl;
    }

    namedWindow("Result", WINDOW_NORMAL);
    imshow("Result", dst);
    waitKey(0);

    return 0;
}



static void CreateAndScanGrid(vector<box_t>& boxes, vector<vector<box_t>>& grid, int averageBoxHeight, Mat puzzle, Templates& templates)
{
    sort(boxes.begin(), boxes.end(), [](const box_t& a, const box_t& b)
    {
        return a.center.y < b.center.y;
    });

    int lowestY = boxes[0].center.y;
    averageBoxHeight /= boxes.size();

    int gridWidth;
    for (gridWidth = 0; gridWidth < boxes.size(); gridWidth++)
    {
        if (boxes[gridWidth].center.y > lowestY + averageBoxHeight / 2)
            break;
    }

    int gridHeight = boxes.size() / gridWidth;
    grid.resize(gridWidth, vector<box_t>(gridHeight));

    for (int y = 0; y < gridHeight; y++)
    {
        sort(boxes.begin() + y * gridWidth, boxes.begin() + (y + 1) * gridWidth,
             [](const box_t& a, const box_t& b)
             {
                 return a.center.x < b.center.x;
             });

        for (int x = 0; x < gridWidth; x++)
        {
            grid[x][y] = boxes[y * gridWidth + x];

            Mat box;
            WarpBox(grid[x][y], puzzle, box);

            templates.Match(box, grid[x][y].content);
        }
    }
}



static void GetBoxes(Mat gridMask, vector<box_t>& boxes, int& averageBoxHeight)
{
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

    averageBoxHeight = 0;

    while (childIndex != -1)
    {
        vector<Point> contour = contours[childIndex];

        if (contourArea(contour) > averageArea * 0.6)
        {
            double peri = arcLength(contour, true);
            approxPolyDP(contour, contour, 0.02 * peri, true);

            vector<Point> hull;
            convexHull(contour, hull);

            Rect rect = boundingRect(hull);

            Point center(rect.x + rect.width / 2, rect.y + rect.height / 2);
            averageBoxHeight += rect.height;

            boxes.emplace_back(box_t{center, hull});
        }

        childIndex = hierarchy[childIndex][0];
    }
}



static void WarpBox(box_t& box, Mat puzzle, Mat& matBox)
{
    vector<Point> approx;
    double peri = arcLength(box.contour, true);
    approxPolyDP(box.contour, approx, 0.02 * peri, true);

    vector<Point> hull;
    convexHull(approx, hull);

    vector<Point2f> corners;
    for (const auto& p : hull)
        corners.emplace_back(p.x, p.y);

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

    Point2f srcCord[4] = {
        Point2f(top[0].x, top[0].y),
        Point2f(top[1].x, top[1].y),
        Point2f(bottom[1].x, bottom[1].y),
        Point2f(bottom[0].x, bottom[0].y),
    };

    Point2f dstCord[4] = {
        Point2f(0, 0),
        Point2f(50, 0),
        Point2f(50, 50),
        Point2f(0, 50)
    };

    Mat transform = getPerspectiveTransform(srcCord, dstCord);

    warpPerspective(puzzle, matBox, transform, Size(50, 50));
}



static void FindPuzzle(Mat grey, Mat& puzzle)
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

    float width = max(
        norm(top[1] - top[0]),
        norm(bottom[0] - bottom[1]));
    float marginW = width * 0.04;
    width += marginW * 2;

    float height = max(
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



static void CreateGridMask(Mat puzzle, Mat& mask)
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



static void CreateHLineMask(Mat binary, Mat& mask)
{
    erode(binary, binary, getStructuringElement(cv::MORPH_RECT, cv::Size(3, 1)), Point(-1, -1), 10);
    morphologyEx(binary, binary, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(5, 5)));

    // dilate(row, row, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 5)), Point(-1, -1), 2);
    dilate(binary, binary, getStructuringElement(cv::MORPH_RECT, cv::Size(5, 1)), Point(-1, -1), 20);

    vector<vector<Point>> contours;
    findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    int maxWidth = 0;
    for (size_t i = 0; i < contours.size(); i++)
    {
        int width = boundingRect(contours[i]).width;
        if (width > maxWidth)
        {
            maxWidth = width;
        }
    }


    mask = Mat::zeros(binary.size(), binary.type());
    for (size_t i = 0; i < contours.size(); i++)
    {
        int width = boundingRect(contours[i]).width;
        if (width > maxWidth * 0.5)
        {
            Mat contour = Mat::zeros(binary.size(), binary.type());
            drawContours(mask, contours, i, Scalar(255), -1);
        }
    }
}
