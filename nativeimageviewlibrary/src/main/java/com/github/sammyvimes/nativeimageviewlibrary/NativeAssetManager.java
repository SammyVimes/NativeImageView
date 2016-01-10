package com.github.sammyvimes.nativeimageviewlibrary;

import android.content.Context;
import android.content.res.AssetManager;

/**
 * Created by Semyon on 10.01.2016.
 */
public class NativeAssetManager {

    private static native void init_asset_manager(final AssetManager assetManager);

    public NativeAssetManager(final Context context) {
        AssetManager assets = context.getAssets();
        init_asset_manager(assets);
    }

    static {
        System.loadLibrary("nativeimageviewlib");
    }

}
