/*
(c) Copyright 2013 - 2016 Xilinx, Inc. All rights reserved. 

This file contains confidential and proprietary information of Xilinx, Inc. and
is protected under U.S. and international copyright and other intellectual
property laws.

DISCLAIMER 
This disclaimer is not a license and does not grant any rights to the materials
distributed herewith. Except as otherwise provided in a valid license issued to
you by Xilinx, and to the maximum extent permitted by applicable law: (1) THESE
MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS, AND XILINX HEREBY
DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY,
INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, OR
FITNESS FOR ANY PARTICULAR PURPOSE; and (2) Xilinx shall not be liable (whether
in contract or tort, including negligence, or under any other theory of
liability) for any loss or damage of any kind or nature related to, arising
under or in connection with these materials, including for any direct, or any
indirect, special, incidental, or consequential loss or damage (including loss
of data, profits, goodwill, or any type of loss or damage suffered as a result
of any action brought by a third party) even if such damage or loss was
reasonably foreseeable or Xilinx had been advised of the possibility of the
same.

CRITICAL APPLICATIONS
Xilinx products are not designed or intended to be fail-safe, or for use in any
application requiring fail-safe performance, such as life-support or safety
devices or systems, Class III medical devices, nuclear facilities, applications
related to the deployment of airbags, or any other applications that could lead
to death, personal injury, or severe property or environmental damage
(individually and collectively, "Critical Applications"). Customer assumes the
sole risk and liability of any use of Xilinx products in Critical Applications,
subject only to applicable laws and regulations governing limitations on product
liability.

THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT
ALL TIMES. 
*/

//==============================================================
// Derived from driver code generated by Vivado(TM) HLS 
// Version: 2014.4
// Copyright (C) 2014-2015 Xilinx Inc. All rights reserved.
// ==============================================================

#include "uio_axi_gpio.h"

static axi_gpio_uio_info uio_info;

static int 
readline(char* filename, char* linebuf) 
{
    FILE* fp = fopen(filename, "r");
    if (!fp) 
         return -1;
    char *s = fgets(linebuf, MAX_UIO_NAME_SIZE, fp);
    fclose(fp);
    if (!s) 
         return -2;
    int i;
    for (i = 0; (*s) && (i < MAX_UIO_NAME_SIZE); i++) {
        if (*s == '\n') *s = 0;
        s++;
    }
    return 0;
}

static int 
uio_info_read_name(axi_gpio_uio_info* info) 
{
    char file[MAX_UIO_PATH_SIZE];
    sprintf(file, "/sys/class/uio/uio%d/name", info->uio_num);
    return readline(file, info->name);
}

static int 
uio_info_read_version(axi_gpio_uio_info* info) 
{
    char file[MAX_UIO_PATH_SIZE];
    sprintf(file, "/sys/class/uio/uio%d/version", info->uio_num);
    return readline(file, info->version);
}

static int 
uio_info_read_map_addr(axi_gpio_uio_info* info, int n) 
{
    char file[MAX_UIO_PATH_SIZE];
    info->maps[n].addr = UIO_INVALID_ADDR;
    sprintf(file, "/sys/class/uio/uio%d/maps/map%d/addr", info->uio_num, n);
    FILE* fp = fopen(file, "r");
    if (!fp) 
         return -1;
    int ret = fscanf(fp, "0x%x", &info->maps[n].addr);
    fclose(fp);
    return (ret < 0 ? -2 : 0);
}

static int 
uio_info_read_map_size(axi_gpio_uio_info* info, int n) 
{
    char file[MAX_UIO_PATH_SIZE];
    sprintf(file, "/sys/class/uio/uio%d/maps/map%d/size", info->uio_num, n);
    FILE* fp = fopen(file, "r");
    if (!fp) 
         return -1;
    int ret = fscanf(fp, "0x%x", &info->maps[n].size);
    fclose(fp);
    return (ret < 0 ? -2 : 0);
}

int 
axi_gpio_init(axi_gpio *inst, const char* instnm) 
{
     axi_gpio_uio_info *info = &uio_info;  
     struct dirent **namelist;
     char* s;
     char file[MAX_UIO_PATH_SIZE];
     char name[MAX_UIO_NAME_SIZE];
     int flag = 0;

     assert(inst != NULL);

     int n = scandir("/sys/class/uio", &namelist, 0, alphasort);
     if (n < 0)
          return -1;
     int i;
     for (i = 0;  i < n; i++) {
          strcpy(file, "/sys/class/uio/");
          strcat(file, namelist[i]->d_name);
          strcat(file, "/name");
          if ((readline(file, name) == 0) && (strcmp(name, instnm) == 0)) {
               flag = 1;
               s = namelist[i]->d_name;
               s += 3; // "uio"
               info->uio_num = atoi(s);
               break;
          }
     }
     if (flag == 0)
          return -1;

     uio_info_read_name(info);
     uio_info_read_version(info);
     for (n = 0; n < MAX_UIO_MAPS; ++n) {
          uio_info_read_map_addr(info, n);
          uio_info_read_map_size(info, n);
     }

     sprintf(file, "/dev/uio%d", info->uio_num);
     if ((info->uio_fd = open(file, O_RDWR)) < 0) {
          return -2;
     }

     inst->baseaddr = (u32)mmap(NULL, info->maps[0].size, 
                                PROT_READ|PROT_WRITE, MAP_SHARED, 
                                info->uio_fd, 0);
     assert(inst->baseaddr);

     return 0;
}

int 
axi_gpio_release(axi_gpio *inst) 
{
     axi_gpio_uio_info *info = &uio_info;
     assert(inst != NULL);
     munmap((void*)inst->baseaddr, info->maps[0].size);
     close(info->uio_fd);
     return 0;
}

// XSIP watermark, do not delete 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
