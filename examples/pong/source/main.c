
/*================================================================
	* Copyright: 2020 John Jackson
	* Pong

	* Simple pong game, using immediate mode rendering
	* Sounds from: https://freesound.org/people/NoiseCollector/

	// TODO(john): Post processing (scan-lines to give off "crtv effect", possible screen bending)
	Reference: https://clemz.io/article-retro-shaders-webgl.html

	Press `esc` to exit the application.
=================================================================*/

#include <gs.h>

/*=======================
// Constants and Defines
========================
*/
#define paddle_width	20.f
#define paddle_height 	80.f
#define paddle_speed 	5.f
#define ball_speed 		4.f
#define ball_width		10.f
#define ball_height		10.f
#define game_field_y	10.f
#define game_field_x	10.f

#define paddle_dims 	gs_v2(paddle_width, paddle_height)
#define ball_dims 		gs_v2(ball_width, ball_height)

#define window_size()\
	((gs_engine_subsystem(platform))->window_size((gs_engine_subsystem(platform))->main_window()))

/*=========
// Paddles
==========*/

typedef enum paddle_side {
	paddle_left, 
	paddle_right,
	paddle_side_count
} paddle_side;

typedef struct paddle_t {
	gs_vec2 position;
} paddle_t;

/*=========
// Ball
==========*/

typedef struct ball_t {
	gs_vec2 position;
	gs_vec2 velocity;
	f32 speed_modifier;
} ball_t;

/*============
// Game Data 
============*/

typedef struct game_data_t {
	gs_command_buffer_t cb;
	paddle_t paddles[paddle_side_count];
	ball_t ball;
	u32 score[paddle_side_count];
	gs_font_t font;
	gs_audio_source_t* ball_hit_src;
	gs_audio_source_t* score_src;
	b32 hit;
} game_data_t;

// Forward Decls.
gs_result app_update();
gs_result app_init();
gs_rect_t get_paddle_rect(paddle_t paddle);
gs_rect_t get_ball_rect(ball_t ball);
gs_rect_t get_field_dims();
void init_ball(ball_t* ball);
void update_paddles();
void update_ball();
void play_ball_hit_sound();
void play_score_sound();
void draw_game();

int main(int argc, char** argv)
{
	game_data_t gd 				= {0};
	gs_application_desc_t app 	= {0};
	app.window_title 			= "Pong";
	app.window_width 			= 800;
	app.window_height 			= 600;
	app.init 					= &app_init;
	app.update 					= &app_update;
	app.user_data 				= &gd;

	gs_result res = gs_engine_construct(app)->run();

	if (res != gs_result_success) {
		gs_println("Error: Engine did not successfully finish running.");
		return -1;
	}
	gs_println("Gunslinger exited successfully.");

	return 0;	
}

gs_result app_init()
{
	gs_platform_i* platform = gs_engine_subsystem(platform);
	gs_graphics_i* gfx = gs_engine_subsystem(graphics);
	gs_audio_i* audio = gs_engine_subsystem(audio);

	// Initialize game data
	game_data_t* gd = gs_engine_user_data(game_data_t);

	// Initialize paddles
	gs_vec2 pd = paddle_dims;
	gs_vec2 ws = window_size();
	gd->paddles[paddle_left].position 	= gs_v2(pd.x * 2.f, 300.f);
	gd->paddles[paddle_right].position 	= gs_v2(ws.x - 3.f * pd.x, 300.f);

	// Initialize ball
	init_ball(&gd->ball);

	// Construct command buffer for rendering
	gd->cb = gs_command_buffer_new();

	// Init font
	gd->font = gfx->construct_font_from_file("./assets/bit9x9.ttf", 48.f);

	// Init audio
	gd->ball_hit_src = audio->load_audio_source_from_file("./assets/ball_hit.wav");
	gd->score_src 	 = audio->load_audio_source_from_file("./assets/score.wav");

	return gs_result_success;
}

