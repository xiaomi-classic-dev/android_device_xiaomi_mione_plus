import android.app.ActivityThread;
import android.content.pm.ApplicationInfo;
import android.graphics.SurfaceTexture;
import android.graphics.BitmapFactory;
import android.hardware.Camera;
import android.media.MediaRecorder;
import android.media.MediaMetadataRetriever;
import android.os.Handler;
import android.os.Looper;
import java.lang.reflect.*;
import java.io.*;
import java.util.*;

public class CameraProbe {
 static Camera c; static SurfaceTexture texture; static Handler h; static int frames=0; static int restartFrames=0; static boolean finished=false; static boolean video=false; static String out; static long started; static int pw=2592,ph=1944,vw=640,vh=480; static boolean zsl=false; static boolean audio=false;
 static void log(String s) { System.out.println("PROBE "+(System.currentTimeMillis()-started)+" "+s); }
 static void finish(int rc) { if(finished)return; finished=true; try{if(c!=null)c.release();}catch(Throwable t){} try{if(texture!=null)texture.release();}catch(Throwable t){} log("RESULT="+(rc==0?"PASS":"FAIL"));System.exit(rc); }
 public static void main(String[] args) throws Exception {
  started=System.currentTimeMillis(); out=args.length>0?args[0]:"/data/local/tmp/mione-probe";video=args.length>1&&args[1].equals("video");if(args.length>2){if(args[2].startsWith("720p")){vw=1280;vh=720;}if(args[2].equals("8mp")){pw=3264;ph=2448;}if(args[2].equals("zsl")){zsl=true;pw=1024;ph=768;}audio=args[2].endsWith("-audio");}
  Looper.prepareMainLooper(); ActivityThread at=ActivityThread.systemMain();
  Class<?> b=Class.forName("android.app.ActivityThread$AppBindData"); Constructor<?> bc=b.getDeclaredConstructor();bc.setAccessible(true); Object bd=bc.newInstance();Field ai=b.getDeclaredField("appInfo");ai.setAccessible(true);ApplicationInfo info=new ApplicationInfo();info.packageName="com.android.shell";ai.set(bd,info);Field ba=ActivityThread.class.getDeclaredField("mBoundApplication");ba.setAccessible(true);ba.set(at,bd);
  h=new Handler(Looper.getMainLooper());h.postDelayed(new Runnable(){public void run(){log("TIMEOUT frames="+frames);finish(2);}},35000);
  h.post(new Runnable(){public void run(){try{start();}catch(Throwable t){t.printStackTrace();finish(1);}}});Looper.loop();
 }
 static void start() throws Exception {
  log("cameras="+Camera.getNumberOfCameras());c=Camera.open(0);log("OPEN");c.setErrorCallback(new Camera.ErrorCallback(){public void onError(int e,Camera cam){log("CAMERA_ERROR="+e);finish(1);}});
  Camera.Parameters p=c.getParameters();log("PARAMETERS "+p.flatten());p.setPreviewSize(vw,vh);p.setPictureSize(pw,ph);p.set("video-size",vw+"x"+vh);p.set("zsl",zsl?"on":"off");p.setFlashMode("off");p.setRecordingHint(video);c.setParameters(p);
  texture=new SurfaceTexture(0);c.setPreviewTexture(texture);c.setPreviewCallback(new Camera.PreviewCallback(){public void onPreviewFrame(byte[] data,Camera camera){frames++;if(frames==1||frames==30){int min=255,max=0;long sum=0;for(int i=0;i<Math.min(data.length,640*480);i+=97){int v=data[i]&255;min=Math.min(min,v);max=Math.max(max,v);sum+=v;}log("FRAME="+frames+" bytes="+data.length+" min="+min+" max="+max+" sum="+sum);}if(frames==30){c.setPreviewCallback(null);h.post(new Runnable(){public void run(){try{if(video)record();else photo();}catch(Throwable t){t.printStackTrace();finish(1);}}});}}});c.startPreview();log("PREVIEW_STARTED");
 }
 static void photo() throws Exception {
  c.autoFocus(new Camera.AutoFocusCallback(){public void onAutoFocus(boolean ok,Camera camera){log("AF="+ok);h.postDelayed(new Runnable(){public void run(){capture();}},200);}});
 }
 static void capture(){try{c.takePicture(null,null,new Camera.PictureCallback(){public void onPictureTaken(byte[] bytes,Camera camera){try{FileOutputStream f=new FileOutputStream(out+".jpg");f.write(bytes);f.close();BitmapFactory.Options o=new BitmapFactory.Options();o.inJustDecodeBounds=true;BitmapFactory.decodeByteArray(bytes,0,bytes.length,o);log("JPEG bytes="+bytes.length+" size="+o.outWidth+"x"+o.outHeight);if(o.outWidth!=pw||o.outHeight!=ph)throw new Exception("JPEG dimensions");c.setPreviewCallback(new Camera.PreviewCallback(){public void onPreviewFrame(byte[] data,Camera camera){if(++restartFrames==10){c.setPreviewCallback(null);log("PREVIEW_RESTART frames="+restartFrames);h.postDelayed(new Runnable(){public void run(){finish(0);}},1200);}}});c.startPreview();}catch(Throwable t){t.printStackTrace();finish(1);}}});}catch(Throwable t){t.printStackTrace();finish(1);}}
 static void record() throws Exception {
  c.stopPreview();c.unlock();final MediaRecorder m=new MediaRecorder();m.setCamera(c);if(audio)m.setAudioSource(MediaRecorder.AudioSource.CAMCORDER);m.setVideoSource(MediaRecorder.VideoSource.CAMERA);m.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);m.setVideoEncoder(MediaRecorder.VideoEncoder.H264);if(audio){m.setAudioEncoder(MediaRecorder.AudioEncoder.AAC);m.setAudioSamplingRate(48000);m.setAudioEncodingBitRate(96000);m.setAudioChannels(1);}m.setVideoSize(vw,vh);m.setVideoFrameRate(30);m.setVideoEncodingBitRate(vw>1280?8000000:2000000);m.setOutputFile(out+".mp4");m.prepare();m.start();log("RECORD_STARTED audio="+audio);h.postDelayed(new Runnable(){public void run(){try{m.stop();m.release();MediaMetadataRetriever meta=new MediaMetadataRetriever();meta.setDataSource(out+".mp4");log("VIDEO bytes="+new File(out+".mp4").length()+" duration="+meta.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION)+" width="+meta.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_WIDTH)+" height="+meta.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_HEIGHT));String duration=meta.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);String width=meta.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_WIDTH);String height=meta.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_HEIGHT);meta.release();if(duration==null||width==null||height==null||Long.parseLong(duration)<4000||!width.equals(Integer.toString(vw))||!height.equals(Integer.toString(vh)))throw new Exception("Invalid video metadata");h.postDelayed(new Runnable(){public void run(){finish(0);}},3000);}catch(Throwable t){t.printStackTrace();finish(1);}}},5000);
 }
}
