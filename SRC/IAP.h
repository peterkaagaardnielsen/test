
#define FLASH_PAGE_SIZE         4096


/*
 * Erase Sector between 'start' and 'end'
 * Return:  IAP error code (0 when OK)
 * NOTES:  start address: The page in which this address resides, will be the first page erased
 *         end address: The page in which this address resides, will be the last page erased
 *         Look in IAP.c for page size!!
 */
unsigned int eraseIAP (unsigned int start, unsigned int end);

/*
 * Program *data to flash_addr. number of bytes specified by size
 * Return:  IAP error code (0 when OK)
 * Note: 
 */
unsigned int programIAP (unsigned int flash_addr, void *data, unsigned int size);

