#include <pebble.h>
    
static int CURRENT_TIME_HOUR = -1;
static int CURRENT_TIME_MINUTE = -1;
static int CURRENT_TIME_SECOND = -1;// when number window is toggled, every number is set to 0

// Hour bars physical parameters
#define BAR_DENSITY 0.29
#define ACCEL_RATIO 0.007
#define ACCEL_STEP_MS 50
static int DIM_RANGE[12] = {7, 7, 7, 7, 7, 7, 7, 7, 8, 9, 9, 7};

// Hour bars struct design
typedef struct Vec2d {
  double  x;
  double  y;
} Vec2d;

typedef struct Vec4d {
  double  top;
  double  down;
  double  left;
  double  right;
} Vec4d;

typedef struct hour_bar {
  Vec2d pos; // left-top corner
  Vec2d vel; // velocity
  double  mass;
  double  dim; // dimension of the bar
  Vec4d outer_square; // the outer lines to detect collision
} hour_bar;

static hour_bar hour_bars[12];

static AppTimer *timer;
static Window *s_main_window;
static Layer *s_backgroundPots_layer;
static Layer *s_canvas_layer;
static InverterLayer *s_inverterlayers[12];

// Helper function to get absolute value
static double absolut(double input){
    if (input < 0){
        input = -input;
    }
    return input;
}

static void get_current_time() {
    // Get a tm structure
    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);
    struct tm current_time = *tick_time;

    CURRENT_TIME_MINUTE = current_time.tm_min;
    CURRENT_TIME_SECOND = current_time.tm_sec;
    CURRENT_TIME_HOUR = current_time.tm_hour;
    
    // change hour range to 1-12 type
    if(clock_is_24h_style() == true) {
        //Use 24 hour format
        CURRENT_TIME_HOUR = CURRENT_TIME_HOUR % 12;
    }
    if (CURRENT_TIME_HOUR == 0){
        CURRENT_TIME_HOUR = 12;
    }
}

static void backgroundPots_update_proc(Layer *this_layer, GContext *ctx) {
    int NUMBER_SECONDS = 60;
    GRect bounds = layer_get_bounds(this_layer);
    GPoint backgroundPots[NUMBER_SECONDS];
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    
    // Write a 10*6 small points matrix and draw them!
    int dx = bounds.size.w / 6 / 2;
    int dy = bounds.size.h / 10 / 2;
    int offset = 3;
    for (int i=0; i<10; i++) {
        for (int j=0;j<6;j++) {
            backgroundPots[i*6+j].x = 2 * dx * j + dx;
            backgroundPots[i*6+j].y = 2 * dy * i + dy + offset;
            graphics_fill_circle(ctx, backgroundPots[i*6+j], 1);
        }
    }    
}

static int turn_sub_to_pos_x(int sub){    
    int dx = 144 / 6 / 2;
    int dy = 168 / 10 / 2;
    int offset = 3;
    int row = sub%10-1;
    int col = (sub - sub%10)/10;
    if (row == -1){
        row = 9;  
        col--;
    }
    
    int x_in_pixel = 2 * dx * col + dx;
    return x_in_pixel;
}

static int turn_sub_to_pos_y(int sub){
    int dx = 144 / 6 / 2;
    int dy = 168 / 10 / 2;
    int offset = 3;
    int row = sub%10-1;
    int col = (sub - sub%10)/10;
    if (row == -1){
        row = 9;  
        col--;
    }
    
    int y_in_pixel = 2 * dy * row + dy + offset; 
    return y_in_pixel;
}

