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

/**
 * <p>The main activity of the application.</p>
 * 
 * <p>Provides operating system services to the LibGDX platform
 * independant code, and handles OpenCV initialization and api calls.</p>
 */
public class MainActivity extends AndroidApplication implements OSFunctionalityProvider, CVProcessor{
	/**
	 * Tag used for logging.
	 */
	private static final String TAG = "NXTAR_ANDROID_MAIN";

	/**
	 * Class name used for logging.
	 */
	private static final String CLASS_NAME = MainActivity.class.getSimpleName();

	/**
	 * Output stream used to codify images as JPEG using Android's Bitmap class.
	 */
	private static final ByteArrayOutputStream outputStream = new ByteArrayOutputStream();

	/**
	 * Indicates if OpenCV was initialized sucessfully.
	 */
	private static boolean ocvOn = false;

	/**
	 * Intrinsic camera matrix.
	 */
	private static Mat cameraMatrix;

	/**
	 * Distortion coeffitients matrix.
	 */
	private static Mat distortionCoeffs;

	/**
	 * Used to set and release multicast locks.
	 */
	private WifiManager wifiManager;

	/**
	 * Used to maintain the multicast lock during the service discovery procedure.
	 */
	private MulticastLock multicastLock;

	/**
	 * Handler used for requesting toast messages from the core LibGDX code.
	 */
	private Handler uiHandler;

	/**
	 * User interface context used to show the toast messages.
	 */
	private Context uiContext;

	/**
	 * OpenCV asynchronous initializer callback for mobile devices.
	 */
	private BaseLoaderCallback loaderCallback;

	/**
	 * Indicates if the current video streaming camera has been calibrated.
	 */
	private boolean cameraCalibrated;

	/**
	 * <p>Wrapper for the getAllMarkers native function.</p>
	 * 
	 * @param inMat INPUT. The image to analize.
	 * @param outMat OUTPUT. The image with the markers highlighted.
	 * @param codes OUTPUT. The codes for each marker detected. Must be 15 elements long.
	 */
	private native void getMarkerCodesAndLocations(long inMat, long outMat, int[] codes);

	/**
	 * <p>Wrapper for the findCalibrationPattern native function.</p>
	 * 
	 * @param inMat INPUT. The image to analize.
	 * @param outMat OUTPUT. The image with the calibration pattern highlighted.
	 * @param points OUTPUT. The spatial location of the calibration points if found.
	 * @return True if the calibration pattern was found. False otherwise.
	 */
	private native boolean findCalibrationPattern(long inMat, long outMat, float[] points);

	/**
	 * <p>Wrapper around the getCameraParameters native function.</p>
	 * 
	 * @param camMat OUTPUT. The intrinsic camera matrix.
	 * @param distMat OUTPUT. The distortion coeffitients matrix.
	 * @param frame INPUT. A sample input image from the camera to calibrate.
	 * @param calibrationPoints INPUT. The calibration points of all samples. 
	 * @return The calibration error as returned by OpenCV.
	 */
	private native double calibrateCameraParameters(long camMat, long distMat, long frame, float[] calibrationPoints);

	/**
	 * <p>Static block. Tries to load OpenCV and the native method implementations
	 * statically if running on an OUYA device.</p> 
	 */
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

