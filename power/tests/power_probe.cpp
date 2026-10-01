/* SPDX-License-Identifier: Apache-2.0 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <hardware/power.h>
#include <healthd.h>
#include "../battery_policy.h"
static int checks, failures;
#define CHECK(expr) do { ++checks; if (!(expr)) { ++failures; printf("FAIL line %d: %s\n", __LINE__, #expr); } } while (0)
static unsigned int frequency(int cpu)
{
    char path[128]; unsigned int value=0;
    snprintf(path,sizeof(path),"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq",cpu);
    FILE *f=fopen(path,"r"); if(f) { if(fscanf(f,"%u",&value)!=1) value=0; fclose(f); }
    return value;
}
static int write_cap(int cpu,unsigned int value)
{
    char path[128];
    snprintf(path,sizeof(path),"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq",cpu);
    FILE *f=fopen(path,"w"); if(!f) return -1;
    int r=fprintf(f,"%u",value); int c=fclose(f); return r>0 && c==0 ? 0 : -1;
}
static void sample(android::BatteryProperties *p,int level,bool present,int expected)
{
    char v[PROPERTY_VALUE_MAX]; p->batteryLevel=level; p->batteryPresent=present;
    healthd_board_battery_update(p); usleep(200000);
    property_get(MIONE_BATTERY_LOW_PROPERTY,v,""); CHECK(strcmp(v,expected ? "1" : "0")==0);
    printf("sample level=%d present=%d low=%s caps=%u/%u\n",level,present,v,frequency(0),frequency(1));
}
int main(void)
{
    hw_module_t const *common=NULL; android::BatteryProperties p;
    power_module_t *power;
    unsigned int original[2]={frequency(0),frequency(1)};
    CHECK(original[0]>=972000 && original[1]>=972000);
    if(failures) { puts("Run with screen awake and cool CPU policies before stopping zygote."); return 1; }
    CHECK(hw_get_module(POWER_HARDWARE_MODULE_ID,&common)==0);
    if(!common) return 1;
    power=(power_module_t *)common;
    property_set(MIONE_BATTERY_LOW_PROPERTY,"0");
    healthd_board_init(NULL); power->init(power); power->setInteractive(power,1); usleep(200000);
    sample(&p,97,true,0);
    sample(&p,10,true,1); CHECK(frequency(0)==972000 && frequency(1)==594000);
    power->setInteractive(power,0); CHECK(frequency(0)==918000 && frequency(1)==594000);
    power->setInteractive(power,0); CHECK(frequency(0)==918000 && frequency(1)==594000);
    sample(&p,12,true,1); sample(&p,-1,true,1); sample(&p,97,false,1);
    sample(&p,15,true,0); CHECK(frequency(0)==918000 && frequency(1)==918000);
    power->setInteractive(power,1); CHECK(frequency(0)==original[0] && frequency(1)==original[1]);
    power->setInteractive(power,0);
    CHECK(write_cap(0,594000)==0 && write_cap(1,540000)==0);
    power->setInteractive(power,1); CHECK(frequency(0)==594000 && frequency(1)==540000);
    sample(&p,9,true,1); CHECK(frequency(0)==594000 && frequency(1)==540000);
    sample(&p,15,true,0); CHECK(frequency(0)==594000 && frequency(1)==540000);
    CHECK(write_cap(0,original[0])==0 && write_cap(1,original[1])==0);
    printf("Power policy: %d checks, %d failures; original caps restored\n",checks,failures);
    return failures != 0;
}