static void canvas_update_proc(Layer *this_layer, GContext *ctx) {
    // GRect bounds = layer_get_bounds(this_layer); // Obviously I use magic numbers to represent bounds
    GPoint secondPoint;
    get_current_time();
    graphics_context_set_fill_color(ctx, GColorBlack);
    
    int minute_value = CURRENT_TIME_MINUTE;
    int second_value = CURRENT_TIME_SECOND;
    
    // Draw the current second
    secondPoint.x = turn_sub_to_pos_x(second_value);
    secondPoint.y = turn_sub_to_pos_y(second_value);
    graphics_fill_circle(ctx, secondPoint, 7);
    
    // Draw the current minute
    int side_length = 7;
    GPoint p0 = GPoint(turn_sub_to_pos_x(minute_value)-side_length, turn_sub_to_pos_y(minute_value)-side_length);
    GPoint p1 = GPoint(turn_sub_to_pos_x(minute_value)+side_length, turn_sub_to_pos_y(minute_value)+side_length);
    GPoint p2 = GPoint(turn_sub_to_pos_x(minute_value)+side_length, turn_sub_to_pos_y(minute_value)-side_length);
    GPoint p3 = GPoint(turn_sub_to_pos_x(minute_value)-side_length, turn_sub_to_pos_y(minute_value)+side_length);
    GPoint p4 = GPoint(turn_sub_to_pos_x(minute_value)-side_length, turn_sub_to_pos_y(minute_value)-side_length-1);
    GPoint p5 = GPoint(turn_sub_to_pos_x(minute_value)+side_length, turn_sub_to_pos_y(minute_value)+side_length-1);
    GPoint p6 = GPoint(turn_sub_to_pos_x(minute_value)+side_length, turn_sub_to_pos_y(minute_value)-side_length-1);
    GPoint p7 = GPoint(turn_sub_to_pos_x(minute_value)-side_length, turn_sub_to_pos_y(minute_value)+side_length-1);
    GPoint p8 = GPoint(turn_sub_to_pos_x(minute_value)-side_length, turn_sub_to_pos_y(minute_value)-side_length+1);
    GPoint p9 = GPoint(turn_sub_to_pos_x(minute_value)+side_length, turn_sub_to_pos_y(minute_value)+side_length+1);
    GPoint p10 = GPoint(turn_sub_to_pos_x(minute_value)+side_length, turn_sub_to_pos_y(minute_value)-side_length+1);
    GPoint p11 = GPoint(turn_sub_to_pos_x(minute_value)-side_length, turn_sub_to_pos_y(minute_value)+side_length+1);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_line(ctx, p0, p1);
    graphics_draw_line(ctx, p2, p3);
    graphics_draw_line(ctx, p4, p5);
    graphics_draw_line(ctx, p6, p7);
    graphics_draw_line(ctx, p8, p9);
    graphics_draw_line(ctx, p10, p11);
    
//     graphics_context_set_fill_color(ctx, GColorBlack);
//     int up, left, side_length;
//     side_length = 10;
//     up = turn_sub_to_pos_y(minute_value) - side_length/2;
//     // down = up + side_length;
//     left = turn_sub_to_pos_x(minute_value) - side_length/2;
//     // right = left + side_length;
//     graphics_fill_rect(ctx, GRect(left, up, side_length, side_length), 0, GCornerNone);
}

static int update_collision(hour_bar bar1, hour_bar bar2){
    int result = 0;
    if ((bar2.outer_square.top <= bar1.outer_square.down) && (bar1.outer_square.top <= bar2.outer_square.down) && 
            (bar2.outer_square.left <= bar1.outer_square.left) && (bar1.outer_square.left <= bar2.outer_square.right)) {
        result += 1; // left-side inside
    }
    if ((bar2.outer_square.top <= bar1.outer_square.down) && (bar1.outer_square.top <= bar2.outer_square.down) && 
            (bar2.outer_square.left <= bar1.outer_square.right) && (bar1.outer_square.right <= bar2.outer_square.right)) {
        result += 10; // right-side inside
    }
    if ((bar2.outer_square.top <= bar1.outer_square.down) && (bar1.outer_square.down <= bar2.outer_square.down) && 
            (bar2.outer_square.left <= bar1.outer_square.right) && (bar1.outer_square.left <= bar2.outer_square.right)) {
        result += 100; // down-side inside
    }
    if ((bar2.outer_square.top <= bar1.outer_square.top) && (bar1.outer_square.top <= bar2.outer_square.down) && 
            (bar2.outer_square.left <= bar1.outer_square.right) && (bar1.outer_square.left <= bar2.outer_square.right)){
        result += 1000; // top-side inside
    }
    return result;
}

static double bar_calc_mass(hour_bar bar) {
  return bar.dim * bar.dim * BAR_DENSITY;
}

