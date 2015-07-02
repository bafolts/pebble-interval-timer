#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static TextLayer *text_layer_2;

static char* interval_text;
static char* timer_text;

static int interval_number = 1;
static int interval_size = 95;

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Down");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void update_timer_text() {
  int minutes = interval_size / 60;
  int seconds = interval_size - (minutes * 60);
  snprintf(timer_text, 6, "0%u:%u", minutes, seconds);
}

static void update_interval_text() {
  snprintf(interval_text, 11, "Interval %u", interval_number);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  interval_text = malloc(11);
  timer_text = malloc(6);
  
  update_timer_text();
  update_interval_text();
  
  text_layer = text_layer_create((GRect) { .origin = { 0, 52 }, .size = { bounds.size.w, 45 } });
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text(text_layer, timer_text);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  
  text_layer_2 = text_layer_create((GRect) { .origin = { 0, 0}, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer_2, interval_text);
  text_layer_set_text_alignment(text_layer_2, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  layer_add_child(window_layer, text_layer_get_layer(text_layer_2));

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