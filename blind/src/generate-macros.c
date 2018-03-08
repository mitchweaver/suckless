/* See LICENSE file for copyright and license details. */
#include <stdint.h>
#include <stdio.h>

int
main(void)
{
	char *u16 = (char *)&(uint16_t){0x0001};
	char *u32 = (char *)&(uint32_t){0x00010203};
	char *u64 = (char *)&(uint64_t){0x0001020304050607};
	int le16 = (u16[1] == 0 && u16[0] == 1);
	int le32 = (u32[3] == 0 && u32[2] == 1 && u32[1] == 2 && u32[0] == 3);
	int le64 = (u64[7] == 0 && u64[6] == 1 && u64[5] == 2 && u64[4] == 3 &&
	            u64[3] == 4 && u64[2] == 5 && u64[1] == 6 && u64[0] == 7);
	int be16 = (u16[0] == 0 && u16[1] == 1);
	int be32 = (u32[0] == 0 && u32[1] == 1 && u32[2] == 2 && u32[3] == 3);
	int be64 = (u64[0] == 0 && u64[1] == 1 && u64[2] == 2 && u64[3] == 3 &&
	            u64[4] == 4 && u64[5] == 5 && u64[6] == 6 && u64[7] == 7);

	if (le16)
		printf("#define HOST_ENDIAN_IS_LITTLE_ENDIAN_16\n");
	if (le32)
		printf("#define HOST_ENDIAN_IS_LITTLE_ENDIAN_32\n");
	if (le64)
		printf("#define HOST_ENDIAN_IS_LITTLE_ENDIAN_64\n");
	if (le16 && le32 && le64)
		printf("#define HOST_ENDIAN_IS_LITTLE_ENDIAN\n");

	if (be16)
		printf("#define HOST_ENDIAN_IS_BIG_ENDIAN_16\n");
	if (be32)
		printf("#define HOST_ENDIAN_IS_BIG_ENDIAN_32\n");
	if (be64)
		printf("#define HOST_ENDIAN_IS_BIG_ENDIAN_64\n");
	if (be16 && be32 && be64)
		printf("#define HOST_ENDIAN_IS_BIG_ENDIAN\n");

	if (sizeof(float) == 4) {
		unsigned long int a, b;
		a = (unsigned long int)*(uint32_t *)&(float){  (float)(1. / 12.) };
		b = (unsigned long int)*(uint32_t *)&(float){ -(float)(1. / 12.) };
		printf("#define USING_BINARY32 %i\n",
		       a == 0x3daaaaabUL && b == 0xbdaaaaabUL);
	}
	if (sizeof(double) == 8) {
		unsigned long long int a, b;
		a = (unsigned long long int)*(uint64_t *)&(double){  1. / 12. };
		b = (unsigned long long int)*(uint64_t *)&(double){ -1. / 12. };
		printf("#define USING_BINARY64 %i\n",
		       a == 0x3fb5555555555555ULL && b == 0xbfb5555555555555ULL);
	}

	return 0;
}
