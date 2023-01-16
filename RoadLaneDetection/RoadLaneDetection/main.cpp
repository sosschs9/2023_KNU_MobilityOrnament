#include <opencv2/highgui/highgui.hpp>
#include <opencv2\opencv.hpp>
#include "RoadLaneDetector.h"
#include <iostream>
#include <string>
#include <vector>

using namespace cv;
using namespace std;

int main()
{
	RoadLaneDetector roadLaneDetector;
	Mat img_frame, img_filter, img_edges, img_mask, img_lines, img_result;
	vector<Vec4i> lines;
	vector<vector<Vec4i> > separated_lines;
	vector<Point> lane;
	string dir;

	VideoCapture video("change.mp4");
	if (!video.isOpened()) {
		cout << "������ ������ �� �� �����ϴ�. \n" << endl;
		return -1;
	}

	video.read(img_frame);
	if (img_frame.empty()) return -1;

	VideoWriter writer;
	int codec = VideoWriter::fourcc('D', 'I', 'V', 'X');
	double fps = video.get(CAP_PROP_FPS);
	string filename = "./result.avi";

	writer.open(filename, codec, fps, img_frame.size(), CV_8UC3);
	if (!writer.isOpened()) {
		cout << "����� ���� ���� ������ �� �� �����ϴ�. \n";
		return -1;
	}

	while (1) {
		// 1. ���� ���� �о����
		if (!video.read(img_frame)) break;

		// 2. ������ �ش��ϴ� ���� ������ �ִ� �͸� ���͸��Ͽ� �ĺ��� ����
		img_filter = roadLaneDetector.filter_colors(img_frame);
		// 3. ������ GrayScale�� ��ȯ
		cvtColor(img_filter, img_filter, COLOR_BGR2GRAY);
		// 4. Canny Edge Detection���� edge�� ����
		Canny(img_filter, img_edges, 100, 120);
		// 5. �ڵ����� ���� ���� �ٴڿ��� �����ϴ� ���� ������ ���� ���� ���� ����
		img_mask = roadLaneDetector.limit_region(img_edges);
		// 6. Hough ��ȯ���� edge���� ���� ������ ����
		lines = roadLaneDetector.houghLines(img_mask);

		if (lines.size() > 0) {
			// 7. ���⸦ ���� �¿�� ���� �� ���� ������ �¿� ���� ã��
			separated_lines = roadLaneDetector.separateLine(img_mask, lines);
			lane = roadLaneDetector.regression(separated_lines, img_frame);

			// 8. ���� ���� �ν��� ������ �׸��� ������ ������Ż ��� ��� ǥ�� ����
			img_result = roadLaneDetector.drawLine(img_frame, lane);
		}
		writer << img_result;
		imshow("result", img_result);

		if (waitKey(1) == 27)
			break;
	}

	return 0;
}