	/**
	 * <p>Initializes this activity</p>
	 * 
	 * <p>This method handles the initialization of LibGDX and OpenCV. OpenCV is
	 * loaded the asynchronous method if the devices is not an OUYA console.</p>
	 * 
	 * @param savedInstanceState The application state if it was saved in a previous run.
	 */
	@Override
	public void onCreate(Bundle savedInstanceState){
		super.onCreate(savedInstanceState);

		cameraCalibrated = false;

		// Set screen orientation. Portrait on mobile devices, landscape on OUYA.
		if(!Ouya.runningOnOuya){
			setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
		}else{
			setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		}

		// Set up the Android related variables.
		uiHandler = new Handler();
		uiContext = this;
		wifiManager = (WifiManager)getSystemService(Context.WIFI_SERVICE);

		// Attempt to initialize OpenCV.
		if(!Ouya.runningOnOuya){
			// If running on a moble device, use the asynchronous method aided
			// by the OpenCV Manager app.
			loaderCallback = new BaseLoaderCallback(this){
				@Override
				public void onManagerConnected(int status){
					switch(status){
					case LoaderCallbackInterface.SUCCESS:
						// If successfully initialized then load the native method implementations and
						// initialize the static matrices.
						System.loadLibrary("cvproc");
						ocvOn = true;
						cameraMatrix = new Mat();
						distortionCoeffs = new Mat();
						break;

					default:
						Toast.makeText(uiContext, R.string.ocv_failed, Toast.LENGTH_LONG).show();
						ocvOn = false;
						break;
					}
				}
			};

			// Launch the asynchronous initializer.
			OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_7, this, loaderCallback);

		}else{
			// If running on an OUYA device.
			if(ocvOn){
				// If OpenCV loaded successfully then initialize the native matrices.
				cameraMatrix = new Mat();
				distortionCoeffs = new Mat();
			}else{
				Toast.makeText(uiContext, R.string.ocv_failed, Toast.LENGTH_LONG).show();
			}
		}

		// Configure LibGDX.
		AndroidApplicationConfiguration cfg = new AndroidApplicationConfiguration();
		cfg.useGL20 = true;
		cfg.useAccelerometer = false;
		cfg.useCompass = false;
		cfg.useWakelock = true;

