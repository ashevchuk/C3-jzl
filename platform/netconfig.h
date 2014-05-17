#ifndef _NETCONFIG_H_
#define _NETCONFIG_H_
#include "arca.h"

struct str_net_addr	//ccc
{
	unsigned  char ip[4];
	unsigned char mask[4];
	unsigned char gateway[4];
	 char name[20];
}gwifiaddr;

void SetIPAddress(char *Action, unsigned char *ipaddress);
BOOL SetGateway(char *Action, unsigned char *ipaddress);
void SetNetworkIP_MASK(BYTE *ipaddress, BYTE *netmask);

#endif
