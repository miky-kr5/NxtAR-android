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
#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <stddef.h>

#include "marker.hpp"

//#define LOG_ENABLED
#define POINTS_PER_CALIBRATION_SAMPLE 54
#define CALIBRATION_SAMPLES 10

#ifdef LOG_ENABLED
#define log(TAG, MSG) (__android_log_write(ANDROID_LOG_DEBUG, TAG, MSG))
#else
#define log(TAG, MSG) ;
#endif

const char * TAG = "CVPROC_NATIVE";

extern "C"{

/**
 * JNI wrapper around the nxtar::getAllMarkers() method.
 */
JNIEXPORT void JNICALL Java_ve_ucv_ciens_ccg_nxtar_MainActivity_getMarkerCodesAndLocations(JNIEnv* env, jobject jobj, jlong addrMatIn, jlong addrMatOut, jintArray codes){
	char codeMsg[128];
	std::vector<int> vCodes;
	cv::Mat temp;

	log(TAG, "getMarkerCodesAndLocations(): Requesting native data.");

	// Get the native object addresses.
	cv::Mat& myuv  = *(cv::Mat*)addrMatIn;
	cv::Mat& mbgr  = *(cv::Mat*)addrMatOut;
	jint * _codes = env->GetIntArrayElements(codes, 0);

	// Convert the input image to the BGR color space.
	log(TAG, "getMarkerCodesAndLocations(): Converting color space before processing.");
	cv::cvtColor(myuv, temp, CV_RGB2BGR);

	// Find all markers in the input image.
	log(TAG, "getMarkerCodesAndLocations(): Finding markers.");
	nxtar::getAllMarkers(vCodes, temp);

	// Copy the marker codes to the output vector.
	log(TAG, "getMarkerCodesAndLocations(): Copying marker codes.");
	for(int i = 0; i < vCodes.size() && i < 15; i++){
		_codes[i] = (jint)vCodes[i];
	}
	vCodes.clear();

	// Convert the output image back to the RGB color space.
	cv::cvtColor(temp, mbgr, CV_BGR2RGB);

	// Release native data.
	log(TAG, "getMarkerCodesAndLocations(): Releasing native data.");
	env->ReleaseIntArrayElements(codes, _codes, 0);
}

/**
 * JNI wrapper around the nxtar::findCalibrationPattern() method.
 */
JNIEXPORT jboolean JNICALL Java_ve_ucv_ciens_ccg_nxtar_MainActivity_findCalibrationPattern(JNIEnv* env, jobject jobj, jlong addrMatIn, jlong addrMatOut, jfloatArray points){
	nxtar::points_vector v_points;
	bool found;
	cv::Mat temp;

	log(TAG, "findCalibrationPattern(): Requesting native data.");

	// Get the native object addresses.
	cv::Mat& myuv  = *(cv::Mat*)addrMatIn;
	cv::Mat& mbgr  = *(cv::Mat*)addrMatOut;
	jfloat * _points = env->GetFloatArrayElements(points, 0);

	// Convert the input image to the BGR color space.
	log(TAG, "findCalibrationPattern(): Converting color space before processing.");
	cv::cvtColor(myuv, temp, CV_RGB2BGR);

	// Find the calibration points in the input image.
	log(TAG, "findCalibrationPattern(): Finding calibration pattern.");
	found = nxtar::findCalibrationPattern(v_points, temp);

	// If the points were found then save them to the output array.
	if(found){
		log(TAG, "findCalibrationPattern(): Copying calibration points.");
		for(size_t i = 0, p = 0; i < v_points.size(); i++, p += 2){
			_points[p] = (jfloat)v_points[i].x;
			_points[p + 1] = (jfloat)v_points[i].y;
		}
	}

	// Convert the output image back to the RGB color space.
	cv::cvtColor(temp, mbgr, CV_BGR2RGB);

	// Release native data.
	log(TAG, "findCalibrationPattern(): Releasing native data.");
	env->ReleaseFloatArrayElements(points, _points, 0);

	return (jboolean)found;
}

/**
 * JNI wrapper around the nxtar::getCameraParameters() method.
 */
JNIEXPORT jdouble JNICALL Java_ve_ucv_ciens_ccg_nxtar_MainActivity_calibrateCameraParameters(JNIEnv* env, jobject jobj, jlong addrMatIn, jlong addrMatOut, jlong addrMatFrame, jfloatArray points){
	double error;
	std::vector<nxtar::points_vector> imagePoints;

	// Get native object addresses.
	log(TAG, "calibrateCameraParameters(): Requesting native data.");
	cv::Mat& mIn  = *(cv::Mat*)addrMatIn;
	cv::Mat& mOut  = *(cv::Mat*)addrMatOut;
	cv::Mat& mFrame  = *(cv::Mat*)addrMatFrame;
	jfloat * _points = env->GetFloatArrayElements(points, 0);

	// Prepare the image points data structure.
	log(TAG, "calibrateCameraParameters(): Preparing image points.");
	for(int i = 0; i < CALIBRATION_SAMPLES; i++){
		nxtar::points_vector tempVector;
		for(int j = 0, p = 0; j < POINTS_PER_CALIBRATION_SAMPLE; j++, p += 2){
			tempVector.push_back(cv::Point2f(_points[p], _points[p + 1]));
		}
		imagePoints.push_back(tempVector);
	}

	// Get the camera parameters.
	log(TAG, "calibrateCameraParameters(): Getting camera parameters.");
	error = nxtar::getCameraParameters(mIn, mOut, imagePoints, mFrame.size());

	// Clear the image points.
	log(TAG, "calibrateCameraParameters(): Clearing memory.");
	for(int i = 0; i < imagePoints.size(); i++){
		imagePoints[i].clear();
	}
	imagePoints.clear();

	// Release native memory.
	log(TAG, "calibrateCameraParameters(): Releasing native data.");
	env->ReleaseFloatArrayElements(points, _points, 0);

	// Return the calibration error as calculated by OpenCV.
	return error;
}

} // extern "C"
