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
		cout << "동영상 파일을 열 수 없습니다. \n" << endl;
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
		cout << "출력을 위한 비디오 파일을 열 수 없습니다. \n";
		return -1;
	}

	while (1) {
		// 1. 원본 영상 읽어오기
		if (!video.read(img_frame)) break;

		// 2. 차선에 해당하는 색상 범위에 있는 것만 필터링하여 후보로 저장
		img_filter = roadLaneDetector.filter_colors(img_frame);
		// 3. 영상을 GrayScale로 변환
		cvtColor(img_filter, img_filter, COLOR_BGR2GRAY);
		// 4. Canny Edge Detection으로 edge를 추출
		Canny(img_filter, img_edges, 100, 120);
		// 5. 자동차의 진행 방향 바닥에만 존재하는 차선 검출을 위해 관심 영역 지정
		img_mask = roadLaneDetector.limit_region(img_edges);
		// 6. Hough 변환으로 edge에서 직선 성분을 추출
		lines = roadLaneDetector.houghLines(img_mask);

		if (lines.size() > 0) {
			// 7. 기울기를 통해 좌우로 나눈 후 가장 적합한 좌우 차선 찾기
			separated_lines = roadLaneDetector.separateLine(img_mask, lines);
			lane = roadLaneDetector.regression(separated_lines, img_frame);

			// 8. 영상에 최종 인식한 차선을 그리고 차량이 차선이탈 경우 경고 표시 띄우기
			img_result = roadLaneDetector.drawLine(img_frame, lane);
		}
		writer << img_result;
		imshow("result", img_result);

		if (waitKey(1) == 27)
			break;
	}

	return 0;
}