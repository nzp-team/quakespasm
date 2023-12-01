#include <iostream>
#include <string>
#include <vitasdk.h>
#include <vitaGL.h>
#include <imgui_vita.h>
#include <imgui_internal.h>
#include <bzlib.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <taihen.h>
#include <psp2/ctrl.h> 
#include "unzip.h"
#include "utils.h"
#include "dialogs.h"
#include "network.h"

#define SCR_WIDTH 960
#define SCR_HEIGHT 544

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

void download_update() 
{
	sceAppMgrUmount("app0:");
	download_file("https://github.com/nzp-team/nzportable/releases/download/nightly/nzportable-vita.zip", "Downloading update");
	extract_file(TEMP_DOWNLOAD_NAME, "ux0:/");
	sceIoRemove(TEMP_DOWNLOAD_NAME);
	extract_file("ux0:data/nzp.vpk", "ux0:app/NZZMBSPTB/");
	sceIoRemove("ux0:data/nzp.vpk");
							
	printf("%s\n", generic_mem_buffer);
	sceAppMgrLoadExec("app0:nzp.bin", NULL, NULL);
	return 0;
}

int main(int argc, char *argv[]) {
	bool optionsmenu = true;
	
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
		sceAppMgrLoadExec("app0:nzp.bin", NULL, NULL);
		return 0;
	}
		
	// Check for libshacccg.suprx existence
	if (!file_exists("ur0:/data/libshacccg.suprx") && !file_exists("ur0:/data/external/libshacccg.suprx"))
		early_fatal_error("Error: libshacccg.suprx is not installed.");
	
	// Starting dear ImGui
	vglInitExtended(0, SCR_WIDTH, SCR_HEIGHT, 0x1800000, SCE_GXM_MULTISAMPLE_NONE);
	
	ImGui::CreateContext();
	
	SceKernelThreadInfo info;
	info.size = sizeof(SceKernelThreadInfo);
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplVitaGL_Init();
	ImGui::GetIO().MouseDrawCursor = false;
	
	bool done = false; 
	while(!done)
	{
		ImGui_ImplVitaGL_NewFrame();
		
		sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
		SceCtrlData ctrl;
		sceCtrlPeekBufferPositive(0, &ctrl, 1);
		
		if(optionsmenu)
		{
			char msg1[256], msg2[256];
			sprintf(msg1, "Press X to check for updates");
			sprintf(msg2, "Or press START to slay zombies!");
			ImVec2 pos1 = ImGui::CalcTextSize(msg1);
			ImVec2 pos2 = ImGui::CalcTextSize(msg2);
			
			ImGui::SetNextWindowPos(ImVec2((SCR_WIDTH / 2) - 200, (SCR_HEIGHT / 2) - 50), ImGuiSetCond_Always);
			ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiSetCond_Always);
			ImGui::Begin("", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav);
			ImGui::SetCursorPos(ImVec2((400 - pos1.x) / 2, 35));
			ImGui::Text(msg1);
			ImGui::SetCursorPos(ImVec2((400 - pos2.x) / 2, 53));
			ImGui::Text(msg2);
			ImGui::End();
			
			// Rendering
			glViewport(0, 0, static_cast<int>(ImGui::GetIO().DisplaySize.x), static_cast<int>(ImGui::GetIO().DisplaySize.y));
			ImGui::Render();
			ImGui_ImplVitaGL_RenderDrawData(ImGui::GetDrawData());
			vglSwapBuffers(GL_FALSE);
		}
		if(ctrl.buttons & SCE_CTRL_CROSS)
		{
			optionsmenu = false;
			
			// Getting current version
			f = fopen("ux0:data/nzp/nzp/version.txt", "r");
			if (f) {
				fread(cur_version, 1, 32, f);
				fclose(f);
			if (cur_version[strlen(cur_version) - 1] == '\n')
					cur_version[strlen(cur_version) - 1] = 0;
			} else {
				download_update();
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
					sceAppMgrLoadExec("app0:nzp.bin", NULL, NULL);
					return 0;
				} else {
					download_update();
				}
			}
		}
		if(ctrl.buttons & SCE_CTRL_START)
		{
			sceAppMgrLoadExec("app0:nzp.bin", NULL, NULL);
			return 0;
		}
	}
	
	// Cleanup
	ImGui_ImplVitaGL_Shutdown();
	ImGui::DestroyContext();
	vglEnd();
	
	return 0;
}