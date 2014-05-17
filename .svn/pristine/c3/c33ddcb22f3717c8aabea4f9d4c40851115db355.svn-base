/*************************************************
                                           
 ZEM 200                                          
                                                    
 netspeed.h                               
                                                      
 Copyright (C) 2003-2005, ZKSoftware Inc.      		
                                                      
*************************************************/

#ifndef	_NETSPEED_H_
#define	_NETSPEED_H_

enum ETH_PHY_mode {
        ETH_10MHD   = 0,
        ETH_100MHD  = 1,
        ETH_10MFD   = 4,
        ETH_100MFD  = 5,
        ETH_AUTO    = 8,
        ETH_1M_HPNA = 0x10
};


#define NET_DEVICE_NAME   "eth0"
// Speed = 0: AUTO 1: 100M 2: 10M
int set_network_speed(unsigned char *net_device, int speed);

#endif
