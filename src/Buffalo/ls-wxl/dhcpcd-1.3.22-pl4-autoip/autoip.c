/*
	autoip.c
	2005/5/17
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <pcap.h>
#include <netinet/in.h>
#include <string.h>
#include <endian.h>
#include <sys/select.h>

#include "autoip.h"
#include "dhcpcd.h"
#include "client.h"

#if 0
 #define FUNCTION printf(">%s %d\n",__FUNCTION__,__LINE__)
#else
 #define FUNCTION
#endif

#ifdef AUTOIP_CONFIG
#define DEB_PRINT printf
extern	char		*IfName, *IfNameExt;
extern	unsigned char	ClientHwAddr[6];
extern	char		*ConfigDir;
extern	int		dhcp_ip_bound;

int arpCheck_probing(unsigned long,int count, unsigned long wait);
int arpCheck_conflicts(unsigned long,int count, unsigned long wait);
int arpCheck_announce(unsigned long l_ip,int count);

//#define IFCONFIG_PATH "/bin/ifconfig"
#define IFCONFIG_PATH "/sbin/ifconfig"
#define AUTOIP_TAG "autoip"
#define AUTOIP_CACHE_FILE		"%s/"AUTOIP_TAG"-%s.cache"
#define AUTOIP_INFO_FILE		"%s/"AUTOIP_TAG"-%s.info"
//#define FINISH_TIME 10
#define FINISH_TIME 300
#define TIME_CHANGE_DETECT -30
static unsigned long base = 0xa9fe0100; //169.254.1.0 

union ip_address
{
	unsigned long l_ip;
	struct
	{
		unsigned char c_ip[4];
	}c;
};

union ip_address ip;


//169.254.1.0/16から169.254.254.255/16までのランダムIPを生成する
//ただし169.254.254.255/16は出現しない
unsigned long getRandomIP()
{
	unsigned long ret, seed;
	double rnd, rnd_base = RAND_MAX;
	seed=time(NULL)+ClientHwAddr[5]+4*ClientHwAddr[4]+8*ClientHwAddr[3]+
	16*ClientHwAddr[2]+32*ClientHwAddr[1]+64*ClientHwAddr[0];
	srandom(seed);
	rnd = random();
	rnd =(double)(rnd / rnd_base);
	ret = (unsigned long)(rnd * 0xfc * 0xff);
	if(ret > 0xfc * 0xff) ret = 0xfc * 0xff; //念のため
	return base + ret;
}

int checkAutoIPFile()
{
	FILE *fp = NULL;
	char infofile[64];
	snprintf(infofile,sizeof(infofile),AUTOIP_INFO_FILE,ConfigDir,IfNameExt);
#ifdef AUTOIP_DEBUG
	DEB_PRINT("file name %s\n",infofile);
#endif
	fp=fopen(infofile,"r");
	if ( fp == NULL )
	{
		return 1;
	}
	fclose (fp);
	return 0;
}

void writeAutoIPFile()
{
	FILE *fp;
	char infofile[64];
	snprintf(infofile,sizeof(infofile),AUTOIP_INFO_FILE,ConfigDir,IfNameExt);
	fp=fopen(infofile,"w");
	if ( fp == NULL )
	{
		syslog(LOG_ERR,"writeAutoIPinfoFile: fopen: %m\n");
		exit(1);
	}
#if __BYTE_ORDER == __BIG_ENDIAN
	fprintf(fp,"%u.%u.%u.%u\n",ip.c.c_ip[0],ip.c.c_ip[1],ip.c.c_ip[2],ip.c.c_ip[3]);
#else
	fprintf(fp,"%u.%u.%u.%u\n",ip.c.c_ip[3],ip.c.c_ip[2],ip.c.c_ip[1],ip.c.c_ip[0]);
#endif
	fclose (fp);
}

void wirteAutoIPCache()
{
	int i;
	char cachefile[64];
	snprintf(cachefile,sizeof(cachefile),AUTOIP_CACHE_FILE,ConfigDir,IfNameExt);
	i=open(cachefile,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR+S_IWUSR);
	if(i){
		write(i,(char *)&ip,sizeof(ip));
		close(i);
	}
}

extern int rService;

int readAutoIPCache()
{
	int i,o;
	char cachefile[64];
	snprintf(cachefile,sizeof(cachefile),AUTOIP_CACHE_FILE,ConfigDir,IfNameExt);
	i=open(cachefile,O_RDONLY);
	if ( i == -1 ) return -1;
	o=read(i,(char *)&ip,sizeof(ip));
	close(i);
	if ( o != sizeof(ip) ) return -1;
	if ( ip.l_ip == 0 ) return -1;
	if ( ip.l_ip < 0xa9fe0100 ) return -1;
	if ( ip.l_ip > 0xa9fefeff ) return -1;
	return 0;
}

int setAutoIP(int i)
{
	char cmd[256];
	int ret, count = 0;
	unsigned long pre_ip = 0;
#ifdef AUTOIP_DEBUG
	DEB_PRINT("setAutoIP %s called\n",IfName);
	DEB_PRINT("MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n",
	ClientHwAddr[0], ClientHwAddr[1], ClientHwAddr[2],
	ClientHwAddr[3], ClientHwAddr[4], ClientHwAddr[5]);
#endif
	if(checkAutoIPFile()) return 0;

	if(readAutoIPCache() || (i == 1))
		ip.l_ip = getRandomIP();
	else 
		pre_ip = ip.l_ip;
	if(dhcp_ip_bound){
		pre_ip = 0;
		dhcp_ip_bound = 0;
	}
IPCONFIG:
	sprintf(cmd,"%s %s %u.%u.%u.%u netmask 255.255.0.0 up",
#if __BYTE_ORDER == __BIG_ENDIAN
	IFCONFIG_PATH, IfName,ip.c.c_ip[0],ip.c.c_ip[1],ip.c.c_ip[2],ip.c.c_ip[3]);
#else
	IFCONFIG_PATH, IfName,ip.c.c_ip[3],ip.c.c_ip[2],ip.c.c_ip[1],ip.c.c_ip[0]);
#endif
	system(cmd);
#ifdef AUTOIP_DEBUG
	DEB_PRINT("%s\n",cmd);
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
	ret = arpCheck_probing(ip.l_ip, PROBE_NUM, 0);
#else
	ret = arpCheck_probing(htonl(ip.l_ip), PROBE_NUM, 0);
#endif
#ifdef AUTOIP_DEBUG
	DEB_PRINT("Arp result %d\n",ret);
#endif
	if(ret 	!= 0){
		while(ip.l_ip == getRandomIP()){sleep(1);}
		ip.l_ip = getRandomIP();
		count++;
		if(count < 65024) goto IPCONFIG;
	}

#if __BYTE_ORDER == __BIG_ENDIAN
	ret = arpCheck_announce(ip.l_ip, ANNOUNCE_NUM);
#else
	ret = arpCheck_announce(htonl(ip.l_ip), ANNOUNCE_NUM);
#endif

	if(pre_ip != ip.l_ip){
		wirteAutoIPCache();
		writeAutoIPFile();
		DEB_PRINT("/etc/init.d/rService\n");
		//system("/etc/init.d/rService");
		if (rService)
			system("/etc/init.d/rService");
		else
			rService = 1;
	}
	else if (!rService)
		rService = 1;
	
	return 1;
}


//void conflictIP()
int conflictIP(int interval)
{
	time_t start_time;
	time_t now_time;
	int delta_time;
	int ret = 0;
	int val1 = 0;
	int count = 0;
	
	start_time = time(NULL);

	while(1){
		now_time = time(NULL);
		delta_time = (int)(now_time - start_time);
		if(delta_time >= interval) break;
		if(delta_time <= TIME_CHANGE_DETECT) break;
		DEB_PRINT("conflictIP dalta_time %d \n",delta_time);
		
	if((int)(delta_time / 60) > val1){
		val1 = (int)(delta_time / 60);
#if __BYTE_ORDER == __BIG_ENDIAN
	ret = arpCheck_probing(ip.l_ip, 1, 1);
#else
	ret = arpCheck_probing(htonl(ip.l_ip), 1, 1);
#endif
		
	}else{
#if __BYTE_ORDER == __BIG_ENDIAN
			ret = arpCheck_conflicts(ip.l_ip,1,1);
#else
			ret = arpCheck_conflicts(htonl(ip.l_ip),1,1);
#endif
	}
	
	DEB_PRINT("conflictIP dalta_time %d aprpCheck2 %d\n",delta_time, ret);
	count += ret;
		if(count > 1){
			 setAutoIP(1);
			 count = 0;
		}
	}
	return 0;
}

#endif //AUTOIP_CONFIG


