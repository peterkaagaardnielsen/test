#ifndef NVSTORE_H
#define NVSTORE_H

#define NVSTORE_ERR_OK      0 
#define NVSTORE_ERR_COMM    1  
#define NVSTORE_ERR_PAGE_NO 2  

void initNVstore(void);
int readNVstore(unsigned char *buffer, int pageNumber);
int writeNVstore(const unsigned char *buffer, int pageNumber);
int eraseNVstore(int pageNumber);

#endif
