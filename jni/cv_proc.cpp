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

#include "marker.hpp"

#define CAN_LOG

extern "C"{
#ifdef CAN_LOG

#define log(TAG, MSG) (__android_log_write(ANDROID_LOG_DEBUG, TAG, MSG))

const char * TAG = "CVPROC_NATIVE";

#else

#define log(TAG, MSG) (1 + 1)

#endif

	JNIEXPORT void JNICALL Java_ve_ucv_ciens_ccg_nxtar_MainActivity_getMarkerCodesAndLocations(
		JNIEnv* env,
		jobject jobj,
		jint width,
		jint height,
		jbyteArray yuv,
		jintArray bgra,
		jintArray codes
	){
		char codeMsg[128];
		std::vector<int> vCodes;

		log(TAG, "Requesting native data.");

		jbyte*  _yuv   = env->GetByteArrayElements(yuv, 0);
		jint *  _bgra  = env->GetIntArrayElements(bgra, 0);
		jint *  _codes = env->GetIntArrayElements(codes, 0);

		log(TAG, "Converting native data to OpenCV Mats.");

		cv::Mat myuv(height + height/2, width, CV_8UC1, (unsigned char *)_yuv);
		cv::Mat mbgra(height, width, CV_8UC4, (unsigned char *)_bgra);

		log(TAG, "Converting color space before processing.");
		cv::Mat mbgr(height, width, CV_8UC3);
		cv::Mat gray;
		cv::cvtColor(myuv, mbgr, CV_YUV420sp2BGR);

		log(TAG, "Finding markers.");
		nxtar::getAllMarkers(vCodes, mbgr);

		log(TAG, "Copying marker codes.");
		for(int i = 0; i < vCodes.size() && i < 15; i++){
			_codes[i] = vCodes[i];
			sprintf(codeMsg, "Code [%d] = %d", i, vCodes[i]);
			log(TAG, codeMsg);
		}
		vCodes.clear();

		log(TAG, "Converting color space after processing.");
		cv::cvtColor(mbgr, gray, CV_BGR2GRAY);
		cv::cvtColor(gray, mbgra, CV_GRAY2BGRA, 4);

		log(TAG, "Releasing native data.");

		env->ReleaseIntArrayElements(codes, _codes, 0);
		env->ReleaseIntArrayElements(bgra, _bgra, 0);
		env->ReleaseByteArrayElements(yuv, _yuv, 0);
	}
}
