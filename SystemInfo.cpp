/*
 * SystemInfo.cpp
 *
 *  Created on: May 21, 2018
 *      Author: root
 */

#include "SystemInfo.h"

double g_cpu_used;
unsigned long g_mem_total;
unsigned long g_mem_used;
unsigned long g_disk_total;
unsigned long g_disk_used;
unsigned long g_disk_read;
unsigned long g_disk_write;
unsigned long g_network_up;
unsigned long g_network_down;
bool quit_out = false;

void get_cpuinfo(CPU_PACKED &cpuinfo)
{
	char buff[256] = {0};
	ifstream in("/proc/stat");
	if (!in)
	{
		cout << "get cpu info failed" << endl;
		return;
	}

	in.getline(buff, sizeof(buff));
	stringstream ss(buff);
	ss >> cpuinfo.name;
	ss >> cpuinfo.user;
	ss >> cpuinfo.nice;
	ss >> cpuinfo.system;
	ss >> cpuinfo.idle;
	ss >> cpuinfo.iowait;
	ss >> cpuinfo.irq;
	ss >> cpuinfo.softirg;

	in.close();
}
double calc_cpuoccupy(CPU_PACKED cpuinfo1, CPU_PACKED cpuinfo2)
{
	double info1d = cpuinfo1.user + cpuinfo1.nice + cpuinfo1.system + cpuinfo1.idle + cpuinfo1.softirg + cpuinfo1.iowait + cpuinfo1.irq;
	double info2d = cpuinfo2.user + cpuinfo2.nice + cpuinfo2.system + cpuinfo2.idle + cpuinfo2.softirg + cpuinfo2.iowait + cpuinfo2.irq;

	double sub1 = cpuinfo1.idle;
	double sub2 = cpuinfo2.idle;
	double cpu_use;

	if ((sub1 - sub2) != 0)
		cpu_use = 100.0 - ((sub2 - sub1) / (info2d - info1d)) * 100.0;
	else
		cpu_use = 0;

	//double cpu_use = cpuinfo1.user/info1d;
	return cpu_use;
}

void calc_memoccupy(MEM_PACKED &meminfo)
{
	char buff[256] = {0};
	string name;
	unsigned long free_mem;
	unsigned long cached;
	int index = 0;
	ifstream in("/proc/meminfo");
	if (!in)
	{
		cout << "get cpu info failed" << endl;
		return;
	}

	stringstream ss;
	while (!in.eof() && index < 4)
	{
		in.getline(buff, sizeof(buff));
		ss.str("");
		ss << buff;
		if (index == 0)
		{
			ss >> name;
			ss >> meminfo.total_mem;
		}
		else if (index == 1)
		{
			ss >> name;
			ss >> free_mem;
		}
		else if (index == 3)
		{
			ss >> name;
			ss >> cached;
		}
		index++;
	}

	meminfo.used_mem = meminfo.total_mem - free_mem - cached;

	in.close();
}

void calc_rwspeed(unsigned long &readsectors, unsigned long &writesectors)
{
	ifstream in("/proc/diskstats");
	if (!in)
	{
		cout << "get disk speed info failed with Reason:" << strerror(errno) << endl;
		return;
	}
	string line;
	while (!in.eof())
	{
		getline(in, line);
		size_t pos = line.find("sda");
		if (pos < line.size())
		{
			line = line.substr(pos + 4, line.size());
			break;
		}
	}

	vector<string> items = splitstring(line);
	readsectors = atol(items[2].c_str());
	writesectors = atol(items[6].c_str());

	in.close();
}

void calc_diskoccupy(string path, DISK_PACKED &diskinfo)
{
	struct statfs disk;
	if (statfs(path.c_str(), &disk) == -1)
	{
		cout << "Failed to get disk info with Reason:" << strerror(errno) << endl;
		return;
	}

	diskinfo.total_disk = disk.f_blocks * disk.f_bsize;
	diskinfo.avail_disk = disk.f_bavail * disk.f_bsize;
	diskinfo.free_disk = disk.f_bfree * disk.f_bsize;

	unsigned long reads1, writes1, reads2, writes2;
	calc_rwspeed(reads1, writes1);
	sleep(1);
	calc_rwspeed(reads2, writes2);

	diskinfo.read_speed = (reads2 - reads1) * disk.f_bsize;
	diskinfo.write_speed = (writes2 - writes1) * disk.f_bsize;
}

vector<string> splitstring(string str)
{
	vector<string> result;
	string item;
	stringstream ss;
	ss << str;
	while (ss >> item)
		result.push_back(item);
	return result;
}

void read_netdev(unsigned long &ups, unsigned long &downs)
{
	ifstream in("/proc/net/dev");
	if (!in)
	{
		cout << "get network info failed" << endl;
		return;
	}
	string line;
	std::vector<string> lines;
	while (!in.eof())
	{
		getline(in, line);
		if (in.fail())
			break;
		lines.push_back(line);
	}
	vector<string> items = splitstring(lines[lines.size() - 1]);
	ups = atol(items[1].c_str());
	downs = atol(items[9].c_str());

	in.close();
}

void calc_netspeed(NET_PACKED &netinfo)
{
	unsigned long ups1, ups2, downs1, downs2;
	read_netdev(ups1, downs1);
	sleep(1);
	read_netdev(ups2, downs2);

	netinfo.upspeed = (float)(ups2 - ups1);
	netinfo.downspeed = (float)(downs2 - downs1);
}

void thread_cpu_monitor()
{
	CPU_PACKED cpuinfo1, cpuinfo2;
	while (1)
	{
		get_cpuinfo(cpuinfo1);
		sleep(1);
		get_cpuinfo(cpuinfo2);

		g_cpu_used = calc_cpuoccupy(cpuinfo1, cpuinfo2);

		if (quit_out)
			break;
	}
}

void thread_mem_monitor()
{
	MEM_PACKED meminfo;
	while (1)
	{
		calc_memoccupy(meminfo);

		g_mem_total = meminfo.total_mem;
		g_mem_used = meminfo.used_mem;

		if (quit_out)
			break;
	}
}

void thread_disk_monitor()
{
	DISK_PACKED diskinfo;
	while (1)
	{
		calc_diskoccupy("/", diskinfo);

		g_disk_total = diskinfo.total_disk;
		g_disk_used = diskinfo.total_disk - diskinfo.avail_disk;
		g_disk_read = (diskinfo.read_speed >> 10);
		g_disk_write = (diskinfo.write_speed >> 10);

		if (quit_out)
			break;
	}
}

void thread_network_monitor()
{
	NET_PACKED netinfo;
	while (1)
	{
		calc_netspeed(netinfo);

		g_network_up = netinfo.upspeed;
		g_network_down = netinfo.downspeed;

		if (quit_out)
			break;
	}
}

void sys_monitor()
{
	thread t1(thread_cpu_monitor);
	t1.detach();
	thread t2(thread_mem_monitor);
	t2.detach();
	thread t3(thread_disk_monitor);
	t3.detach();
	thread t4(thread_network_monitor);
	t4.detach();
}
