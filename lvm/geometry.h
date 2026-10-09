/* SPDX-License-Identifier: Apache-2.0
 * Read-only Qualcomm MBR/EBR validation. No partition-table write routine. */
static const uint64_t kDiskSectors=7815168, kExt=208801;
static const unsigned kNodes=17, kMiBSectors=2048;
struct Node { uint64_t lba; unsigned char bytes[512]; };
struct Layout { Node n[kNodes]; };
struct Part { uint64_t start, size; unsigned type; };
static uint32_t get32(const unsigned char *p) {
    return uint32_t(p[0]) | uint32_t(p[1])<<8 | uint32_t(p[2])<<16 | uint32_t(p[3])<<24;
}
static Part part(const Node &n) {
    const unsigned char *p=n.bytes+446;
    Part r={n.lba+get32(p+8),get32(p+12),p[4]}; return r;
}
static void validate(const Layout &l) {
    const unsigned char *m=l.n[0].bytes;
    need(l.n[0].lba==0 && m[510]==0x55 && m[511]==0xaa,"invalid MBR");
    static const uint32_t ps[]={1,204801,205801,208801}, sz[]={204800,1000,3000,7606367};
    static const unsigned pt[]={0x0c,0x4d,0x51,5};
    for(unsigned i=0;i<4;i++) {
        const unsigned char *p=m+446+16*i;
        need(p[4]==pt[i] && get32(p+8)==ps[i] && get32(p+12)==sz[i],"not the MiOne 4GB MBR");
    }
    static const uint64_t starts[]={208817,209817,213913,218913,239393,259873,260873,262144,268288,327680};
    static const uint64_t sizes[]={1000,4096,5000,20480,20480,1000,1000,6144,6144,6144};
    static const unsigned types[]={0x47,0x45,0x4c,0x48,0x64,0x46,0x65,0x4a,0x4b,0x58,0x83,0x83,0x60,0x83,0x83,0x83};
    need(l.n[1].lba==kExt,"wrong extended base");
    std::vector<std::pair<uint64_t,uint64_t> > ranges;
    for(unsigned i=1;i<kNodes;i++) {
        const Node &n=l.n[i]; Part p=part(n);
        need(n.bytes[510]==0x55 && n.bytes[511]==0xaa,"bad EBR signature");
        need(n.lba>=kExt && n.lba<kDiskSectors && p.size && p.type==types[i-1],"wrong EBR type or bounds");
        need(p.start>n.lba && p.start+p.size<=kDiskSectors,"partition outside disk");
        for(unsigned s=2;s<4;s++) for(unsigned b=0;b<16;b++) need(n.bytes[446+s*16+b]==0,"unexpected EBR slot");
        if(i<=10) need(n.lba==kExt+i-1 && p.start==starts[i-1] && p.size==sizes[i-1],"protected prefix differs");
        const unsigned char *next=n.bytes+462;
        if(i+1<kNodes) need(next[4]==5 && kExt+get32(next+8)==l.n[i+1].lba && get32(next+12)>0,"broken EBR chain");
        else for(unsigned b=0;b<16;b++) need(next[b]==0,"unexpected final EBR link");
        ranges.push_back(std::make_pair(n.lba,n.lba+1));
        ranges.push_back(std::make_pair(p.start,p.start+p.size));
    }
    std::sort(ranges.begin(),ranges.end());
    for(unsigned i=1;i<ranges.size();i++) need(ranges[i-1].second<=ranges[i].first,"overlapping partitions or EBRs");
    need(l.n[11].lba==333856 && part(l.n[11]).start==333872,"unexpected system start");
    for(unsigned i=11;i<kNodes;i++) need(part(l.n[i]).start==l.n[i].lba+16,"unexpected tail EBR gap");
    Part sys=part(l.n[11]), legacy=part(l.n[12]), recovery=part(l.n[13]), persist=part(l.n[14]), cache=part(l.n[15]);
    need(sys.size%kMiBSectors==0 && sys.size>=MIONE_SYSTEM_MIN_MIB*kMiBSectors && sys.size<=2560*kMiBSectors,"unsupported system size");
    need(legacy.size>=kMiBSectors && legacy.size<=512*kMiBSectors && legacy.size%kMiBSectors==0,"unsupported legacy system1 size");
    need((recovery.size==20480 || recovery.size==40960) && persist.size==16384 && cache.size==368640,"unexpected protected tail sizes");
    for(unsigned i=11;i<16;i++) need(l.n[i+1].lba==part(l.n[i]).start+part(l.n[i]).size,"unexpected gap in tail");
    Part data=part(l.n[16]); need(data.start+data.size==kDiskSectors,"userdata does not end at disk boundary");
}
static Layout readLayout(int fd) {
    Layout l={}; readAt(fd,l.n[0].bytes,512,0);
    uint64_t at=kExt;
    for(unsigned i=1;i<kNodes;i++) {
        need(at>=kExt && at<kDiskSectors,"EBR pointer outside disk");
        l.n[i].lba=at; readAt(fd,l.n[i].bytes,512,at*512);
        at=kExt+get32(l.n[i].bytes+470);
    }
    validate(l); return l;
}
