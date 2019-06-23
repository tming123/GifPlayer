package com.tming.wy.gifplayer;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE"
    };


    private Bitmap bitmap;
    private ImageView imageView;
    private GifHandler gifHandler;
    @SuppressLint("HandlerLeak")
    private Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int mNextFrame = gifHandler.updateFrame(bitmap);
            handler.sendEmptyMessageDelayed(1, mNextFrame);
            imageView.setImageBitmap(bitmap);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imageView = findViewById(R.id.img_gif);
        findViewById(R.id.btn_play_gif).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (bitmap != null) {
                    int nextFrame = gifHandler.updateFrame(bitmap);
                    handler.sendEmptyMessageDelayed(1, nextFrame);
                } else {
                    ndkLoadGif(imageView);
                }
            }
        });

        verifyStoragePermissions(this);

        new Thread(new Runnable() {
            @Override
            public void run() {
                ndkLoadGif(imageView);
            }
        }).start();
    }

    public void ndkLoadGif(View view) {
        File file = new File(Environment.getExternalStorageDirectory(), "demo.gif");
        if (file.exists()) {
            gifHandler = new GifHandler(file.getAbsolutePath());
            int width = gifHandler.getWidth();
            int height = gifHandler.getHeight();
            bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        }
    }

    public static void verifyStoragePermissions(Activity activity) {
        try {
            int permission = ActivityCompat.checkSelfPermission(activity,
                    "android.permission.WRITE_EXTERNAL_STORAGE");
            if (permission != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE, REQUEST_EXTERNAL_STORAGE);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
