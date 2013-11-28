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
		Gdx.app.debug(TAG, CLASS_NAME + ".disableMulticast() :: Requesting multicast lock.");
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