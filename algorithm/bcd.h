#ifndef BCD_H
#define BCD_H

#include "miner.h"

extern int bcd_test(unsigned char *pdata, const unsigned char *ptarget,
			uint32_t nonce);
extern void bcd_regenhash(struct work *work);

#endif /* BCD_H */
