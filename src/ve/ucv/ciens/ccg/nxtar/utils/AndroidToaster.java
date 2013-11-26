package ve.ucv.ciens.ccg.nxtar.utils;

import ve.ucv.ciens.ccg.nxtar.interfaces.Toaster;
import android.content.Context;
import android.os.Handler;
import android.widget.Toast;

/**
 * Class used for showing toast messages from a LibGDX game.
 * 
 * @author Miguel Angel Astor Romero
 */
public class AndroidToaster implements Toaster {
	private Handler uiHandler;
	private Context uiContext; 

	public AndroidToaster(Context context){
		uiHandler = new Handler();
		uiContext = context;
	}

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
}
