package com.example.backport.webp;

import android.app.Activity;
import android.backport.webp.WebPFactory;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;

import org.apache.commons.io.IOUtils;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class SampleActivity extends Activity {

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
        BitmapFactory.Options options = null;
        InputStream rawImageStream = null;
        byte[] data = null;
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
            case R.id.loadImage4:
                imageId = R.raw.image_alpha_lossy;
                //resize using inSampleSize
                options = new BitmapFactory.Options();
                options.inSampleSize = 2;
                break;
            default:
                return;
        }
        if(data==null) {
            data = getBytes(imageId);
        }
        final Bitmap webpBitmap = WebPFactory.decode(data, options);
        _imageView.setImageBitmap(webpBitmap);

    }

    private byte[] getBytes(int imageId) {
        byte[] data;
        try {
            data = IOUtils.toByteArray(getResources().openRawResource(imageId));
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        return data;
    }

    public void onButtonClickFile(View view) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inSampleSize = 4;
        //copy file from stream
        File outputFile;
        InputStream rawImageStream=null;
        FileOutputStream fos=null;
        try {
            rawImageStream = getResources().openRawResource(R.raw.image_alpha_lossy);
            outputFile = File.createTempFile("sample", "", getCacheDir());
            fos = new FileOutputStream(outputFile);
            IOUtils.copy(getResources().openRawResource(R.raw.image_alpha_lossy), fos);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }finally{
            IOUtils.closeQuietly(rawImageStream);
            IOUtils.closeQuietly(fos);
        }
        final Bitmap webpBitmap = WebPFactory.decode(outputFile.getAbsolutePath(), options);
        _imageView.setImageBitmap(webpBitmap);

    }
}