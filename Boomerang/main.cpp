#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>

#include "utils.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    // Collect parameters
    if (argc < 2 || argc > 6)
    {
        cout << "usage: Boomerang <Input_Video_Path> <Output_Video_Path> <Max_Seconds> <Contrast_Factor> <Angle_Rotation>" << endl;
        cin.get();
        return -1;
    }
    string input_file, output_file = "";
    float contrast_factor = 1.0;
    int seconds = 0, angle_rotation = 0;
    if (argc >= 2)
        input_file = argv[1];
    if (argc >= 3)
        output_file = argv[2];
    if (argc >= 4)
        seconds = stoi(argv[3]);
    if (argc >= 5)
        contrast_factor = stof(argv[4]);
    if (argc >= 6)
        angle_rotation = stoi(argv[5]);

    video_preprocessing(input_file, output_file, seconds, contrast_factor, angle_rotation);

    return 0;
}