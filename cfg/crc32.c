static unsigned long g_crc32_table[256] = {0};

void init_crc32_table()
{
	int i, j;

	for(i = 0; i != 256; i++)
	{
		unsigned long crc = i;
		for (j = 0; j != 8; j++)
		{
			if (crc & 1)
				crc = (crc >> 1) ^ 0xEDB88320;
			else
				crc >>= 1;
		}
		g_crc32_table[i] = crc;
	}
}

unsigned long crc32(char* buf, unsigned long len)
{
	unsigned long oldcrc32 = 0xFFFFFFFF;
	unsigned long i;

	for (i = 0; i != len; ++i)
	{
		unsigned long t = (oldcrc32 ^ buf[i]) & 0xFF;
		oldcrc32 = ((oldcrc32 >> 8) & 0xFFFFFF) ^ g_crc32_table[t];
	}

	return ~oldcrc32;
}
