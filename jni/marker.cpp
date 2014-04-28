/*
 * Copyright (C) 2014 Miguel Angel Astor Romero
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <algorithm>
#include <utility>
#include <limits>
#ifdef DESKTOP
#include <iostream>
#endif

#include "marker.hpp"

namespace nxtar{

typedef std::vector<cv::Point3f>             points_vector_3D;
typedef std::vector<std::vector<cv::Point> > contours_vector;
typedef std::vector<Marker>                  markers_vector;

/******************************************************************************
 * PRIVATE CONSTANTS                                                          *
 ******************************************************************************/

/**
 * Minimal number of points in a contour.
 */
static const int MIN_POINTS = 40;

/**
 * Size of a square cell in the calibration pattern.
 */
static const float SQUARE_SIZE = 1.0f;

/**
 * Minimal lenght of a contour to be considered as a marker candidate.
 */
static const float MIN_CONTOUR_LENGTH = 0.1;

/**
 * Flags for the calibration pattern detecion function.
 */
static const int PATTERN_DETECTION_FLAGS = cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK;

/**
 * Color for rendering the marker outlines.
 */
static const cv::Scalar COLOR = cv::Scalar(255, 255, 255);

/**
 * Size of the chessboard pattern image (columns, rows).
 */
static const cv::Size CHESSBOARD_PATTERN_SIZE(6, 9);

/**
 * Termination criteria for OpenCV's iterative algorithms.
 */
static const cv::TermCriteria TERM_CRITERIA = cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1);

/******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                *
 ******************************************************************************/

float perimeter(points_vector &);

int hammDistMarker(cv::Mat);

cv::Mat rotate(cv::Mat);

int decodeMarker(cv::Mat &);

void renderMarkers(markers_vector &, cv::Mat &);

void isolateMarkers(const contours_vector &, markers_vector &);

void findContours(cv::Mat &, contours_vector &, int);

void warpMarker(Marker &, cv::Mat &, cv::Mat &);

/******************************************************************************
 * PUBLIC API                                                                 *
 ******************************************************************************/

void getAllMarkers(std::vector<int> & codes, cv::Mat & img){
	cv::Mat gray, thresh, cont, mark;
	contours_vector contours;
	markers_vector markers;
	markers_vector valid_markers;
#ifdef DESKTOP
	std::ostringstream oss;
#endif

	codes.clear();

	// Find all marker candidates in the input image.
	//   1) First, convert the image to grayscale.
	//   2) Then, binarize the grayscale image.
	//   3) Finally indentify all 4 sided figures in the binarized image.
	cv::cvtColor(img, gray, CV_BGR2GRAY);
	cv::adaptiveThreshold(gray, thresh, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, 7, 7);
	findContours(thresh, contours, MIN_POINTS);
	isolateMarkers(contours, markers);

	// Remove the perspective distortion from the detected marker candidates.
	// Then attempt to decode them and push the valid ones into the valid
	// markers vector.
	for(int i = 0; i < markers.size(); i++){
		warpMarker(markers[i], gray, mark);

		int code = decodeMarker(mark);

		if(code != -1){
			markers[i].code = code;
			valid_markers.push_back(markers[i]);
		}
	}

	for(int i = 0; i < valid_markers.size(); i++){
#ifdef DESKTOP
		// Render the detected valid markers with their codes for debbuging
		// purposes.
		oss << valid_markers[i].code;

		cv::putText(mark, oss.str(), cv::Point(5, 250), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar::all(128), 3, 8);

		oss.str("");
		oss.clear();

		oss << "Marker[" << i << "]";

		cv::imshow(oss.str(), mark);

		oss.str("");
		oss.clear();
#endif
		// Fix the detected corners to better approximate the markers. And
		// push their codes to the output vector.
		cv::cornerSubPix(gray, valid_markers[i].points, cvSize(10, 10), cvSize(-1, -1), TERM_CRITERIA);
		codes.push_back(valid_markers[i].code);
	}

	// Render the detected markers on top of the input image.
	cont = cv::Mat::zeros(img.size(), CV_8UC3);
	renderMarkers(valid_markers, cont);
	img = img + cont;

	// Clear the local vectors.
	markers.clear();
	contours.clear();
	valid_markers.clear();
}

bool findCalibrationPattern(points_vector & corners, cv::Mat & img){
	bool patternfound;
	cv::Mat gray;

	// Convert the input image to grayscale and attempt to find the
	// calibration pattern.
	cv::cvtColor(img, gray, CV_BGR2GRAY);
	patternfound = cv::findChessboardCorners(gray, CHESSBOARD_PATTERN_SIZE, corners, PATTERN_DETECTION_FLAGS);

	// If the pattern was found then fix the detected points a bit.
	if(patternfound)
		cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), TERM_CRITERIA);

	// Render the detected pattern.
	cv::drawChessboardCorners(img, CHESSBOARD_PATTERN_SIZE, cv::Mat(corners), patternfound);

	return patternfound;
}

