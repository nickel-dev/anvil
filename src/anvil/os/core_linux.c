#if OS_LINUX

#include "../base.h"
#include "../core.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#define GLAD_GL_IMPLEMENTATION
#include "../extern/glad.h"
#include <GL/glx.h>
#include <time.h>

//
// OS
//

// globals
global Atom _wm_delete_message;


// glx extensions
typedef int32_t (*PFNGLXSWAPINTERVALSGIPROC)(int32_t);
global PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalSGI;


// time
float64_t os_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (float64_t)ts.tv_sec + (float64_t)ts.tv_nsec / 1000000000.0;
}


// messaging
// @todo: implement this function to open a messagebox
global string_t message_labels[] = { "error", "question", "warning", "info" };
void os_message(os_message_icon_e icon, string_t format, ...) {
    fprintf(stderr, "%s: ", message_labels[icon]);    
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}


// window
struct os_window {
    Display *display;
    Window root;
    Window handle;
    GLXContext context;
};

os_window_o *os_window_create(const string_t title, uint16_t width, uint16_t height, int32_t x, int32_t y, uint32_t flags) {
    os_window_o *window = (os_window_o *)malloc(sizeof(os_window_o));
	
    window->display = XOpenDisplay(NULL);
    window->root = XDefaultRootWindow(window->display);
	
    XVisualInfo *visual = glXChooseVisual(window->display, 0, (int32_t[]){
											  GLX_RGBA,
											  GLX_DEPTH_SIZE, 24,
											  GLX_DOUBLEBUFFER, 0
										  });
	
    if (!visual) {
		os_message(OS_MESSAGE_ERROR, "Failed to choose visual");
		exit(EXIT_FAILURE);
    }
	
    window->context = glXCreateContext(window->display, visual, 0, True);
    if (!window->context) {
        fprintf(stderr, "Unable to create GL context\n");
        exit(EXIT_FAILURE);
    }
	
    window->handle = XCreateSimpleWindow( window->display, window->root, x, y, width, height, 0, 0, 0);
	
    glXMakeCurrent(window->display, window->handle, window->context);
    XStoreName(window->display, window->handle, title);
    XSelectInput(window->display, window->handle, KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | ExposureMask);
    XMapWindow(window->display, window->handle);
	
    if (!gladLoaderLoadGL()) {
		os_message(OS_MESSAGE_ERROR, "Failed to load OpenGL");
		exit(EXIT_FAILURE);
		return NULL;
	}
	
    _wm_delete_message = XInternAtom(window->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(window->display, window->handle, &_wm_delete_message, 1);

	// load glXSwapIntervalSGI
	glXSwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)glXGetProcAddressARB("glXSwapIntervalSGI");
	if (!glXSwapIntervalSGI) {
		os_message(OS_MESSAGE_ERROR, "Failed to load VSYNC function");
		exit(EXIT_FAILURE);
	}

	os_window_vsync(window, false);
	
    // @todo: window flags
    return window;
}

void os_window_delete(os_window_o *window) {
    glXDestroyContext(window->display, window->context);
    XCloseDisplay(window->display);
    free(window);
}

void os_window_swap_buffers(os_window_o *window) {
    glXSwapBuffers(window->display, window->handle);
}

void os_window_vsync(os_window_o *window, bool8_t enabled) {
	if (window) {
		glXSwapIntervalSGI(enabled);
	}
}


// event
internal void _os_event_process(os_window_o *window, os_event_t *event, XEvent *xev) {
    uint32_t vk_code = xev->xkey.keycode;
	
    switch (xev->type) {
        // quit
	case ClientMessage:
        if ((Atom)xev->xclient.data.l[0] == _wm_delete_message) {
            event->should_quit = true;
        }
        break;
		
		// resize/move
	case ConfigureNotify:
        event->x = xev->xconfigure.x;
        event->y = xev->xconfigure.y;
        event->width = xev->xconfigure.width;
        event->height = xev->xconfigure.height;
        break;
		
	case KeyPress:
        _curr_keyboard[vk_code] = true;
        break;
		
	case KeyRelease:
        _curr_keyboard[vk_code] = false;
        break;
		
		// mouse
	case MotionNotify:
        event->cursor.x = (float32_t)xev->xmotion.x / event->width  *  2 - 1.0f;
        event->cursor.y = (float32_t)xev->xmotion.y / event->height * -2 + 1.0f;
        break;

        // mouse button down
    case ButtonPress:
        switch (xev->xbutton.button) {
		case 1:
			_curr_mouse_buttons[MOUSE_BUTTON_LEFT] = true;
			break;

		case 2:
			_curr_mouse_buttons[MOUSE_BUTTON_MIDDLE] = true;
			break;

		case 3:
			_curr_mouse_buttons[MOUSE_BUTTON_RIGHT] = true;
			break;

		default:
			break;
        }
        break;
        
        // mouse button up
    case ButtonRelease:
        switch (xev->xbutton.button) {
		case 1:
			_curr_mouse_buttons[MOUSE_BUTTON_LEFT] = false;
			break;

		case 2:
		    _curr_mouse_buttons[MOUSE_BUTTON_MIDDLE] = false;
			break;

		case 3:
			_curr_mouse_buttons[MOUSE_BUTTON_RIGHT] = false;
			break;

		default:
			break;
        }
        break;
		
	default:
        break;
    }
}

void os_event_pull(os_window_o *window, os_event_t *event) {
    memcpy(_last_keyboard, _curr_keyboard, KEY_COUNT);
	
    XEvent xev = { 0 };
	while (XPending(window->display) > 0) {
		XNextEvent(window->display, &xev);
        _os_event_process(window, event, &xev);
	}
}

#endif
