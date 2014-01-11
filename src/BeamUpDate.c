/**
 * Beam Up Date Pebble Watchface
 * Author: Chris Lewis
 * Date: 31st December 2013
 */

#include <pebble.h>

#define DEBUG false

#define INV_LAYER_WIDTH 30
#define INV_LAYER_HEIGHT 101
#define TIMEY 53
#define SECSY 105
#define DATEY 105
#define DAYY DATEY + 26
#define H0X -12
#define H1X  22
#define M0X  64
#define M1X  98
#define OFFSET 14

//Prototypes
void setTimeDigits(struct tm * t);
void setDate(struct tm * t);
struct TimeDigits getTimeDigits(struct tm * t);
void onBatteryStateChanged(BatteryChargeState charge);
void animateLayer(Layer *layer, GRect start, GRect finish, int duration, int delay);

//Globals
static TextLayer     *colonLayer, *weekLayer, *dateLayer, *dayLayer;
static TextLayer     *layerH0,    *layerH1,    *layerM0,    *layerM1;
static InverterLayer *invLayerH0, *invLayerH1, *invLayerM0, *invLayerM1;
static InverterLayer *bottomInvLayer;
static InverterLayer *batteryLayer;

struct TimeDigits {
    int h0;
    int h1;
    int m0;
    int m1;
};

static struct TimeDigits curDigits  = {0,0,0,0};
static struct TimeDigits prevDigits = {0,0,0,0};

static void animateLayerIn(TextLayer * textLayer, InverterLayer * inverterLayer, int x)
{
    animateLayer(inverter_layer_get_layer(inverterLayer), GRect(x+OFFSET, 0, INV_LAYER_WIDTH, 0), GRect(x+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), 600, 0);
    animateLayer(text_layer_get_layer(textLayer), GRect(x, TIMEY, 50, 60), GRect(x, -50, 50, 60), 200, 700);
}

static void animateLayerOut(TextLayer * textLayer, InverterLayer * inverterLayer, int x)
{
    animateLayer(text_layer_get_layer(textLayer), GRect(x, -50, 50, 60), GRect(x, TIMEY, 50, 60), 200, 100);
    animateLayer(inverter_layer_get_layer(inverterLayer), GRect(x+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), GRect(x+OFFSET, 0, INV_LAYER_WIDTH, 0), 500, 500);
}

/**
 * Handle tick function
 */
static void handle_tick(struct tm *t, TimeUnits units_changed) 
{    
    //Get the time
    const int seconds = t->tm_sec;
     
    const int curQ  = (int)seconds / 15;
    const int prevQ = curQ == 0 ? 4 : curQ - 1;

    const int fromX = prevQ * 36;
    const int tillX = curQ  * 36;

    static bool firstRun = true;
    if (firstRun) {
        animateLayer(inverter_layer_get_layer(bottomInvLayer), GRect(0, SECSY, 0, 5), GRect(0, SECSY, tillX, 5), 500, 0);
        firstRun = false;
    }

    if(seconds == 58)
    {
        animateLayer(inverter_layer_get_layer(bottomInvLayer), GRect(0, SECSY, 108, 5), GRect(0, SECSY, 144, 5), 500, 1000);
    }

    //Animations and time change
    else if(seconds == 59)
    {
        //Predict next changes
        struct tm nextTime = *t;
        nextTime.tm_min += 1;
        mktime(&nextTime);

        const struct TimeDigits nextDigits = getTimeDigits(&nextTime);

        if((nextDigits.h0 != prevDigits.h0) || (DEBUG)) {
            animateLayerIn(layerH0, invLayerH0, H0X);
        }

        if((nextDigits.h1 != prevDigits.h1) || (DEBUG)) {
            animateLayerIn(layerH1, invLayerH1, H1X);
        }

        if((nextDigits.m0 != prevDigits.m0) || (DEBUG)) {
            animateLayerIn(layerM0, invLayerM0, M0X);
        }

        if((nextDigits.m1 != prevDigits.m1) || (DEBUG)) {
            animateLayerIn(layerM1, invLayerM1, M1X);
        }
    }
    else if(seconds == 0)
    {
        //Set the time off screen
        setTimeDigits(t);

        //Animate stuff back into place
        if((curDigits.h0 != prevDigits.h0) || (DEBUG)) {
            animateLayerOut(layerH0, invLayerH0, H0X);
        }

        if((curDigits.h1 != prevDigits.h1) || (DEBUG)) {
            animateLayerOut(layerH1, invLayerH1, H1X);
        }

        if((curDigits.m0 != prevDigits.m0) || (DEBUG)) {
            animateLayerOut(layerM0, invLayerM0, M0X);
        }

        if((curDigits.m1 != prevDigits.m1) || (DEBUG)) {
            animateLayerOut(layerM1, invLayerM1, M1X);
        }

        prevDigits = curDigits;
    }

    //Bottom suface
    if(seconds % 15 == 0)
    {
        const int delay = seconds == 0 ? 500 : 0;
        animateLayer(inverter_layer_get_layer(bottomInvLayer), GRect(0, SECSY, fromX, 5), GRect(0, SECSY, tillX, 5), 500, delay);
    }

    if (units_changed & DAY_UNIT) {
        setDate(t);
    }
}

