#include "base.h"
#include "math.h"
#include "core.h"
#define STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>

// globals
global bool8_t _curr_keyboard[KEY_COUNT], _last_keyboard[KEY_COUNT];
global bool8_t _curr_mouse_buttons[MOUSE_BUTTON_COUNT], _last_mouse_buttons[MOUSE_BUTTON_COUNT];


// files
string_t os_read_entire_file(string_t path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        _exit_os_read_file:
		os_message(OS_MESSAGE_WARNING, "Failed to read file\nPath: %s", path);
		return NULL;
    }
    
    int64_t fsize = ftell(f);
    if (fsize == -1L) {
        fclose(f);
		goto _exit_os_read_file;
    }
    
    rewind(f);
    
    string_t string = malloc(fsize + 1);
    if (!string) {
        fclose(f);
        return NULL;
    }
    
    uint64_t bytes_read = fread(string, 1, fsize, f);
    fclose(f);
    
    if (bytes_read != (size_t)fsize) {
        free(string);
        goto _exit_os_read_file;
    }
    
    string[fsize] = 0;
    return string;
}


//
// input
//

// keyboard
bool8_t key_down(key_code_e key) { return _curr_keyboard[key]; }
bool8_t key_up(key_code_e key) { return !_curr_keyboard[key]; }
bool8_t key_pressed(key_code_e key) { return _curr_keyboard[key] && !_last_keyboard[key]; }
bool8_t key_released(key_code_e key) { return !_curr_keyboard[key] && _last_keyboard[key]; }

// mouse
bool8_t mouse_button_down(mouse_button_e button) { return _curr_mouse_buttons[button]; }
bool8_t mouse_button_up(mouse_button_e button) { return !_curr_mouse_buttons[button]; }
bool8_t mouse_button_pressed(mouse_button_e button) { return _curr_mouse_buttons[button] && !_last_mouse_buttons[button]; }
bool8_t mouse_button_released(mouse_button_e button) { return !_curr_mouse_buttons[button] && _last_mouse_buttons[button]; }


//
// strings
//

string_t string_create(uint32_t length) {
	string_t string = (string_t)malloc(length + 1);
	if (!string) {
		os_message(OS_MESSAGE_ERROR, "Failed to allocate memory for string");
		exit(EXIT_FAILURE);
	}
	
	string[length] = 0;
	return string;
}

void string_delete(string_t string) {
	if (string) {
		free(string);
	}
}

string_t string_format(string_t format, ...) {
	va_list args;
	
	va_start(args, format);
	int32_t needed_size = stbsp_vsprintf(NULL, format, args) + 1; // +1 for null terminator
	va_end(args);
	
	if (needed_size <= 0) {
		os_message(OS_MESSAGE_WARNING, "Cannot get string format length");
		return NULL;
	}
	
	string_t string = string_create(needed_size);
	
	va_start(args, format);
    stbsp_vsprintf(string, format, args);
	va_end(args);
	
	return string;
}

string_t string_concat(string_t a, string_t b) {
	uint32_t la = strlen(a);
	uint32_t lb = strlen(b);
	
	string_t string = string_create(la + lb);
	
	memcpy(string, a, la);
	memcpy(string + la, b, lb);
	return string;
}


//
// OS
//

#if OS_WIN32
#include "os/core_win32.c"
#endif

#if OS_LINUX
#include "os/core_linux.c"
#endif
