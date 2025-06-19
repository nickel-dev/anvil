#include "../base.h"
#include "../core.h"
#include <Windows.h>
#define GLAD_GL_IMPLEMENTATION
#include "../extern/glad.h"

//
// OS
//

// globals
global os_event_t _event;
global bool8_t _curr_keyboard[KEY_COUNT], _last_keyboard[KEY_COUNT];
global bool8_t _curr_mouse_buttons[MOUSE_BUTTON_COUNT], _last_mouse_buttons[MOUSE_BUTTON_COUNT];


// wgl extensions
#define WGL_CONTEXT_MAJOR_VERSION_ARB   0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB   0X2092
#define WGL_CONTEXT_FLAGS_ARB           0X2094
#define WGL_CONTEXT_COREPROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB    0x9126

typedef HGLRC (WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int32_t *attribList);
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;


// time
float64_t os_time() {
    LARGE_INTEGER frequency;
    LARGE_INTEGER currentTimeValue;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&currentTimeValue);
    return (float64_t)currentTimeValue.QuadPart / frequency.QuadPart;
}


// messaging
void os_message(os_message_icon_e icon, string_t format, ...) {
	va_list args;
	
    // calculate required buffer size
    va_start(args, format);
    int32_t needed_size = _vscprintf(format, args) + 1; // +1 for null terminator
    va_end(args);
	
    if (needed_size <= 0) {
        MessageBoxA(NULL, "Invalid message format", NULL, MB_OK | MB_ICONERROR);
        return;
    }
	
    // allocate buffer
    string_t buffer = (string_t)malloc(needed_size);
    if (!buffer) {
        MessageBoxA(NULL, "Memory allocation failed", NULL, MB_OK | MB_ICONERROR);
        return;
    }
	
    // format string
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
	
    // display messagebox
    MessageBoxA(NULL, buffer, NULL, MB_OK | (16 * (icon + 1)));
    free(buffer);
}


// window
struct os_window {
    WNDCLASSEXA wc;
    HWND handle;
    HDC device;
    HGLRC context;
};

