import android.media.*;
import java.nio.*;
public class CodecProbe {
 public static void main(String[] a) throws Exception {
  int cycles=a.length==0?1:Integer.parseInt(a[0]);
  for(int cycle=0;cycle<cycles;cycle++){
   System.out.println("CODEC cycle="+cycle);
  MediaCodec codec=MediaCodec.createEncoderByType("video/avc");
  MediaFormat fmt=MediaFormat.createVideoFormat("video/avc",640,480);
  fmt.setInteger(MediaFormat.KEY_COLOR_FORMAT,21);
  fmt.setInteger(MediaFormat.KEY_BIT_RATE,2000000);
  fmt.setInteger(MediaFormat.KEY_FRAME_RATE,30);
  fmt.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL,1);
  codec.configure(fmt,null,null,MediaCodec.CONFIGURE_FLAG_ENCODE);
  codec.start();System.out.println("CODEC started");
  ByteBuffer[] inputs=codec.getInputBuffers();MediaCodec.BufferInfo info=new MediaCodec.BufferInfo();
  int fed=0,done=0;boolean eos=false,receivedEos=false;
  long deadline=System.currentTimeMillis()+15000;
  while(System.currentTimeMillis()<deadline) {
   if(!eos) {
    int idx=codec.dequeueInputBuffer(10000);
    if(idx>=0) {
     if(fed==60){codec.queueInputBuffer(idx,0,0,2000000,MediaCodec.BUFFER_FLAG_END_OF_STREAM);eos=true;}
     else {ByteBuffer b=inputs[idx];b.clear();byte[] data=new byte[460800];java.util.Arrays.fill(data,(byte)128);b.put(data);codec.queueInputBuffer(idx,0,data.length,fed*33333L,0);fed++;}
    }
   }
   int out=codec.dequeueOutputBuffer(info,10000);
   if(out>=0){done++;boolean end=(info.flags&MediaCodec.BUFFER_FLAG_END_OF_STREAM)!=0;codec.releaseOutputBuffer(out,false);if(end){receivedEos=true;break;}}
  }
  System.out.println("CODEC fed="+fed+" outputs="+done);
  codec.stop();codec.release();if(fed!=60||done<60||!receivedEos)throw new Exception("Incomplete encoding");Thread.sleep(500);
  }
  Thread.sleep(3000);System.out.println("CODEC PASS cycles="+cycles);
 }
}
