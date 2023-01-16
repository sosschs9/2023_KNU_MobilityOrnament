#include "opencv2/opencv.hpp"
#include <opencv2/highgui/highgui.hpp>
#include "RoadLaneDetector.h"
#include <iostream>
#include <string>
#include <vector>


Mat RoadLaneDetector::filter_colors(Mat img_frame) {
	/*
		���/����� ������ ������ ���� �ش�Ǵ� ������ ���͸��Ѵ�.
	*/
	Mat output;
	UMat img_hsv;
	UMat white_mask, white_image;
	UMat yellow_mask, yellow_image;
	img_frame.copyTo(output);

	//���� ���� ���� 
	Scalar lower_white = Scalar(140, 140, 140); //��� ���� (RGB)
	Scalar upper_white = Scalar(255, 255, 255);
	Scalar lower_yellow = Scalar(100, 50, 50); //�Ķ��� ���� (HSV)
	Scalar upper_yellow = Scalar(255, 100, 100);

	//��� ���͸�
	inRange(output, lower_white, upper_white, white_mask);
	bitwise_and(output, output, white_image, white_mask);

	cvtColor(output, img_hsv, COLOR_BGR2HSV);

	//����� ���͸�
	inRange(img_hsv, lower_yellow, upper_yellow, yellow_mask);
	bitwise_and(output, output, yellow_image, yellow_mask);

	//�� ������ ��ģ��.
	addWeighted(white_image, 1.0, yellow_image, 1.0, 0.0, output);
	return output;
}


Mat RoadLaneDetector::limit_region(Mat img_edges) {
	/*
		���� ������ �����ڸ��� �����ǵ��� ����ŷ�Ѵ�.
		���� ������ �����ڸ��� ǥ�õǴ� ���� ������ ��ȯ�Ѵ�.
	*/
	int width = img_edges.cols;
	int height = img_edges.rows;

	Mat output;
	Mat mask = Mat::zeros(height, width, CV_8UC1);

	//���� ���� ���� ���
	Point points[4]{
		Point((width * (1 - poly_bottom_width)) / 2, height),
		Point((width * (1 - poly_top_width)) / 2, height - height * poly_height),
		Point(width - (width * (1 - poly_top_width)) / 2, height - height * poly_height),
		Point(width - (width * (1 - poly_bottom_width)) / 2, height)
	};

	//�������� ���ǵ� �ٰ��� ������ ������ ä�� �׸���.
	fillConvexPoly(mask, points, 4, Scalar(255, 0, 0));

	//����� ��� ���� edges �̹����� mask�� ���Ѵ�.
	bitwise_and(img_edges, mask, output);
	return output;
}

vector<Vec4i> RoadLaneDetector::houghLines(Mat img_mask) {
	/*
		���ɿ������� ����ŷ �� �̹������� ��� ���� �����Ͽ� ��ȯ
	*/
	vector<Vec4i> line;

	//Ȯ������ ������ȯ ���� ���� �Լ� 
	HoughLinesP(img_mask, line, 2, CV_PI / 180, 50, 40, 100);
	return line;
}

vector<vector<Vec4i>> RoadLaneDetector::separateLine(Mat img_edges, vector<Vec4i> lines) {
	/*
		����� ��� ������ȯ �������� ���� ���� �����Ѵ�.
		���� ����� �뷫���� ��ġ�� ���� �¿�� �з��Ѵ�.
	*/

	vector<vector<Vec4i>> output(2);
	Point p1, p2;
	vector<double> slopes;
	vector<Vec4i> final_lines, left_lines, right_lines;
	double slope_thresh = 0.3;

	//����� �������� ���⸦ ���
	for (int i = 0; i < lines.size(); i++) {
		Vec4i line = lines[i];
		p1 = Point(line[0], line[1]);
		p2 = Point(line[2], line[3]);

		double slope;
		if (p2.x - p1.x == 0)  //�ڳ� �� ���
			slope = 999.0;
		else
			slope = (p2.y - p1.y) / (double)(p2.x - p1.x);

		//���Ⱑ �ʹ� ������ ���� ����
		if (abs(slope) > slope_thresh) {
			slopes.push_back(slope);
			final_lines.push_back(line);
		}
	}

	//������ �¿� ������ �з�
	img_center = (double)((img_edges.cols / 2));

	for (int i = 0; i < final_lines.size(); i++) {
		p1 = Point(final_lines[i][0], final_lines[i][1]);
		p2 = Point(final_lines[i][2], final_lines[i][3]);

		if (slopes[i] > 0 && p1.x > img_center && p2.x > img_center) {
			right_detect = true;
			right_lines.push_back(final_lines[i]);
		}
		else if (slopes[i] < 0 && p1.x < img_center && p2.x < img_center) {
			left_detect = true;
			left_lines.push_back(final_lines[i]);
		}
	}

	output[0] = right_lines;
	output[1] = left_lines;
	return output;
}

