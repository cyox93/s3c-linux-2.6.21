/*
 * nand_cp.h
 * nand utils for CANOPUS
 */

#ifndef __CANOPUS_H__
#define __CANOPUS_H__

#include <linux/types.h>
#include <linux/mtd/nand.h>

typedef struct mtd_info nand_info_t;

int q_nand_block_isbad(struct mtd_info *mtd, loff_t offs);
int q_nand_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, uint8_t *buf);
int q_nand_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const uint8_t *buf);
int q_nand_erase(struct mtd_info *mtd, loff_t pos, size_t len);

int nand_write_skip_bad(nand_info_t *nand, ulong offset, size_t *length, unsigned char *buffer);
int nand_read_skip_bad(nand_info_t *nand, ulong offset, size_t *length, uint8_t *buffer);

/* for CRC calculation routine */
ulong q_uboot_crc32(ulong crc, const char *buf, uint len);
#endif

