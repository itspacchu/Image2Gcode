#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include<iostream>
#define IOSTREAM_H
#include "gcode.cpp"

#define THRESH_BW 50
#define GAUSSIAN_SIZE 7
#define SCALE_FACTOR_MM 100

enum TYPES {
    TYPE_LINEART = 0,
    TYPE_ANIME = 1,
}; // implement adaptive for different inputs

typedef struct {
    float xmm;
    float ymm;
} Gpoint;

using namespace cv;


int main(int argc, char** argv){
    Gpoint pen_offset = {40,0}; //mm
    Mat original,dstn;
    std::string filename = argv[1];
    original = imread(filename, IMREAD_GRAYSCALE);
    std::cout << "Loaded : " << filename << std::endl;
    if(original.empty()){
        std::cout << "Image not found at :" << filename << std::endl;
        std::cout << "Image2Gcode <filename>" << std::endl;
        return 1;
    }

    Mat LowFreqDetails,MidFreqDetails,HighFreqDetails;
    Mat buff;

    equalizeHist(original,original);
    GaussianBlur(original,original,Size(GAUSSIAN_SIZE,GAUSSIAN_SIZE),0,0);
    threshold(original,original,100,255,THRESH_BINARY_INV);
    dilate(original,MidFreqDetails,Mat(),Point(-1,-1),2);
    morphologyEx(MidFreqDetails,MidFreqDetails,MORPH_HITMISS,Mat(),Point(-1,-1),2);
    
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours( MidFreqDetails, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE );
    
    cv::Mat contourImage(MidFreqDetails.size(), CV_8UC3, cv::Scalar(0,0,0));
    std::cout << "Found " << contours.size() << " contours" << std::endl;
    for(int i = 0; i < contours.size(); i++){
        cv::drawContours(contourImage, contours, i, cv::Scalar(255,255,255), 1, 8, std::vector<cv::Vec4i>(), 0, cv::Point());
    }
    cv::imshow("Contours", contourImage);
    std::cout << "Done Processing Image Contours\n" << std::endl;
    std::cout << "=== GCODE SETTINGS ===" << std::endl;
    std::cout << "Scale Factor (mm/pixel): " << SCALE_FACTOR_MM << std::endl;
    std::cout << "Pen Offset: " << pen_offset.xmm << "mm, " << pen_offset.ymm << "mm" << std::endl;
    std::cout << "=== END SETTINGS ===" << std::endl;
    
    //Normalize Contour coordinates to lie between 0 and 1 and scale it
    std::vector<std::vector<Gpoint>> normalizedContours;
    for(int i = 0; i < contours.size(); i++){
        std::vector<Gpoint> normalizedContour;
        for(int j = 0; j < contours[i].size(); j++){
            Gpoint normalizedPoint;
            normalizedPoint.xmm = pen_offset.xmm + (SCALE_FACTOR_MM * contours[i][j].x / (double)original.cols);
            normalizedPoint.ymm = pen_offset.ymm + (SCALE_FACTOR_MM * contours[i][j].y / (double)original.rows);
            normalizedContour.push_back(normalizedPoint);
        }
        normalizedContours.push_back(normalizedContour);
    }

    GCODE_Generator gcode;
    gcode.use_default_header(); // set predefined header
    for(int i=0;i<normalizedContours.size();i++){
        gcode.G1(-1,-1,0.0,1200); //Pen Down
        for(int j=0;j<normalizedContours[i].size();j++){
            // No Feedrate G0 Linear move
            gcode.G0(normalizedContours[i][j].xmm ,normalizedContours[i][j].ymm); // G1 Linear Move
        }
        gcode.G1(-1,-1,5.5,1200); //Lift Pen Up
    }
    gcode.use_default_footer(); // auto home after sketching
    gcode.save_file("/home/pac/Documents/secretstuff/Image2Gcode/test.gcode");
    int k = waitKey(0);
    return 0;
}