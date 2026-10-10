/* SPDX-License-Identifier: Apache-2.0 */
#define _LARGEFILE64_SOURCE
#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <set>
#include <fcntl.h>
#include <dirent.h>
#include <linux/fs.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>
#include <unistd.h>
#ifdef MIONE_RECOVERY
#include <cutils/properties.h>
#endif
#include "capacity.h"
static bool mutationStarted=false;
static void prop(const char *key,const std::string &value);
static void closeRollbackStore();
// Static Android 7 ARM exception unwinding crashes before reaching a catch.
// Guard failures are terminal: publish the recovery state and exit directly.
[[noreturn]] static void fail(const std::string &message) {
    fprintf(stderr,"MiOne LVM: %s\n",message.c_str());
    prop("ready","0"); prop("rollback","0"); prop("phase",mutationStarted?"reboot":"failed");
    prop("error",message); closeRollbackStore(); fflush(nullptr); _exit(1);
}
static void need(bool ok,const std::string &s) { if(!ok) fail(s); }
static std::string num(uint64_t n) { return std::to_string(n); }
static void readAt(int fd,void *data,size_t len,uint64_t at) {
    char *p=static_cast<char*>(data);
    while(len) { ssize_t n=pread64(fd,p,len,at); if(n<0 && errno==EINTR) continue;
        need(n>0,"short geometry read"); p+=n; len-=n; at+=n; }
}
#include "geometry.h"
static const char *const pv[] = {"/dev/block/mmcblk0p15","/dev/block/mmcblk0p20"};
static const char *const lv[] = {"/dev/block/mapper/mione-system","/dev/block/mapper/mione-userdata"};
static const char *tool="/sbin/mione-lvm2";
static void prop(const char *key,const std::string &value) {
#ifdef MIONE_RECOVERY
    property_set((std::string("mione.resize.")+key).c_str(),value.substr(0,90).c_str());
#else
    (void)key; (void)value;
#endif
}
static std::string file(const std::string &name) {
    FILE *f=fopen(name.c_str(),"r"); need(f!=nullptr,"cannot read "+name);
    std::string out; char b[1024]; while(size_t n=fread(b,1,sizeof(b),f)) out.append(b,n);
    need(!ferror(f),"read failed "+name); fclose(f); return out;
}
static bool exists(const std::string &p) { return access(p.c_str(),F_OK)==0; }
static std::string trim(std::string s) {
    size_t a=s.find_first_not_of(" \t\r\n"); if(a==std::string::npos) return "";
    return s.substr(a,s.find_last_not_of(" \t\r\n")-a+1);
}
static std::vector<std::vector<std::string>> rows(const std::string &s) {
    std::vector<std::vector<std::string>> out; std::istringstream lines(s); std::string line;
    while(std::getline(lines,line)) { if(trim(line).empty()) continue;
        std::vector<std::string> r; std::istringstream fields(line); std::string f;
        while(std::getline(fields,f,'|')) r.push_back(trim(f));
        out.push_back(r); }
    return out;
}
// TWRP's command launcher inherits its open input/font/FIFO handles.
// Only standard streams belong to this controller or its command children.
static void closeExtraFds() {
    DIR *d=opendir("/proc/self/fd");
    if(!d) { perror("MiOne LVM: cannot enumerate inherited descriptors"); _exit(1); }
    int own=dirfd(d); struct dirent *entry;
    while((entry=readdir(d))) {
        int fd=atoi(entry->d_name);
        if(fd>2 && fd!=own) close(fd);
    }
    closedir(d);
}
static std::string run(std::vector<std::string> args,const char *argv0=nullptr) {
    need(!args.empty(),"empty command");
    std::vector<char*> av; for(auto &s:args) av.push_back(&s[0]); av.push_back(nullptr);
    if(argv0) av[0]=const_cast<char*>(argv0);
    int pipefd[2]; need(pipe(pipefd)==0,"pipe failed"); pid_t child=fork(); need(child>=0,"fork failed");
    if(child==0) { close(pipefd[0]); dup2(pipefd[1],1); close(pipefd[1]); closeExtraFds(); execv(args[0].c_str(),av.data()); _exit(127); }
    close(pipefd[1]); std::string out; char b[1024]; ssize_t n;
    while((n=read(pipefd[0],b,sizeof(b)))!=0) { if(n<0 && errno==EINTR) continue;
        need(n>0,"child read failed"); out.append(b,n); need(out.size()<1024*1024,"command output too large"); }
    close(pipefd[0]); int st=0; need(waitpid(child,&st,0)==child,"wait failed");
    need(WIFEXITED(st) && WEXITSTATUS(st)==0,"command failed: "+args[0]+" "+(args.size()>1?args[1]:""));
    return out;
}
static std::string lvm(std::vector<std::string> a) { a.insert(a.begin(),tool); // LVM dispatches by basename(argv[0]); mione-lvm2 is not a recognised multi-call name.
    return run(a,"lvm.static"); }
