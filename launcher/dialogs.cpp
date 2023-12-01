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
#include "utils.h"

#define SCR_WIDTH 960
#define SCR_HEIGHT 544

void early_fatal_error(const char *msg) {
	vglInit(0);
	SceMsgDialogUserMessageParam msg_param;
	sceClibMemset(&msg_param, 0, sizeof(SceMsgDialogUserMessageParam));
	msg_param.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_OK;
	msg_param.msg = (const SceChar8*)msg;
	SceMsgDialogParam param;
	sceMsgDialogParamInit(&param);
	param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
	param.userMsgParam = &msg_param;
	sceMsgDialogInit(&param);
	while (sceMsgDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
		vglSwapBuffers(GL_TRUE);
	}
	sceKernelExitProcess(0);
}

void DrawExtractorDialog(int index, float file_extracted_bytes, float extracted_bytes, float file_total_bytes, float total_bytes, char *filename, int num_files) {
	sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DEFAULT);
	
	ImGui_ImplVitaGL_NewFrame();
	
	char msg1[256], msg2[256];
	sprintf(msg1, "%s (%d / %d)", "Extracting archive...", index, num_files);
	sprintf(msg2, "%s (%.2f %s / %.2f %s)", filename, format_size(file_extracted_bytes), format_size_str(file_extracted_bytes), format_size(file_total_bytes), format_size_str(file_total_bytes));
	ImVec2 pos1 = ImGui::CalcTextSize(msg1);
	ImVec2 pos2 = ImGui::CalcTextSize(msg2);
	
	ImGui::GetIO().MouseDrawCursor = false;
	ImGui::SetNextWindowPos(ImVec2((SCR_WIDTH / 2) - 200, (SCR_HEIGHT / 2) - 50), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiSetCond_Always);
	ImGui::Begin("", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav);
	ImGui::SetCursorPos(ImVec2((400 - pos1.x) / 2, 20));
	ImGui::Text(msg1);
	ImGui::SetCursorPos(ImVec2((400 - pos2.x) / 2, 40));
	ImGui::Text(msg2);
	ImGui::SetCursorPos(ImVec2(100, 60));
	ImGui::ProgressBar(extracted_bytes / total_bytes, ImVec2(200, 0));
	
	ImGui::End();
	
	glViewport(0, 0, static_cast<int>(ImGui::GetIO().DisplaySize.x), static_cast<int>(ImGui::GetIO().DisplaySize.y));
	ImGui::Render();
	ImGui_ImplVitaGL_RenderDrawData(ImGui::GetDrawData());
	vglSwapBuffers(GL_FALSE);
}

void DrawDownloaderDialog(int index, float downloaded_bytes, float total_bytes, char *text, int passes, bool self_contained) {
	sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DEFAULT);
	
	if (self_contained)
		ImGui_ImplVitaGL_NewFrame();
	
	char msg[512];
	sprintf(msg, "%s (%d / %d)", text, index, passes);
	ImVec2 pos = ImGui::CalcTextSize(msg);

	ImGui::SetNextWindowPos(ImVec2((SCR_WIDTH / 2) - 200, (SCR_HEIGHT / 2) - 50), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiSetCond_Always);
	ImGui::Begin("downloader", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav);
	
	ImGui::SetCursorPos(ImVec2((400 - pos.x) / 2, 20));
	ImGui::Text(msg);
	if (total_bytes < 4000000000.0f) {
		sprintf(msg, "%.2f %s / %.2f %s", format_size(downloaded_bytes), format_size_str(downloaded_bytes), format_size(total_bytes), format_size_str(total_bytes));
		pos = ImGui::CalcTextSize(msg);
		ImGui::SetCursorPos(ImVec2((400 - pos.x) / 2, 40));
		ImGui::Text(msg);
		ImGui::SetCursorPos(ImVec2(100, 60));
		ImGui::ProgressBar(downloaded_bytes / total_bytes, ImVec2(200, 0));
	} else {
		sprintf(msg, "%.2f %s", format_size(downloaded_bytes), format_size_str(downloaded_bytes));
		pos = ImGui::CalcTextSize(msg);
		ImGui::SetCursorPos(ImVec2((400 - pos.x) / 2, 50));
		ImGui::Text(msg);
	}
	
	ImGui::End();
	
	if (self_contained) {
		glViewport(0, 0, static_cast<int>(ImGui::GetIO().DisplaySize.x), static_cast<int>(ImGui::GetIO().DisplaySize.y));
		ImGui::Render();
		ImGui_ImplVitaGL_RenderDrawData(ImGui::GetDrawData());
		vglSwapBuffers(GL_FALSE);
	}
	
}
