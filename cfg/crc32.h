/*

	@author: zombie.fml

	@histroy:
		11/6/2009	create file

*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void init_crc32_table();
unsigned long crc32(char* buf, unsigned long len);

#ifdef __cplusplus
}
#endif