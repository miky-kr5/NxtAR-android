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

class Marker;

typedef std::vector<cv::Point2f> points_vector;
typedef std::vector<Marker>      markers_vector;

/**
 * A class representing a marker with the geometric transformations needed to
 * render something on top of it.
 */
class Marker{
public:
	~Marker();

	int           code;
	points_vector points;
	cv::Mat       translation;
	cv::Mat       rotation;
};

/**
 * Detect all 5x5 markers in the input image and return their codes in the
 * output vector.
 */
void getAllMarkers(markers_vector &, cv::Mat &);

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

/**
 * Obtains the necesary geometric transformations necessary to move a reference
 * unitary polygon to the position and rotation of the markers passed as input.
 * The obtained transformations are given relative to a camera centered in the
 * origin and are saved inside the input markers.
 */
void estimateMarkerPosition(markers_vector &, cv::Mat &, cv::Mat &);

} // namespace nxtar

#endif // MARKER_HPP
