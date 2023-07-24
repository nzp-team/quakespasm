#ifndef _NETWORK_H
#define _NETWORK_H

#define MEM_BUFFER_SIZE (32 * 1024 * 1024)
#define TEMP_DOWNLOAD_NAME "ux0:data/nzp.tmp"

extern uint8_t *generic_mem_buffer;
extern volatile uint64_t total_bytes;
extern volatile uint64_t downloaded_bytes;
extern volatile uint8_t downloader_pass;

int downloadThread(unsigned int args, void *arg);

void download_file(char *url, char *text);

#endif
