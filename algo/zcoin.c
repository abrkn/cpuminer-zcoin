#include "miner.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "../lyra2/Lyra2.h"

extern void zcoinhash(int16_t height, void *state, const void *input)
{
	// int LYRA2(void *K, int64_t kLen, const void *pwd, int32_t pwdlen, const void *salt, int32_t saltlen, int64_t timeCost, const int16_t nRows, const int16_t nCols)

	uint32_t hash[32];

	LYRA2(hash, 32, input, 80, input, 80, 2, height, 256);

	memcpy(state, hash, 32);
}

int scanhash_zcoin(int16_t height, int thr_id, struct work *work, uint32_t max_nonce, uint64_t *hashes_done)
{
	uint32_t _ALIGN(128) hash32[8];
	uint32_t _ALIGN(128) endiandata[20];
	uint32_t *pdata = work->data;
	uint32_t *ptarget = work->target;

	uint32_t n = pdata[19] - 1;
	const uint32_t first_nonce = pdata[19];

	for (int k=0; k < 19; k++)
		be32enc(&endiandata[k], pdata[k]);

	const uint32_t Htarg = ptarget[7];
	do {

		pdata[19] = ++n;
		be32enc(&endiandata[19], n);
		zcoinhash(height, hash32, endiandata);

		if (hash32[7] <= Htarg && fulltest(hash32, ptarget)) {
			work_set_target_ratio(work, hash32);
			pdata[19] = n;
			*hashes_done = pdata[19] - first_nonce;
			return true;
		}
	} while (n < max_nonce && !work_restart[thr_id].restart);

	*hashes_done = n - first_nonce + 1;
	pdata[19] = n;
	return 0;
}
