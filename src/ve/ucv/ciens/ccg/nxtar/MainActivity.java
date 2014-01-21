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

import ve.ucv.ciens.ccg.nxtar.interfaces.MulticastEnabler;
import ve.ucv.ciens.ccg.nxtar.interfaces.Toaster;
import android.content.Context;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;
import android.os.Bundle;
import android.os.Handler;
import android.widget.Toast;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.backends.android.AndroidApplication;
import com.badlogic.gdx.backends.android.AndroidApplicationConfiguration;

public class MainActivity extends AndroidApplication implements Toaster, MulticastEnabler{
	private static final String TAG = "NXTAR_ANDROID_MAIN";
	private static final String CLASS_NAME = MainActivity.class.getSimpleName();

	private WifiManager wifiManager;
	private MulticastLock multicastLock;
	private Handler uiHandler;
	private Context uiContext;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		uiHandler = new Handler();
		uiContext = this;
		wifiManager = (WifiManager)getSystemService(Context.WIFI_SERVICE);

		AndroidApplicationConfiguration cfg = new AndroidApplicationConfiguration();
		cfg.useGL20 = false;
		cfg.useAccelerometer = false;
		cfg.useCompass = false;
		cfg.useWakelock = true;

		initialize(new NxtARCore(this), cfg);
	}

	////////////////////////////////
	// Toaster interface methods. //
	////////////////////////////////
	@Override
	public void showShortToast(final String msg) {
		uiHandler.post(new Runnable() {
			@Override
			public void run() {
				Toast.makeText(uiContext, msg, Toast.LENGTH_SHORT).show();
			}
		});
	}

	@Override
	public void showLongToast(final String msg) {
		uiHandler.post(new Runnable() {
			@Override
			public void run() {
				Toast.makeText(uiContext, msg, Toast.LENGTH_LONG).show();
			}
		});
	}

	/////////////////////////////////////////
	// MulticastEnabler interface methods. //
	/////////////////////////////////////////
	@Override
	public void enableMulticast(){
		Gdx.app.debug(TAG, CLASS_NAME + ".enableMulticast() :: Requesting multicast lock.");
		multicastLock = wifiManager.createMulticastLock(TAG);
		multicastLock.setReferenceCounted(true);
		multicastLock.acquire();
	}

	@Override
	public void disableMulticast() {
		Gdx.app.debug(TAG, CLASS_NAME + ".disableMulticast() :: Releasing multicast lock.");
		if(multicastLock != null){
			multicastLock.release();
			multicastLock = null;
		}
	}
}