static void bars_init() {
    hour_bars[0].pos.x = 1;
    hour_bars[0].pos.y = 1;
    hour_bars[0].dim = rand() % DIM_RANGE[0] + 7;
    hour_bars[0].outer_square.top = hour_bars[0].pos.y;
    hour_bars[0].outer_square.down = hour_bars[0].outer_square.top + hour_bars[0].dim + 1;
    hour_bars[0].outer_square.left = hour_bars[0].pos.x;
    hour_bars[0].outer_square.right = hour_bars[0].outer_square.left + hour_bars[0].dim + 1;
    hour_bars[0].vel.x = 1;
    hour_bars[0].vel.y = 1;
    hour_bars[0].mass = bar_calc_mass(hour_bars[0]);
    
    for(int i=1;i<12;i++){
        hour_bars[i].pos.x = hour_bars[i-1].outer_square.right+1;
        hour_bars[i].pos.y = 1;
        hour_bars[i].dim = rand() % DIM_RANGE[i] + 7;
        hour_bars[i].outer_square.top = hour_bars[i].pos.y;
        hour_bars[i].outer_square.down = hour_bars[i].outer_square.top + hour_bars[i].dim + 1;
        hour_bars[i].outer_square.left = hour_bars[i].pos.x;
        hour_bars[i].outer_square.right = hour_bars[i].outer_square.left + hour_bars[i].dim + 1;
        hour_bars[i].vel.x = 1;
        hour_bars[i].vel.y = 1;
        hour_bars[i].mass = bar_calc_mass(hour_bars[i]);
    }
}

static void bars_apply_force(Vec2d force) {
    for (int i=0;i<CURRENT_TIME_HOUR;i++){
        hour_bars[i].vel.x += force.x/hour_bars[i].mass;
        hour_bars[i].vel.y += force.y/hour_bars[i].mass;
    }
}

static void bars_apply_accel(AccelData accel) {
  Vec2d force;
  force.x = accel.x * ACCEL_RATIO;
  force.y = -accel.y * ACCEL_RATIO;
  bars_apply_force(force);
}

static void bars_update() {
    double decay_coe = 0.8;
    for (int i=0;i<CURRENT_TIME_HOUR;i++){
        bool collide_on_walls = false;
        bool collide_on_ceil_or_floor = false;

        // Detect collision and update velocity
        if ((hour_bars[i].outer_square.left < 0 && hour_bars[i].vel.x < 0)
            || (hour_bars[i].outer_square.right > 144 && hour_bars[i].vel.x > 0)) {
            hour_bars[i].vel.x = -hour_bars[i].vel.x * decay_coe;
            collide_on_walls = true;
        }
        if ((hour_bars[i].outer_square.top < 0 && hour_bars[i].vel.y < 0)
            || (hour_bars[i].outer_square.down > 168 && hour_bars[i].vel.y > 0)) {
            hour_bars[i].vel.y = -hour_bars[i].vel.y * decay_coe;
            collide_on_ceil_or_floor = true;
        }

        int collision_of_all_bars = 0;
        for (int j=0;j<CURRENT_TIME_HOUR;j++){
            // collision parameter:
            // 0: no collision
            // 1: left side of bar1
            // 10: right side of bar1
            // 100: down side of bar1
            // 1000: top side of bar1
            if (i != j){
                int collision_para = update_collision(hour_bars[i], hour_bars[j]);
                collision_of_all_bars += collision_para;
                if (collision_para > 0){
                    if (collision_para % 10 == 1){
                        hour_bars[i].vel.x = absolut(hour_bars[i].vel.x) * 1.01;
                    } if (collision_para % 100 > 1){
                        hour_bars[i].vel.x = -absolut(hour_bars[i].vel.x) * 1.01;
                    } if (collision_para % 1000 > 11){
                        hour_bars[i].vel.y = -absolut(hour_bars[i].vel.y) * 1.01;
                    } if (collision_para % 10000 > 111){
                        hour_bars[i].vel.y = absolut(hour_bars[i].vel.y) * 1.01;
                    }
                    
                    if (collide_on_walls){
                        hour_bars[i].vel.x = 0;
                    }
                    if (collide_on_ceil_or_floor){
                        hour_bars[i].vel.y = 0;
                    }
                  }
              }
        } if (collision_of_all_bars == 0){
             hour_bars[i].vel.x *= 0.98;
             hour_bars[i].vel.y *= 0.98;
        }
        
        // Update all parameters basing on new location
        hour_bars[i].pos.x += hour_bars[i].vel.x;
        hour_bars[i].pos.y += hour_bars[i].vel.y;
        hour_bars[i].outer_square.top = hour_bars[i].pos.y;
        hour_bars[i].outer_square.down = hour_bars[i].outer_square.top + hour_bars[i].dim + 1;
        hour_bars[i].outer_square.left = hour_bars[i].pos.x;
        hour_bars[i].outer_square.right = hour_bars[i].outer_square.left + hour_bars[i].dim + 1;
    }
}