vector<Point> RoadLaneDetector::regression(vector<vector<Vec4i>> separatedLines, Mat img_input) {
	/*
		���� ȸ�͸� ���� �¿� ���� ������ ���� ������ ���� ã�´�.
	*/
	vector<Point> output(4);
	Point p1, p2, p3, p4;
	Vec4d left_line, right_line;
	vector<Point> left_points, right_points;

	if (right_detect) {
		for (auto i : separatedLines[0]) {
			p1 = Point(i[0], i[1]);
			p2 = Point(i[2], i[3]);

			right_points.push_back(p1);
			right_points.push_back(p2);
		}

		if (right_points.size() > 0) {
			//�־��� contour�� ����ȭ�� ���� ����
			fitLine(right_points, right_line, DIST_L2, 0, 0.01, 0.01);

			right_m = right_line[1] / right_line[0];  //����
			right_b = Point(right_line[2], right_line[3]);
		}
	}

	if (left_detect) {
		for (auto j : separatedLines[1]) {
			p3 = Point(j[0], j[1]);
			p4 = Point(j[2], j[3]);

			left_points.push_back(p3);
			left_points.push_back(p4);
		}

		if (left_points.size() > 0) {
			//�־��� contour�� ����ȭ�� ���� ����
			fitLine(left_points, left_line, DIST_L2, 0, 0.01, 0.01);

			left_m = left_line[1] / left_line[0];  //����
			left_b = Point(left_line[2], left_line[3]);
		}
	}

	//�¿� �� ������ �� ���� ����Ѵ�.
	//y = m*x + b  --> x = (y-b) / m
	int y1 = img_input.rows;
	int y2 = 270;

	double right_x1 = ((y1 - right_b.y) / right_m) + right_b.x;
	double right_x2 = ((y2 - right_b.y) / right_m) + right_b.x;

	double left_x1 = ((y1 - left_b.y) / left_m) + left_b.x;
	double left_x2 = ((y2 - left_b.y) / left_m) + left_b.x;

	output[0] = Point(right_x1, y1);
	output[1] = Point(right_x2, y2);
	output[2] = Point(left_x1, y1);
	output[3] = Point(left_x2, y2);

	return output;
}

Mat RoadLaneDetector::drawLine(Mat img_input, vector<Point> lane) {
	/*
		���� ��Ż �� ��� ǥ�� ����
		�¿� ������ ���� ������ �׸���.
	*/

	vector<Point> poly_points;
	Mat output;
	img_input.copyTo(output);

	poly_points.push_back(lane[2]);
	poly_points.push_back(lane[0]);
	poly_points.push_back(lane[1]);
	poly_points.push_back(lane[3]);

	addWeighted(output, 0.3, img_input, 0.7, 0, img_input);  //���� ���ϱ�

	double offset;
	double a = 180 - lane[2].x;
	double b = lane[0].x - 180;
	double width = lane[0].x - lane[2].x;

	if (a >= b) offset = a / width * 3.7 - 3.7 / 2.0;
	else offset = 3.7 / 2.0 - b / width * 3.7;

	if (abs(offset) > 1.5)
		putText(img_input, "Warning", Point(120, 80), FONT_HERSHEY_SIMPLEX, 3, Scalar(255, 255, 255), 3, LINE_AA);

	//�¿� ���� �� �׸���
	line(img_input, lane[0], lane[1], Scalar(0, 255, 255), 5, LINE_AA);
	line(img_input, lane[2], lane[3], Scalar(0, 255, 255), 5, LINE_AA);

	return img_input;
}