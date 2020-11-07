#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;


Mat histogram_equalization(Mat image)
{
    Mat hist_equalized_image;
    cvtColor(image, hist_equalized_image, COLOR_BGR2YCrCb);
    vector<Mat> vec_channels;
    split(hist_equalized_image, vec_channels);
    equalizeHist(vec_channels[0], vec_channels[0]);
    merge(vec_channels, hist_equalized_image);
    cvtColor(hist_equalized_image, hist_equalized_image, COLOR_YCrCb2BGR);
    return hist_equalized_image;
}

Mat change_contrast(Mat image, float contrast_factor)
{
    Mat contrasted_image;
    image.convertTo(contrasted_image, -1, contrast_factor, 0);
    return contrasted_image;
}

Mat rotate_angle(Mat image, Mat rotation, Rect2d bbox, int border_mode)
{
    Mat image_rotated;
    warpAffine(image, image_rotated, rotation, bbox.size(), INTER_LINEAR, border_mode, Scalar());
    return image_rotated;
}

vector<string> split_path(const string& str, const set<char> delimiters)
{
    vector<string> result;

    char const* pch = str.c_str();
    char const* start = pch;
    for (; *pch; ++pch)
    {
        if (delimiters.find(*pch) != delimiters.end())
        {
            if (start != pch)
            {
                string str(start, pch);
                result.push_back(str);
            }
            else
                result.push_back("");
            start = pch + 1;
        }
    }
    result.push_back(start);

    return result;
}

int video_preprocessing(string input_name, string output_name, int max_seconds, float contrast_factor, int angle_rotation)
{
    // Check name files
    VideoCapture cap(input_name);
    if (output_name == "")
    {
        set<char> delimiter{ '/' };
        vector<string> path = split_path(input_name, delimiter);
        string path_only = "";
        if (path.size() > 1)
        {
            for (int i = 0; i < path.size()-1; i++)
                path_only = path_only + path[i] + "/";
        }
        string filename = path.back();
        delimiter = { '.' };
        vector<string> filename_string = split_path(filename, delimiter);
        output_name = path_only + filename_string[0] + "_boomerang." + filename_string[1];
    }

    bool show = false;
    bool histeq = false;

    // Get video parameters
    long int fps = static_cast<int>(cap.get(CAP_PROP_FPS));
    int total_frames = cap.get(CAP_PROP_FRAME_COUNT);
    int frame_width = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
    int frame_height = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));

    // Determine the duration
    float seconds;
    if (max_seconds <= 0)
        seconds = total_frames * fps;
    else
        seconds = max_seconds / 2;
    int num_frames = fps * seconds;
    if (total_frames < num_frames)
        num_frames = total_frames;

    // Print parameters
    cout << "Processing file " << input_name << endl;
    cout << "   Output file = " << output_name << endl;
    cout << "   Angle rotation = " << angle_rotation << endl;
    cout << "   Contrast factor = " << contrast_factor << endl;
    cout << "   Duration >= " << max_seconds << endl;
    

    Size frame_size(frame_width, frame_height);

    Mat first_frame;
    bool success = cap.read(first_frame);
    if (!success) {
        cout << "Cannot open the video file " << input_name << endl;
        cin.get();
        return 0;
    }

    // Set index to first frame
    cap.set(CAP_PROP_POS_FRAMES, 0); 

    // Set rotation parameters
    int scale = 1;
    int iImageCenterY = first_frame.rows / 2;
    int iImageCenterX = first_frame.cols / 2;
    int border_mode = 0;

    Point2f center((first_frame.cols - 1) / 2, (first_frame.rows - 1) / 2);
    Rect2f bbox = RotatedRect(center, first_frame.size(), angle_rotation).boundingRect2f();
    Mat rotation = getRotationMatrix2D(center, angle_rotation, scale);
    rotation.at<double>(0, 2) += bbox.width / 2.0 - first_frame.cols / 2.0;
    rotation.at<double>(1, 2) += bbox.height / 2.0 - first_frame.rows / 2.0;

    // Create and initialize the VideoWriter object 
    VideoWriter oVideoWriter(output_name, VideoWriter::fourcc('M', 'P', '4', 'V'), fps, bbox.size(), true);


    // Load frames
    vector<Mat> final_frames;
    vector<double> score;
    int idx_frame = 0;
    while (idx_frame != num_frames)
    {
        Mat frame;
        cap.read(frame);

        final_frames.push_back(frame.clone());

        idx_frame++;
    }


    int frame_window = fps/4;
    int ini = 0 + frame_window;
    int end = fps*2; //time slot where the algorithm will search the first frame
    int first_idx;
    int last_idx;
    double single_score;
    cout << "Number of frames " << end << endl;
    score.clear();
    for (int i = ini; i < end - frame_window; i++)
    {
        single_score = 0;
        for (int j = 1; j <= frame_window; j++)
            single_score += norm(final_frames[i - j], final_frames[i + j]);
        score.push_back(single_score);
    }
    // Compute the minimum score, i.e. the closest frame to the first one
    auto smallest = min_element(score.begin()+frame_window+1, score.end());
    first_idx = static_cast<int>(distance(score.begin(), smallest) + ini);

    cout << "First frame index " << first_idx << endl;


    // Declare first and last frame of our boomerang video
    ini = first_idx + 1;
    end = num_frames;
    
    score.clear();
    
    for (int i = ini; i < end; i++)
    {
        score.push_back(norm(final_frames[first_idx], final_frames[i]));
    }
    smallest = min_element(score.begin() + 5, score.end());
    last_idx = static_cast<int>(distance(score.begin(), smallest) + ini);

    cout << "Last frame index " << last_idx << endl;

    // Write frame-by-frame to the file
    Mat new_frame;
    vector<Mat> first_half;
    for (int i = 0; i < last_idx; i++)
    {
        new_frame = change_contrast(final_frames[i], contrast_factor);
        new_frame = rotate_angle(new_frame, rotation, bbox, border_mode);
        first_half.push_back(new_frame);
        oVideoWriter.write(new_frame);
    }

    // Write the other half reversely
    for (int i = last_idx - 1; i >= 0; i--)
        oVideoWriter.write(first_half[i]);

    cap.release();
    oVideoWriter.release();

    destroyAllWindows();

    return 0;

}


