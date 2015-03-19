#include <pebble.h>

static Window *window;
TextLayer *text_date_layer;
TextLayer *text_time_layer;
TextLayer *text_temp_layer;
TextLayer *text_batt_layer;
GFont *font12;
GFont *font16;

#define NUMBER_OF_IMAGES 11
static GBitmap *image = NULL;
static BitmapLayer *image_layer;

#define TOTAL_TIME_DIGITS 4
static GBitmap *s_time_digits[TOTAL_TIME_DIGITS];
static BitmapLayer *s_time_digits_layers[TOTAL_TIME_DIGITS];

const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_CLEAR_DAY,
  RESOURCE_ID_CLEAR_NIGHT,
  RESOURCE_ID_CLOUDY,
  RESOURCE_ID_FOG,
  RESOURCE_ID_PARTLY_CLOUDY_DAY,
  RESOURCE_ID_PARTLY_CLOUDY_NIGHT,
  RESOURCE_ID_RAIN,
  RESOURCE_ID_SLEET,
  RESOURCE_ID_SNOW,
  RESOURCE_ID_WIND,
  RESOURCE_ID_ERROR
};

static const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
    RESOURCE_ID_NUM_0,
    RESOURCE_ID_NUM_1,
    RESOURCE_ID_NUM_2,
    RESOURCE_ID_NUM_3,
    RESOURCE_ID_NUM_4,
    RESOURCE_ID_NUM_5,
    RESOURCE_ID_NUM_6,
    RESOURCE_ID_NUM_7,
    RESOURCE_ID_NUM_8,
    RESOURCE_ID_NUM_9
};

enum {
  WEATHER_TEMPERATURE_F,
  WEATHER_TEMPERATURE_C,
  WEATHER_ICON,
  WEATHER_ERROR,
  LOCATION
};

int FtoC(int f) {
  return (f - 32) * 5 / 9;
}

void in_received_handler(DictionaryIterator *received, void *context) {
  // incoming message received
  Tuple *temperature = dict_find(received,WEATHER_TEMPERATURE_C);
  Tuple *icon = dict_find(received, WEATHER_ICON);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Loop index now %s", temperature->value->cstring);
  if (temperature) {
    text_layer_set_text(text_temp_layer, strcat(temperature->value->cstring, "º"));
  }

  if (icon) {
    // figure out which resource to use
    int8_t id = icon->value->int8;
    if (image != NULL) {
      gbitmap_destroy(image);
      layer_remove_from_parent(bitmap_layer_get_layer(image_layer));
      bitmap_layer_destroy(image_layer);
    }

    Layer *window_layer = window_get_root_layer(window);

    image = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[id]);
    image_layer = bitmap_layer_create(GRect(10, 100, 60, 60));
    bitmap_layer_set_bitmap(image_layer, image);
    layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));
  }
}

void handle_battery(BatteryChargeState charge) {
    static char buf[] = "123";
    snprintf(buf, sizeof(buf), "%d", charge.charge_percent);
    text_layer_set_text(text_batt_layer, strcat(buf, "%"));
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  font12 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MYRIAD_PRO_REGULAR_12));
  font16 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MYRIAD_PRO_REGULAR_16));


  // create date layer - this is where the date goes
  text_date_layer = text_layer_create(GRect(144-32, 6, 32, 10));
  text_layer_set_text_alignment(text_date_layer, GTextAlignmentRight);
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_font(text_date_layer, font16);
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));
    
  // create batery layer
  text_batt_layer = text_layer_create(GRect(168-6, 120, 20, 10));
  text_layer_set_text_alignment(text_batt_layer, GTextAlignmentLeft);
  text_layer_set_text_color(text_batt_layer, GColorWhite);
  text_layer_set_background_color(text_batt_layer, GColorClear);
  text_layer_set_font(text_batt_layer, font12);
  layer_add_child(window_layer, text_layer_get_layer(text_batt_layer));

  // create temperature layer - this is where the temperature goes
  text_temp_layer = text_layer_create(GRect(80, 108, 144-80, 168-108));
  text_layer_set_text_color(text_temp_layer, GColorWhite);
  text_layer_set_background_color(text_temp_layer, GColorClear);
  text_layer_set_font(text_temp_layer, font16);
  layer_add_child(window_layer, text_layer_get_layer(text_temp_layer));
    
  // Create time and date layers
  GRect dummy_frame = GRect(0, 0, 0, 0);
  for (int i = 0; i < 4; ++i) {
    s_time_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_time_digits_layers[i]));
  }
    
  handle_battery(battery_state_service_peek());
  battery_state_service_subscribe(&handle_battery);
}

static void window_unload(Window *window) {
  // destroy the text layers - this is good
  text_layer_destroy(text_date_layer);
  text_layer_destroy(text_time_layer);
  text_layer_destroy(text_temp_layer);
  text_layer_destroy(text_batt_layer);

  // destroy the image layers
  gbitmap_destroy(image);
  layer_remove_from_parent(bitmap_layer_get_layer(image_layer));
  bitmap_layer_destroy(image_layer);

  // unload the fonts
  fonts_unload_custom_font(font12);
  fonts_unload_custom_font(font16);
    
  battery_state_service_unsubscribe();
}

static void app_message_init(void) {
  app_message_open(64, 16);
  app_message_register_inbox_received(in_received_handler);
}

static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;

  *bmp_image = gbitmap_create_with_resource(resource_id);
#ifdef PBL_PLATFORM_BASALT
  GRect bitmap_bounds = gbitmap_get_bounds((*bmp_image));
#else
  GRect bitmap_bounds = (*bmp_image)->bounds;
#endif
  GRect frame = GRect(origin.x, origin.y, bitmap_bounds.size.w, bitmap_bounds.size.h);
  bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
  layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);

  if (old_image != NULL) {
  	gbitmap_destroy(old_image);
  }
}

static unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }

  unsigned short display_hour = hour % 12;
  return display_hour ? display_hour : 12;
}
// show the date and time every minute
void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  static char date_text[] = "Xxxxxxxxx 00";
  strftime(date_text, sizeof(date_text), "%B %e", tick_time);
  text_layer_set_text(text_date_layer, date_text);

  unsigned short display_hour = get_display_hour(tick_time->tm_hour);
  set_container_image(&s_time_digits[0], s_time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour / 10], GPoint(2, 2));
  set_container_image(&s_time_digits[1], s_time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour % 10], GPoint(54, 2));

  set_container_image(&s_time_digits[2], s_time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min / 10], GPoint(2, 74));
  set_container_image(&s_time_digits[3], s_time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min % 10], GPoint(54, 74));

  if (!clock_is_24h_style()) {
    if (display_hour / 10 == 0) {
    	layer_set_hidden(bitmap_layer_get_layer(s_time_digits_layers[0]), true);
    } else {
    	layer_set_hidden(bitmap_layer_get_layer(s_time_digits_layers[0]), false);
    }
  }
}

static void init(void) {
  window = window_create();
  app_message_init();

  window_set_background_color(window, GColorBlack);

  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

  // subscribe to update every minute
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
    
  
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  app_message_deregister_callbacks();

  window_stack_remove(window, true);

  window_destroy(window);
}

int main(void) {
  init();

  app_event_loop();
  deinit();
}