gs_result app_update()
{
	gs_platform_i* platform = gs_engine_subsystem(platform);

	if (platform->key_pressed(gs_keycode_esc)) {
		return gs_result_success;
	}

	// Update game
	update_paddles();
	update_ball();
	draw_game();
	
	// Otherwise, continue
	return gs_result_in_progress;
}

void draw_game()
{
	// Cache necessary apis and game data
	gs_graphics_i* gfx = gs_engine_subsystem(graphics);
	game_data_t* gd = gs_engine_user_data(game_data_t);
	gs_graphics_immediate_draw_i* id = &gfx->immediate;
	gs_command_buffer_t* cb = &gd->cb;
	const gs_vec2 ws = window_size();

	id->begin_drawing(cb);
	{
		// If we hit last frame, then flash the screen briefly with a lighter clear color
		if (gd->hit) {
			gd->hit = false;
			id->clear(cb, 0.2f, 0.2f, 0.2f, 1.f);
		}
		else {
			id->clear(cb, 0.1f, 0.1f, 0.1f, 1.f);
		}

		// Game field
		const f32 rect_x = 10.f;
		const f32 rect_y = 10.f;
		id->draw_rect_lines(cb, rect_x, rect_y, ws.x - rect_x, ws.y - rect_y, gs_color_white);

		// Dividing line (series of rects from mid screen)
		const f32 y_offset = 5.f;
		gs_vec2 div_dim = gs_v2(5.f, 10.f);
		s32 num_steps = (ws.y - rect_y * 2.f) / (div_dim.y + y_offset);
		for (u32 i = 0; i <= num_steps; ++i)
		{
			gs_vec2 a = gs_v2((ws.x - div_dim.x) * 0.5f, rect_y + i * (div_dim.y + y_offset));
			gs_vec2 b = gs_v2(a.x + div_dim.x, a.y + div_dim.y);
			id->draw_rectv(cb, a, b, gs_color_alpha(gs_color_white, 100));
		}

		// Paddles
		gs_for_range_i(paddle_side_count)
		{
			gs_vec2 a = gd->paddles[i].position;
			gs_vec2 b = gs_v2(a.x + paddle_width, a.y + paddle_height);
			id->draw_rectv(cb, a, b, gs_color_white);
		}

		// Ball
		{
			gs_vec2 a = gd->ball.position;
			gs_vec2 b = gs_v2(a.x + ball_width, a.y + ball_height);
			id->draw_rectv(cb, a, b, gs_color_white);
		}

		// Title
		gs_vec2 td = gfx->text_dimensions("Pong", &gd->font);
		id->draw_text_ext(cb, (ws.x - td.x) * 0.5f, 75.f, "Pong", &gd->font, gs_color_white);

		// Scores
		gs_for_range_i(paddle_side_count) 
		{
			gs_snprintfc(score_buf, 256, "%zu", gd->score[i]);
			td = gfx->text_dimensions(score_buf, &gd->font);
			id->draw_text_ext(cb, (ws.x - td.x) * 0.5f - 75.f + 150.f * i, 150.f, score_buf, &gd->font, gs_color_white);
		}
	}
	id->end_drawing(cb);

	// Final submit of command buffer for drawing to screen
	gfx->submit_command_buffer(cb);
}

gs_rect_t get_paddle_rect(paddle_t paddle)
{
	gs_rect_t rect = {0};
	rect.min = paddle.position;
	rect.max = gs_vec2_add(rect.min, paddle_dims);
	return rect;
}

gs_rect_t get_ball_rect(ball_t ball)
{
	gs_rect_t rect = {0};
	rect.min = ball.position;
	rect.max = gs_vec2_add(rect.min, ball_dims);
	return rect;
}

gs_rect_t get_field_rect()
{
	gs_vec2 ws = window_size();
	gs_rect_t rect = {0};
	rect.min = gs_v2(game_field_x, game_field_y);
	rect.max = gs_vec2_sub(gs_v2(ws.x, ws.y), rect.min);
	return rect;
}

