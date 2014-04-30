/*
 * Copyright (C) 2013 Miguel Angel Astor Romero
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
package ve.ucv.ciens.ccg.nxtar;

import java.io.ByteArrayOutputStream;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.Mat;
import org.opencv.imgproc.Imgproc;

import ve.ucv.ciens.ccg.nxtar.interfaces.CVProcessor;
import ve.ucv.ciens.ccg.nxtar.interfaces.OSFunctionalityProvider;
import ve.ucv.ciens.ccg.nxtar.utils.ProjectConstants;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;
import android.os.Bundle;
import android.os.Handler;
import android.widget.Toast;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.backends.android.AndroidApplication;
import com.badlogic.gdx.backends.android.AndroidApplicationConfiguration;
import com.badlogic.gdx.controllers.mappings.Ouya;

public class MainActivity extends AndroidApplication implements OSFunctionalityProvider, CVProcessor{
	private static final String TAG = "NXTAR_ANDROID_MAIN";
	private static final String CLASS_NAME = MainActivity.class.getSimpleName();

	private static final ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
	private static boolean ocvOn = false;
	private static Mat cameraMatrix, distortionCoeffs;

	private WifiManager wifiManager;
	private MulticastLock multicastLock;
	private Handler uiHandler;
	private Context uiContext;
	private BaseLoaderCallback loaderCallback;

	public native void getMarkerCodesAndLocations(long inMat, long outMat, int[] codes);
	public native boolean findCalibrationPattern(long inMat, long outMat, float[] points);
	public native double calibrateCameraParameters(long camMat, long distMat, long frame, float[] calibrationPoints);

	static{
		if(Ouya.runningOnOuya){
			if(!OpenCVLoader.initDebug())
				ocvOn = false;

			try{
				System.loadLibrary("cvproc");
				ocvOn = true;
			}catch(UnsatisfiedLinkError u){
				ocvOn = false;
			}
		}
	}

	@Override
	public void onCreate(Bundle savedInstanceState){
		super.onCreate(savedInstanceState);

		cameraMatrix = new Mat();
		distortionCoeffs = new Mat();

		if(!Ouya.runningOnOuya){
			setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
		}else{
			setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		}

		uiHandler = new Handler();
		uiContext = this;
		wifiManager = (WifiManager)getSystemService(Context.WIFI_SERVICE);

		AndroidApplicationConfiguration cfg = new AndroidApplicationConfiguration();
		cfg.useGL20 = true;
		cfg.useAccelerometer = false;
		cfg.useCompass = false;
		cfg.useWakelock = true;

		if(!ocvOn && !Ouya.runningOnOuya){
			loaderCallback = new BaseLoaderCallback(this){
				@Override
				public void onManagerConnected(int status){
					switch(status){
					case LoaderCallbackInterface.SUCCESS:
						System.loadLibrary("cvproc");
						ocvOn = true;
						break;
					default:
						Toast.makeText(uiContext, R.string.ocv_failed, Toast.LENGTH_LONG).show();
						Gdx.app.exit();
						break;
					}
				}
			};

			OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_8, this, loaderCallback);
		}

		initialize(new NxtARCore(this), cfg);
	}

	////////////////////////////////////////////////
	// OSFunctionalityProvider interface methods. //
	////////////////////////////////////////////////
	@Override
	public void showShortToast(final String msg){
		uiHandler.post(new Runnable(){
			@Override
			public void run(){
				Toast.makeText(uiContext, msg, Toast.LENGTH_SHORT).show();
			}
		});
	}

	@Override
	public void showLongToast(final String msg){
		uiHandler.post(new Runnable(){
			@Override
			public void run(){
				Toast.makeText(uiContext, msg, Toast.LENGTH_LONG).show();
			}
		});
	}

	@Override
	public void enableMulticast(){
		Gdx.app.log(TAG, CLASS_NAME + ".enableMulticast() :: Requesting multicast lock.");
		multicastLock = wifiManager.createMulticastLock(TAG);
		multicastLock.setReferenceCounted(true);
		multicastLock.acquire();
	}

	@Override
	public void disableMulticast(){
		Gdx.app.log(TAG, CLASS_NAME + ".disableMulticast() :: Releasing multicast lock.");
		if(multicastLock != null){
			multicastLock.release();
			multicastLock = null;
		}
	}

	////////////////////////////////////
	// CVProcessor interface methods. //
	////////////////////////////////////

	/**
	 * 
	 */
	@Override
	public CVMarkerData findMarkersInFrame(byte[] frame){
		if(ocvOn){
			int codes[] = new int[15];
			Bitmap tFrame, mFrame;

			tFrame = BitmapFactory.decodeByteArray(frame, 0, frame.length);

			Mat inImg = new Mat();
			Mat outImg = new Mat();
			Utils.bitmapToMat(tFrame, inImg);

			getMarkerCodesAndLocations(inImg.getNativeObjAddr(), outImg.getNativeObjAddr(), codes);

			//Mat temp = new Mat();
			//Imgproc.cvtColor(outImg, temp, Imgproc.COLOR_BGR2RGB);

			mFrame = Bitmap.createBitmap(outImg.cols(), outImg.rows(), Bitmap.Config.RGB_565);
			Utils.matToBitmap(outImg, mFrame);
			mFrame.compress(CompressFormat.JPEG, 100, outputStream);

			CVMarkerData data = new CVMarkerData();
			data.outFrame = outputStream.toByteArray();
			data.markerCodes = codes;

			tFrame.recycle();
			mFrame.recycle();
			outputStream.reset();

			return data;
		}else{
			Gdx.app.debug(TAG, CLASS_NAME + ".findMarkersInFrame(): OpenCV is not ready or failed to load.");
			return null;
		}
	}

	/**
	 * 
	 */
	@Override
	public CVCalibrationData findCalibrationPattern(byte[] frame){
		if(ocvOn){
			boolean found;
			float points[] = new float[ProjectConstants.CALIBRATION_PATTERN_POINTS * 2];
			Bitmap tFrame, mFrame;
			Mat inImg = new Mat(), outImg = new Mat();
			CVCalibrationData data = new CVCalibrationData();

			// Decode the input frame and convert it to an OpenCV Matrix.
			tFrame = BitmapFactory.decodeByteArray(frame, 0, frame.length);
			Utils.bitmapToMat(tFrame, inImg);

			// Attempt to find the calibration pattern in the input frame.
			found = findCalibrationPattern(inImg.getNativeObjAddr(), outImg.getNativeObjAddr(), points);

			// Encode the output image as a JPEG image.
			mFrame = Bitmap.createBitmap(outImg.cols(), outImg.rows(), Bitmap.Config.RGB_565);
			Utils.matToBitmap(outImg, mFrame);
			mFrame.compress(CompressFormat.JPEG, 100, outputStream);

			// Prepare the output data structure.
			data.outFrame = outputStream.toByteArray();
			data.calibrationPoints = found ? points : null;

			// Clean up memory.
			tFrame.recycle();
			mFrame.recycle();
			outputStream.reset();

			return data;
		}else{
			Gdx.app.debug(TAG, CLASS_NAME + ".findCalibrationPattern(): OpenCV is not ready or failed to load.");
			return null;
		}
	}

	/**
	 * 
	 */
	@Override
	public byte[] undistortFrame(byte[] frame){
		if(ocvOn){
			byte undistortedFrame[];
			Bitmap tFrame, mFrame;
			Mat inImg = new Mat(), outImg = new Mat();

			// Decode the input frame and convert it to an OpenCV Matrix.
			tFrame = BitmapFactory.decodeByteArray(frame, 0, frame.length);
			Utils.bitmapToMat(tFrame, inImg);

			// Apply the undistort correction to the input frame.
			Imgproc.undistort(inImg, outImg, cameraMatrix, distortionCoeffs);

			// Encode the output image as a JPEG image.
			mFrame = Bitmap.createBitmap(outImg.cols(), outImg.rows(), Bitmap.Config.RGB_565);
			Utils.matToBitmap(outImg, mFrame);
			mFrame.compress(CompressFormat.JPEG, 100, outputStream);

			// Prepare the return frame.
			undistortedFrame = outputStream.toByteArray();

			// Clean up memory.
			tFrame.recycle();
			mFrame.recycle();
			outputStream.reset();

			return undistortedFrame;
		}else{
			Gdx.app.debug(TAG, CLASS_NAME + ".undistortFrame(): OpenCV is not ready or failed to load.");
			return null;
		}
	}

	/**
	 * 
	 */
	@Override
	public void calibrateCamera(float[][] calibrationSamples, byte[] frame) {
		if(ocvOn){
			float[] calibrationPoints = new float[ProjectConstants.CALIBRATION_PATTERN_POINTS * 2 * ProjectConstants.CALIBRATION_SAMPLES];
			int w = ProjectConstants.CALIBRATION_PATTERN_POINTS * 2;
			Bitmap tFrame;
			Mat inImg = new Mat();

			for(int i = 0; i < ProjectConstants.CALIBRATION_SAMPLES; i++){
				for(int j = 0, p = 0; j < ProjectConstants.CALIBRATION_PATTERN_POINTS; j++, p += 2){
					calibrationPoints[p + (w * i)] = calibrationSamples[i][p];
					calibrationPoints[(p + 1) + (w * i)] = calibrationSamples[i][p + 1];
				}
			}

			tFrame = BitmapFactory.decodeByteArray(frame, 0, frame.length);
			Utils.bitmapToMat(tFrame, inImg);

			calibrateCameraParameters(cameraMatrix.getNativeObjAddr(), distortionCoeffs.getNativeObjAddr(), inImg.getNativeObjAddr(), calibrationPoints);

		}else{
			Gdx.app.debug(TAG, CLASS_NAME + ".calibrateCamera(): OpenCV is not ready or failed to load.");
		}
	}
}
