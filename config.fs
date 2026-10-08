[AID_VENDOR_QCOM_DIAG]
value: 2950

[AID_VENDOR_QCOM_RFS]
value: 2951

[AID_VENDOR_QCOM_RFS_SHARED]
value: 2952

# The legacy Wi-Fi HAL loads bcmdhd.ko itself while running as wifi.
[vendor/bin/hw/android.hardware.wifi@1.0-service]
mode: 0755
user: AID_WIFI
group: AID_WIFI
caps: NET_ADMIN NET_RAW SYS_MODULE
