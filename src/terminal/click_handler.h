#ifndef CLICK_HANDLER_H
#define CLICK_HANDLER_H

#include "term_handler.h"
#include "../data-management/file_management.h"
#include "../data-management/file_structure.h"
#include "../io_management/io_explorer.h"

////// -------------- CLICK FUNCTIONS --------------

void handleEditorClick(GUIContext* gui_context, Cursor* cursor, Cursor* select_cursor, int* desired_column, int* screen_x, int* screen_y, MEVENT* m_event, bool button1_down);

void handleOpenedFileClick(GUIContext* gui_context, FileContainer* files, int* file_count, int* current_file, MEVENT m_event, bool* refresh_local_vars, bool mouse_drag);

void handleFileExplorerClick(GUIContext* gui_context, FileContainer** files, int* file_count, int* current_file, ExplorerFolder* pwd, MEVENT m_event, bool* refresh_local_vars);

// true if found, false if not. if file_index == -1 => Res_folder was clicked. If file_index != -1 => file clicked is res_folder.files[file_index].
bool getFileClickedFileExplorer(ExplorerFolder* pwd, int y_click, int few_x_offsset, int few_y_offset, ExplorerFolder** res_folder, int* file_index);


#endif //CLICK_HANDLER_H
