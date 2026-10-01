/* SPDX-License-Identifier: Apache-2.0 */
import android.app.ActivityThread;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Handler;
import android.os.Looper;
import android.os.PowerManager;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

public class SensorProbe implements SensorEventListener {
    private final Map<Integer, Integer> counts = new HashMap<Integer, Integer>();
    private final Map<Integer, float[]> last = new HashMap<Integer, float[]>();
    private long badValues;
    public void onAccuracyChanged(Sensor sensor, int accuracy) {}
    public void onSensorChanged(SensorEvent event) {
        Integer type = event.sensor.getType();
        counts.put(type, counts.containsKey(type) ? counts.get(type) + 1 : 1);
        last.put(type, event.values.clone());
        for (float value : event.values)
            if (Float.isNaN(value) || Float.isInfinite(value)) ++badValues;
    }
    public static void main(String[] args) {
        Looper.prepareMainLooper();
        final Context context = ActivityThread.systemMain().getSystemContext();
        final PowerManager power = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
        // HMC5883L is a non-wakeup sensor and early-suspends with the display.
        // Bound the test wake lock so an unattended probe cannot keep it awake.
        final PowerManager.WakeLock wake = power.newWakeLock(
            PowerManager.SCREEN_BRIGHT_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP,
            "MioneSensorProbe");
        wake.acquire(15000);
        final SensorManager manager = (SensorManager)context.getSystemService(Context.SENSOR_SERVICE);
        final SensorProbe probe = new SensorProbe();
        final int[] types = {Sensor.TYPE_ACCELEROMETER, Sensor.TYPE_MAGNETIC_FIELD,
            Sensor.TYPE_ORIENTATION, Sensor.TYPE_LIGHT, Sensor.TYPE_PROXIMITY};
        boolean failed = false;
        for (int type : types) {
            Sensor sensor = manager.getDefaultSensor(type);
            boolean ok = sensor != null && manager.registerListener(probe, sensor, 50000);
            System.out.println("SENSOR register type=" + type + " ok=" + ok);
            if (!ok) failed = true;
        }
        if (failed) { manager.unregisterListener(probe); wake.release(); System.exit(1); }
        new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
            public void run() {
                manager.unregisterListener(probe);
                int failures = probe.badValues == 0 ? 0 : 1;
                for (int type : types) {
                    int count = probe.counts.containsKey(type) ? probe.counts.get(type) : 0;
                    float[] values = probe.last.get(type);
                    System.out.print("SENSOR type=" + type + " events=" + count + " last=");
                    if (values != null)
                        for (float value : values) System.out.printf(Locale.US, " %.3f", value);
                    System.out.println();
                    if (count == 0) ++failures;
                }
                System.out.println("SENSOR software test failures=" + failures + "; calibration accuracy untested");
                if (wake.isHeld()) wake.release();
                System.exit(failures == 0 ? 0 : 1);
            }
        }, 10000);
        Looper.loop();
    }
}
