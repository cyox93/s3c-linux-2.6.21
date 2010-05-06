/*
 * nand_cp.c
 * nand utils for CANOPUS
 */

#include <linux/mtd/nand.h>
#include "canopus.h"

extern struct mtd_info *s3c_mtd;

/**
 * get_len_incl_bad
 *
 * Check if length including bad blocks fits into device.
 *
 * @param nand NAND device
 * @param offset offset in flash
 * @param length image length
 * @return image length including bad blocks
 */
static size_t get_len_incl_bad (nand_info_t *nand, ulong offset,
				const size_t length)
{
	size_t len_incl_bad = 0;
	size_t len_excl_bad = 0;
	size_t block_len;

	while (len_excl_bad < length) {
		block_len = nand->erasesize - (offset & (nand->erasesize - 1));

		if (!q_nand_block_isbad (nand, offset & ~(nand->erasesize - 1))) {
			len_excl_bad += block_len;
		}

		len_incl_bad += block_len;
		offset       += block_len;

		if (offset >= nand->size)
			break;
	}

	return len_incl_bad;
}

/**
 * nand_write_skip_bad:
 *
 * Write image to NAND flash.
 * Blocks that are marked bad are skipped and the is written to the next
 * block instead as long as the image is short enough to fit even after
 * skipping the bad blocks.
 *
 * @param nand  	NAND device
 * @param offset	offset in flash
 * @param length	buffer length
 * @param buf           buffer to read from
 * @return		0 in case of success
 */
int nand_write_skip_bad(nand_info_t *nand, ulong offset, size_t *length,
			unsigned char *buffer)
{
	int rval;
	size_t left_to_write = *length;
	size_t len_incl_bad;
	uint8_t *p_buffer = (uint8_t*)buffer;

	/* Reject writes, which are not page aligned */
	if ((offset & (nand->writesize - 1)) != 0 ||
	    (*length & (nand->writesize - 1)) != 0) {
		printk ("Attempt to write non page aligned data\n");
		return -EINVAL;
	}

	len_incl_bad = get_len_incl_bad (nand, offset, *length);

	if ((offset + len_incl_bad) > nand->size) {
		printk ("Attempt to write outside the flash area\n");
		return -EINVAL;
	}

	if (len_incl_bad == *length) {
		rval = q_nand_write (nand, offset, *length, length, buffer);
		if (rval != 0)
			printk ("NAND write to offset 0x%x failed %d\n",
				(unsigned int)offset, rval);

		return rval;
	}

	while (left_to_write > 0) {
		size_t block_offset = offset & (nand->erasesize - 1);
		size_t write_size;

#if 0
		WATCHDOG_RESET ();
#endif

		if (q_nand_block_isbad (nand, offset & ~(nand->erasesize - 1))) {
			printk ("Skip bad block 0x%08x\n",
				(unsigned int)offset & ~(nand->erasesize - 1));
			offset += nand->erasesize - block_offset;
			continue;
		}

		if (left_to_write < (nand->erasesize - block_offset))
			write_size = left_to_write;
		else
			write_size = nand->erasesize - block_offset;

		rval = q_nand_write (nand, offset, write_size, &write_size, p_buffer);
		if (rval != 0) {
			printk ("NAND write to offset 0x%x failed %d\n",
				(unsigned int)offset, rval);
			*length -= left_to_write;
			return rval;
		}

		left_to_write -= write_size;
		offset        += write_size;
		p_buffer      += write_size;
	}

	return 0;
}

/**
 * nand_read_skip_bad:
 *
 * Read image from NAND flash.
 * Blocks that are marked bad are skipped and the next block is readen
 * instead as long as the image is short enough to fit even after skipping the
 * bad blocks.
 *
 * @param nand NAND device
 * @param offset offset in flash
 * @param length buffer length, on return holds remaining bytes to read
 * @param buffer buffer to write to
 * @return 0 in case of success
 */
int nand_read_skip_bad(nand_info_t *nand, ulong offset, size_t *length,
		       uint8_t *buffer)
{
	int rval;
	ulong left_to_read = *length;
	size_t len_incl_bad;
	u_char *p_buffer = buffer;

	len_incl_bad = get_len_incl_bad (nand, offset, *length);

	if ((offset + len_incl_bad) > nand->size) {
		printk ("Attempt to read outside the flash area\n");
		return -EINVAL;
	}

	if (len_incl_bad == *length) {
		rval = q_nand_read (nand, offset, *length, length, buffer);
		if (!rval || rval == -EUCLEAN)
			return 0;
		printk ("NAND read from offset 0x%x failed %d\n",
			(unsigned int)offset, rval);
		return rval;
	}

	while (left_to_read > 0) {
		size_t block_offset = offset & (nand->erasesize - 1);
		size_t read_length;

#if 0
		WATCHDOG_RESET ();
#endif
		if (q_nand_block_isbad (nand, offset & ~(nand->erasesize - 1))) {
			printk ("Skipping bad block 0x%08x\n",(unsigned int) offset & ~(nand->erasesize - 1));
			offset += nand->erasesize - block_offset;
			continue;
		}

		if (left_to_read < (nand->erasesize - block_offset))
			read_length = left_to_read;
		else
			read_length = nand->erasesize - block_offset;

		rval = q_nand_read (nand, offset, read_length, &read_length, p_buffer);
		if (rval && rval != -EUCLEAN) {
			printk ("NAND read from offset 0x%x failed %d\n",
				(unsigned int)offset, rval);
			*length -= left_to_read;
			return rval;
		}

		left_to_read -= read_length;
		offset       += read_length;
		p_buffer     += read_length;

	}

	return 0;
}
