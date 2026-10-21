# Puzzle Scanner

A Computer Vision application built in C++ using OpenCV to automatically detect, extract, and digitize printed newspaper puzzles—specifically **Sudoku**, **Binaire** (Binary puzzle), and **Tectonic** (Suguru) puzzles from the Dutch newspaper *De Gelderlander*.

---

## Project Overview

The objective of this project is to take a camera image or scan of a newspaper page containing a logic puzzle, rectify its perspective, extract the individual grid cells, and recognize the printed numbers/digits using template matching.

### Supported Puzzle Types
Basically any grid puzzles with numbers, as long as you only want the numbers and the position of them in the grid.

---

## Pipeline & Extraction Steps

The extraction process consists of five main computer vision stages implemented in OpenCV:

```
Input Image 
    │
    ▼
1. Puzzle Detection & Perspective Warp (FindPuzzle)
    │
    ▼
2. Grid & Line Mask Generation (CreateGridMask, CreateHLineMask)
    │
    ▼
3. Cell / Box Extraction (GetBoxes)
    │
    ▼
4. Grid Ordering & Cell Normalization (CreateAndScanGrid, WarpBox)
    │
    ▼
5. Digit Recognition via Template Matching (Templates::Match)
    │
    ▼
Digitized Grid Output
```

### 1. Puzzle Detection & Perspective Rectification (`FindPuzzle`)
- **Preprocessing**: Converts the input image to grayscale, applies a Gaussian Blur (`5x5`) to suppress print noise, and computes an adaptive binary threshold (`ADAPTIVE_THRESH_GAUSSIAN_C`).
- **Contour & Hull Extraction**: Identifies the largest external contour representing the puzzle's outer boundary. It computes the convex hull and approximates the polygonal contour using `approxPolyDP` to identify the 4 corner points.
- **Warping**: Sorts the 4 corners (top-left, top-right, bottom-right, bottom-left), calculates the target dimensions (with a 4% margin), and performs a perspective transformation (`getPerspectiveTransform` and `warpPerspective`) to produce a top-down, rectified view of the puzzle grid.

### 2. Grid Mask Generation (`CreateGridMask` & `CreateHLineMask`)
- **Morphological Line Filtering**: Applies horizontal morphological operations (erosion with a `3x1` rectangular kernel, closing with a `5x5` kernel, followed by dilation with a `5x1` kernel) to isolate horizontal grid lines.
- **Orthogonal Extraction**: Rotates the binary image by 90 degrees to extract vertical lines using the same horizontal filter, then rotates the result back.
- **Mask Combination**: Combines horizontal and vertical line masks using a bitwise OR (`bitwise_or`) to create a clean mask of the entire grid structure.

### 3. Cell Extraction (`GetBoxes`)
- **Contour Hierarchy**: Analyzes contours using `RETR_TREE` hierarchy to inspect child contours of the largest outer grid container.
- **Filtering**: Computes the average area of child contours and discards noise/artifacts with area below `60%` of the average cell area.
- **Box Descriptors**: Fits bounding polygons and hulls around valid cells, calculating each cell's center coordinates, bounding rectangle, and the average box height.

### 4. Grid Organization & Cell Warping (`CreateAndScanGrid` & `WarpBox`)
- **Row & Column Clustering**: Sorts detected boxes vertically by Y-coordinate, calculates grid width based on the number of cells within the first row (using half the average box height as a threshold), and groups boxes into a 2D matrix (`grid[x][y]`).
- **Horizontal Ordering**: Sorts each row horizontally by X-coordinate.
- **Cell Normalization**: Warps each individual cell contour using perspective transformation (`WarpBox`) into a uniform `50x50` pixel image.

### 5. Digit Recognition (`Templates::Match`)
- **Template Matching**: Matches each warped `50x50` cell against a set of grayscale templates located in the `numbers/` directory using normalized cross-correlation (`matchTemplate` with `TM_CCOEFF_NORMED`).
- **Confidence Thresholding**: Identifies the template with the highest correlation score. A confidence threshold (e.g. `0.75`) is applied to distinguish filled cells from empty cells.
- **Output**: The digitized numbers are printed to the console in matrix format and rendered onto the output image overlay.

---

## Font Variations & Template Handling

Puzzles printed in *De Gelderlander* vary in typography and font weight depending on where the puzzle is placed on the page. 

To achieve high-recognition reliability:
- The `numbers/` folder contains multiple template variants for each digit to account for differing font styles.
- When matching, the algorithm evaluates all available templates and selects the candidate with the highest correlation score.

---

## Current Limitations & Future Improvements

- **Tectonic Zone Detection**: In Tectonic puzzles, thick/bold borderlines partition the grid into irregular polyomino blocks (zones of 1 to 5 cells). While the grid cells and numbers are accurately extracted, detection and grouping of these distinct zone borders is currently not implemented.
- **Using confidence to correct mistakes**: Currently, the highest-scoring template is always selected, but it may be possible to improve the accuracy by considering the confidence score of each template in consideration with the matching row and colum or zone.
---

## Requirements & Building

- **C++ Standard**: C++20
- **CMake**: >= 3.20
- **OpenCV**: >= 4.x (Core, ImgProc, ImgCodecs, HighGUI)

