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
#ifndef MARKER_HPP
#define MARKER_HPP

#include <vector>

#include <opencv2/opencv.hpp>

namespace nxtar{

typedef std::vector<cv::Point2f> points_vector;

class Marker{
public:
	~Marker();
	points_vector points;
	int code;
};

/**
 * Detect all 5x5 markers in the input image and return their codes in the
 * output vector.
 */
void getAllMarkers(std::vector<int> &, cv::Mat &);

/**
 * Find a chessboard calibration pattern in the input image. Returns true
 * if the pattern was found, false otherwise. The calibration points
 * detected on the image are saved in the output vector.
 */
bool findCalibrationPattern(points_vector &, cv::Mat &);

/**
 * Sets the camera matrix and the distortion coefficients for the camera
 * that captured the input image points into the output matrices. Returns
 * the reprojection error as returned by cv::calibrateCamera.
 */
double getCameraParameters(cv::Mat &, cv::Mat &, std::vector<points_vector> &, cv::Size);

} // namespace nxtar

#endif // MARKER_HPP