void init_ball(ball_t* ball)
{
	gs_vec2 ws = window_size();
	ball->position = gs_v2((ws.x - ball_width) * 0.5f, (ws.y - ball_height) * 0.5f);
	ball->velocity = gs_v2(-1.f, -1.f);
	ball->speed_modifier = 1.f;
}

void update_paddles()
{
	gs_platform_i* platform = gs_engine_subsystem(platform);
	gs_vec2 ws = window_size();
	game_data_t* gd = gs_engine_user_data(game_data_t);

	// Left paddle movement
	f32* y = &gd->paddles[paddle_left].position.y;
	if (platform->key_down(gs_keycode_w)) {
		*y = gs_clamp(*y - paddle_speed, game_field_y, ws.y - paddle_height - game_field_y);
	}
	if (platform->key_down(gs_keycode_s)) {
		*y = gs_clamp(*y + paddle_speed, game_field_y, ws.y - paddle_height - game_field_y);
	}

	// Right paddle movement
	y = &gd->paddles[paddle_right].position.y;
	if (platform->key_down(gs_keycode_up)) {
		*y = gs_clamp(*y - paddle_speed, game_field_y, ws.y - paddle_height - game_field_y);
	}
	if (platform->key_down(gs_keycode_down)) {
		*y = gs_clamp(*y + paddle_speed, game_field_y, ws.y - paddle_height - game_field_y);
	}
}

void update_ball()
{
	gs_vec2 ws = window_size();
	game_data_t* gd = gs_engine_user_data(game_data_t);

	// Ball moves based on its velocity
	gd->ball.position.x += gd->ball.velocity.x * ball_speed * gd->ball.speed_modifier;
	gd->ball.position.y += gd->ball.velocity.y * ball_speed * gd->ball.speed_modifier;

	b32 need_pos_reset = false;
	b32 need_ball_reset = false;

	// Check bottom wall
	if (gd->ball.position.y > ws.y - game_field_y - ball_height) {
		gd->ball.velocity.y *= -1.f;
		need_pos_reset = true;
	}

	// Check top wall
	if (gd->ball.position.y < game_field_y) {
		gd->ball.velocity.y *= -1.f;
		need_pos_reset = true;
	}

	// Check right wall
	if (gd->ball.position.x > ws.x - game_field_x - ball_width) {
		gd->score[paddle_left] += 1;
		need_ball_reset = true;
	}

	// Check left wall
	if (gd->ball.position.x < game_field_x) {
		gd->score[paddle_right] += 1;
		need_ball_reset = true;
	}

	// Check for collision against paddles
	if (
		gs_rect_vs_rect(get_paddle_rect(gd->paddles[paddle_left]), get_ball_rect(gd->ball)) || 
		gs_rect_vs_rect(get_paddle_rect(gd->paddles[paddle_right]), get_ball_rect(gd->ball))
	)
	{
		// Increase speed modifier
		gd->ball.velocity.x *= -1.f;
		gd->ball.speed_modifier *= 1.1f;
		need_pos_reset = true;
	}

	if (need_pos_reset) {
		gd->ball.position.y += gd->ball.velocity.y * ball_speed * gd->ball.speed_modifier;
		gd->ball.position.x += gd->ball.velocity.x * ball_speed * gd->ball.speed_modifier;
		play_ball_hit_sound();
	}

	if (need_ball_reset) {
		play_score_sound();
		init_ball(&gd->ball);
		gd->hit = true;
	}
}

void play_ball_hit_sound()
{
	gs_audio_i* audio = gs_engine_subsystem(audio);
	game_data_t* gd = gs_engine_user_data(game_data_t);
	audio->play_source(gd->ball_hit_src, 0.5f);
}

void play_score_sound()
{
	gs_audio_i* audio = gs_engine_subsystem(audio);
	game_data_t* gd = gs_engine_user_data(game_data_t);
	audio->play_source(gd->score_src, 0.5f);
}





