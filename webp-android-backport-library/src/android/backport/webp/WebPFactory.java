package android.backport.webp;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapFactory.Options;
import android.os.Build;
import android.util.Log;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Factory to encode and decode WebP images into Android Bitmap
 * @author Alexey Pelykh
 * @author Ivan Churkin
 */
public final class WebPFactory {

	private static final String TAG = "WebPFactory";

	// can't use WebP before Android 2.2 Froyo because lack of libjnigraphics in core
	public static final boolean IS_SUPPORTED = (Build.VERSION.SDK_INT >= Build.VERSION_CODES.FROYO);

	private static final boolean IS_NATIVELY_SUPPORTED = hasWebPLibraryInAndroidCore();

	static {
		if (IS_SUPPORTED && !IS_NATIVELY_SUPPORTED) {
			System.loadLibrary("webpbackport");
		}
	}

	/**
	 * Decodes byte array to bitmap 
	 * @param data Byte array with WebP bitmap data
	 * @param options Options to control decoding. Accepts null
	 * @return Decoded bitmap
	 */
	private static native Bitmap nativeDecodeByteArray(byte[] data, BitmapFactory.Options options);

	/**
     * Decodes file to bitmap
     *
     * @param path    WebP file path
     * @param options Options to control decoding. Accepts null
     * @return Decoded bitmap
     */
    private static native Bitmap nativeDecodeFile(String path, Options options);
	
	/**
	 * Encodes bitmap into byte array
	 * @param bitmap Bitmap
	 * @param quality Quality, should be between 0 and 100
	 * @return Encoded byte array
	 */
//	public static native byte[] nativeEncodeBitmap(Bitmap bitmap, int quality);

	/**
	 * Verify bitmap's format
	 *
	 * @param data
	 * @return is it WebP byte array
	 */
	public static boolean isWebP(byte[] data) {
		return data != null && data.length > 12 && data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F'
				&& data[8] == 'W' && data[9] == 'E' && data[10] == 'B' && data[11] == 'P';
	}

    /**
     * Decodes a WebP image.
     *
     * @param data
     *            The encoded WebP data (e.g. from a stream, resource, etc.).
     * @return The decoded image.
     */
    public static Bitmap decode(byte[] data, BitmapFactory.Options options) {
        if (!IS_SUPPORTED) {
            throw new RuntimeException("WebP is not supported, check WebPFactory.IS_SUPPORTED before usage");
        }
        if (!IS_NATIVELY_SUPPORTED) {
            if (BuildConfig.DEBUG) {
                Log.d(TAG, "decoding by custom library");
            }
            return nativeDecodeByteArray(data, options);
        } else {
            if (BuildConfig.DEBUG) {
                Log.d(TAG, "decoding by android core");
            }
            return BitmapFactory.decodeByteArray(data, 0, data.length, options);
        }
    }


    /**
     * Decodes a WebP image.
     *
     * @param path Path to encoded WebP file.
     * @param options bitmap options
     * @return The decoded image.
     */
    public static Bitmap decode(String path, BitmapFactory.Options options) {
        if (!IS_SUPPORTED) {
            throw new RuntimeException("WebP is not supported, check WebPFactory.IS_SUPPORTED before usage");
        }
        if (!IS_NATIVELY_SUPPORTED) {
            if (BuildConfig.DEBUG) {
                Log.d(TAG, "decoding file by custom library");
            }
            return nativeDecodeFile(path, options);
        } else {
            if (BuildConfig.DEBUG) {
                Log.d(TAG, "decoding file by android core");
            }
            return BitmapFactory.decodeFile(path, options);
        }
    }


	/**
	 *
	 * @return is webp implementation embedded into android core or not
     */
	private static boolean hasWebPLibraryInAndroidCore() {
		if (!IS_SUPPORTED) {
			return false;
		}
		// webp with transparency supported natively from android 4.2.1 (it's api 17 but not every api 17 cores support webp with transparency)
		if (Build.VERSION.SDK_INT >= 18) {
			return true;
		} else if (Build.VERSION.SDK_INT < 17) {
			return false;
		} else {
			try {
				// api==17
				// only third digit is valuable
				Pattern pattern = Pattern.compile("\\d+\\.\\d+\\.(\\d+).*");
				Matcher matcher = pattern.matcher(Build.VERSION.RELEASE);
				if (matcher.matches()) {
					return Integer.parseInt(matcher.group(1)) >= 1;
				} else {
					return false;
				}
			} catch (Throwable e) {
				// should not happen
				Log.e(TAG, e.getMessage());
				return false;
			}
		}

	}

}
