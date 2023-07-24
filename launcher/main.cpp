#include <iostream>
#include <string>
#include <vitasdk.h>
#include <vitaGL.h>
#include <imgui_vita.h>
#include <imgui_internal.h>
#include <bzlib.h>
#include <stdio.h>
#include <string>
#include <taihen.h>
#include "unzip.h"
#include "utils.h"
#include "dialogs.h"
#include "network.h"

int _newlib_heap_size_user = 256 * 1024 * 1024;

int file_exists(const char *path) {
	SceIoStat stat;
	return sceIoGetstat(path, &stat) >= 0;
}

static char fname[512], ext_fname[512], read_buffer[8192];
void extract_file(char *file, char *dir) {
	FILE *f;
	unz_global_info global_info;
	unz_file_info file_info;
	unzFile zipfile = unzOpen(file);
	unzGetGlobalInfo(zipfile, &global_info);
	unzGoToFirstFile(zipfile);
	uint64_t total_extracted_bytes = 0;
	uint64_t curr_extracted_bytes = 0;
	uint64_t curr_file_bytes = 0;
	int num_files = global_info.number_entry;
	for (int zip_idx = 0; zip_idx < num_files; ++zip_idx) {
		unzGetCurrentFileInfo(zipfile, &file_info, fname, 512, NULL, 0, NULL, 0);
		total_extracted_bytes += file_info.uncompressed_size;
		if ((zip_idx + 1) < num_files) unzGoToNextFile(zipfile);
	}
	unzGoToFirstFile(zipfile);
	for (int zip_idx = 0; zip_idx < num_files; ++zip_idx) {
		unzGetCurrentFileInfo(zipfile, &file_info, fname, 512, NULL, 0, NULL, 0);
		sprintf(ext_fname, "%s/%s", dir, fname);
		const size_t filename_length = strlen(ext_fname);
		if (ext_fname[filename_length - 1] != '/') {
			curr_file_bytes = 0;
			unzOpenCurrentFile(zipfile);
			//printf("%s\n", ext_fname);
			if (!strcmp(ext_fname, "ux0://nzp.vpk")) {
				strcpy(ext_fname, "ux0:data/nzp.vpk");
			} else {
				recursive_mkdir(ext_fname);
			}
			FILE *f = fopen(ext_fname, "wb");
			while (curr_file_bytes < file_info.uncompressed_size) {
				int rbytes = unzReadCurrentFile(zipfile, read_buffer, 8192);
				if (rbytes > 0) {
					fwrite(read_buffer, 1, rbytes, f);
					curr_extracted_bytes += rbytes;
					curr_file_bytes += rbytes;
				}
				DrawExtractorDialog(zip_idx + 1, curr_file_bytes, curr_extracted_bytes, file_info.uncompressed_size, total_extracted_bytes, fname, num_files);
			}
			fclose(f);
			unzCloseCurrentFile(zipfile);
		}
		if ((zip_idx + 1) < num_files) unzGoToNextFile(zipfile);
	}
	unzClose(zipfile);
	ImGui::GetIO().MouseDrawCursor = false;
}

int main(int argc, char *argv[]) {
	scePowerSetArmClockFrequency(444);
	scePowerSetBusClockFrequency(222);
	sceIoRemove(TEMP_DOWNLOAD_NAME);
	
	// Initializing sceNet
	generic_mem_buffer = (uint8_t*)malloc(MEM_BUFFER_SIZE);
	sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	int ret = sceNetShowNetstat();
	SceNetInitParam initparam;
	if (ret == SCE_NET_ERROR_ENOTINIT) {
		initparam.memory = malloc(141 * 1024);
		initparam.size = 141 * 1024;
		initparam.flags = 0;
		sceNetInit(&initparam);
	}
	
	// Checking for network connection
	char cur_version[32] = {0};
	char *start;
	FILE *f;
	sceNetCtlInit();
	sceNetCtlInetGetState(&ret);
	if (ret != SCE_NETCTL_STATE_CONNECTED) {
		goto launch_nzp;
	}
	
	// Check for libshacccg.suprx existence
	if (!file_exists("ur0:/data/libshacccg.suprx") && !file_exists("ur0:/data/external/libshacccg.suprx"))
		early_fatal_error("Error: libshacccg.suprx is not installed.");
	
	// Starting dear ImGui
	vglInitExtended(0, 960, 544, 0x1800000, SCE_GXM_MULTISAMPLE_NONE);
	ImGui::CreateContext();
	SceKernelThreadInfo info;
	info.size = sizeof(SceKernelThreadInfo);
	ImGui_ImplVitaGL_Init_Extended();
	ImGui::GetIO().MouseDrawCursor = false;
	
	// Getting current version
	f = fopen("ux0:data/nzp/nzp/version.txt", "r");
	if (f) {
		fread(cur_version, 1, 32, f);
		fclose(f);
		if (cur_version[strlen(cur_version) - 1] == '\n')
			cur_version[strlen(cur_version) - 1] = 0;
	} else {
		goto download_update;
	}
	printf("Current version: %s\n", cur_version);
	
	// Checking for updates
	download_file("https://api.github.com/repos/nzp-team/nzportable/releases/tags/nightly", "Checking for updates");
	f = fopen(TEMP_DOWNLOAD_NAME, "r");
	if (f) {
		fread(generic_mem_buffer, 1, 32 * 1024 * 1024, f);
		fclose(f);
		start = strstr(generic_mem_buffer, "\"name\": \"") + 9;
		start[strlen(cur_version)] = 0;
		printf("Last version: %s\n", start);
		if (!strcmp(cur_version, start)) {
			printf("Up to date\n");
			goto launch_nzp;
		} else {
download_update:
			sceAppMgrUmount("app0:");
			printf("Downloading update\n");
			download_file("https://github.com/nzp-team/nzportable/releases/download/nightly/nzportable-vita.zip", "Downloading update");
			printf("Extracting data files\n");
			extract_file(TEMP_DOWNLOAD_NAME, "ux0:/");
			sceIoRemove(TEMP_DOWNLOAD_NAME);
			printf("Extracting vpk\n");
			extract_file("ux0:data/nzp.vpk", "ux0:app/NZZMBSPTB/");
		}
	}
	printf("%s\n", generic_mem_buffer);
	
launch_nzp:
	sceAppMgrLoadExec("app0:nzp.bin", NULL, NULL);
	return 0;
}