static void inverterlayers_draw() {
    for (int i=0;i<12;i++){
        inverter_layer_destroy(s_inverterlayers[i]);
        s_inverterlayers[i] = inverter_layer_create(GRect(hour_bars[i].outer_square.left, hour_bars[i].outer_square.top, hour_bars[i].dim, hour_bars[i].dim));
    }
    
    for (int i=0;i<CURRENT_TIME_HOUR;i++){
        layer_add_child(window_get_root_layer(s_main_window), inverter_layer_get_layer(s_inverterlayers[i]));
    }
}

static void timer_callback(void *data) {
    AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };

    accel_service_peek(&accel);

    bars_apply_accel(accel);

    bars_update();
    inverterlayers_draw();
    
    timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}

static void main_window_load(Window *window) {
    // set global time
    get_current_time();
    
    // Create Background Pots Layer
    s_backgroundPots_layer = layer_create(GRect(0, 0, 144, 168));
    layer_add_child(window_get_root_layer(window), s_backgroundPots_layer);
    
    // Set Layer Update Procedure for Background Pots Layer
    layer_set_update_proc(s_backgroundPots_layer, backgroundPots_update_proc);
    
    // Create Canvas Layer
    s_canvas_layer = layer_create(GRect(0, 0, 144, 168));
    layer_add_child(window_get_root_layer(window), s_canvas_layer);
    
    // Set Layer Update Procedure for Canvas Layer
    layer_set_update_proc(s_canvas_layer, canvas_update_proc);
    
    // Set Interter Layers
    bars_init();

    // Mass with inverter layers
    for (int i=0;i<12;i++){
        s_inverterlayers[i] = inverter_layer_create(GRect(hour_bars[i].outer_square.left, hour_bars[i].outer_square.top, hour_bars[i].dim, hour_bars[i].dim));
    }
 
    //CURRENT_TIME_HOUR = 2;
    for (int i=0;i<CURRENT_TIME_HOUR;i++){
        layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(s_inverterlayers[i]));
    }
    
    timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}

static void main_window_unload(Window *window) {
    // Destroy Backgound Pots Layer
    layer_destroy(s_backgroundPots_layer);
    
    // Destroy Canvas Layer
    layer_destroy(s_canvas_layer);
    
    for (int i=0;i<12;i++){
        inverter_layer_destroy(s_inverterlayers[i]);
    }
    
    app_timer_cancel(timer);
}

static void update_second() {
    layer_mark_dirty(s_canvas_layer);
}

static void update_minute(){
    layer_mark_dirty(s_canvas_layer);
}

static void update_hour(){ /////////////////////////////// Need Modification
    app_timer_cancel(timer);
    get_current_time();
    bars_init();
    
    // Mass with inverter layers
    for (int i=0;i<12;i++){
        s_inverterlayers[i] = inverter_layer_create(GRect(hour_bars[i].outer_square.left, hour_bars[i].outer_square.top, hour_bars[i].dim, hour_bars[i].dim));
    }
 
    //CURRENT_TIME_HOUR = 2;
    for (int i=0;i<CURRENT_TIME_HOUR;i++){
        layer_add_child(window_get_root_layer(s_main_window), inverter_layer_get_layer(s_inverterlayers[i]));
    }
    
    timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}

static void hour_handler(struct tm *tick_time, TimeUnits units_changed){
    update_hour();
}

static void minute_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_minute();
}

static void second_handler(struct tm *tick_time, TimeUnits units_changed){
    update_second();
}

static void init() {
    // Create main Window element and assign to pointer
    s_main_window = window_create();
    
    // Set handlers to manage the elements inside the Window
    window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = main_window_load,
      .unload = main_window_unload
    });
    // Set Full Screen
    window_set_fullscreen(s_main_window, true);

    // Show the Window on the watch, with animated=true
    window_stack_push(s_main_window, true);
  
    // Register with TickTimerService
    tick_timer_service_subscribe(HOUR_UNIT, hour_handler);
    tick_timer_service_subscribe(MINUTE_UNIT, minute_handler);
    tick_timer_service_subscribe(SECOND_UNIT, second_handler);
    
    accel_data_service_subscribe(0, NULL);
}

static void deinit() {
    // Destroy Window
    window_destroy(s_main_window);
    
    accel_data_service_unsubscribe();
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
