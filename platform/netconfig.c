#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <crypt.h>
#define _XOPEN_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include "options.h"
#include "netconfig.h"
#include "serial.h"

//ccc add for gprs
int fd_serial_cmux1=-1;
unsigned char gsim_id[64];

void SetIPAddress(char *Action, unsigned char *ipaddress)
{
	char buffer[128];    
	
	if(strcmp(Action, "IP")==0)
		sprintf(buffer, "ifconfig eth0 %d.%d.%d.%d", ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3]);
	else if (strcmp(Action, "NETMASK")==0)
		sprintf(buffer, "ifconfig eth0 netmask %d.%d.%d.%d", ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3]);
	system(buffer);
//	system("reboot");
}

BOOL SetGateway(char *Action, unsigned char *ipaddress)
{
	char buffer[128];    
	int rc;
                   
	if(ipaddress[0]==0) return TRUE;
	system("route del default gw 0.0.0.0");	
	sprintf(buffer, "route %s default gw %d.%d.%d.%d", Action, ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3]);
	rc=system(buffer);
	return (rc==EXIT_SUCCESS);
}

void SetNetworkIP_MASK(BYTE *ipaddress, BYTE *netmask)
{
	char buffer[128];
	
	sprintf(buffer, "ifconfig eth0 %d.%d.%d.%d netmask %d.%d.%d.%d",
		ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3],
		netmask[0], netmask[1], netmask[2], netmask[3]);
	system(buffer);
	DBPRINTF("Setup network ip&netmask OK: %s\n",buffer);	
}
void SetNetIP_MASK(const char *netname, BYTE *ipaddress, BYTE *netmask) //ccc
{
	char buffer[128];
	
	sprintf(buffer, "ifconfig %s %d.%d.%d.%d netmask %d.%d.%d.%d",
		netname,ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3],
		netmask[0], netmask[1], netmask[2], netmask[3]);
	system(buffer);
	DBPRINTF("Setup network %s ip&netmask OK!\n", netname);	
}

static int failures;

static const char * addr_string (struct sockaddr *sa, char *buf, size_t size)
{
  if (sa == NULL)
    return "<none>";

  switch (sa->sa_family)
    {
    case AF_INET:
      return inet_ntop (AF_INET, &((struct sockaddr_in *) sa)->sin_addr,
			buf, size);
    case AF_INET6:
      return inet_ntop (AF_INET6, &((struct sockaddr_in6 *) sa)->sin6_addr,
			buf, size);
#ifdef AF_LINK
    case AF_LINK:
      return "<link>";
#endif
    case AF_UNSPEC:
      return "---";

#ifdef AF_PACKET
    case AF_PACKET:
      return "<packet>";
#endif

    default:
      ++failures;
      printf ("sa_family=%d %08x\n", sa->sa_family,
	      *(int*)&((struct sockaddr_in *) sa)->sin_addr.s_addr);
      return "<unexpected sockaddr family>";
    }
}


//get net interface info
void getnetinfo(const char *netname, struct str_net_addr *net_info) //ccc
{
  struct ifaddrs *ifaces, *ifa;
  char abuf[64], mbuf[64], dbuf[64];
  
  if (getifaddrs (&ifaces) < 0)
  {
       printf ("Couldn't get any interfaces\n");
	   if(ifaces)
	   	  freeifaddrs (ifaces);
	   return;
  }
 
  for (ifa = ifaces; ifa != NULL; ifa = ifa->ifa_next)
  {

	  if(strcmp(ifa->ifa_name, netname)==0)
	  {
	  
      	  printf ("%-15s%#.4x  %-15s %-15s %-15s\n",
	      ifa->ifa_name, ifa->ifa_flags,
	      addr_string (ifa->ifa_addr, abuf, sizeof (abuf)),
	      addr_string (ifa->ifa_netmask, mbuf, sizeof (mbuf)),
	      addr_string (ifa->ifa_broadaddr, dbuf, sizeof (dbuf)));
	  	  str2ip(abuf, net_info->ip);
		  str2ip(mbuf, net_info->mask);
	  }
  }
  freeifaddrs(ifaces);
}

//get net interface info
int getnetisalive(const char *netname)
{
	struct ifaddrs *ifaces, *ifa;
	//char abuf[64], mbuf[64], dbuf[64];
	int ret=0;

	if (getifaddrs (&ifaces) < 0)
	{
		printf ("Couldn't get any interfaces\n");
		if(ifaces)
		freeifaddrs (ifaces);
		return ret;
	}

	for (ifa = ifaces; ifa != NULL; ifa = ifa->ifa_next)
	{
		if(strcmp(ifa->ifa_name, netname)==0)
		{
			/*
			printf ("%-15s%#.4x  %-15s %-15s %-15s\n",
		ifa->ifa_name, ifa->ifa_flags,
		addr_string (ifa->ifa_addr, abuf, sizeof (abuf)),
		addr_string (ifa->ifa_netmask, mbuf, sizeof (mbuf)),
		addr_string (ifa->ifa_broadaddr, dbuf, sizeof (dbuf)));
		*/
			ret =1;
			break;
			/*
			str2ip(abuf, net_info->ip);
		str2ip(mbuf, net_info->mask);
		*/
		}
	}
	freeifaddrs(ifaces);
	return ret;
}

