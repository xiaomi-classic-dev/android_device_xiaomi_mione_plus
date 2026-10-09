/* SPDX-License-Identifier: Apache-2.0
 * Device-owned PID 1 prelude. Restores /init before handing off, so the
 * stock init re-exec and ueventd/watchdogd aliases never re-run this prelude.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>
#include <unistd.h>
static int logfd=-1, diagfd=-1, dev_mounted=0, proc_mounted=0, sys_mounted=0;
static char **init_argv;
#ifdef MIONE_AOSP_RECOVERY
static const char *source="/system/etc/recovery.fstab.physical", *dest="/system/etc/recovery.fstab";
#elif defined(MIONE_RECOVERY)
static const char *source="/etc/recovery.fstab.physical", *dest="/etc/recovery.fstab";
#else
static const char *source="/fstab.qcom.physical", *dest="/fstab.qcom";
#endif
static void trace(const char *what) {
    if(diagfd>=0) dprintf(diagfd,"mione-lvm-init: %s\n",what);
    if(logfd>=0) dprintf(logfd,"<6>mione-lvm-init: %s\n",what);
}
static __attribute__((noreturn)) void halt(const char *what) {
    trace(what);
    for(;;) pause();
}
static __attribute__((noreturn)) void handoff(void) {
    trace("handoff: detach temporary mounts, restore stock /init");
    if(logfd>=0) {close(logfd); logfd=-1;}
    if((sys_mounted && umount2("/sys",MNT_DETACH)) ||
       (proc_mounted && umount2("/proc",MNT_DETACH)) ||
       (dev_mounted && umount2("/dev",MNT_DETACH))) halt("bootstrap mount cleanup failed");
    // Android 7 init re-execs argv[0] as --second-stage. Its /sbin/ueventd
    // alias also points at /init. Leave both routes pointing at STOCK init,
    // including after SELinux is loaded; do not add an extra exec in a domain.
    unlink("/init.mione-stock");
    if(link("/sbin/init.android","/init.mione-stock") ||
       rename("/init.mione-stock","/init")) halt("cannot restore stock /init");
    trace("stock /init restored; executing stock init");
    init_argv[0]="/init";
    execv("/init",init_argv);
    halt("exec stock init failed");
}
static __attribute__((noreturn)) void error(const char *what) {
    int saved=errno; char text[512];
    snprintf(text,sizeof(text),"%s: errno=%d (%s)",what,saved,strerror(saved)); trace(text);
#if defined(MIONE_RECOVERY) || defined(MIONE_AOSP_RECOVERY)
    // Keep recovery/ADB reachable, but never expose raw System/Data after
    // an activation failure. The conversion controller still validates them.
    FILE *in=fopen(source,"r"), *out=fopen(dest,"w"); char line[2048];
    if(!in || !out) halt("cannot disable unsafe recovery mounts");
    while(fgets(line,sizeof(line),in)) {
        if(line[0]!='#' && (strstr(line,"by-name/system") || strstr(line,"by-name/userdata"))) continue;
        if(fputs(line,out)==EOF) halt("cannot write safe recovery fstab");
    }
    if(ferror(in) || fclose(out)) halt("cannot finish safe recovery fstab");
    fclose(in);
    trace("bootstrap failed; System/Data disabled; continuing recovery");
    handoff();
#else
    halt("boot refused; use LVM recovery to repair; raw PV fallback forbidden");
#endif
}
static void node(const char *name,const char *sysfs) {
    unsigned ma,mi; FILE *f=fopen(sysfs,"r");
    if(!f || fscanf(f,"%u:%u",&ma,&mi)!=2) error("block device number unavailable");
    fclose(f); if(mknod(name,S_IFBLK|0600,makedev(ma,mi)) && errno!=EEXIST) error("mknod");
}
int main(int argc,char **argv) {
    if(getpid()!=1) { execv("/sbin/init.android",argv); return 1; }
    // Defensive pass-through for an explicit later init stage.
    if(argc>1 && (!strcmp(argv[1],"--second-stage") || !strcmp(argv[1],"second_stage") ||
                  !strcmp(argv[1],"selinux_setup"))) {execv("/sbin/init.android",argv); return 1;}
    init_argv=argv; umask(022);
    diagfd=open("/mione-lvm-bootstrap.log",O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC,0600);
    trace("first-stage bootstrap started");
    if(mount("tmpfs","/dev","tmpfs",MS_NOSUID,"mode=0755")) error("mount /dev");
    dev_mounted=1;
    mknod("/dev/kmsg",S_IFCHR|0600,makedev(1,11)); logfd=open("/dev/kmsg",O_WRONLY|O_CLOEXEC);
    trace("temporary /dev ready");
    if(mount("proc","/proc","proc",MS_NOSUID|MS_NODEV|MS_NOEXEC,NULL)) error("mount /proc");
    proc_mounted=1;
    if(mount("sysfs","/sys","sysfs",0,NULL)) error("mount /sys");
    sys_mounted=1;
    trace("temporary /proc and /sys ready");
    mknod("/dev/null",S_IFCHR|0666,makedev(1,3));
    mknod("/dev/zero",S_IFCHR|0666,makedev(1,5));
    mknod("/dev/random",S_IFCHR|0666,makedev(1,8));
    mknod("/dev/urandom",S_IFCHR|0666,makedev(1,9));
    mkdir("/dev/block",0755); mkdir("/dev/mapper",0755);
    unsigned control=0; FILE *f=fopen("/sys/class/misc/device-mapper/dev","r"); unsigned ma;
    if(!f || fscanf(f,"%u:%u",&ma,&control)!=2) error("kernel device mapper unavailable");
    fclose(f); if(mknod("/dev/mapper/control",S_IFCHR|0600,makedev(ma,control)) && errno!=EEXIST) error("mknod dm control");
    trace("device mapper control ready; waiting for eMMC");
    for(unsigned i=0;i<500 && access("/sys/class/block/mmcblk0p20/dev",R_OK);i++) usleep(20000);
    node("/dev/block/mmcblk0","/sys/class/block/mmcblk0/dev");
    node("/dev/block/mmcblk0p15","/sys/class/block/mmcblk0p15/dev");
    node("/dev/block/mmcblk0p20","/sys/class/block/mmcblk0p20/dev");
    trace("eMMC nodes ready; starting read-only pool/fstab preparation");
    pid_t p=fork(); if(p<0) error("fork");
    if(p==0) {
        setpgid(0,0);
        if(diagfd>=0) {dup2(diagfd,STDOUT_FILENO);dup2(diagfd,STDERR_FILENO);}
        execl("/sbin/mione-lvm","mione-lvm","prepare",source,dest,(char*)NULL); _exit(127);
    }
    setpgid(p,p);
    int status=0,done=0;
    for(unsigned i=0;i<1500;i++) {
        pid_t r=waitpid(p,&status,WNOHANG);
        if(r==p) {done=1;break;}
        if(r<0 && errno!=EINTR) error("wait prepare");
        usleep(20000);
    }
    if(!done) {kill(-p,SIGKILL);errno=ETIMEDOUT;error("prepare exceeded 30 seconds");}
    if(!WIFEXITED(status) || WEXITSTATUS(status)) {errno=EIO;error("pool/fstab preparation failed (details in bootstrap log)");}
    trace("pool/fstab preparation complete");
    handoff();
}