static void setupTextLayer(Layer * rootLayer, TextLayer ** layer, GRect location, ResHandle f_handle, GTextAlignment alignment)
{
    *layer = text_layer_create(location);
    text_layer_set_background_color(*layer, GColorClear);
    text_layer_set_text_color(*layer, GColorWhite);
    text_layer_set_font(*layer, fonts_load_custom_font(f_handle));
    text_layer_set_text_alignment(*layer, alignment);
    layer_add_child(rootLayer, (Layer*) *layer);
}

/**
 * Load window members
 */
static void window_load(Window *window)
{
    window_set_background_color(window, GColorBlack);

    //Get Font
    ResHandle fontImagine48 = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_48);
    ResHandle fontImagine24 = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_24);
    ResHandle fontImagine18 = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_18);
    ResHandle fontImagine10 = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_10);

    //Allocate text layers
    Layer * rootLayer = window_get_root_layer(window);
    setupTextLayer(rootLayer, &layerH0,    GRect(H0X, TIMEY,  50, 60), fontImagine48, GTextAlignmentRight);
    setupTextLayer(rootLayer, &layerH1,    GRect(H1X, TIMEY,  50, 60), fontImagine48, GTextAlignmentRight);
    setupTextLayer(rootLayer, &colonLayer, GRect(69,  TIMEY,  50, 60), fontImagine48, GTextAlignmentLeft);
    setupTextLayer(rootLayer, &layerM0,    GRect(M0X, TIMEY,  50, 60), fontImagine48, GTextAlignmentRight);
    setupTextLayer(rootLayer, &layerM1,    GRect(M1X, TIMEY,  50, 60), fontImagine48, GTextAlignmentRight);
    setupTextLayer(rootLayer, &weekLayer,  GRect( 0,  DATEY,  30, 30), fontImagine10, GTextAlignmentLeft);
    setupTextLayer(rootLayer, &dateLayer,  GRect(30,  DATEY, 114, 30), fontImagine24, GTextAlignmentRight);
    setupTextLayer(rootLayer, &dayLayer,   GRect(-20, DAYY,  158, 30), fontImagine18, GTextAlignmentRight);

    //Allocate inverter layers
    invLayerH0 = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));
    invLayerH1 = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));
    invLayerM0 = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));
    invLayerM1 = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));
    bottomInvLayer = inverter_layer_create(GRect(0, 0, 144, 0));
    batteryLayer   = inverter_layer_create(GRect(0, 0, 144, 0));

    layer_add_child(window_get_root_layer(window), (Layer*) invLayerH0);
    layer_add_child(window_get_root_layer(window), (Layer*) invLayerH1);
    layer_add_child(window_get_root_layer(window), (Layer*) invLayerM0);
    layer_add_child(window_get_root_layer(window), (Layer*) invLayerM1);
    layer_add_child(window_get_root_layer(window), (Layer*) bottomInvLayer);
    layer_add_child(window_get_root_layer(window), (Layer*) batteryLayer);

    //Make sure the face is not blank
    const time_t now = time(0);
    struct tm * t = localtime(&now);
    setTimeDigits(t);
    setDate(t);

    //Stop 'all change' on first minute
    prevDigits = curDigits;

    // initial update of battery
    onBatteryStateChanged(battery_state_service_peek());
}

/**
 * Handle tap acctions
 */
void accelTapHandler(AccelAxisType axis, int32_t direction)
{
//    app_log(APP_LOG_LEVEL_DEBUG, "", 0, "%d %d", (int)axis, (int)direction);

    static bool hideDate = true;
    layer_set_hidden((Layer*)weekLayer, hideDate);
    layer_set_hidden((Layer*)dateLayer, hideDate);
    layer_set_hidden((Layer*)dayLayer, hideDate);

    hideDate = !hideDate;
}

/**
 * Unload window members
 */
static void window_unload(Window *window)
{
    //Free text layers
    text_layer_destroy(layerH0);
    text_layer_destroy(layerH1);
    text_layer_destroy(colonLayer);
    text_layer_destroy(layerM0);
    text_layer_destroy(layerM1);
    text_layer_destroy(weekLayer);
    text_layer_destroy(dateLayer);
    text_layer_destroy(dayLayer);

    //Free inverter layers
    inverter_layer_destroy(invLayerH0);
    inverter_layer_destroy(invLayerH1);
    inverter_layer_destroy(bottomInvLayer);
    inverter_layer_destroy(batteryLayer);
    inverter_layer_destroy(invLayerM0);
    inverter_layer_destroy(invLayerM1);

    //Unsubscribe from events
    tick_timer_service_unsubscribe();
}

