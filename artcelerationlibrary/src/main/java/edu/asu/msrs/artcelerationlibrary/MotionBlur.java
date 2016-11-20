/**
 * Thread which handles all requests for MotionBlur transform.
 * Parameters: Messenger messenger, Bitmap input, int requestNo, MemoryFile memoryFile
 * Sends a message to the handler in Library after processing the image using its Messenger object to
 * let the library know that the processed Bitmap output image is ready.
 */

package edu.asu.msrs.artcelerationlibrary;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.Bundle;
import android.os.MemoryFile;
import android.os.Message;
import android.os.Messenger;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import java.io.ByteArrayOutputStream;


public class MotionBlur implements Runnable {
    private Bitmap input;
    private Messenger messenger;
    private int requestNo;
    private MemoryFile memoryFile;


    MotionBlur(Messenger messenger, Bitmap input, int requestNo, MemoryFile memoryFile) {
        this.messenger = messenger;
        this.input = input;
        this.requestNo = requestNo;
        this.memoryFile = memoryFile;
    }


    /**
     * Run method includes all the transform logic for processing the input Bitmap image.
     * Writes the output image to the Memory File and shares the file descriptor of this file
     * with the library.
     */
    @Override
    public void run() {
        // TODO transform Logic
        Log.d("fd", "Motion Blur!");

        // Horizontal motion blur
        int a0 = 1;

        // radius
        int a1=8;

        // Image size, w-> width & h->height
        int w = input.getWidth();
        int h = input.getHeight();

        // Creating bitmap to be returned as a modified (mutable output bitmap)
        Bitmap output = Bitmap.createBitmap(w,h,input.getConfig());
        int size = 2*a1+1;
        int[][] Pr = new int[w][h];
        int[][] Pg = new int[w][h];
        int[][] Pb = new int[w][h];

        if (a0==0) {
            for (int i = 0; i < w; i++) {
                for (int j = 0; j < h; j++) {
                    for (int k = 0; k < size; k++) {
                        int xval = i - a1 + k;
                        if (!(xval < 0 || xval >= w)) {
                            Pr[i][j] += Color.red(input.getPixel(xval, j));
                            Pg[i][j] += Color.green(input.getPixel(xval, j));
                            Pb[i][j] += Color.blue(input.getPixel(xval, j));
                        }
                    }
                    Pr[i][j] /= size;
                    Pg[i][j] /= size;
                    Pb[i][j] /= size;

                    output.setPixel(i, j, Color.argb(255, Pr[i][j], Pg[i][j], Pb[i][j]));
                }
            }
        }else if (a0==1){
            for (int i = 0; i < w; i++) {
                for (int j = 0; j < h; j++) {
                    for (int k = 0; k < size; k++) {
                        int yval = j - a1 + k;
                        if (!(yval < 0 || yval >= h)) {
                            Pr[i][j] += Color.red(input.getPixel(i, yval));
                            Pg[i][j] += Color.green(input.getPixel(i, yval));
                            Pb[i][j] += Color.blue(input.getPixel(i, yval));
                        }
                    }
                    Pr[i][j] /= size;
                    Pg[i][j] /= size;
                    Pb[i][j] /= size;

                    output.setPixel(i, j, Color.argb(255, Pr[i][j], Pg[i][j], Pb[i][j]));
                }
            }
        }else{
            output=input;
        }

        try {
            ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
            output.compress(Bitmap.CompressFormat.JPEG, 100, outputStream);
            byte[] oparray = outputStream.toByteArray();
            memoryFile.getOutputStream().write(oparray);

            Bundle bundle = new Bundle();
            Message message = Message.obtain(null, 10, oparray.length, requestNo);
            ParcelFileDescriptor pfd = MemoryFileUtil.getParcelFileDescriptor(memoryFile);
            bundle.putParcelable("ClassPFD", pfd);
            message.setData(bundle);

            messenger.send(message);
            //Thread.sleep(1000,0);
            memoryFile.allowPurging(true);
            memoryFile.close();

        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}
