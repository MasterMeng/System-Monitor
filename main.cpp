/*
 * main.cpp
 *
 *  Created on: May 18, 2018
 *      Author: root
 */

#include <iostream>
#include <fstream>
#include <sstream>

#include <unistd.h>

#include "cJSON.h"
#include "SystemInfo.h"

using namespace std;

bool quit_out = false;

int main(int argc, char *argv[])
{

	CPU_PACKED cpuinfo1, cpuinfo2;
	get_cpuinfo(cpuinfo1);
	cout << "user = " << cpuinfo1.user << endl;

	sleep(1);

	get_cpuinfo(cpuinfo2);
	double diff = calc_cpuoccupy(cpuinfo1, cpuinfo2);
	cout << "diff = " << diff << "%" << endl;

	 MEM_PACKED meminfo;
	 calc_memoccupy(meminfo);
	 cout << "total mem:" << (meminfo.total_mem >> 10) << "M" << endl;
	 cout << "used mem:" << (meminfo.used_mem >> 10) << "M" << endl;

	DISK_PACKED diskinfo;
	calc_diskoccupy("/", diskinfo);
	cout << "path:/ total size:" << (diskinfo.total_disk >> 30) << "GB" << endl;
	cout << "path:/ avail size:" << (diskinfo.avail_disk >> 30) << "GB" << endl;
	cout << "path:/ free size:" << (diskinfo.free_disk >> 30) << "GB" << endl;
	cout << "read speed:" << (diskinfo.readspeed >> 10) << "KB" << endl;
	cout << "write speed:" << (diskinfo.writespeed >> 20) << "MB" << endl;

	calc_diskoccupy("/home", diskinfo);
	cout << "path:/home total size:" << (diskinfo.total_disk >> 30) << "GB" << endl;
	cout << "path:/home avail size:" << (diskinfo.avail_disk >> 30) << "GB" << endl;
	cout << "path:/home free size:" << (diskinfo.free_disk >> 30) << "GB" << endl;

	NET_PACKED netinfo;
	calc_netspeed(netinfo);
	cout<<"up speed:"<<netinfo.upspeed<<"kb/s"<<endl;
	cout<<"down speed:"<<netinfo.downspeed<<"kb/s"<<endl;

	// sys_monitor();
	// getchar();
	// quit_out = true;
	getchar();
	return 0;
}
