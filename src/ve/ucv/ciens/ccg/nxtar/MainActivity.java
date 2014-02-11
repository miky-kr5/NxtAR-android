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

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;

import ve.ucv.ciens.ccg.nxtar.interfaces.MulticastEnabler;
import ve.ucv.ciens.ccg.nxtar.interfaces.Toaster;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;
import android.os.Bundle;
import android.os.Handler;
import android.widget.Toast;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.backends.android.AndroidApplication;
import com.badlogic.gdx.backends.android.AndroidApplicationConfiguration;
import com.badlogic.gdx.controllers.mappings.Ouya;

public class MainActivity extends AndroidApplication implements Toaster, MulticastEnabler, CvCameraViewListener{
	private static final String TAG = "NXTAR_ANDROID_MAIN";
	private static final String CLASS_NAME = MainActivity.class.getSimpleName();

	private WifiManager wifiManager;
	private MulticastLock multicastLock;
	private Handler uiHandler;
	private Context uiContext;
	private boolean ocvOn;
	private BaseLoaderCallback loaderCallback;

	/*static{
	    if (!OpenCVLoader.initDebug()){
	        Gdx.app.exit();
	    }
	}*/

	@Override
	public void onCreate(Bundle savedInstanceState){
		super.onCreate(savedInstanceState);

		ocvOn = false;

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

		loaderCallback = new BaseLoaderCallback(this){
			@Override
			public void onManagerConnected(int status){
				switch(status){
				case LoaderCallbackInterface.SUCCESS:
					ocvOn = true;
					break;
				default:
					Toast.makeText(uiContext, R.string.ocv_failed, Toast.LENGTH_LONG).show();
					Gdx.app.exit();
					break;
				}
			}
		};

		OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_7, this, loaderCallback);
		initialize(new NxtARCore(this), cfg);
	}

	////////////////////////////////
	// Toaster interface methods. //
	////////////////////////////////
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

	/////////////////////////////////////////
	// MulticastEnabler interface methods. //
	/////////////////////////////////////////
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

	/////////////////////////////////////////////
	// CvCameraViewListener interface methods. //
	/////////////////////////////////////////////
	/**
	 * <p>This method does nothing. It is here because it must be implemented in order to use OpenCV.</p>
	 */
	@Override
	public void onCameraViewStarted(int width, int height){ }

	/**
	 * <p>This method does nothing. It is here because it must be implemented in order to use OpenCV.</p>
	 */
	@Override
	public void onCameraViewStopped(){ }

	/**
	 * <p>This method does nothing. It is here because it must be implemented in order to use OpenCV.</p>
	 */
	@Override
	public Mat onCameraFrame(Mat inputFrame){
		return null;
	}
}