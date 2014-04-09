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

typedef std::vector<std::vector<cv::Point> > contours_vector;
typedef std::vector<cv::Point2f>             points_vector;
typedef std::vector<Marker>                  markers_vector;

class Marker{
public:
	~Marker();
	points_vector points;
	int code;
};

void getAllMarkers(std::vector<int> &, cv::Mat &);
void calibrateCamera(cv::Mat &);
}

#endif
