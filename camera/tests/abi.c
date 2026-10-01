#include <stdio.h>
#include <stddef.h>
#include "mm_camera_interface2.h"
#define SZ(t) printf("sizeof(" #t ")=%u\n", (unsigned)sizeof(t))
#define OF(t,m) printf("offsetof(" #t "," #m ")=%u\n", (unsigned)offsetof(t,m))
#define VAL(v) printf(#v "=0x%x\n", (unsigned)(v))
typedef char verify_camera_handle[(sizeof(mm_camera_t)==68)?1:-1];
typedef char verify_camera_properties[(sizeof(cam_prop_t)==100)?1:-1];
typedef char verify_camera_dimension[(sizeof(cam_ctrl_dimension_t)==420)?1:-1];
typedef char verify_focus_distances[(sizeof(focus_distances_info_t)==12)?1:-1];
typedef char verify_fps_range[(sizeof(cam_sensor_fps_range_t)==8)?1:-1];
typedef char verify_camera_position[(offsetof(mm_camera_t,camera_info.position)==24)?1:-1];
typedef char verify_camera_angle[(offsetof(mm_camera_t,camera_info.sensor_mount_angle)==28)?1:-1];
int main(void) {
 SZ(void*); SZ(mm_camera_t); SZ(cam_prop_t); SZ(cam_ctrl_dimension_t);
 SZ(struct msm_frame); SZ(struct msm_ctrl_cmd); SZ(struct msm_camera_v4l2_ioctl_t);
 SZ(mm_camera_ch_data_buf_t); SZ(mm_camera_event_t); SZ(cam_sock_packet_t);
 SZ(common_crop_t); SZ(focus_distances_info_t); SZ(cam_sensor_fps_range_t);
 OF(mm_camera_t,camera_info); OF(mm_camera_t,video_dev_name);
 OF(cam_ctrl_dimension_t,prev_format); OF(cam_ctrl_dimension_t,display_frame_offset);
 OF(cam_ctrl_dimension_t,channel_interface_mask);
 OF(struct msm_frame,buffer); OF(struct msm_frame,fd); OF(struct msm_frame,ion_dev_fd);
 OF(cam_prop_t,max_pict_width); OF(cam_prop_t,preview_sizes_cnt);
 VAL(CAMERA_GET_CAPABILITIES); VAL(CAMERA_SET_PARM_DIMENSION);
 VAL(CAMERA_GET_PARM_FPS_RANGE); VAL(MM_CAMERA_PARM_RECORDING_HINT);
 VAL(MSM_CAM_V4L2_IOCTL_GET_CAMERA_INFO); VAL(MSM_CAM_V4L2_IOCTL_GET_EVENT_PAYLOAD);
 VAL(ION_IOC_ALLOC); VAL(ION_IOC_IMPORT); VAL(ION_IOC_CLEAN_CACHES);
 return 0;
}
