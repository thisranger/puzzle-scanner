#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

typedef struct
{
    Point center;
    vector<Point> contour;
} box_t;


//
//
//
//
// int main()
// {
//     Mat src;
//
//     src = imread("C:/Users/Thijs/Projects/EmbeddedVision/puzzle_scanner/images/20261011_142023.jpg",
//                  IMREAD_GRAYSCALE);
//     src = imread("C:/Users/Thijs/Projects/EmbeddedVision/puzzle_scanner/images/img_5.png", IMREAD_GRAYSCALE);
//     if (!src.data)
//     {
//         cout << "Could not open image!";
//         return 1;
//     }
//
//     namedWindow("Orginal", WINDOW_NORMAL);
//     imshow("Orginal", src);
//
//     Mat puzzle;
//
//     FindPuzzle(src, puzzle);
//
//
//     GaussianBlur(puzzle, puzzle, Size(13, 13), 0);
//     adaptiveThreshold(puzzle, puzzle, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 41, 2);
//
//     // morphologyEx(puzzle, puzzle, MORPH_OPEN, getStructuringElement(MORPH_RECT, Size(5, 5)));
//
//     Mat left;
//     Mat right;
//
//     // morphologyEx(puzzle, left, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(15, 1)), Point(0, -1));
//     // morphologyEx(puzzle, puzzle, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(15, 1)), Point(14, -1));
//
//     // bitwise_and(left, right, puzzle);
//
//     // morphologyEx(puzzle, puzzle, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(1, 15)));
//
//
//
//     // vector<Vec4i> lines;
//     //
//     // double minLineLength = MIN(puzzle.rows, puzzle.cols)/40;
//     //
//     // HoughLinesP(puzzle, lines, 1, CV_PI/180, 40);
//     //
//     //
//     // Mat linesImage = Mat::zeros(puzzle.size(), puzzle.type());
//     //
//     // for( size_t i = 0; i < lines.size(); i++ )
//     // {
//     //     Vec4i l = lines[i];
//     //     line( puzzle, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255), 1, LINE_8);
//     //     line( linesImage, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255), 1, LINE_8);
//     // }
//     //
//     // namedWindow("Lines", WINDOW_NORMAL);
//     // imshow("Lines", linesImage);
//
//
//     namedWindow("Puzzle", WINDOW_NORMAL);
//     imshow("Puzzle", puzzle);
//     waitKey(0);
//
//     Mat dst;
//     bitwise_not(puzzle, dst);
//
//     vector<vector<Point> > contours;
//     vector<Vec4i> hierarchy;
//     findContours( puzzle, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
//
//     double maxArea = 0;
//     int maxAreaIndex = 0;
//
//     for( size_t i = 0; i< contours.size(); i++ )
//     {
//         double area = contourArea(contours[i]);
//         if(area > maxArea)
//         {
//             maxArea = area;
//             maxAreaIndex = i;
//         }
//     }
//     Mat mask = Mat::zeros(puzzle.size(), puzzle.type());
//     drawContours(mask, contours, maxAreaIndex, Scalar(255), -1);
//     bitwise_and(dst, mask, dst);
//     drawContours(dst, contours, maxAreaIndex, Scalar(0), 50);
//
//     int childIndex = hierarchy[maxAreaIndex][2];
//
//     double averageArea = 0;
//
//     int numChildren = 0;
//     while (childIndex != -1)
//     {
//         vector<Point> contour = contours[childIndex];
//
//         averageArea += contourArea(contour);
//
//         childIndex = hierarchy[childIndex][0];
//
//         numChildren++;
//     }
//     averageArea /= numChildren;
//
//     int gridCount = 0;
//     childIndex = hierarchy[maxAreaIndex][2];
//     vector<box_t> boxes;
//
//     int averageBoxHeight = 0;
//
//     while (childIndex != -1)
//     {
//         vector<Point> contour = contours[childIndex];
//
//         if (contourArea(contour) > averageArea * 0.6)
//         {
//             double peri = arcLength(contour, true);
//             approxPolyDP(contour, contour, 0.02*peri, true);
//
//             vector<Point> hull;
//             convexHull(contour, hull);
//
//             drawContours(dst, hull, 0, Scalar(255), -1);
//             Rect rect = boundingRect(hull);
//
//             Point center(rect.x + rect.width / 2, rect.y + rect.height / 2);
//             averageBoxHeight += rect.height;
//
//             boxes.emplace_back(box_t{center, hull});
//         }
//
//         childIndex = hierarchy[childIndex][0];
//     }
//     averageBoxHeight /= boxes.size();
//     cout << "Grid count: " << boxes.size() << endl;
//
//     std::sort(boxes.begin(), boxes.end(), [](const box_t &a, const box_t &b) {
//         return a.center.y < b.center.y;
//     });
//
//     int gridWidth;
//     int lowestY = boxes[0].center.y;
//     for (gridWidth = 0; gridWidth < boxes.size(); gridWidth++)
//     {
//         if (boxes[gridWidth].center.y > lowestY + averageBoxHeight / 2)
//             break;
//     }
//     cout << "Grid height: " << gridWidth << endl;
//
//     int gridHeight = boxes.size() / gridWidth;
//     vector<vector<box_t>> grid(gridWidth, vector<box_t>(gridHeight));
//     for (int y = 0; y < gridHeight; y++)
//     {
//         std::sort(boxes.begin() + y * gridWidth, boxes.begin() + (y + 1) * gridWidth,
//             [](const box_t &a, const box_t &b)
//             {
//                 return a.center.x < b.center.x;
//             });
//
//         for (int x = 0; x < gridWidth; x++)
//         {
//             grid[x][y] = boxes[y * gridWidth + x];
//
//             putText(dst, to_string(x+y*gridWidth), grid[x][y].center, FONT_HERSHEY_SIMPLEX, 1, Scalar(128), 2, LINE_AA);
//         }
//     }
//
//
//
//     namedWindow("Grid", WINDOW_NORMAL);
//     do {
//         imshow("Grid", dst);
//
//         Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
//         // dilate(dst, dst, kernel);
//
//     } while (waitKey(0)!=27);
//
//     return 0;
// }

