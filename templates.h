//
// Created by Thijs on 19/10/2026.
//

#ifndef PUZZLE_SCANNER_TEMPLATES_H
#define PUZZLE_SCANNER_TEMPLATES_H

#include <string>
#include <vector>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/mat.hpp>
#include <filesystem>
#include <iostream>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

typedef struct
{
    Mat plate;
    string name;
} template_t;

class Templates
{
    public:
    Templates(const string& path)
    {
        for (const auto& entry : fs::directory_iterator(path))
        {
            template_t plate = {
                imread(entry.path().string(), IMREAD_GRAYSCALE)
                , entry.path().stem().string()
            };

            cout << "Loaded template: " << plate.name << endl;

            templates.push_back(plate);
        }
    }

    string Match(const Mat& image, double threshold)
    {
        double highestValue = 0;
        int highestValueIndex = 0;

        for (int i = 0; i < templates.size(); i++)
        {
            Mat dst;
            matchTemplate(image, templates[i].plate, dst, TM_CCOEFF_NORMED);

            double maxValue;
            minMaxLoc( dst, NULL, &maxValue);

            if (maxValue > highestValue)
            {
                highestValueIndex = i;
                highestValue = maxValue;
            }
        }

        if (highestValue < threshold)
            return "\0";

        return templates[highestValueIndex].name;
    }



    private:
    vector<template_t> templates;
};


#endif //PUZZLE_SCANNER_TEMPLATES_H
