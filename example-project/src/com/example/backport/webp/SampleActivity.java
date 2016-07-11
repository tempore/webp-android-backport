package com.example.backport.webp;

import android.app.Activity;
import android.backport.webp.WebPFactory;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;

public class SampleActivity extends Activity {
    private static byte[] streamToBytes(InputStream is) {
        ByteArrayOutputStream os = new ByteArrayOutputStream(1024);
        byte[] buffer = new byte[1024];
        int len;
        try {
            while ((len = is.read(buffer)) >= 0) {
                os.write(buffer, 0, len);
            }
        } catch (java.io.IOException e) {
        }
        return os.toByteArray();
    }

    ImageView _imageView = null;

    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        _imageView = (ImageView) findViewById(R.id.imageView);
    }

    public void onButtonClick(View view) {
        int imageId;
        switch (view.getId()) {
            case R.id.loadImage1:
                imageId = R.raw.image;
                break;
            case R.id.loadImage2:
                imageId = R.raw.image_alpha_lossless;
                break;
            case R.id.loadImage3:
                imageId = R.raw.image_alpha_lossy;
                break;
            default:
                return;
        }
        InputStream rawImageStream = getResources().openRawResource(imageId);
        byte[] data = streamToBytes(rawImageStream);
        final Bitmap webpBitmap = WebPFactory.nativeDecodeByteArray(
                data, null);
        _imageView.setImageBitmap(webpBitmap);

    }
}