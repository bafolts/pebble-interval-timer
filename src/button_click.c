#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static TextLayer *text_layer_2;

static char* interval_text;
static char* timer_text;

const int MINUTES_KEY = 0;
const int SECONDS_KEY = 1;

static bool setting_minutes = false;
static bool setting_seconds = false;

static int selected_minutes = 0;
static int selected_seconds = 0;

static int interval_number = 1;
static int interval_size = 0;
static int our_interval;

static AppTimer* next_tick;

static int get_current_minutes() {
  return our_interval / 60;
}

static int get_current_seconds() {
  return our_interval - (get_current_minutes() * 60);
}

static void update_timer_text() {
  snprintf(timer_text, 6, "%02u:%02u", get_current_minutes(), get_current_seconds());
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
  if (interval_size > 0) {
    app_timer_cancel(next_tick);
    next_tick = app_timer_register(1000, timer_countdown_tick, NULL);
  }
}

static void reset_countdown() {
  app_timer_cancel(next_tick);
  interval_number = 1;
  our_interval = interval_size;
  update_timer_text();
  update_interval_text();
}

static void pause_countdown() {
  if (next_tick == NULL) {
    reset_countdown();
  } else {
    app_timer_cancel(next_tick);
    next_tick = NULL;
  }
}

static void seconds_window_selected(struct NumberWindow *number_window, void *context) {
  selected_seconds = number_window_get_value(number_window);
  window_destroy(window_stack_pop(false));
  window_destroy(window_stack_pop(false));
  setting_seconds = false;
  interval_size = (selected_minutes * 60) + selected_seconds;
  persist_write_int(MINUTES_KEY, selected_minutes);
  persist_write_int(SECONDS_KEY, selected_seconds);
  reset_countdown();
}

static void start_setting_seconds() {  
  NumberWindow* seconds_window = number_window_create("Set seconds", (NumberWindowCallbacks) {.selected = seconds_window_selected}, NULL);
  number_window_set_min(seconds_window, 0);
  number_window_set_max(seconds_window, 59);
  number_window_set_value(seconds_window, interval_size - ((interval_size / 60) * 60));
  setting_seconds = true;
  setting_minutes = false;
  window_stack_push(number_window_get_window(seconds_window), true);
}

static void minutes_window_selected(struct NumberWindow *number_window, void *context) {
  selected_minutes = number_window_get_value(number_window);
  start_setting_seconds();
  setting_minutes = false;
}

static void start_setting_minutes() {
  NumberWindow* minutes_window = number_window_create("Set minutes", (NumberWindowCallbacks) {.selected = minutes_window_selected}, NULL);
  number_window_set_min(minutes_window, 0);
  number_window_set_max(minutes_window, 59);
  number_window_set_value(minutes_window, interval_size / 60);
  setting_seconds = false;
  setting_minutes = true;
  window_stack_push(number_window_get_window(minutes_window), true);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (!setting_minutes && !setting_seconds) {
    reset_countdown();
    start_setting_minutes();
  } else {
    interval_size = our_interval;
    reset_countdown();
    text_layer_set_background_color(text_layer, GColorWhite);
    text_layer_set_text_color(text_layer, GColorBlack);
    setting_minutes = false;
    setting_seconds = false;
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (!setting_minutes && !setting_seconds) {
    start_countdown();
  } else {
    our_interval++;
    update_timer_text();
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (!setting_minutes && !setting_seconds) {
    pause_countdown();
  } else {
    our_interval--;
    update_timer_text();
  }
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

  interval_size = persist_read_int(SECONDS_KEY) + (60 * persist_read_int(MINUTES_KEY));

  our_interval = interval_size;

  init_text();

  init_interval_text_layer();
  init_timer_text_layer();

  update_timer_text();
  update_interval_text();
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  text_layer_destroy(text_layer_2);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
	  .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
}

static void deinit(void) {
  while (window_stack_get_top_window() != NULL) {
    window_destroy(window_stack_pop(false));
  }
  app_timer_cancel(next_tick);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
