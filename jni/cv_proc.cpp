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

#define CAN_LOG
#define POINTS_PER_CALIBRATION_SAMPLE 54
#define CALIBRATION_SAMPLES 10

#ifdef CAN_LOG
#define log(TAG, MSG) (__android_log_write(ANDROID_LOG_DEBUG, TAG, MSG))
#else
#define log(TAG, MSG) (1 + 1)
#endif

const char * TAG = "CVPROC_NATIVE";

extern "C"{

/**
 *
 */
JNIEXPORT void JNICALL Java_ve_ucv_ciens_ccg_nxtar_MainActivity_getMarkerCodesAndLocations(JNIEnv* env, jobject jobj, jlong addrMatIn, jlong addrMatOut, jintArray codes){
	char codeMsg[128];
	std::vector<int> vCodes;

	log(TAG, "getMarkerCodesAndLocations(): Requesting native data.");

	cv::Mat& myuv  = *(cv::Mat*)addrMatIn;
	cv::Mat& mbgr  = *(cv::Mat*)addrMatOut;
	jint * _codes = env->GetIntArrayElements(codes, 0);
	cv::Mat temp;

	log(TAG, "getMarkerCodesAndLocations(): Converting color space before processing.");
	cv::cvtColor(myuv, temp, CV_RGB2BGR);

	log(TAG, "getMarkerCodesAndLocations(): Finding markers.");
	nxtar::getAllMarkers(vCodes, temp);

	log(TAG, "getMarkerCodesAndLocations(): Copying marker codes.");
	for(int i = 0; i < vCodes.size() && i < 15; i++){
		_codes[i] = (jint)vCodes[i];
	}
	vCodes.clear();

	cv::cvtColor(temp, mbgr, CV_BGR2RGB);

	log(TAG, "getMarkerCodesAndLocations(): Releasing native data.");
	env->ReleaseIntArrayElements(codes, _codes, 0);
}

/**
 *
 */
JNIEXPORT jboolean JNICALL Java_ve_ucv_ciens_ccg_nxtar_MainActivity_findCalibrationPattern(JNIEnv* env, jobject jobj, jlong addrMatIn, jlong addrMatOut, jfloatArray points){
	nxtar::points_vector v_points;
	bool found;

	log(TAG, "findCalibrationPattern(): Requesting native data.");

	cv::Mat& myuv  = *(cv::Mat*)addrMatIn;
	cv::Mat& mbgr  = *(cv::Mat*)addrMatOut;
	jfloat * _points = env->GetFloatArrayElements(points, 0);
	cv::Mat temp;

	log(TAG, "findCalibrationPattern(): Converting color space before processing.");
	cv::cvtColor(myuv, temp, CV_RGB2BGR);

	log(TAG, "findCalibrationPattern(): Finding calibration pattern.");
	found = nxtar::findCalibrationPattern(v_points, temp);

	log(TAG, "findCalibrationPattern(): Copying calibration points.");
	for(size_t i = 0, p = 0; i < v_points.size(); i++, p += 2){
		_points[p] = (jfloat)v_points[i].x;
		_points[p + 1] = (jfloat)v_points[i].y;
	}

	cv::cvtColor(temp, mbgr, CV_BGR2RGB);

	log(TAG, "findCalibrationPattern(): Releasing native data.");
	env->ReleaseFloatArrayElements(points, _points, 0);

	return (jboolean)found;
}

/**
 *
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

}
