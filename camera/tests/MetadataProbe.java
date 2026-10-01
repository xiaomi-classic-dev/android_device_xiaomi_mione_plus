/* SPDX-License-Identifier: Apache-2.0 */
import android.media.MediaMetadataRetriever;
import java.io.BufferedReader;
import java.io.FileReader;

public class MetadataProbe {
    public static void main(String[] args) throws Exception {
        BufferedReader reader = new BufferedReader(new FileReader("/proc/self/cgroup"));
        String line;
        while ((line = reader.readLine()) != null) System.out.println("CGROUP " + line);
        reader.close();
        for (int i = 0; i < 3; ++i) {
            MediaMetadataRetriever retriever = new MediaMetadataRetriever();
            retriever.setDataSource(args[0]);
            String duration = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);
            String width = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_WIDTH);
            String height = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_HEIGHT);
            retriever.release();
            if (duration == null || Long.parseLong(duration) < 4000 ||
                    !"1280".equals(width) || !"720".equals(height))
                throw new Exception("Invalid recorded-video metadata");
            System.out.println("METADATA PASS " + i + " duration=" + duration + " size=" + width + "x" + height);
        }
    }
}
