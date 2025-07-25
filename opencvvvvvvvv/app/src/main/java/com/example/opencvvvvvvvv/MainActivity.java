package com.example.opencvvvvvvvv;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.*;
import android.graphics.Bitmap;
import android.content.res.AssetManager;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("opencvvvvvvvv");
    }

    public native String classifyShape(byte[] imageData, int width, int height, AssetManager assetManager);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        DrawingView drawingView = findViewById(R.id.drawingView);
        Button classifyButton = findViewById(R.id.classifyButton);
        Button clearButton = findViewById(R.id.clearButton);
        TextView resultText = findViewById(R.id.resultText);

        classifyButton.setOnClickListener(v -> {
            Bitmap bmp = drawingView.getBitmap();
            int width = bmp.getWidth();
            int height = bmp.getHeight();

            int size = width * height;
            byte[] pixels = new byte[size];
            int[] intPixels = new int[size];
            bmp.getPixels(intPixels, 0, width, 0, 0, width, height);
            for (int i = 0; i < size; i++) {
                int color = intPixels[i];
                int gray = (color >> 16 & 0xff);
                pixels[i] = (byte) gray;
            }

            String result = classifyShape(pixels, width, height, getAssets());
            resultText.setText("Resultado: " + result);
        });

        clearButton.setOnClickListener(v -> {
            drawingView.clear();
            resultText.setText("Resultado: ");
        });
    }
}
