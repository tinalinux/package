/* cpu spec files defined */
#ifndef __SUN9IW1P1_H__
#define __SUN9IW1P1_H__

#define CPU0LOCK    "/sys/devices/system/cpu/cpu0/cpufreq/boot_lock"
#define CPU4LOCK    "/sys/devices/system/cpu/cpu4/cpufreq/boot_lock"
#define ROOMAGE     "/sys/devices/platform/sunxi-budget-cooling/roomage"
#define CPUFREQ     "/sys/devices/system/cpu/cpu0/cpufreq/scale_cur_freq"
#define CPUONLINE   "/sys/devices/system/cpu/online"
#define CPUHOT      "/sys/kernel/autohotplug/enable"
#define CPU0GOV     "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
#define CPU4GOV     "/sys/devices/system/cpu/cpu4/cpufreq/scaling_governor"
/* gpu spec files defined */
#define GPUFREQ     "/sys/devices/platform/pvrsrvkm/dvfs/android"
/* ddr spec files defined */
#define DRAMFREQ    "/sys/devices/platform/sunxi-ddrfreq/devfreq/sunxi-ddrfreq/cur_freq"
#define DRAMSCEN    "/sys/class/devfreq/sunxi-ddrfreq/dsm/scene"
/* task spec files defined */
#define TASKS       "/dev/cpuctl/tasks"

#endif

