#include <stdio.h>
//#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sds_lib.h>
#include <sys/ioctl.h>
//#include <sys/time.h>

#define IOCTL_WAIT_INTERRUPT _IOR(100, 0, char *)    /*wait for interrupt*/

#include "aes_enc.h"

void kernelAES(
uint32_t *state,
uint32_t *cipher,
uint8_t ekey[240],
int *interrupt,
int *status,
int file_desc,
int begin,
int end)
{

  int ret_value;
  int block_count = end-begin;
  uint32_t *state_temp = state + begin*4;
  uint32_t *cipher_temp = cipher + begin*4;
  
 // printf("Calling hardware with interrupt at %x\n",status);
  #pragma SDS async(1)
  aes_enc_hw(state_temp, cipher_temp, ekey, block_count,interrupt,status);
 // printf("Waiting for interrupt ...\n");
  ret_value = ioctl(file_desc,IOCTL_WAIT_INTERRUPT,0); //this should put this process to sleep until interrupt takes place
 // printf("Waking up ...\n");
  #pragma SDS wait(1)
  //printf("Simulating sleep calling before performing wait\n");
  //printf("Press Any Key to Continue\n");
  //getchar();
  //#pragma SDS wait(1)
  //printf("Exit value interrupt %x\n", *interrupt);
  //printf("Exit value status %x\n", *status);
  //#pragma SDS wait(1)
  //now wait for completion by waiting from interrupt from acceletaror
 /* printf("Waiting for interrupt ...\n");
  int ret_value = ioctl(file_desc,IOCTL_WAIT_INTERRUPT,0); //this should put this process to sleep until interrupt takes place
  printf("Waking up ...\n");
  #pragma SDS wait(1)*/
}
