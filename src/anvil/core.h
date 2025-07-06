#ifndef CORE_H
#define CORE_H

#include "base.h"
#include "math.h"


// files
string_t os_read_entire_file(string_t path);


//
// input
//

// keyboard
#if OS_WIN32
#include "os/core_win32.h"
#endif

#if OS_LINUX
#include "os/core_linux.h"
#endif

bool8_t key_down(key_code_e key);
bool8_t key_up(key_code_e key);
bool8_t key_pressed(key_code_e key);
bool8_t key_released(key_code_e key);

// mouse
typedef enum mouse_button {
	MOUSE_BUTTON_LEFT,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_RIGHT,
	MOUSE_BUTTON_COUNT
} mouse_button_e;

bool8_t mouse_button_down(mouse_button_e button);
bool8_t mouse_button_up(mouse_button_e button);
bool8_t mouse_button_pressed(mouse_button_e button);
bool8_t mouse_button_released(mouse_button_e button);


//
// strings
//

string_t string_create(uint32_t length);
void string_delete(string_t string);
string_t string_format(string_t format, ...);
string_t string_concat(string_t a, string_t b);


//
// OS
//

// time
float64_t os_time();

// messaging
typedef enum os_message_icon {
    OS_MESSAGE_ERROR,
    OS_MESSAGE_QUESTION,
    OS_MESSAGE_WARNING,
    OS_MESSAGE_INFO
} os_message_icon_e;

void os_message(os_message_icon_e icon, string_t format, ...);

// window
typedef struct os_window os_window_o;

typedef enum os_window_flags {
    OS_WINDOW_NONE       = 0,
	OS_WINDOW_CENTERED   = BIT(0),
	OS_WINDOW_FULLSCREEN = BIT(1),
	OS_WINDOW_RESIZABLE  = BIT(2),
	OS_WINDOW_MINIMIZED  = BIT(3),
	OS_WINDOW_MAXIMIZED  = BIT(4)
} os_window_flags_e;

os_window_o *os_window_create(const string_t title, uint16_t width, uint16_t height, int32_t x, int32_t y, uint32_t flags);
void os_window_delete(os_window_o *window);
void os_window_swap_buffers(os_window_o *window);
void os_window_vsync(os_window_o *window, bool8_t enabled);

// event
typedef struct os_event {
    bool8_t should_quit;
    uint16_t width, height, x, y;
	vec2_t cursor;
} os_event_t;

void os_event_pull(os_window_o *window, os_event_t *event);

#endif // CORE_H
