/*#include "anvil/anvil.h"

global struct {
	mesh_t mesh_box;
} assets;

global texture_t t0, t1;
global mesh_t m, teapot, mesh_box;
global transform_t camera;
global framebuffer_t depth_fb;
global float64_t dt;

internal void render_scene(shader_t shader) {
	render_clear((vec3_t){ 0.1f, 0.1f, 0.1f });
	
	matrix_t xform = IDENTITY_MATRIX;
	
	xform = xform_scale(IDENTITY_MATRIX, (vec3_t){ 15.0f, 15.0f, 1.0f });
	xform = xform_rotate(xform, (vec3_t){ 1.0f, 0.0f, 0.0f }, -PI / 2);
	xform = xform_translate(xform, (vec3_t){ 0.0f, -0.75f, -5.0f });
	
	shader_uniform_matrix(shader, "xform", xform);
	texture_bind(&t1, 0);
	mesh_draw(&m);
	
	texture_bind(&t0, 0);
	
	xform = xform_translate(IDENTITY_MATRIX, (vec3_t){ 0.0f, 0.0f, -5.0f });
	shader_uniform_matrix(shader, "xform", xform);
	mesh_draw(&m);
	
	static int64_t n;
	++n;
	
	xform = xform_translate(xform_rotate(IDENTITY_MATRIX, (vec3_t){ 0, 1, 0 }, DEG_TO_RAD(n / 100)), (vec3_t){ 0.5f, 0.0f, -4.0f });
	shader_uniform_matrix(shader, "xform", xform);
	mesh_draw(&m);
	
	texture_bind(&t1, 0);
	shader_uniform_matrix(shader, "xform", IDENTITY_MATRIX);
	mesh_draw(&assets.mesh_box);
}

int32_t main(int32_t argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);
	
    os_window_o *window = os_window_create("anvil", 1280, 720, 0, 0, OS_WINDOW_CENTERED);
	os_event_t event = { 0 };
	os_window_vsync(window, false);
	
    render_init(&event);
    audio_init();
	ui_init();
	
	shader_t shader = shader_load("data/shaders/default.glsl");
	shader_t shadow_map_shader = shader_load("data/shaders/shadow_map.glsl");
	
	camera = (transform_t) {
		.pos   = (vec3_t){ 0.0f, 1.0f, 5.0f },
		.rot   = (vec3_t){ 0.0f, 0.0f, 0.0f },
		.scale = (vec3_t){ 0.0f, 0.0f, 0.0f }
	};
	
    m = mesh_create(4, 6);
	teapot = mesh_load("data/teapot.fbx");
	assets.mesh_box = mesh_load("data/meshes/box.glb");
	
	vertex_t v[4] = {
        { (vec3_t){ -.5f, 0.5f, 0.0f }, (vec2_t){ 0.0f, 1.0f }, (vec4_t){ 1.0f, 1.0f, 1.0f, 1.0f }, (vec3_t){ 0.0f, 0.0f, 1.0f } },
        { (vec3_t){ -.5f, -.5f, 0.0f }, (vec2_t){ 0.0f, 0.0f }, (vec4_t){ 1.0f, 1.0f, 1.0f, 1.0f }, (vec3_t){ 0.0f, 0.0f, 1.0f } },
        { (vec3_t){ 0.5f, -.5f, 0.0f }, (vec2_t){ 1.0f, 0.0f }, (vec4_t){ 1.0f, 1.0f, 1.0f, 1.0f }, (vec3_t){ 0.0f, 0.0f, 1.0f } },
        { (vec3_t){ 0.5f, 0.5f, 0.0f }, (vec2_t){ 1.0f, 1.0f }, (vec4_t){ 1.0f, 1.0f, 1.0f, 1.0f }, (vec3_t){ 0.0f, 0.0f, 1.0f } }
    };
	
    uint32_t i[6] = { 0, 1, 3, 1, 2, 3 };
	
    mesh_push_vertices(&m, (vertex_t *)v, 4);
    mesh_push_indices(&m, (uint32_t *)i, 6);
	
    t0 = texture_load("data/horse.png", ZERO_STRUCT(texture_params_t));
    t1 = texture_load("data/test.png", ZERO_STRUCT(texture_params_t));
	
    shader_uniform_texture(shader, "texture0", 0);
	
    render_state_set((render_state_t){ .blending = false, .depth_testing = true, .wireframe = false, .face_culling = true });
	
    framebuffer_t fb = framebuffer_create(1280, 720, ZERO_STRUCT(texture_params_t), FRAMEBUFFER_COLOR);
	depth_fb = framebuffer_create(10000, 10000, ZERO_STRUCT(texture_params_t), FRAMEBUFFER_DEPTH);
	
    while (!event.should_quit) {
        os_event_pull(window, &event);
        ui_event_push(&event);
		
		if (key_pressed(KEY_ESCAPE)) {
			event.should_quit = true;
		}
		
		
		{
			if (mouse_button_down(MOUSE_BUTTON_RIGHT)) {
				camera.rot.y += dt * 100.0f * event.cursor.x;
				camera.rot.x -= dt * 100.0f * event.cursor.y;
				CLAMP(camera.rot.x, -89.0f, 89.0f);
			}
			
			vec3_t dir = {
				cosf(DEG_TO_RAD(camera.rot.y + 90.0f)) * cosf(DEG_TO_RAD(camera.rot.x)),
				sinf(DEG_TO_RAD(camera.rot.x)),
				sinf(DEG_TO_RAD(camera.rot.y + 90.0f)) * cosf(DEG_TO_RAD(camera.rot.x))
			};
			
			vec3_t front = normalize3(dir);
			vec3_t right = normalize3(cross3(front, (vec3_t){ 0.0f, 1.0f, 0.0f }));
			
			camera.pos = sub3(camera.pos, mul3(vec3_scalar(dt * 10.0f * (key_down(KEY_W) - key_down(KEY_S))), front));
			camera.pos = sub3(camera.pos, mul3(vec3_scalar(dt * 10.0f * (key_down(KEY_D) - key_down(KEY_A))), right));
		}
		
		{
			static float64_t last_time;
			float64_t curr_time = os_time();
			if (last_time == 0.0f) {
				last_time = os_time();
			}
			
			dt = curr_time - last_time;
			last_time = curr_time;
		}
		
		{
			matrix_t projection, view, view_light;
			
			framebuffer_bind(&depth_fb);
			{
				view_light = matrix_mul(xform_lookat((vec3_t){ -1.0f, 2.0f, 5.0f }, ZERO_STRUCT(vec3_t), (vec3_t){ 0.0f, 1.0f, 0.0f }),
										matrix_projection_ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 20.5f));
				
				shader_bind(shadow_map_shader);
				shader_uniform_matrix(shadow_map_shader, "view_light", view_light);
				
				render_scene(shadow_map_shader); 
			}
			framebuffer_unbind();
			
			// Render
			projection = matrix_projection_perspective(60.0f, 1.7f, 0.1f, 1000.0f);
			view = xform_camera(camera.pos, camera.rot);
			
			shader_bind(shader);
			texture_bind(&depth_fb.texture, 1);
			shader_uniform_texture(shader, "shadow_map", 1);
			shader_uniform_matrix(shader, "projection", projection);
			shader_uniform_matrix(shader, "view", view);
			shader_uniform_matrix(shader, "view_light", view_light);
			shader_uniform_vec3(shader, "view_pos", camera.pos);
			
			render_scene(shader);
			
			// ui
			static float32_t v = 5;
			ui_text("Text", (vec2_t){ 0.0f, 50.0f }, 1.0f, UI_ANCHOR_CENTER);
			ui_button("Button", (vec2_t){ 0.0f, 0.0f }, (vec2_t){ 200.0f, 20.0f }, UI_ANCHOR_CENTER, UI_ANCHOR_CENTER);
			ui_slider("Slider", (vec2_t){ 0.0f, -50.0f }, (vec2_t){ 200.0f, 20.0f }, &v, 0.0f, 10.0f, UI_ANCHOR_CENTER, UI_ANCHOR_CENTER);
		}
		
		os_window_swap_buffers(window);
	}
	
	os_window_delete(window);
	
	ui_close();
	audio_close();
	render_close();
	
	return EXIT_SUCCESS;
}
*/
#include "anvil/anvil.h"
#include "main.h"