double getCameraParameters(cv::Mat & camera_matrix, cv::Mat & dist_coeffs, std::vector<points_vector> & image_points, cv::Size image_size){
	std::vector<cv::Mat> rvecs, tvecs;
	std::vector<points_vector_3D> object_points;
	points_vector_3D corner_points;

	// Build the reference object points vector.
	for(int i = 0; i < CHESSBOARD_PATTERN_SIZE.height; i++){
		for(int j = 0; j < CHESSBOARD_PATTERN_SIZE.width; j++){
			corner_points.push_back(cv::Point3f(float( j * SQUARE_SIZE ), float( i * SQUARE_SIZE ), 0));
		}
	}
	object_points.push_back(corner_points);
	object_points.resize(image_points.size(), object_points[0]);

	// Build a camera matrix.
	camera_matrix = cv::Mat::eye(3, 3, CV_64F);

	// Build the distortion coefficients matrix.
	dist_coeffs = cv::Mat::zeros(8, 1, CV_64F);

	// Calibrate and return the reprojection error.
	return cv::calibrateCamera(object_points, image_points, image_size, camera_matrix, dist_coeffs, rvecs, tvecs, 0, TERM_CRITERIA);
}

/******************************************************************************
 * PRIVATE HELPER FUNCTIONS                                                   *
 ******************************************************************************/

/**
 *  Find all contours in the input image and save them to the output
 * vector.
 */
