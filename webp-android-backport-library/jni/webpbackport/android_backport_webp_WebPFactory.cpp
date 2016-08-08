#include "android_backport_webp_WebPFactory.h"
#include "android_backport_webp.h"

#include <stdio.h>
#include <string.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <webp/decode.h>
#include <webp/encode.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     android_backport_webp_WebPFactory
 * Method:    nativeDecodeByteArray
 * Signature: ([BLandroid/graphics/BitmapFactory/Options;)Landroid/graphics/Bitmap;
 */
JNIEXPORT jobject JNICALL Java_android_backport_webp_WebPFactory_nativeDecodeByteArray
  (JNIEnv *jniEnv, jclass, jbyteArray byteArray, jobject options)
{
	// Check if input is valid
	if(!byteArray)
	{
		jniEnv->ThrowNew(jrefs::java::lang::NullPointerException->jclassRef, "Input buffer can not be null");
		return 0;
	}

	// Log what version of WebP is used
	// __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Using WebP Decoder %08x", WebPGetDecoderVersion());

    //reset outWidth and outHeight in case of error it should be -1
    if(options){
	    jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outWidth, -1);
	    jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outHeight, -1);
	}

	// Lock buffer
	jbyte* inputBuffer = jniEnv->GetByteArrayElements(byteArray, NULL);
	size_t inputBufferLen = jniEnv->GetArrayLength(byteArray);

	// Validate image
	int bitmapWidth = 0;
	int bitmapHeight = 0;
	if(!WebPGetInfo((uint8_t*)inputBuffer, inputBufferLen, &bitmapWidth, &bitmapHeight))
	{
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Invalid WebP format");
		return 0;
	}

	// Check if size is all what we were requested to do
	if(options && jniEnv->GetBooleanField(options, jrefs::android::graphics::BitmapFactory->Options.inJustDecodeBounds) == JNI_TRUE)
	{
		// Set values
		jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outWidth, bitmapWidth);
		jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outHeight, bitmapHeight);

		// Unlock buffer
		jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);

		return 0;
	}

    // Initialize decoder config and configure scaling if requested
    WebPDecoderConfig config;
    if (!WebPInitDecoderConfig(&config))
    {
        jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);
        jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Unable to init WebP decoder config");
        return 0;
    }

    if (options)
    {
        jint inSampleSize = jniEnv->GetIntField(options, jrefs::android::graphics::BitmapFactory->Options.inSampleSize);
        bool inScaled = (jniEnv->GetBooleanField(options, jrefs::android::graphics::BitmapFactory->Options.inScaled) == JNI_TRUE);
        jint inDensity = jniEnv->GetIntField(options, jrefs::android::graphics::BitmapFactory->Options.inDensity);
        jint inTargetDensity = jniEnv->GetIntField(options, jrefs::android::graphics::BitmapFactory->Options.inTargetDensity);
        if(inScaled && inDensity > 0 && inTargetDensity > 0){
                jint divider = (inSampleSize > 1 ? inSampleSize : 1) * inDensity;
                config.options.use_scaling = 1;
                config.options.scaled_width = bitmapWidth = (bitmapWidth * inTargetDensity) / divider;
                config.options.scaled_height = bitmapHeight = (bitmapHeight * inTargetDensity) / divider;
        }
        else if (inSampleSize > 1)
        {
                config.options.use_scaling = 1;
                config.options.scaled_width = bitmapWidth /= inSampleSize;
                config.options.scaled_height = bitmapHeight /= inSampleSize;
        }
    }
	// __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Decoding %dx%d bitmap", bitmapWidth, bitmapHeight);

	// Create bitmap
	jobject value__ARGB_8888 = jniEnv->GetStaticObjectField(jrefs::android::graphics::Bitmap->Config.jclassRef, jrefs::android::graphics::Bitmap->Config.ARGB_8888);
	jobject outputBitmap = jniEnv->CallStaticObjectMethod(jrefs::android::graphics::Bitmap->jclassRef, jrefs::android::graphics::Bitmap->createBitmap,
		(jint)bitmapWidth, (jint)bitmapHeight,
		value__ARGB_8888);
	if(!outputBitmap)
	{
		jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to allocate Bitmap");
		return 0;
	}
	outputBitmap = jniEnv->NewLocalRef(outputBitmap);

	// Get information about bitmap passed
	AndroidBitmapInfo bitmapInfo;
	if(AndroidBitmap_getInfo(jniEnv, outputBitmap, &bitmapInfo) != ANDROID_BITMAP_RESUT_SUCCESS)
	{
		jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);
		jniEnv->DeleteLocalRef(outputBitmap);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to get Bitmap information");
		return 0;
	}

	// Lock pixels
	void* bitmapPixels = 0;
	if(AndroidBitmap_lockPixels(jniEnv, outputBitmap, &bitmapPixels) != ANDROID_BITMAP_RESUT_SUCCESS)
	{
		jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);
		jniEnv->DeleteLocalRef(outputBitmap);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to lock Bitmap pixels");
		return 0;
	}


    //Decode to rgbA bitmap image is Premultiplied in android
    config.output.colorspace = MODE_rgbA;
    config.output.u.RGBA.rgba = (uint8_t*)bitmapPixels;
    config.output.u.RGBA.stride = bitmapInfo.stride;
    config.output.u.RGBA.size = bitmapInfo.height * bitmapInfo.stride;
    config.output.is_external_memory = 1;
    if (WebPDecode((uint8_t*)inputBuffer, inputBufferLen, &config) != VP8_STATUS_OK)
    {
            AndroidBitmap_unlockPixels(jniEnv, outputBitmap);
            jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);
            jniEnv->DeleteLocalRef(outputBitmap);
            jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to decode WebP pixel data");
            return 0;
    }

	// Unlock pixels
	if(AndroidBitmap_unlockPixels(jniEnv, outputBitmap) != ANDROID_BITMAP_RESUT_SUCCESS)
	{
		jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);
		jniEnv->DeleteLocalRef(outputBitmap);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to unlock Bitmap pixels");
		return 0;
	}
	
	// Unlock buffer
	jniEnv->ReleaseByteArrayElements(byteArray, inputBuffer, JNI_ABORT);

    if (options)
    {
	    // Set width and height values
	    jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outWidth, bitmapWidth);
	    jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outHeight, bitmapHeight);
	}

	return outputBitmap;
}