static void mkdirs() {
    for(const char *p:{"/dev/mione-lvm","/dev/mione-lvm/run","/dev/mione-lvm/lock","/dev/mione-lvm/archive","/dev/mione-lvm/backup","/dev/block/mapper"})
        need(mkdir(p,0700)==0 || errno==EEXIST,"mkdir failed");
    if(access(tool,X_OK)) tool="/vendor/bin/mione-lvm2";
    setenv("LVM_SYSTEM_DIR",!strcmp(tool,"/sbin/mione-lvm2")?"/lvm/etc":"/vendor/etc/lvm",1);
    need(mkdir("/dev/mapper",0755)==0 || errno==EEXIST,"cannot create mapper directory");
    unsigned ma=0,mi=0; FILE *dm=fopen("/sys/class/misc/device-mapper/dev","r");
    need(dm && fscanf(dm,"%u:%u",&ma,&mi)==2,"kernel lacks device mapper"); fclose(dm);
    need(mknod("/dev/mapper/control",S_IFCHR|0600,makedev(ma,mi))==0 || errno==EEXIST,"cannot create dm control"); setenv("LC_ALL","C",1);
}
static uint64_t integer(const std::string &s) {
    need(!s.empty() && s.find_first_not_of("0123456789")==std::string::npos,"invalid numeric value");
    char *end; errno=0; unsigned long long v=strtoull(s.c_str(),&end,10); need(!errno && !*end,"numeric overflow"); return v;
}
static Layout geometry() {
    int fd=open("/dev/block/mmcblk0",O_RDONLY|O_CLOEXEC); need(fd>=0,"eMMC unavailable");
    uint64_t bytes=0; unsigned sector=0; struct stat st;
    need(!fstat(fd,&st) && S_ISBLK(st.st_mode) && !ioctl(fd,BLKGETSIZE64,&bytes) && !ioctl(fd,BLKSSZGET,&sector)
         && bytes==kDiskSectors*512 && sector==512,"not MiOne 4GB eMMC");
    need(trim(file("/sys/block/mmcblk0/device/type"))=="MMC","not internal eMMC");
    Layout g=readLayout(fd); close(fd);
    for(unsigned i=0;i<2;i++) {
        std::string p="/sys/class/block/mmcblk0p"+std::string(i?"20":"15")+"/";
        Part expected=part(g.n[i?16:11]);
        need(integer(trim(file(p+"start")))==expected.start && integer(trim(file(p+"size")))==expected.size,"kernel/table geometry mismatch");
        need(!stat(pv[i],&st) && S_ISBLK(st.st_mode) && major(st.st_rdev)==179 && minor(st.st_rdev)==(i?20:15),"wrong PV device");
    }
    return g;
}
static bool label(const char *p) {
    int fd=open(p,O_RDONLY|O_CLOEXEC); need(fd>=0,"PV read failed"); unsigned char b[2048]; readAt(fd,b,sizeof(b),0); close(fd);
    for(unsigned i=0;i<4;i++) if(!memcmp(b+i*512,"LABELONE",8)) {
        need(!memcmp(b+i*512+24,"LVM2 001",8),"unknown volume label"); return true; }
    return false;
}
static bool pool(bool incomplete=false) {
    bool a=label(pv[0]), b=label(pv[1]);
    if(!a && !b) return false;
    if(incomplete) {
        // Resume a first conversion interrupted before vgcreate, only if every
        // surviving PV is an orphan. Never adopt a foreign/missing VG.
        bool orphan=true;
        for(unsigned i=0;i<2;i++) if(i?b:a) {
            auto r=rows(lvm({"pvs","--noheadings","-o","vg_name",pv[i]}));
            if(!r.empty()) orphan=false;
        }
        if(orphan) return false;
    }
    need(a && b,"incomplete conversion: one PV label missing; do not use raw System/Data");
    auto r=rows(lvm({"pvs","--noheadings","--separator","|","-o","pv_name,vg_name,pv_attr",pv[0],pv[1]}));
    need(r.size()==2,"expected two PVs"); std::set<std::string> found;
    for(auto &v:r) { need(v.size()==3 && (v[0]==pv[0] || v[0]==pv[1]),"unexpected PV");
        need(v[1]=="mione" && v[2].find('m')==std::string::npos,"foreign/missing VG; raw fallback forbidden"); found.insert(v[0]); }
    need(found.size()==2,"duplicate PV");
    auto vg=rows(lvm({"vgs","--units","b","--nosuffix","--noheadings","--separator","|","-o","vg_name,pv_count,vg_attr,vg_extent_size","mione"}));
    need(vg.size()==1 && vg[0].size()==4 && vg[0][0]=="mione" && vg[0][1]=="2" && vg[0][2].find('p')==std::string::npos,"partial or unexpected VG");
    need(strtod(vg[0][3].c_str(),nullptr)==1048576,"VG extent must be 1 MiB");
    auto seg=rows(lvm({"lvs","-a","--segments","--noheadings","--separator","|","-o","lv_name,segtype,devices","mione"}));
    std::set<std::string> names;
    for(auto &v:seg) {
        need(v.size()==3 && (v[0]=="system" || v[0]=="userdata") && v[1]=="linear","unexpected LV or non-linear segment");
        auto pos=v[2].find('('); need(pos!=std::string::npos && (v[2].substr(0,pos)==pv[0] || v[2].substr(0,pos)==pv[1]),"LV refers to an unexpected PV");
        names.insert(v[0]);
    }
    if(!incomplete) need(names.size()==2,"incomplete LV creation; boot refused");
    return true;
}
static std::string dmNode(const char *name) {
    DIR *dir=opendir("/sys/class/block"); need(dir,"cannot read block devices"); struct dirent *ent;
    std::string result;
    while((ent=readdir(dir))) {
        std::string n=ent->d_name; if(n.compare(0,3,"dm-")) continue;
        if(trim(file("/sys/class/block/"+n+"/dm/name"))!=name) continue;
        need(result.empty(),"duplicate dm name");
        need(trim(file("/sys/class/block/"+n+"/dm/uuid")).compare(0,4,"LVM-")==0,"foreign dm map");
        std::string slaves="/sys/class/block/"+n+"/slaves";
        DIR *sd=opendir(slaves.c_str()); need(sd,"cannot validate dm backing devices");
        struct dirent *se; unsigned count=0;
        while((se=readdir(sd))) {
            if(se->d_name[0]=='.') continue;
            need(!strcmp(se->d_name,"mmcblk0p15") || !strcmp(se->d_name,"mmcblk0p20"),"dm map uses a protected or foreign partition"); count++;
        }
        closedir(sd); need(count>0,"empty dm map");
        result="/dev/block/"+n;
        auto dev=trim(file("/sys/class/block/"+n+"/dev")); auto p=dev.find(':'); need(p!=std::string::npos,"bad dm device number");
        dev_t number=makedev(integer(dev.substr(0,p)),integer(dev.substr(p+1)));
        if(mknod(result.c_str(),S_IFBLK|0600,number)!=0) { struct stat st; need(errno==EEXIST && !stat(result.c_str(),&st) && S_ISBLK(st.st_mode) && st.st_rdev==number,"dm node conflict"); }
    }
    closedir(dir); need(!result.empty(),"missing dm map"); return result;
}
static void activate() {
    lvm({"vgchange","-ay","mione"});
    for(unsigned i=0;i<2;i++) {
        auto path=dmNode(i?"mione-userdata":"mione-system");
        unlink(lv[i]); need(!symlink(path.c_str(),lv[i]),"cannot link LV");
    }
}
// Stock init replaces the bootstrap's temporary /dev. Restore aliases in the
// final tmpfs after validating both active maps; never activate or replace one.
static void linkActive() {
    std::string paths[2]; bool missing[2]={false,false};
    for(unsigned i=0;i<2;i++) {
        paths[i]=dmNode(i?"mione-userdata":"mione-system");
        struct stat expected,actual;
        need(!stat(paths[i].c_str(),&expected) && S_ISBLK(expected.st_mode),"missing active LV block node");
        if(lstat(lv[i],&actual)) {
            need(errno==ENOENT,"cannot inspect LV alias"); missing[i]=true;
        } else {
            need(!stat(lv[i],&actual) && S_ISBLK(actual.st_mode) && actual.st_rdev==expected.st_rdev,
                 "existing LV alias does not match the active map");
        }
    }
    for(unsigned i=0;i<2;i++) if(missing[i])
        need(!symlink(paths[i].c_str(),lv[i]),"cannot restore active LV alias");
    puts("MiOne LVM: active System/Data aliases ready for OTA installation");
}
static void replace(std::string &s,const std::string &from,const std::string &to) {
    size_t at=0; while((at=s.find(from,at))!=std::string::npos) { s.replace(at,from.size(),to); at+=to.size(); }
}
static void save(const char *p,const std::string &s) {
    std::string temp=std::string(p)+".tmp"; int fd=open(temp.c_str(),O_WRONLY|O_CREAT|O_TRUNC|O_NOFOLLOW,0600);
    need(fd>=0,"cannot write generated fstab"); const char *a=s.data(); size_t left=s.size();
    while(left) { ssize_t n=write(fd,a,left); if(n<0 && errno==EINTR) continue; need(n>0,"fstab write failed"); a+=n; left-=n; }
    need(!fsync(fd),"fstab sync failed"); close(fd); need(!rename(temp.c_str(),p),"fstab rename failed");
}
static void fstab(const char *source,const char *dest,bool converted) {
    std::string s=file(source);
    if(converted) {
        replace(s,"/dev/block/platform/msm_sdcc.1/by-name/system",dmNode("mione-system"));
        replace(s,"/dev/block/platform/msm_sdcc.1/by-name/userdata",dmNode("mione-userdata"));
    }
    #ifdef MIONE_RECOVERY
    if(converted) replace(s,";encryptable=footer","");
#endif
    save(dest,s);
}
static bool depends(dev_t dev,unsigned depth=0) {
    need(depth<16,"block dependency loop");
    if(major(dev)==179 && (minor(dev)==0 || minor(dev)==15 || minor(dev)==20)) return true;
    std::string base="/sys/dev/block/"+num(major(dev))+":"+num(minor(dev))+"/slaves";
    DIR *d=opendir(base.c_str()); if(!d) return false; struct dirent *ent; bool busy=false;
    while((ent=readdir(d))) { if(ent->d_name[0]=='.') continue;
        std::string s=trim(file(base+"/"+ent->d_name+"/dev")); size_t p=s.find(':'); need(p!=std::string::npos,"bad dependency");
        busy=busy || depends(makedev(integer(s.substr(0,p)),integer(s.substr(p+1))),depth+1); }
    closedir(d); return busy;
}
static void idle() {
    FILE *f=fopen("/proc/mounts","r"); need(f,"cannot check mounts"); char src[512],dst[512],t[128],o[1024]; int x,y;
    while(fscanf(f,"%511s %511s %127s %1023s %d %d",src,dst,t,o,&x,&y)==6) {
        struct stat st; need(stat(dst,&st)!=0 || !depends(st.st_dev),"System/Data or a dependent filesystem is mounted"); }
    fclose(f); f=fopen("/proc/swaps","r"); need(f,"cannot check swaps"); char line[2048];
    while(fgets(line,sizeof(line),f)) { if(sscanf(line,"%511s",src)!=1) continue;
        struct stat st; if(!stat(src,&st)) need(!depends(S_ISBLK(st.st_mode)?st.st_rdev:st.st_dev),"System/Data swap active"); }
    fclose(f);
    for(const char *p:{"/sys/class/android_usb/android0/f_mass_storage/lun/file","/sys/class/android_usb/android0/f_mass_storage/lun0/file"}) {
        if(!exists(p)) continue;
        std::string s=trim(file(p)); struct stat st;
        if(!s.empty() && !stat(s.c_str(),&st)) need(!depends(S_ISBLK(st.st_mode)?st.st_rdev:st.st_dev),"System/Data USB export active");
    }
}
#ifdef MIONE_RECOVERY
#include "rollback.h"
#else
static void closeRollbackStore() {}
static bool rollbackPending(const Layout &) { return false; }
#endif
static unsigned budget(const Layout &g,bool converted) {
    if(converted) { auto r=rows(lvm({"vgs","--units","m","--nosuffix","--noheadings","-o","vg_extent_count","mione"}));
        need(r.size()==1 && r[0].size()==1,"bad capacity report"); return integer(r[0][0]); }
    // 1 MiB data offset in each PV; remainder smaller than an extent is unavailable.
    return part(g.n[11]).size/2048+part(g.n[16]).size/2048-2;
}
static bool resizeReady(unsigned target,const Layout &g) {
    need(access("/sbin/recovery",X_OK)==0,"destructive operation requires recovery");
    need(exists("/etc/mione-lvm-ready"),"LVM-aware recovery packaging incomplete");
    need(!rollbackPending(g),"Rollback pending; finish Restore stock partitions first");
    bool converted=pool(true);
    need(target>=MIONE_SYSTEM_MIN_MIB && target<=2560 && target+512<=budget(g,converted),"size outside safe capacity range");
    need(access("/sbin/make_ext4fs",X_OK)==0,"formatter unavailable");
    lvm({"version"}); // Read-only: validate the packaged executable before any write.
    idle();
    return converted;
}
static void apply(unsigned target,const char *yes,const Layout &g) {
    need(!strcmp(yes,"yes"),"type exactly yes to authorize data loss");
    bool converted=resizeReady(target,g);
    fprintf(stderr,"MiOne LVM: preflight passed; starting authorised System/Data recreation\n");
    prop("phase","working"); mutationStarted=true; prop("reload","1");
    if(!converted) {
        for(unsigned i=0;i<2;i++) if(!label(pv[i]))
            lvm({"pvcreate","--yes","--metadatacopies","1","--metadatasize","256k","--dataalignment","1m",pv[i]});
        lvm({"vgcreate","--physicalextentsize","1m","mione",pv[0],pv[1]});
    } else {
        // The VG/PVs stay in place. Do not remove/recreate their redundant metadata.
        auto r=rows(lvm({"lvs","--noheadings","-o","lv_name","mione"}));
        for(auto &v:r) { need(v.size()==1 && (v[0]=="system"||v[0]=="userdata"),"unexpected LV"); lvm({"lvremove","--force","mione/"+v[0]}); }
    }
    lvm({"lvcreate","--zero","n","-L",num(target)+"m","-n","system","mione"});
    lvm({"lvcreate","--zero","n","-l","100%FREE","-n","userdata","mione"});
    need(pool(),"pool verification failed"); activate();
    run({"/sbin/make_ext4fs","-L","system",lv[0]}); run({"/sbin/make_ext4fs","-L","data",lv[1]});
    int fd=open("/dev/block/mmcblk0",O_RDONLY); need(fd>=0,"cannot recheck partition table"); Layout now=readLayout(fd); close(fd);
    need(!memcmp(&g,&now,sizeof(g)),"partition table changed unexpectedly");
    // TWRP has already cached its partition objects. Reload via recovery reboot before installing.
    prop("phase","reboot"); prop("current",num(target)); prop("error","");
    puts("System/Data formatted. Reboot THIS recovery to reload LV paths, then install an LVM-aware ROM. MBR/EBR, recovery, persist and cache unchanged.");
}
int main(int argc,char **argv) {
    need(geteuid()==0,"root required"); closeExtraFds(); mkdirs();
        int lock=open("/dev/mione-lvm/controller.lock",O_WRONLY|O_CREAT|O_CLOEXEC,0600);
        need(lock>=0 && !flock(lock,LOCK_EX|LOCK_NB),"another LVM operation is active");
        need(argc>=2,"command required"); std::string cmd=argv[1]; Layout g=geometry();
        if(cmd=="apply") { need(argc==4,"apply requires MiB and yes"); uint64_t n=integer(argv[2]); need(n<=2560,"size overflow"); apply(n,argv[3],g); }
#ifdef MIONE_RECOVERY
        else if(cmd=="rollback") { need(argc==3,"rollback requires yes"); rollback(argv[2],g); }
        else if(cmd=="check-rollback") {
            need(argc==2,"check-rollback takes no arguments");
            if(!rollbackPending(g) && !label(pv[0]) && !label(pv[1])) puts("Already using physical System/Data; no rollback needed.");
            else { rollbackReady(g); puts("MiOne LVM: rollback preflight passed; no partition writes performed"); }
        }
#endif
        else if(cmd=="check-resize") {
            need(argc==3,"check-resize requires MiB"); uint64_t n=integer(argv[2]); need(n<=2560,"size overflow");
            resizeReady(n,g); puts("MiOne LVM: resize preflight passed; no partition writes performed");
        } else if(cmd=="probe") {
            prop("system_path",exists("/system_root")?"/system_root":"/system");
            prop("ready","0"); prop("rollback","0"); prop("reload","0"); prop("error","");
            prop("raw_system",num(part(g.n[11]).size/2048)); prop("raw_data",num(part(g.n[16]).size/2048));
            if(rollbackPending(g)) {
#ifdef MIONE_RECOVERY
                RollbackRecord pending; need(readRollback(g,pending),"rollback record disappeared"); validateRollbackPvs(pending);
#endif
                prop("phase","rollback"); prop("rollback","1");
                prop("error","Rollback interrupted; confirm yes to finish. System/Data mounts are disabled.");
                closeRollbackStore(); close(lock); return 0;
            }
            bool c=pool(true); unsigned current=part(g.n[11]).size/2048;
            if(c) { bool foundSystem=false; auto r=rows(lvm({"lvs","--noheadings","--units","m","--nosuffix","-o","lv_name,lv_size","--separator","|","mione"}));
                for(auto &v:r) { need(v.size()==2,"bad LV size report"); if(v[0]=="system") { current=static_cast<unsigned>(strtod(v[1].c_str(),nullptr)); foundSystem=true; } }
                if(!foundSystem) prop("error","Incomplete LV layout; confirm yes to recreate System/Data");
            } else if(label(pv[0]) || label(pv[1])) prop("error","Incomplete first conversion; confirm yes to resume initialization");
            prop("current",num(current)); prop("budget",num(budget(g,c)));
            prop("phase",c?"lvm":((label(pv[0]) || label(pv[1]))?"incomplete":"legacy"));
            prop("ready","1"); prop("rollback",c?"1":"0");
        } else if(cmd=="prepare") {
            need(argc==4,"prepare source-fstab output-fstab");
            need(!rollbackPending(g),"Rollback pending; System/Data mounts disabled until completion");
            bool c=pool(); if(c) activate(); fstab(argv[2],argv[3],c);
        } else if(cmd=="link-active") {
            need(argc==2,"link-active takes no arguments");
            need(!rollbackPending(g),"Rollback pending; LV aliases remain disabled");
            if(pool()) linkActive();
        } else if(cmd=="check-install") {
            need(!rollbackPending(g),"Rollback pending; installation refused until completion");
            need(argc==3,"check-install required image bytes"); need(pool(),"convert System/Data in the new recovery first"); activate();
            int f=open(lv[0],O_RDONLY); uint64_t n=0; need(f>=0 && !ioctl(f,BLKGETSIZE64,&n),"cannot read LV capacity"); close(f);
            need(n>=integer(argv[2]),"System LV is too small for this ROM; resize in Recovery first");
        } else fail("unknown command");
        closeRollbackStore(); close(lock); return 0;
}