void findContours(cv::Mat & img, contours_vector & v, int minP){
	contours_vector c;

	// A contour is discarded if it possess less than the specified
	// minimum number of points.
	cv::findContours(img, c, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

	v.clear();
	for(size_t i = 0; i < c.size(); i++){
		if(c[i].size() > minP){
			v.push_back(c[i]);
		}
	}
}

/**
 * Render the input marker vector onto the output image.
 */
void renderMarkers(markers_vector & v, cv::Mat & dst){
	contours_vector cv;

	// Extract the points that form every marker into a contours vector.
	for(size_t i = 0; i < v.size(); i++){
		std::vector<cv::Point> pv;
		for(size_t j = 0; j < v[i].points.size(); ++j)
			pv.push_back(cv::Point2f(v[i].points[j].x, v[i].points[j].y));
		cv.push_back(pv);
	}

	// Render.
	cv::drawContours(dst, cv, -1, COLOR, 1, CV_AA);
}

/**
 * Identify and return all marker candidates.
 */
void isolateMarkers(const contours_vector & vc, markers_vector & vm){
	std::vector<cv::Point> appCurve;
	markers_vector posMarkers;

	// For every detected contour.
	for(size_t i = 0; i < vc.size(); ++i){
		double eps = vc[i].size() * 0.05;

		// Approximate the contour with a polygon.
		cv::approxPolyDP(vc[i], appCurve, eps, true);

		// If the polygon is not a cuadrilateral then this is not a marker
		// candidate.
		if(appCurve.size() != 4 || !cv::isContourConvex(appCurve)) continue;

		// Calculate the lenght of the perimeter of this candidate. If it
		// is too short then discard it.
		float minD = std::numeric_limits<float>::max();

		for(int i = 0; i < 4; i++){
			cv::Point side = appCurve[i] - appCurve[(i + 1) % 4];
			float sqSideLen = side.dot(side);
			minD = std::min(minD, sqSideLen);
		}

		if(minD < MIN_CONTOUR_LENGTH) continue;

		// Save the marker and order it's points counter-clockwise.
		Marker m;

		for(int i = 0; i < 4; i++)
			m.points.push_back(cv::Point2f(appCurve[i].x, appCurve[i].y));

		cv::Point v1 = m.points[1] - m.points[0];
		cv::Point v2 = m.points[2] - m.points[0];

		double o = (v1.x * v2.y) - (v1.y * v2.x);
		if(o < 0.0) std::swap(m.points[1], m.points[3]);

		posMarkers.push_back(m);
	}

	// Identify contours that are to close to each other to eliminate
	// possible duplicates.
	std::vector<std::pair<int, int> > tooNear;
	for(size_t i = 0; i < posMarkers.size(); ++i){
		const Marker & m1 = posMarkers[i];

		for(size_t j = i + 1; j < posMarkers.size(); j++){
			const Marker & m2 = posMarkers[j];

			float dSq = 0.0f;

			for(int c = 0; c < 4; c++){
				cv::Point v = m1.points[c] - m2.points[c];
				dSq += v.dot(v);
			}

			dSq /= 4.0f;

			if(dSq < 100) tooNear.push_back(std::pair<int, int>(i, j));
		}
	}

	// Mark one of every pair of duplicates to be discarded.
	std::vector<bool> remMask(posMarkers.size(), false);
	for(size_t i = 0; i < tooNear.size(); ++i){
		float p1 = perimeter(posMarkers[tooNear[i].first].points);
		float p2 = perimeter(posMarkers[tooNear[i].second].points);

		size_t remInd;
		if(p1 > p2) remInd = tooNear[i].second;
		else remInd = tooNear[i].first;

		remMask[remInd] = true;
	}

	// Save the candidates that survided the duplicates test.
	vm.clear();
	for(size_t i = 0; i < posMarkers.size(); ++i){
		if(!remMask[i]) vm.push_back(posMarkers[i]);
	}
}

/**
 * Warp a marker image to remove it's perspective distortion.
 */
void warpMarker(Marker & m, cv::Mat & in, cv::Mat & out){
	cv::Mat bin;
	cv::Size markerSize(350, 350);
	points_vector v;

	// Assemble a unitary reference polygon.
	v.push_back(cv::Point2f(0,0));
	v.push_back(cv::Point2f(markerSize.width-1,0));
	v.push_back(cv::Point2f(markerSize.width-1,markerSize.height-1));
	v.push_back(cv::Point2f(0,markerSize.height-1));

	// Warp the input image's perspective to transform it into the reference
	// polygon's perspective.
	cv::Mat M = cv::getPerspectiveTransform(m.points, v);
	cv::warpPerspective(in, bin, M, markerSize);

	// Binarize the warped image into the output image.
	cv::threshold(bin, out, 128, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
}

/**
 * Calculate the hamming distance of a 5x5 bit matrix.
 */
int hammDistMarker(cv::Mat bits){
	int ids[4][5] = {
			{1,0,0,0,0},
			{1,0,1,1,1},
			{0,1,0,0,1},
			{0,1,1,1,0}
	};

	int dist = 0;

	for (int y = 0; y < 5; y++){
		int minSum = 1e5;

		for (int p = 0; p < 4; p++){
			int sum = 0;

			for (int x = 0; x < 5; x++){
				sum += bits.at<uchar>(y, x) == ids[p][x] ? 0 : 1;
			}

			if(minSum > sum)
				minSum = sum;
		}

		dist += minSum;
	}

	return dist;
}

/**
 * Rotate a matrix by 90 degrees clockwise.
 */
cv::Mat rotate(cv::Mat in){
	cv::Mat out;

	in.copyTo(out);
	for (int i=0;i<in.rows;i++){
		for (int j=0;j<in.cols;j++){
			out.at<uchar>(i,j)=in.at<uchar>(in.cols-j-1,i);
		}
	}

	return out;
}

/**
 * Decode a marker image and return it's code. Returns -1 if the image is
 * not a valid marker.
 */
int decodeMarker(cv::Mat & marker){
	bool found = false;
	int code = 0;
	cv::Mat bits;

	// Verify that the outer rim of marker cells are all black.
	for(int y = 0; y < 7; y++){
		int inc = (y == 0 || y == 6) ? 1 : 6;

		for(int x = 0; x < 7; x += inc){
			int cX = x * 50;
			int cY = y * 50;
			cv::Mat cell = marker(cv::Rect(cX, cY, 50, 50));

			int nZ = cv::countNonZero(cell);

			// If one of the rim cells is 50% white or more then this
			// is not a valid marker.
			if(nZ > (50 * 50) / 2) return -1;
		}
	}

	// Create a 5x5 matrix to hold a simplified representation of this
	// marker.
	bits = cv::Mat::zeros(5, 5, CV_8UC1);

	// For every cell in the marker flip it's corresponding 'bit' in the
	// bit matrix if it is at least 50 % white.
	for(int y = 0; y < 5; y++){
		for(int x = 0; x < 5; x++){
			int cX = (x + 1) * 50;
			int cY = (y + 1) * 50;
			cv::Mat cell = marker(cv::Rect(cX, cY, 50, 50));
			int nZ = cv::countNonZero(cell);

			if(nZ > (50 * 50) / 2) bits.at<uchar>(y, x) = 1;
		}
	}

	// Calcultate the hamming distance of the bit matrix and each of it's
	// 90 degree rotations to determine if this marker has a valid code.
	if(hammDistMarker(bits) != 0){
		for(int r = 1; r < 4; r++){
			bits = rotate(bits);
			if(hammDistMarker(bits) != 0) continue;
			else{ found = true; break;}
		}
	}else found = true;

	// If the code is valid then decode it to an int and return it.
	if(found){
		for(int y = 0; y < 5; y++){
			code <<= 1;
			if(bits.at<uchar>(y, 1))
				code |= 1;

			code <<= 1;
			if(bits.at<uchar>(y, 3))
				code |= 1;
		}


		return code;
	}else
		return -1;
}

/**
 * Calculate the perimeter of a polygon defined as a vector of points.
 */
float perimeter(points_vector & p){
	float per = 0.0f, dx, dy;

	for(size_t i; i < p.size(); ++i){
		dx = p[i].x - p[(i + 1) % p.size()].x;
		dy = p[i].y - p[(i + 1) % p.size()].y;
		per += sqrt((dx * dx) + (dy * dy));
	}

	return per;
}

/******************************************************************************
 * CLASS METHODS                                                              *
 ******************************************************************************/

/**
 * Clear the points vector associated with this marker.
 */
Marker::~Marker(){
	points.clear();
}
}