/*
 * Class:     android_backport_webp_WebPFactory
 * Method:    nativeDecodeFile
 * Signature: (Ljava/lang/String;Landroid/graphics/BitmapFactory/Options;)Landroid/graphics/Bitmap;
 */
JNIEXPORT jobject JNICALL Java_android_backport_webp_WebPFactory_nativeDecodeFile
  (JNIEnv *jniEnv, jclass, jstring path, jobject options)
{
	// Check if input is valid
	if(!path)
	{
		jniEnv->ThrowNew(jrefs::java::lang::NullPointerException->jclassRef, "path can not be null");
		return 0;
	}

	// Log what version of WebP is used
	//__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Using WebP Decoder %08x", WebPGetDecoderVersion());

	if(options){
	    //reset outWidth and outHeight in case of error it should be -1
	    jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outWidth, -1);
	    jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outHeight, -1);
	}


    char *inputBuffer;
    size_t inputBufferLen;
    const char* filePath = jniEnv->GetStringUTFChars(path, 0);
    FILE *file = NULL;
    file = fopen(filePath, "rb");
    jniEnv->ReleaseStringUTFChars(path, filePath);
    if(file)
    {
		fseek(file, 0, SEEK_END);
		long file_len = ftell(file);
		fseek(file, 0, SEEK_SET);
        inputBuffer = (char *) malloc(file_len * sizeof(char));
		if (inputBuffer == NULL)
		{
			fclose(file);
			jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "malloc error");
			return 0;
		}
        inputBufferLen = fread(inputBuffer, sizeof(char), file_len, file);
        if (inputBufferLen != file_len)
        {
            free(inputBuffer);
			fclose(file);
			jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Read file error");
			return 0;
        }
		fclose(file);
    } else {
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Can not open file");
		return 0;
	}

	// Validate image
	int bitmapWidth = 0;
	int bitmapHeight = 0;
	if(!WebPGetInfo((uint8_t*)inputBuffer, inputBufferLen, &bitmapWidth, &bitmapHeight))
	{
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Invalid WebP format");
		return 0;
	}

	// Check if size is all what we were requested to do
	if(options && jniEnv->GetBooleanField(options, jrefs::android::graphics::BitmapFactory->Options.inJustDecodeBounds) == JNI_TRUE)
	{
		// Set values
		jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outWidth, bitmapWidth);
		jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outHeight, bitmapHeight);

		// Release buffer
        free(inputBuffer);

		return 0;
	}

    // Initialize decoder config and configure scaling if requested
    WebPDecoderConfig config;
    if (!WebPInitDecoderConfig(&config))
    {
        free(inputBuffer);
        jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Unable to init WebP decoder config");
        return 0;
    }

    if (options)
    {
        jint inSampleSize = jniEnv->GetIntField(options, jrefs::android::graphics::BitmapFactory->Options.inSampleSize);
        bool inScaled = (jniEnv->GetBooleanField(options, jrefs::android::graphics::BitmapFactory->Options.inScaled) == JNI_TRUE);
        jint inDensity = jniEnv->GetIntField(options, jrefs::android::graphics::BitmapFactory->Options.inDensity);
        jint inTargetDensity = jniEnv->GetIntField(options, jrefs::android::graphics::BitmapFactory->Options.inTargetDensity);
        if(inScaled && inDensity > 0 && inTargetDensity > 0){
                jint divider = (inSampleSize > 1 ? inSampleSize : 1) * inDensity;
                config.options.use_scaling = 1;
                config.options.scaled_width = bitmapWidth = (bitmapWidth * inTargetDensity) / divider;
                config.options.scaled_height = bitmapHeight = (bitmapHeight * inTargetDensity) / divider;
        }
        else if (inSampleSize > 1)
        {
                config.options.use_scaling = 1;
                config.options.scaled_width = bitmapWidth /= inSampleSize;
                config.options.scaled_height = bitmapHeight /= inSampleSize;
        }
    }
	//__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Decoding %dx%d bitmap", bitmapWidth, bitmapHeight);

	// Create bitmap
	jobject value__ARGB_8888 = jniEnv->GetStaticObjectField(jrefs::android::graphics::Bitmap->Config.jclassRef, jrefs::android::graphics::Bitmap->Config.ARGB_8888);
	jobject outputBitmap = jniEnv->CallStaticObjectMethod(jrefs::android::graphics::Bitmap->jclassRef, jrefs::android::graphics::Bitmap->createBitmap,
														  (jint)bitmapWidth, (jint)bitmapHeight,
														  value__ARGB_8888);
	if(!outputBitmap)
	{
        free(inputBuffer);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to allocate Bitmap");
		return 0;
	}
	outputBitmap = jniEnv->NewLocalRef(outputBitmap);

	// Get information about bitmap passed
	AndroidBitmapInfo bitmapInfo;
	if(AndroidBitmap_getInfo(jniEnv, outputBitmap, &bitmapInfo) != ANDROID_BITMAP_RESUT_SUCCESS)
	{
        free(inputBuffer);
		jniEnv->DeleteLocalRef(outputBitmap);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to get Bitmap information");
		return 0;
	}

	// Lock pixels
	void* bitmapPixels = 0;
	if(AndroidBitmap_lockPixels(jniEnv, outputBitmap, &bitmapPixels) != ANDROID_BITMAP_RESUT_SUCCESS)
	{
        free(inputBuffer);
		jniEnv->DeleteLocalRef(outputBitmap);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to lock Bitmap pixels");
		return 0;
	}

    //Decode to rgbA bitmap image is Premultiplied in android
    config.output.colorspace = MODE_rgbA;
    config.output.u.RGBA.rgba = (uint8_t*)bitmapPixels;
    config.output.u.RGBA.stride = bitmapInfo.stride;
    config.output.u.RGBA.size = bitmapInfo.height * bitmapInfo.stride;
    config.output.is_external_memory = 1;
    if (WebPDecode((uint8_t*)inputBuffer, inputBufferLen, &config) != VP8_STATUS_OK)
	{
		AndroidBitmap_unlockPixels(jniEnv, outputBitmap);
        free(inputBuffer);
		jniEnv->DeleteLocalRef(outputBitmap);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to unlock Bitmap pixels");
		return 0;
	}

	// Unlock pixels
	if(AndroidBitmap_unlockPixels(jniEnv, outputBitmap) != ANDROID_BITMAP_RESUT_SUCCESS)
	{
        free(inputBuffer);
		jniEnv->DeleteLocalRef(outputBitmap);
		jniEnv->ThrowNew(jrefs::java::lang::RuntimeException->jclassRef, "Failed to unlock Bitmap pixels");
		return 0;
	}

	// Release buffer
    free(inputBuffer);

    if (options)
    {
	    // Set width and height values
	    jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outWidth, bitmapWidth);
	    jniEnv->SetIntField(options, jrefs::android::graphics::BitmapFactory->Options.outHeight, bitmapHeight);
	}

	return outputBitmap;
}


#ifdef __cplusplus
}
#endif