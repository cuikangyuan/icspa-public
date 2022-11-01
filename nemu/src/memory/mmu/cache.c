#include "memory/mmu/cache.h"
#include "memory/memory.h"
#include <stdlib.h>

/*
在NEMU中实现一个cache，它的性质如下:

cache block存储空间的大小为64B（2^6）

cache存储空间的大小为64KB ---> 2^10个行

8-way set associative --> 2^7个组 一个组里 2^3个行

物理地址：低3位 低7位 低6位
低b位(b = 6) 块内地址
中间q位(q = 7) Cache组号
高m-q(19)位   标记

标志位只需要valid bit即可

替换算法采用随机方式

write through 写cache同时写主存
not write allocate 写缺失不导致Cache装载
*/

#define DEBUG

#ifdef DEBUG
#define trace_cache(format, args...)                                         \
	do                                                                       \
	{                                                                        \
		printf("\33[33;31m[CACHE:%d] " format "\33[0m\n", __LINE__, ##args); \
	} while (0)
#else
#define trace_cache(format, args...) ((void)0)
#endif

#define CACHE_DATA_SIZE_PER_LINE 64
#define CACHE_LINE_SIZE_PER_SET 8
#define CACHE_SET_SIZE 128

typedef struct CACHE_LINE
{
	uint8_t data[CACHE_DATA_SIZE_PER_LINE];
	uint32_t tag;
	bool valid;
} CACHE_LINE;

typedef struct CACHE_SET
{
	CACHE_LINE lines[CACHE_LINE_SIZE_PER_SET];
} CACHE_SET;

typedef struct L1CACHE
{
	CACHE_SET sets[CACHE_SET_SIZE];
} L1CACHE;

L1CACHE cache;

extern void hw_mem_write(paddr_t paddr, size_t len, uint32_t data);
extern uint32_t hw_mem_read(paddr_t paddr, size_t len);

uint32_t cache_enable_counter = 0;

uint32_t get_cache_enable_counter() {
	return cache_enable_counter;
}

// init the cache
void init_cache()
{
	//将所有valid bit置为无效
	for (int i = 0; i < CACHE_SET_SIZE; i++)
	{
		for (int j = 0; j < CACHE_LINE_SIZE_PER_SET; j++)
		{
			cache.sets[i].lines[j].valid = false;
		}
	}
}

// write data to cache
void cache_write(paddr_t paddr, size_t len, uint32_t data)
{
	// write through 写cache同时写主存
	memcpy(hw_mem + paddr, &data, len);

	// not write allocate 写缺失不导致Cache装载
	uint32_t set_index = (paddr >> 6) & 0x7F;
	uint32_t offset_in_line = paddr & 0x3F;
	uint32_t tag_to_match = (paddr >> (6 + 7)) & 0x7FFFF;

	if (offset_in_line + len <= CACHE_DATA_SIZE_PER_LINE)
	{
		CACHE_SET *set_matched = cache.sets + set_index;
		for (int i = 0; i < CACHE_LINE_SIZE_PER_SET; i++)
		{
			CACHE_LINE *line = &set_matched->lines[i];

			if ((line->tag == tag_to_match) && line->valid)
			{
				memcpy(line->data + offset_in_line, &data, len);
			}
		}
		hw_mem_write(paddr, len, data);
	}
	else
	{

		for (int k = 0; k < 2; k++)
		{
			uint32_t set_index_to_use = set_index + k;

			if (set_index + k == 128)
			{
				set_index_to_use = 0;
				tag_to_match = tag_to_match + 1;
			}

			CACHE_SET *set_item = cache.sets + set_index_to_use;

			for (int i = 0; i < CACHE_LINE_SIZE_PER_SET; i++)
			{
				CACHE_LINE *line = &set_item->lines[i];

				if ((line->tag == tag_to_match) && line->valid)
				{
					if (k == 0)
					{
						memcpy(line->data + offset_in_line, &data, 64 - offset_in_line);
					}
					else
					{
						memcpy(line->data, &data, len - (64 - offset_in_line));
					}

					break;
				}
			}
		}

		hw_mem_write(paddr, len, data);
	}
}

// read data from cache
uint32_t cache_read(paddr_t paddr, size_t len)
{
	// implement me in PA 3-1
	uint32_t set_index = (paddr >> 6) & 0x7F;
	uint32_t offset_in_line = paddr & 0x3F;
	uint32_t tag_to_match = (paddr >> (6 + 7)) & 0x7FFFF;

	if (offset_in_line + len <= CACHE_DATA_SIZE_PER_LINE)
	{
		CACHE_SET *set_matched = cache.sets + set_index;
		for (int i = 0; i < CACHE_LINE_SIZE_PER_SET; i++)
		{
			CACHE_LINE *line = &set_matched->lines[i];
			if ((line->tag == tag_to_match) && line->valid)
			{	
				//cache hit
				cache_enable_counter += 1;
				uint32_t ret = 0;
				memcpy(&ret, line->data + offset_in_line, len);
				return ret;
			}
		}

		int line_index_to_fill = 0;
		for (line_index_to_fill = 0; line_index_to_fill < CACHE_LINE_SIZE_PER_SET; line_index_to_fill++)
		{
			if (!set_matched->lines[line_index_to_fill].valid)
			{
				break;
			}
		}

		if (line_index_to_fill == CACHE_LINE_SIZE_PER_SET)
		{
			line_index_to_fill = rand() % CACHE_LINE_SIZE_PER_SET;
		}

		CACHE_LINE *line_to_update = set_matched->lines + line_index_to_fill;
		uint32_t data_offset = paddr & 0xFFFFFFC0;
		memcpy(line_to_update->data, hw_mem + data_offset, CACHE_DATA_SIZE_PER_LINE);
		line_to_update->valid = true;
		line_to_update->tag = tag_to_match;

		//cache miss
		cache_enable_counter += 10;
		uint32_t ret = 0;
		memcpy(&ret, line_to_update->data + offset_in_line, len);
		return ret;
	}
	else
	{

		uint32_t ret = 0;
		for (int k = 0; k < 2; k++)
		{
			uint32_t set_index_to_use = set_index + k;

			if (set_index + k == 128)
			{
				set_index_to_use = 0;
				tag_to_match = tag_to_match + 1;
			}

			CACHE_SET *set_item = cache.sets + set_index_to_use;

			// search for valid line in set
			bool flag = false;
			for (int i = 0; i < CACHE_LINE_SIZE_PER_SET; i++)
			{
				CACHE_LINE *line = &set_item->lines[i];

				if ((line->tag == tag_to_match) && line->valid)
				{
					uint32_t tmp = 0;
					if (k == 0)
					{
						memcpy(&tmp, line->data + offset_in_line, 64 - offset_in_line);
						cache_enable_counter += 1;
						ret = tmp;
					}
					else
					{
						memcpy(&tmp, line->data, len - (64 - offset_in_line));
						cache_enable_counter += 1;
						ret = ret + (tmp << (8 * (64 - offset_in_line)));
					}

					flag = true;
					break;
				}
			}
			if (flag)
			{
				continue;
			}

			// search for invalid line to cache data read from memory
			for (int i = 0; i < CACHE_LINE_SIZE_PER_SET; i++)
			{
				CACHE_LINE *line = &set_item->lines[i];

				if (!line->valid)
				{
					//caution: data_offset is the start address of this block, so the data len can set to 64
					uint32_t data_offset = ((tag_to_match << 7) + set_index_to_use) << 6;
					memcpy(line->data, hw_mem + data_offset, CACHE_DATA_SIZE_PER_LINE);
					line->valid = true;
					line->tag = tag_to_match;

					uint32_t tmp = 0;
					if (k == 0)
					{
						memcpy(&tmp, line->data + offset_in_line, 64 - offset_in_line);
						cache_enable_counter += 10;
						ret = tmp;
					}
					else
					{
						memcpy(&tmp, line->data, len - (64 - offset_in_line));
						cache_enable_counter += 10;
						ret = ret + (tmp << (8 * (64 - offset_in_line)));
					}

					flag = true;
					break;
				}
			}
			if (flag)
			{
				continue;
			}

			int line_index_to_fill = rand() % CACHE_LINE_SIZE_PER_SET;
			CACHE_LINE *line = &set_item->lines[line_index_to_fill];
			//caution: data_offset is the start address of this block, so the data len can set to 64
			uint32_t data_offset = ((tag_to_match << 7) + set_index_to_use) << 6;
			memcpy(line->data, hw_mem + data_offset, CACHE_DATA_SIZE_PER_LINE);
			line->valid = true;
			line->tag = tag_to_match;

			uint32_t tmp = 0;
			if (k == 0)
			{
				memcpy(&tmp, line->data + offset_in_line, 64 - offset_in_line);
				cache_enable_counter += 10;
				ret = tmp;
			}
			else
			{
				memcpy(&tmp, line->data, len - (64 - offset_in_line));
				cache_enable_counter += 10;
				ret = ret + (tmp << (8 * (64 - offset_in_line)));
			}
		}

		return ret;
	}
}
