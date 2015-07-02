#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static TextLayer *text_layer_2;

static char* interval_text;
static char* timer_text;

static int interval_number = 1;
static int interval_size = 15 + (60 * 4);
static int our_interval;

static AppTimer* next_tick;

static void update_timer_text() {
  int minutes = our_interval / 60;
  int seconds = our_interval - (minutes * 60);
  snprintf(timer_text, 6, "%02u:%02u", minutes, seconds);
  text_layer_set_text(text_layer, timer_text);
}

static void update_interval_text() {
  snprintf(interval_text, 12, "Interval %u", interval_number);
  text_layer_set_text(text_layer_2, interval_text);
}

static void timer_countdown_tick(void* data) {
  our_interval--;
  update_timer_text();
  if(our_interval == 0) {
    vibes_double_pulse();
    app_timer_cancel(next_tick);
    interval_number++;
    our_interval = interval_size;
    update_interval_text();
  }
  update_timer_text();
  next_tick = app_timer_register(1000, timer_countdown_tick, NULL);
}

static void start_countdown() {
  app_timer_cancel(next_tick);
  next_tick = app_timer_register(1000, timer_countdown_tick, NULL);
}

static void pause_countdown() {
  app_timer_cancel(next_tick);
}

static void reset_countdown() {
  app_timer_cancel(next_tick);
  interval_number = 1;
  our_interval = interval_size;
  update_timer_text();
  update_interval_text();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  reset_countdown();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  start_countdown();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  pause_countdown();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void init_text() {
  free(interval_text);
  free(timer_text);
  interval_text = malloc(12);
  timer_text = malloc(6);
}

static void init_interval_text_layer() {
  text_layer = text_layer_create((GRect) { .origin = { 0, 52 }, .size = { layer_get_bounds(window_get_root_layer(window)).size.w, 45 } });
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer));
}

static void init_timer_text_layer() {
  text_layer_2 = text_layer_create((GRect) { .origin = { 0, 0}, .size = { layer_get_bounds(window_get_root_layer(window)).size.w, 20 } });
  text_layer_set_text_alignment(text_layer_2, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer_2));
}

static void window_load(Window *window) {

  our_interval = interval_size;

  init_text();

  init_interval_text_layer();
  init_timer_text_layer();

  update_timer_text();
  update_interval_text();
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
	.load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}