internal LRESULT CALLBACK window_proc_cb(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	uint32_t vk_code = (uint32_t)wParam;
	
	switch (uMsg) {
		case WM_DESTROY:
		case WM_CLOSE:
		_event.should_quit = true;
	    PostQuitMessage(0);
		break;
		
		case WM_SIZE:
		_event.width  = LOWORD(lParam);
		_event.height = HIWORD(lParam);
		glViewport(0, 0, _event.width, _event.height);
		break;
		
		case WM_MOVE:
		_event.x = LOWORD(lParam);
		_event.y = HIWORD(lParam);
		break;
		
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		_curr_keyboard[vk_code] = true;
		if (vk_code == VK_MENU) { // Stop halting when ALT key is pressed
			return FALSE;
		}
		break;
		
		case WM_SYSKEYUP:
		case WM_KEYUP:
		_curr_keyboard[vk_code] = false;
		break;
		
		case WM_MOUSEMOVE: {
			int32_t x = LOWORD(lParam);
			int32_t y = HIWORD(lParam);
			
			if (x <= 0) { _event.cursor.x = 0.0f; } else { _event.cursor.x = (float64_t)x / _event.width  *  2; }
			if (y <= 0) { _event.cursor.y = 0.0f; } else { _event.cursor.y = (float64_t)y / _event.height * -2; }
			
			_event.cursor.x -= 1.0f;
			_event.cursor.y += 1.0f;
		} break;
		
		// mouse buttons
		case WM_LBUTTONDOWN:
		_curr_mouse_buttons[MOUSE_BUTTON_LEFT] = true;
		break;
		
		case WM_LBUTTONUP:
		_curr_mouse_buttons[MOUSE_BUTTON_LEFT] = false;
		break;
		
		case WM_MBUTTONDOWN:
		_curr_mouse_buttons[MOUSE_BUTTON_MIDDLE] = true;
		break;
		
		case WM_MBUTTONUP:
		_curr_mouse_buttons[MOUSE_BUTTON_MIDDLE] = false;
		break;
		
		case WM_RBUTTONDOWN:
		_curr_mouse_buttons[MOUSE_BUTTON_RIGHT] = true;
		break;
		
		case WM_RBUTTONUP:
		_curr_mouse_buttons[MOUSE_BUTTON_RIGHT] = false;
		break;
		
		default:
		break;
	}
	
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

os_window_o * os_window_create(const string_t title, uint16_t width, uint16_t height, int32_t x, int32_t y, uint32_t flags) {
	os_window_o *window = (os_window_o *)malloc(sizeof(os_window_o));
	ZERO_MEMORY(window);
	
	uint32_t style = WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_EX_TOPMOST;
	range2_t viewport = { 0 };
	
	// window class
	{
		window->wc = (WNDCLASSEXA){
			.cbSize = sizeof(window->wc),
			.style  = CS_HREDRAW | CS_VREDRAW,
			.lpfnWndProc = (WNDPROC)window_proc_cb,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance  = NULL,
			.hIcon   = LoadIcon(NULL, IDI_APPLICATION),
			.hIconSm = window->wc.hIcon,
			.hCursor = LoadCursor(NULL, IDC_ARROW),
			.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1),
			.lpszMenuName  = NULL,
			.lpszClassName = (LPCSTR)title
		};
		
		if (!RegisterClassExA(&window->wc)) {
			os_message(OS_MESSAGE_ERROR, "Failed to register window class");
			exit(EXIT_FAILURE);
			return NULL;
		}
	}
	
	// window config
	{
		int32_t screen_width = GetSystemMetrics(SM_CXSCREEN);
		int32_t screen_height = GetSystemMetrics(SM_CYSCREEN);
		
		// Maximized, Minimized
		if (flags & OS_WINDOW_MAXIMIZED) {
			style |= WS_MAXIMIZE;
		} else if (flags & OS_WINDOW_MINIMIZED) {
			style |= WS_MINIMIZE;
		}
		
		// Centered
		if (flags & OS_WINDOW_CENTERED) {
			x = (screen_width - width) / 2;
			y = (screen_height - height) / 2;
		}
		
		// Fullscreen
		if (flags & OS_WINDOW_FULLSCREEN) {
			style = WS_POPUP | WS_EX_TOPMOST;
			x = 0;
			y = 0;
			
			// TODO - pack this into the window_swap_buffers function
			{
				// has to be float, because the vec2_t uses floats
				float32_t bar = (screen_width - width * ((float)screen_height / height));
				viewport.min = (vec2_t){ bar / 2, 0 };
				viewport.max = (vec2_t){ screen_width - bar, screen_height };
			}
			
			width = screen_width;
			height = screen_height;
		}
		
		// Resizable
		if (flags & OS_WINDOW_RESIZABLE) {
			style |= WS_MAXIMIZEBOX | WS_SIZEBOX;
		}
	}
	
	// create window
	{
		window->handle = CreateWindowExA(0, window->wc.lpszClassName, window->wc.lpszClassName, style, x, y, width, height, 0, 0, 0, NULL);
		if (!window->handle) {
			os_message(OS_MESSAGE_ERROR, "Failed to create window");
			exit(EXIT_FAILURE);
			return NULL;
		}
		
		window->device = GetDC(window->handle);
		if (!window->device) {
			os_message(OS_MESSAGE_ERROR, "Failed to get window device context");
			exit(EXIT_FAILURE);
			return NULL;
		}
	}
	
	// set fullscreen dimensions
	// TODO - refactor this to make it more elegant
	if (!(flags & OS_WINDOW_FULLSCREEN)) {
		RECT rect;
		GetClientRect(window->handle, &rect);
		viewport.min = (vec2_t){ 0 };
		viewport.max = (vec2_t){ rect.right, rect.bottom };
	}
	
	// OpenGL context
	{
		PIXELFORMATDESCRIPTOR pfd = { 0 };
		
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 24;
		pfd.cDepthBits = 32;
		pfd.cStencilBits = 8;
		
		// pixel_format
		int32_t pixel_format = ChoosePixelFormat(window->device, &pfd);
		SetPixelFormat(window->device, pixel_format, &pfd);
		
		// We create a temporary HLGRC to init OpenGL
		HGLRC temprc = wglCreateContext(window->device);
		wglMakeCurrent(window->device, temprc);
		
		wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
		
		const int32_t attrib_list[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			WGL_CONTEXT_MINOR_VERSION_ARB, 3,
			WGL_CONTEXT_FLAGS_ARB, 0,
			WGL_CONTEXT_PROFILE_MASK_ARB,
			WGL_CONTEXT_COREPROFILE_BIT_ARB, 0
		};
		
		window->context = wglCreateContextAttribsARB(window->device, 0, attrib_list);
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(temprc);
		wglMakeCurrent(window->device, window->context);
		
		if (!gladLoaderLoadGL()) {
			os_message(OS_MESSAGE_ERROR, "Failed to load OpenGL");
			exit(EXIT_FAILURE);
			return NULL;
		}
		
		glViewport(viewport.min.x, viewport.min.y, viewport.max.x, viewport.max.y);
	}
	
	ShowWindow(window->handle, SW_SHOW);
	SetForegroundWindow(window->handle);
	return window;
}

void os_window_delete(os_window_o *window) {
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(window->context);
	
	CloseWindow(window->handle);
	ReleaseDC(window->handle, window->device);
	DestroyWindow(window->handle);
	UnregisterClassA(window->wc.lpszClassName, window->wc.hInstance);
	
	free(window);
}

void os_window_swap_buffers(os_window_o *window) {
	SwapBuffers(window->device);
}


// event
void os_event_pull(os_window_o *window, os_event_t *event) {
	UNUSED(window);
	memcpy(_last_keyboard, _curr_keyboard, KEY_COUNT);
	memcpy(_last_mouse_buttons, _curr_mouse_buttons, MOUSE_BUTTON_COUNT);
	
	MSG msg = { 0 };
	
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			_event.should_quit = true;
		}
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	memcpy(event, &_event, sizeof(os_event_t));
}


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
	int32_t needed_size = _vscprintf(format, args) + 1; // +1 for null terminator
	va_end(args);
	
	if (needed_size <= 0) {
		os_message(OS_MESSAGE_WARNING, "Cannot get string format length");
		return NULL;
	}
	
	string_t string = string_create(needed_size);
	
	va_start(args, format);
	vsprintf(string, format, args);
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
