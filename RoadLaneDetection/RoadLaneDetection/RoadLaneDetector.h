#pragma once
#include <opencv2/highgui/highgui.hpp>
#include<iostream>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

class RoadLaneDetector
{
private:
	double img_size, img_center;
	double left_m, right_m;
	Point left_b, right_b;
	bool left_detect = false, right_detect = false;

	double poly_bottom_width = 0.85;
	double poly_top_width = 0.07;
	double poly_height = 0.4;


public:
	Mat filter_colors(Mat img_frame);
	Mat limit_region(Mat img_edges);
	vector<Vec4i> houghLines(Mat img_mask);
	vector<vector<Vec4i> > separateLine(Mat img_edges, vector<Vec4i> lines);
	vector<Point> regression(vector<vector<Vec4i> > separated_lines, Mat img_input);
	Mat drawLine(Mat img_input, vector<Point> lane);
};