#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <algorithm>

// Find line positions from a 1D projection histogram.
// The histogram is a CV_32F column vector.
std::vector<int> findLinePeaks(
    const cv::Mat& histogram,
    float threshold,
    int smoothSize = 7)
{
    cv::Mat smooth;
    cv::GaussianBlur(
        histogram, smooth, cv::Size(1, smoothSize), 0);

    std::vector<int> positions;
    int start = -1;
    float weightedPosition = 0.0f;
    float totalWeight = 0.0f;

    for (int i = 0; i < smooth.rows; ++i)
    {
        float value = smooth.at<float>(i, 0);

        if (value >= threshold)
        {
            if (start == -1)
            {
                start = i;
                weightedPosition = 0.0f;
                totalWeight = 0.0f;
            }

            weightedPosition += i * value;
            totalWeight += value;
        }
        else if (start != -1)
        {
            positions.push_back(
                cvRound(weightedPosition / totalWeight));
            start = -1;
        }
    }

    if (start != -1 && totalWeight > 0)
    {
        positions.push_back(
            cvRound(weightedPosition / totalWeight));
    }

    return positions;
}

int main()
{
    cv::Mat image = cv::imread("C:/Users/Thijs/Projects/EmbeddedVision/puzzle_scanner/images/img_4.png");
    if (image.empty())
        return -1;

    cv::Mat gray;
    if (image.channels() == 3)
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    else if (image.channels() == 4)
        cv::cvtColor(image, gray, cv::COLOR_BGRA2GRAY);
    else
        gray = image.clone();

    cv::Mat puzzle;
    FindPuzzle(gray, puzzle);

    imshow("puzzle", puzzle);

    cv::Mat binary;
    cv::adaptiveThreshold(
        puzzle, binary, 255,
        cv::ADAPTIVE_THRESH_GAUSSIAN_C,
        cv::THRESH_BINARY_INV,
        51, 5);

    erode(binary, binary, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 1)), Point(-1, -1), 20);
    dilate(binary, binary, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 4)), Point(-1, -1), 1);
    dilate(binary, binary, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(4, 1)), Point(-1, -1), 20);

    // erode(binary, binary, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 2)), Point(-1, -1), 20);
    // dilate(binary, binary, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(4, 1)), Point(-1, -1), 1);
    // dilate(binary, binary, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 4)), Point(-1, -1), 20);



    imshow("binary", binary);
    waitKey(0);

    return 0;


    // Ideally replace this with the actual grid bounding rectangle.
    cv::Rect roiRect(0, 0, binary.cols, binary.rows);
    cv::Mat roi = binary(roiRect);

    // Count foreground pixels in every row and column.
    cv::Mat rowHist(roi.rows, 1, CV_32F, cv::Scalar(0));
    cv::Mat colHist(1, roi.cols, CV_32F, cv::Scalar(0));

    for (int y = 0; y < roi.rows; ++y)
        rowHist.at<float>(y, 0) =
            static_cast<float>(cv::countNonZero(roi.row(y)));

    for (int x = 0; x < roi.cols; ++x)
        colHist.at<float>(0, x) =
            static_cast<float>(cv::countNonZero(roi.col(x)));

    cv::Mat row;
    cv::Mat col;

    cv::normalize(rowHist, rowHist, 0, 1, cv::NORM_MINMAX);
    cv::normalize(colHist, colHist, 0, 1, cv::NORM_MINMAX);

    rowHist.convertTo(row, CV_8UC1, 255);
    colHist.convertTo(col, CV_8UC1, 255);

    threshold(row, row, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    threshold(col, col, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
    cv::dilate(row, row, kernel);
    cv::dilate(col, col, kernel);

    cv::Mat imageRow;
    cv::Mat imageCol;

    cv::resize(row, row, cv::Size(image.cols, image.rows), 0, 0, cv::INTER_LINEAR);
    cv::resize(col, col, cv::Size(image.cols, image.rows), 0, 0, cv::INTER_LINEAR);

    std::cout << "row: " << row.rows << " col: " << row.cols << std::endl;
    std::cout << "row: " << image.rows << " col: " << image.cols << std::endl;


    namedWindow("row", cv::WINDOW_NORMAL);
    imshow("row", row);
    namedWindow("col", cv::WINDOW_NORMAL);
    imshow("col", col);

    cv::bitwise_and(puzzle, row, imageRow);
    cv::bitwise_and(puzzle, col, imageCol);

    namedWindow("row image", cv::WINDOW_NORMAL);
    imshow("row image", imageRow);
    namedWindow("col image", cv::WINDOW_NORMAL);
    imshow("col image", imageCol);

    cv::waitKey(0);

    return 0;

    // Require a line to have foreground support across a fraction
    // of the ROI. Tune these fractions for your images.
    float rowThreshold = roi.cols * 0.20f;
    float colThreshold = roi.rows * 0.20f;

    auto rows = findLinePeaks(rowHist, rowThreshold);

    // findLinePeaks expects a column vector.
    cv::Mat colHistColumn;
    cv::transpose(colHist, colHistColumn);
    auto cols = findLinePeaks(colHistColumn, colThreshold);

    // Draw reconstructed lines in the ROI.
    cv::Mat reconstructed =
        cv::Mat::zeros(roi.size(), CV_8UC1);

    for (int y : rows)
        cv::line(reconstructed, {0, y},
                 {roi.cols - 1, y}, cv::Scalar(255), 1);

    for (int x : cols)
        cv::line(reconstructed, {x, 0},
                 {x, roi.rows - 1}, cv::Scalar(255), 1);

    cv::Mat overlay = image.clone();

    cv::Mat fullGrid = cv::Mat::zeros(
        image.size(), CV_8UC1);
    reconstructed.copyTo(fullGrid(roiRect));

    overlay.setTo(cv::Scalar(0, 0, 255), fullGrid);

    cv::imshow("Binary", binary);
    cv::imshow("Reconstructed grid", fullGrid);
    cv::imshow("Grid overlay", overlay);
    cv::waitKey(0);
}