		// Launch the LibGDX core game class.
		initialize(new NxtARCore(this), cfg);
	}

	////////////////////////////////////////////////
	// OSFunctionalityProvider interface methods. //
	////////////////////////////////////////////////

	/**
	 * <p>Shows a short message on screen using Android's toast mechanism.</p>
	 * 
	 * @param msg The message to show.
	 */
	@Override
	public void showShortToast(final String msg){
		uiHandler.post(new Runnable(){
			@Override
			public void run(){
				Toast.makeText(uiContext, msg, Toast.LENGTH_SHORT).show();
			}
		});
	}

	/**
	 * <p>Shows a long message on screen using Android's toast mechanism.</p>
	 * 
	 * @param msg The message to show.
	 */
	@Override
	public void showLongToast(final String msg){
		uiHandler.post(new Runnable(){
			@Override
			public void run(){
				Toast.makeText(uiContext, msg, Toast.LENGTH_LONG).show();
			}
		});
	}

	/**
	 * <p>Enable the transmision and reception of multicast network messages.</p>
	 */
	@Override
	public void enableMulticast(){
		Gdx.app.log(TAG, CLASS_NAME + ".enableMulticast() :: Requesting multicast lock.");
		multicastLock = wifiManager.createMulticastLock(TAG);
		multicastLock.setReferenceCounted(true);
		multicastLock.acquire();
	}

	/**
	 * <p>Disables the transmision and reception of multicast network messages.</p>
	 */
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
	 * <p>Implementation of the findMarkersInFrame method.</p>
	 * 
	 * <p>This implementation finds up to 15 markers in the input
	 * image and returns their codes and locations in the CVMarkerData
	 * structure. The markers are higlihted in the input image.</p>
	 * 
	 * @param frame The JPEG encoded input image.
	 * @return A data structure containing the processed output image, the
	 * detected marker codes and their respective locations.
	 */
	@Override
	public CVMarkerData findMarkersInFrame(byte[] frame){
		if(ocvOn){
			int codes[] = new int[15];
			Bitmap tFrame, mFrame;
			Mat inImg = new Mat();
			Mat outImg = new Mat();

			// Decode the input image and convert it to an OpenCV matrix.
			tFrame = BitmapFactory.decodeByteArray(frame, 0, frame.length);
			Utils.bitmapToMat(tFrame, inImg);

			// Find up to 15 markers in the input image.
			getMarkerCodesAndLocations(inImg.getNativeObjAddr(), outImg.getNativeObjAddr(), codes);

			// Encode the output image as a JPEG image.
			mFrame = Bitmap.createBitmap(outImg.cols(), outImg.rows(), Bitmap.Config.RGB_565);
			Utils.matToBitmap(outImg, mFrame);
			mFrame.compress(CompressFormat.JPEG, 100, outputStream);

			// Create the output data structure.
			CVMarkerData data = new CVMarkerData();
			data.outFrame = outputStream.toByteArray();
			data.markerCodes = codes;

			// Clean up memory.
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
	 * <p>Implementation of the findCalibrationPattern method.</p>
	 * 
	 * <p>Attempts to detect a checkerboard calibration pattern in the input image.
	 * If the pattenr is found the method returns an image with the pattern
	 * highlighted and the spatial location of the calibration points in the 
	 * output data structure.</p>
	 * 
	 * @param frame The JPEG encoded input image.
	 * @return A data structure containing the processed output image and the
	 * location of the calibration points. If the pattern was not found, the returnd
	 * calibration points array is null.
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
	 * <p>Implementation of the calibrateCamera method.</p>
	 * 
	 * <p>Obtains the intrinsic camera parameters necesary for calibration.</p>
	 */
	@Override
	public void calibrateCamera(float[][] calibrationSamples, byte[] frame) {
		if(ocvOn){
			float[] calibrationPoints = new float[ProjectConstants.CALIBRATION_PATTERN_POINTS * 2 * ProjectConstants.CALIBRATION_SAMPLES];
			int w = ProjectConstants.CALIBRATION_PATTERN_POINTS * 2;
			Bitmap tFrame;
			Mat inImg = new Mat();

			// Save the calibration points on a one dimensional array for easier parameter passing
			// to the native code.
			for(int i = 0; i < ProjectConstants.CALIBRATION_SAMPLES; i++){
				for(int j = 0, p = 0; j < ProjectConstants.CALIBRATION_PATTERN_POINTS; j++, p += 2){
					calibrationPoints[p + (w * i)] = calibrationSamples[i][p];
					calibrationPoints[(p + 1) + (w * i)] = calibrationSamples[i][p + 1];
				}
			}

			// Decode the input image and convert it to an OpenCV matrix.
			tFrame = BitmapFactory.decodeByteArray(frame, 0, frame.length);
			Utils.bitmapToMat(tFrame, inImg);

			// Attempt to obtain the camera parameters.
			double error = calibrateCameraParameters(cameraMatrix.getNativeObjAddr(), distortionCoeffs.getNativeObjAddr(), inImg.getNativeObjAddr(), calibrationPoints);
			Gdx.app.log(TAG, CLASS_NAME + "calibrateCamera(): calibrateCameraParameters retured " + Double.toString(error));
			cameraCalibrated = true;

		}else{
			Gdx.app.debug(TAG, CLASS_NAME + ".calibrateCamera(): OpenCV is not ready or failed to load.");
		}
	}


	/**
	 * <p>Implementation of the undistorFrame method.</p>
	 * 
	 * <p>Removes camera lens distortion from the input image using the
	 * camera parameters obtained by the calibrateCamera method.</p>
	 * 
	 * @return A JPEG encoded image that is the input image after distortion correction. If the
	 * camera has not been calibrated or OpenCV failed to load returns null.
	 */
	@Override
	public byte[] undistortFrame(byte[] frame){
		if(ocvOn){
			if(cameraCalibrated){
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
				Gdx.app.debug(TAG, CLASS_NAME + ".undistortFrame(): Camera has not been calibrated.");
				return null;
			}
		}else{
			Gdx.app.debug(TAG, CLASS_NAME + ".undistortFrame(): OpenCV is not ready or failed to load.");
			return null;
		}
	}

	/**
	 * <p>Indicates if OpenCV has been sucessfully initialized and used
	 * to obtain the camera parameters for calibration.</p>
	 * 
	 * @return True if and only if OpenCV initialized succesfully and calibrateCamera has been called previously.
	 */
	@Override
	public boolean cameraIsCalibrated() {
		return ocvOn && cameraCalibrated;
	}
}