//
// globals
//

global os_event_t event;

global struct {
	texture_t tex_white;
	mesh_t mesh_box;
} assets;

global enum { SCENE_MENU, SCENE_GAME } curr_scene;

//
// main
//

int32_t main(int32_t argc, string_t *argv) {
	UNUSED(argc);
    UNUSED(argv);

	// init anvil
    os_window_o *window = os_window_create("anvil", 1280, 720, 0, 0, OS_WINDOW_CENTERED);
	os_window_vsync(window, true);
	
    render_init(&event);
    audio_init();
	ui_init();

	// init game
	assets_load();		

	// mainloop
	while (!event.should_quit) {
        os_event_pull(window, &event);
        ui_event_push(&event);
		
		if (key_pressed(KEY_ESCAPE)) {
			event.should_quit = true;
		}

		// game loop
		switch (curr_scene) {
		default:
		case SCENE_MENU: {
		    menu_update();
			break;
		}

		case SCENE_GAME: {
			
			break;
		}
		}
		
		os_window_swap_buffers(window);
	}

	// cleanup
	ui_close();
	audio_close();
	render_close();	

	os_window_delete(window);	
	return EXIT_SUCCESS;
}


//
// assets
//

void assets_load() {
	// textures
	uint32_t color = 0xFFFFFFFF;
	assets.tex_white = texture_create((uint8_t *)&color, 1, 1, 4, ZERO_STRUCT(texture_params_t));
	
	// meshes
	assets.mesh_box = mesh_load("data/meshes/box.glb");

	// binding
	texture_bind(&assets.tex_white, 0);
}


//
// menu
//

void menu_update() {
	render_clear((vec3_t){ 0.4f, 0.4f, 0.5f });

	ui_text("Menu", (vec2_t){ 0.0f, 150.0f }, 2.0f, UI_ANCHOR_CENTER);

	if (ui_button("Play", (vec2_t){ 0.0f, -50.0f}, (vec2_t){ 200.0f, 20.0f}, UI_ANCHOR_CENTER, UI_ANCHOR_CENTER)) {
		curr_scene = SCENE_GAME;
	}

	if (ui_button("Exit", (vec2_t){ 0.0f, -100.0f}, (vec2_t){ 200.0f, 20.0f}, UI_ANCHOR_CENTER, UI_ANCHOR_CENTER)) {
		event.should_quit = true;
	}
}
