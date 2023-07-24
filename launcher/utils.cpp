/*
 * This file is part of VitaDB Downloader
 * Copyright 2022 Rinnegatamante
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include <stdio.h>
#include <string>
#include <vitasdk.h>
#include <vitaGL.h>
#include <imgui_vita.h>
#include <imgui_internal.h>
#include "network.h"
#include "utils.h"

static const char *sizes[] = {
	"B",
	"KB",
	"MB",
	"GB"
};

float format_size(float len) {
	while (len > 1024) len = len / 1024.0f;
	return len;
}

const char *format_size_str(uint64_t len) {
	uint8_t ret = 0;
	while (len > 1024) {
		ret++;
		len = len / 1024;
	}
	return sizes[ret];
}

void copy_file(const char *src, const char *dst) {
	FILE *fs = fopen(src, "r");
	FILE *fd = fopen(dst, "w");
	size_t fsize = fread(generic_mem_buffer, 1, MEM_BUFFER_SIZE, fs);
	fwrite(generic_mem_buffer, 1, fsize, fd);
	fclose(fs);
	fclose(fd);
}

void recursive_rmdir(const char *path) {
	SceUID d = sceIoDopen(path);
	if (d >= 0) {
		SceIoDirent g_dir;
		while (sceIoDread(d, &g_dir) > 0) {
			char fpath[512];
			sprintf(fpath, "%s/%s", path, g_dir.d_name);
			if (SCE_S_ISDIR(g_dir.d_stat.st_mode))
				recursive_rmdir(fpath);
			else
				sceIoRemove(fpath);
		}
		sceIoDclose(d);
		sceIoRmdir(path);
	}
}

void recursive_mkdir(char *dir) {
	char *p = dir;
	while (p) {
		char *p2 = strstr(p, "/");
		if (p2) {
			p2[0] = 0;
			sceIoMkdir(dir, 0777);
			p = p2 + 1;
			p2[0] = '/';
		} else break;
	}
}
