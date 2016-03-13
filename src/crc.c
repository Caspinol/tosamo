#include "include/crc.h"

#define CRCPOLY 0xab
#define BMASK 0x80

/*
  nothing fancy here just a crc 8 bit implementation
  taken from wikipedia
  Apparently its just a naive implementation.
  if it'll ever become a bottleneck i'll see if 
  i can understand it and add table implementation
*/
unsigned char crc(char const *message, int meslen){
	unsigned char crc = 0;
  
	for(int i = 0; i < meslen; i++){
		crc ^= message[i];
		for(int j = 8; j > 0; --j){
			if (crc & BMASK){
				crc = (crc << 1) ^ CRCPOLY;
			}else{
				crc = (crc << 1);
			}
		}
	}
	return crc;
}
