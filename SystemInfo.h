/*
 * SystemInfo.h
 *
 *  Created on: May 21, 2018
 *      Author: root
 */

#ifndef SYSTEMINFO_H_
#define SYSTEMINFO_H_

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <thread>
#include <vector>

#include <sys/statfs.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>

#include "cJSON.h"

using namespace std;

struct CPU_PACKED
{
	std::string name;
	unsigned int user;
	unsigned int nice;
	unsigned int system;
	unsigned int idle;
	unsigned int iowait;
	unsigned int irq;
	unsigned int softirg;
};

void get_cpuinfo(CPU_PACKED& cpuinfo);
double calc_cpuoccupy(CPU_PACKED cpuinfo1,CPU_PACKED cpuinfo2);

struct MEM_PACKED
{
	unsigned long total_mem;
	unsigned long used_mem;
};

void calc_memoccupy(MEM_PACKED& meminfo);

struct DISK_PACKED
{
	unsigned long total_disk;
	unsigned long avail_disk;
	unsigned long free_disk;
	unsigned long read_speed;
	unsigned long write_speed;
};

void calc_diskoccupy(string path,DISK_PACKED& diskinfo);

struct NET_PACKED
{
	unsigned long upspeed;
	unsigned long downspeed;
};

void read_netdev(unsigned long& ups,unsigned long& downs);
void calc_netspeed(NET_PACKED& netinfo);

struct system_info{
	int				serviceid;
	int 			state;
	int     		cpu_used;
	long int        time;
	DISK_PACKED 		diskinfo;
	NET_PACKED  		netinfo;
	MEM_PACKED  		meminfo;
	struct timeval 	curtime;
	string 			version;
};

void sys_monitor();

vector<string> splitstring(string str);

#endif /* SYSTEMINFO_H_ */
