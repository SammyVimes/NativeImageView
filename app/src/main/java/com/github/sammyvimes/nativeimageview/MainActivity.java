package com.github.sammyvimes.nativeimageview;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import com.github.sammyvimes.nativeimageviewlibrary.NativeImageView;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    @Override
    protected void onResume() {
        super.onResume();
        ((NativeImageView) findViewById(R.id.niv)).onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        ((NativeImageView) findViewById(R.id.niv)).onPause();
    }
}
