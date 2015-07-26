#include <pebble.h>
  
static Window *my_window;

static Layer *scale_layer;

static GBitmap *hand_his;
static GPoint hand_his_rotation_center;
static float hand_his_correction;

static GBitmap *hand_her;
static GPoint hand_her_rotation_center;
static float hand_her_correction;

static GBitmap *batch;

static Layer *hand_layer;

static void setup_bitmaps() {
  hand_his = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HER_ARM);
  hand_his_rotation_center = GPoint(10, 50);
  hand_his_correction = 8;

  hand_her = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HER_ARM);
  hand_her_rotation_center = GPoint(0, 50);
  hand_her_correction = 1.8f;
  
  batch = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATCH);
}

static void update_hands(Layer *layer, GContext *context) {
    // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  GRect bounds = layer_get_bounds(layer);
  GPoint layer_center_point = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  
  graphics_context_set_antialiased(context, true);
  graphics_context_set_compositing_mode(context, GCompOpSet);

  int his_angle = (tick_time->tm_min - hand_his_correction) / 60.0f * TRIG_MAX_ANGLE;  
  graphics_draw_rotated_bitmap(context, hand_his, hand_his_rotation_center, his_angle, layer_center_point);

  float hours = tick_time->tm_hour + tick_time->tm_min / 60.0f;
  int her_angle = (hours - hand_her_correction) / 12.0f * TRIG_MAX_ANGLE;
  graphics_draw_rotated_bitmap(context, hand_her, hand_her_rotation_center, her_angle, layer_center_point);
  
  GRect batch_rect = GRect(layer_center_point.x - 14, layer_center_point.y - 15, 30, 30);
  graphics_draw_bitmap_in_rect(context, batch, batch_rect);
}

static void update_scale(Layer *layer, GContext *context) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = GPoint(bounds.size.w / 2, bounds.size.h / 2);

  graphics_context_set_stroke_color(context, GColorWhite);
  
  GPoint start;
  GPoint end;
  
  for( int i = 0; i < 12; ++i) {
    int32_t current_angle = TRIG_MAX_ANGLE * i / 12;
    
    // create an oval shape indicator start point
    start.x = (sin_lookup(current_angle) * 65 / TRIG_MAX_RATIO) + center.x;
    start.y = (-cos_lookup(current_angle) * 75 / TRIG_MAX_RATIO) + center.y;
    end.x = (sin_lookup(current_angle) * 100 / TRIG_MAX_RATIO) + center.x;
    end.y = (-cos_lookup(current_angle) * 100 / TRIG_MAX_RATIO) + center.y;

    if (i % 3  == 0) {
      // reached a quarter, draw bigger indicator
      graphics_context_set_stroke_width(context, 6);
    } else {
      graphics_context_set_stroke_width(context, 3);
    }

    graphics_draw_line(context, start, end);          
  }
}

static void main_window_load(Window *window) {
  setup_bitmaps();
  
  window_set_background_color(window, GColorBlack);
  
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  
  scale_layer = layer_create(GRect(0,0, bounds.size.w, bounds.size.h));
  layer_set_update_proc(scale_layer, update_scale);
  layer_add_child(root, scale_layer);
  
  hand_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  layer_set_update_proc(hand_layer, update_hands);

  layer_add_child(root, hand_layer);
}

static void main_window_unload(Window *window) {
  gbitmap_destroy(hand_his);
  layer_destroy(hand_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(hand_layer);  
}

void handle_init(void) {
  my_window = window_create();
  
  window_set_window_handlers(my_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(my_window, true);
  
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

void handle_deinit(void) {
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