void onBatteryStateChanged(BatteryChargeState charge)
{
    static int lastOffset = 0;
    const int offset = 144 * charge.charge_percent/100;
    animateLayer(inverter_layer_get_layer(batteryLayer), GRect(0, 166, lastOffset, 2), GRect(0, 166, offset, 2), 500, 0);
    lastOffset = offset;
}

/**
 * Init app
 */
static Window * init(void)
{
    Window * window = window_create();
    WindowHandlers handlers = {
        .load = window_load,
        .unload = window_unload
    };
    window_set_window_handlers(window, (WindowHandlers) handlers);

    //Subscribe to events
    tick_timer_service_subscribe(SECOND_UNIT, handle_tick);

    //Finally
    window_stack_push(window, true);

    accel_tap_service_subscribe(&accelTapHandler);
    battery_state_service_subscribe(&onBatteryStateChanged);

    return window;
}

/**
 * De-init app
 */
static void deinit(Window * window)
{
    accel_tap_service_unsubscribe();
    battery_state_service_unsubscribe();

    window_destroy(window);
}

/**
 * Entry point
 */
int main(void) {
    Window * window = init();
    app_event_loop();
    deinit(window);
    return 0;
}

/**
 * Function definitions
 ***************************************************************************8
 */

/**
 * Function to get time digits
 */
struct TimeDigits getTimeDigits(struct tm * t)
{
    char txt[] = "00:00";
    strftime(txt, sizeof(txt), clock_is_24h_style() ? "%H:%M" : "%I:%M", t);

    //Get digits
    struct TimeDigits retval;
    retval.h0 = txt[0] - '0';
    retval.h1 = txt[1] - '0';
    retval.m0 = txt[3] - '0';
    retval.m1 = txt[4] - '0';
    return retval;
}

/**
 * Function to set the time and date digits on the TextLayers
 */
void setTimeDigits(struct tm * t)
{
    curDigits = getTimeDigits(t);

    static char HTText[] = "0";
    static char HUText[] = "0";
    static char colonText[] = ":";
    static char MTText[] = "0";
    static char MUText[] = "0";

    //Copy digits
    HTText[0] = '0' + curDigits.h0;
    HUText[0] = '0' + curDigits.h1;
    MTText[0] = '0' + curDigits.m0;
    MUText[0] = '0' + curDigits.m1;

    //Fix digits for debugging purposes
    if(DEBUG)
    {
        HTText[0] = '2';
        HUText[0] = '3';
        MTText[0] = '5';
        MUText[0] = '9';
    }
    
    //Set digits in TextLayers
    text_layer_set_text(layerH0, HTText);
    text_layer_set_text(layerH1, HUText);
    text_layer_set_text(colonLayer, colonText);
    text_layer_set_text(layerM0, MTText);
    text_layer_set_text(layerM1, MUText);
}

const char * DAY_NAME_GERMAN[] = {
    "Sonntag",
    "Montag",
    "Dienstag",
    "Mittwoch",
    "Donnerstag",
    "Freitag",
    "Samstag"
};

void setDate(struct tm * t)
{
    //Set date to TextLayer
    static char dateText[] = "12.03.";
    strftime(dateText, sizeof(dateText), "%d.%m.", t);	//Sun 01
    text_layer_set_text(dateLayer, dateText);

    static char weekText[] = "KW\nxx";
    strftime(weekText, sizeof(weekText), "KW\n%V.", t);	//KW26
    text_layer_set_text(weekLayer, weekText);

    text_layer_set_text(dayLayer, DAY_NAME_GERMAN[t->tm_wday]);
}

/**
 * New dymanic animations
 */
void on_animation_stopped(Animation *anim, bool finished, void *context)
{
    //Free the memoery used by the Animation
    property_animation_destroy((PropertyAnimation*) anim);
}

void animateLayer(Layer *layer, GRect start, GRect finish, int duration, int delay)
{
    //Declare animation
    PropertyAnimation *anim = property_animation_create_layer_frame(layer, &start, &finish);

    //Set characteristics
    animation_set_duration((Animation*) anim, duration);
    animation_set_delay((Animation*) anim, delay);
    animation_set_curve((Animation*) anim, AnimationCurveEaseInOut);

    //Set stopped handler to free memory
    AnimationHandlers handlers = {
        //The reference to the stopped handler is the only one in the array
        .stopped = (AnimationStoppedHandler) on_animation_stopped
    };
    animation_set_handlers((Animation*) anim, handlers, NULL);

    //Start animation!
    animation_schedule((Animation*) anim);
}

