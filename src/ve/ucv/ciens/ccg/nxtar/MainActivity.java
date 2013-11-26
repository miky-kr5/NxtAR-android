package ve.ucv.ciens.ccg.nxtar;

import ve.ucv.ciens.ccg.nxtar.interfaces.MulticastEnabler;
import ve.ucv.ciens.ccg.nxtar.utils.AndroidToaster;
import android.content.Context;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;
import android.os.Bundle;

import com.badlogic.gdx.backends.android.AndroidApplication;
import com.badlogic.gdx.backends.android.AndroidApplicationConfiguration;

public class MainActivity extends AndroidApplication implements MulticastEnabler{
	private static final String TAG = "NXTAR_ANDROID_MAIN";
	private static final String CLASS_NAME = MainActivity.class.getSimpleName();

	private AndroidToaster toaster;
	private WifiManager wifiManager;
	private MulticastLock multicastLock;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		wifiManager = (WifiManager)getSystemService(Context.WIFI_SERVICE);
		toaster = new AndroidToaster(this);

		AndroidApplicationConfiguration cfg = new AndroidApplicationConfiguration();
		cfg.useGL20 = false;

		initialize(new Main(toaster, this), cfg);
	}

	@Override
	public void enableMulticast(){
		multicastLock = wifiManager.createMulticastLock(TAG);
		multicastLock.setReferenceCounted(true);
		multicastLock.acquire();
	}

	@Override
	public void disableMulticast() {
		if(multicastLock != null){
			multicastLock.release();
			multicastLock = null;
		}
	}
}