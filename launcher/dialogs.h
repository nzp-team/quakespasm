#ifndef _DIALOGS_H
#define _DIALOGS_H

void early_fatal_error(const char *msg);

void DrawExtractorDialog(int index, float file_extracted_bytes, float extracted_bytes, float file_total_bytes, float total_bytes, char *filename, int num_files);
void DrawDownloaderDialog(int index, float downloaded_bytes, float total_bytes, char *text, int passes, bool self_contained);

#endif
