/* SPDX-License-Identifier: Apache-2.0
 * Recovery-only rollback. The partition table is never written. A durable
 * Cache record binds interrupted work to the validated table and original PVs.
 */
#include <sys/mount.h>
static const char *const rollbackStore="/dev/mione-lvm/rollback-cache";
static const char *const rollbackName="/dev/mione-lvm/rollback-cache/mione-lvm-rollback";
static bool rollbackMounted=false;
static void closeRollbackStore() {
    if(rollbackMounted) { umount(rollbackStore); rollbackMounted=false; }
}
static void openRollbackStore(const Layout &g) {
    if(rollbackMounted) return;
    const char *node="/dev/block/mmcblk0p19";
    Part expected=part(g.n[15]); struct stat st;
    need(integer(trim(file("/sys/class/block/mmcblk0p19/start")))==expected.start &&
         integer(trim(file("/sys/class/block/mmcblk0p19/size")))==expected.size,"Cache geometry mismatch");
    if(mknod(node,S_IFBLK|0600,makedev(179,19))) need(errno==EEXIST,"cannot create Cache node");
    need(!stat(node,&st) && S_ISBLK(st.st_mode) && st.st_rdev==makedev(179,19),"wrong Cache device");
    need(!mkdir(rollbackStore,0700) || errno==EEXIST,"cannot create rollback store");
    need(!lstat(rollbackStore,&st) && S_ISDIR(st.st_mode),"invalid rollback store directory");
    if(st.st_dev!=makedev(179,19))
        need(!mount(node,rollbackStore,"ext4",MS_NOSUID|MS_NODEV|MS_NOEXEC,nullptr),"cannot mount Cache for rollback record");
    rollbackMounted=true;
}
static std::string tableIdentity(const Layout &g) {
    static const char hex[]="0123456789abcdef";
    std::string out;
    for(const auto &node:g.n) {
        out+=num(node.lba)+":";
        for(unsigned char b:node.bytes) { out+=hex[b>>4]; out+=hex[b&15]; }
        out+='\n';
    }
    return out;
}
static std::string pvIdentity(const char *path) {
    int fd=open(path,O_RDONLY|O_CLOEXEC); need(fd>=0,"cannot read PV identity");
    unsigned char bytes[2048]; readAt(fd,bytes,sizeof(bytes),0); close(fd);
    for(unsigned i=0;i<4;i++) {
        const unsigned char *sector=bytes+i*512;
        if(memcmp(sector,"LABELONE",8)) continue;
        need(!memcmp(sector+24,"LVM2 001",8),"foreign PV label");
        unsigned offset=get32(sector+20); need(offset>=32 && offset<=480,"invalid PV header offset");
        std::string id(reinterpret_cast<const char*>(sector+offset),32);
        need(id.find_first_not_of("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")==std::string::npos,"invalid PV UUID");
        return id;
    }
    return "";
}
struct RollbackRecord { std::string ids[2], table; };
static bool readRollback(const Layout &g,RollbackRecord &record) {
    openRollbackStore(g);
    int fd=open(rollbackName,O_RDONLY|O_CLOEXEC|O_NOFOLLOW);
    if(fd<0) { need(errno==ENOENT,"cannot read rollback record"); return false; }
    struct stat st; need(!fstat(fd,&st) && S_ISREG(st.st_mode) && st.st_size>0 && st.st_size<32768,"invalid rollback record");
    std::string data(static_cast<size_t>(st.st_size),'\0'); readAt(fd,&data[0],data.size(),0); close(fd);
    std::istringstream input(data); std::string magic;
    std::getline(input,magic); std::getline(input,record.ids[0]); std::getline(input,record.ids[1]);
    need(magic=="MIONE-LVM-ROLLBACK-1" && record.ids[0].size()==32 && record.ids[1].size()==32,"invalid rollback record header");
    size_t start=magic.size()+record.ids[0].size()+record.ids[1].size()+3;
    record.table=data.substr(start); need(record.table==tableIdentity(g),"partition table differs from rollback record");
    return true;
}
static bool rollbackPending(const Layout &g) { RollbackRecord record; return readRollback(g,record); }
static void syncRollbackStore() {
    int fd=open(rollbackStore,O_RDONLY|O_DIRECTORY|O_CLOEXEC);
    need(fd>=0 && !fsync(fd),"cannot sync rollback record directory"); close(fd);
}
static bool validateRollbackPvs(const RollbackRecord &record) {
    bool grouped=false;
    for(unsigned i=0;i<2;i++) {
        auto id=pvIdentity(pv[i]); if(id.empty()) continue;
        need(id==record.ids[i],"PV identity changed since rollback began");
        auto owner=rows(lvm({"pvs","--noheadings","-o","vg_name",pv[i]}));
        if(owner.empty()) continue;
        need(owner.size()==1 && owner[0].size()==1 && owner[0][0]=="mione","foreign VG in rollback PV"); grouped=true;
    }
    if(grouped) need(pool(true),"incomplete VG during rollback; repair metadata before continuing");
    return grouped;
}
static void rawDevicesIdle() {
    for(const char *name:{"mmcblk0p15","mmcblk0p20"}) {
        auto path=std::string("/sys/class/block/")+name+"/holders";
        DIR *dir=opendir(path.c_str()); need(dir,"cannot check physical partition holders");
        struct dirent *entry;
        while((entry=readdir(dir))) need(entry->d_name[0]=='.',"physical partition still has a block-device holder");
        closedir(dir);
    }
}
static RollbackRecord rollbackReady(const Layout &g) {
    need(access("/sbin/recovery",X_OK)==0 && exists("/etc/mione-lvm-ready"),"rollback requires this LVM recovery");
    need(access("/sbin/make_ext4fs",X_OK)==0,"formatter unavailable");
    lvm({"version"}); idle(); RollbackRecord record;
    if(readRollback(g,record)) validateRollbackPvs(record);
    else {
        need(pool(true),"System/Data are not in the MiOne LVM pool");
        record.ids[0]=pvIdentity(pv[0]); record.ids[1]=pvIdentity(pv[1]);
        need(!record.ids[0].empty() && !record.ids[1].empty(),"missing rollback PV identity");
        record.table=tableIdentity(g);
    }
    return record;
}
static void rollback(const char *yes,const Layout &g) {
    need(!strcmp(yes,"yes"),"type exactly yes to authorize data loss");
    if(!rollbackPending(g) && !label(pv[0]) && !label(pv[1])) {
        prop("phase","legacy"); prop("rollback","0"); prop("reload","0");
        prop("error","Already using physical partitions; no rollback needed.");
        puts("Already using physical System/Data; no rollback needed. No formatting performed.");
        return;
    }
    auto record=rollbackReady(g);
    // Persist intent before the first destructive command, including removal
    // of an LV. Until commit, the PID 1 prelude disables System/Data mounts.
    save(rollbackName,"MIONE-LVM-ROLLBACK-1\n"+record.ids[0]+"\n"+record.ids[1]+"\n"+record.table);
    syncRollbackStore(); mutationStarted=true; prop("phase","working"); prop("reload","1");
    fprintf(stderr,"MiOne LVM: restoring existing physical System/Data; ALL internal contents will be erased\n");
    if(validateRollbackPvs(record)) {
        lvm({"vgchange","-an","mione"});
        auto volumes=rows(lvm({"lvs","--noheadings","-o","lv_name","mione"}));
        for(const auto &volume:volumes) {
            need(volume.size()==1 && (volume[0]=="system" || volume[0]=="userdata"),"unexpected rollback LV");
            lvm({"lvremove","--force","mione/"+volume[0]});
        }
        need(rows(lvm({"lvs","--noheadings","-o","lv_name","mione"})).empty(),"LV remains before VG removal");
        // This packaged LVM 2.02.98 has no vgremove --yes option. With every
        // validated LV already removed, an empty VG needs no confirmation.
        lvm({"vgremove","mione"});
    }
    need(!validateRollbackPvs(record),"VG still owns rollback PVs"); rawDevicesIdle(); idle();
    for(unsigned i=0;i<2;i++) {
        auto id=pvIdentity(pv[i]); if(id.empty()) continue;
        need(id==record.ids[i],"PV changed before label removal");
        lvm({"pvremove","--yes",pv[i]});
    }
    need(!label(pv[0]) && !label(pv[1]),"PV label remains; raw formatting refused");
    run({"/sbin/make_ext4fs","-L","system",pv[0]});
    run({"/sbin/make_ext4fs","-L","data",pv[1]});
    int fd=open("/dev/block/mmcblk0",O_RDONLY|O_CLOEXEC); need(fd>=0,"cannot recheck partition table");
    Layout now=readLayout(fd); close(fd); need(record.table==tableIdentity(now),"partition table changed during rollback");
    sync(); need(!unlink(rollbackName),"cannot commit rollback completion"); syncRollbackStore();
    prop("phase","reboot"); prop("current",num(part(g.n[11]).size/2048)); prop("rollback","0"); prop("error","");
    puts("LVM removed; physical System/Data formatted. Reboot Recovery, then install a ROM that supports this physical partition layout.");
}
