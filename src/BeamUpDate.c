/**
 * Beam Up Date Pebble Watchface
 * Author: Chris Lewis
 * Date: 31st December 2013
 */

#include <pebble.h>
#include "pebbleapi.h"

//#define CUSTOM_TIME "23:59:10"
#include "customtime.h"

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

static GBitmap     *imgBatteryCharging,   *imgBatteryEmpty,   *imgBluetoothDisconnected;
static BitmapLayer *batteryChargingLayer, *batteryEmptyLayer, *bluetoothDisconnectedLayer;

///////////////////////////////////////////////////////////////////////////////////////////

struct TimeDigits getTimeDigits(const struct tm * t)
{
    char txt[] = "0000";
    strftime(txt, sizeof(txt), clock_is_24h_style() ? "%H%M" : "%I%M", t);

    //Get digits
    struct TimeDigits retval;
    retval.h0 = txt[0] - '0';
    retval.h1 = txt[1] - '0';
    retval.m0 = txt[2] - '0';
    retval.m1 = txt[3] - '0';
    return retval;
}

///////////////////////////////////////////////////////////////////////////////////////////

static void updateTextLayersTime(const struct tm * t)
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

    //Set digits in TextLayers
    text_layer_set_text(layerH0, HTText);
    text_layer_set_text(layerH1, HUText);
    text_layer_set_text(colonLayer, colonText);
    text_layer_set_text(layerM0, MTText);
    text_layer_set_text(layerM1, MUText);
}

const char * DAY_NAME_GERMAN[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};

static void updateTextLayersDate(const struct tm * t)
{
    // date
    static char dateText[] = "12.03.";
    strftime(dateText, sizeof(dateText), "%d.%m.", t);	//Sun 01
    text_layer_set_text(dateLayer, dateText);

    // calender week
    static char weekText[] = "KW\nxx";
    strftime(weekText, sizeof(weekText), "KW\n%V.", t);	//KW26
    text_layer_set_text(weekLayer, weekText);

    // day
    text_layer_set_text(dayLayer, DAY_NAME_GERMAN[t->tm_wday]);
}

///////////////////////////////////////////////////////////////////////////////////////////

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

static void tickTimerHandler(struct tm * t, TimeUnits unitsChanged)
{
    ADJUST_TIME_TO_CUSTOM_TIME(t);

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
        if((nextDigits.h0 != prevDigits.h0)) animateLayerIn(layerH0, invLayerH0, H0X);
        if((nextDigits.h1 != prevDigits.h1)) animateLayerIn(layerH1, invLayerH1, H1X);
        if((nextDigits.m0 != prevDigits.m0)) animateLayerIn(layerM0, invLayerM0, M0X);
        if((nextDigits.m1 != prevDigits.m1)) animateLayerIn(layerM1, invLayerM1, M1X);
    }
    else if(seconds == 0)
    {
        //Set the time off screen
        updateTextLayersTime(t);

        //Animate stuff back into place
        if((curDigits.h0 != prevDigits.h0)) animateLayerOut(layerH0, invLayerH0, H0X);
        if((curDigits.h1 != prevDigits.h1)) animateLayerOut(layerH1, invLayerH1, H1X);
        if((curDigits.m0 != prevDigits.m0)) animateLayerOut(layerM0, invLayerM0, M0X);
        if((curDigits.m1 != prevDigits.m1)) animateLayerOut(layerM1, invLayerM1, M1X);

        prevDigits = curDigits;
    }

    //Bottom suface
    if(seconds % 15 == 0)
    {
        const int delay = seconds == 0 ? 500 : 0;
        animateLayer(inverter_layer_get_layer(bottomInvLayer), GRect(0, SECSY, fromX, 5), GRect(0, SECSY, tillX, 5), 500, delay);
    }

    if (unitsChanged & DAY_UNIT) {
        updateTextLayersDate(t);
    }
}

void accelTapHandler(AccelAxisType axis, int32_t direction)
{
//    app_log(APP_LOG_LEVEL_DEBUG, "", 0, "%d %d", (int)axis, (int)direction);

    static bool hideDate = true;
    layer_set_hidden((Layer*)weekLayer, hideDate);
    layer_set_hidden((Layer*)dateLayer, hideDate);
    layer_set_hidden((Layer*)dayLayer,  hideDate);

    hideDate = !hideDate;
}

void batteryStateHandler(BatteryChargeState charge)
{
    static int lastOffset = 0;
    const int offset = 144 * charge.charge_percent/100;
    animateLayer(inverter_layer_get_layer(batteryLayer), GRect(0, 166, lastOffset, 2), GRect(0, 166, offset, 2), 500, 0);
    lastOffset = offset;

    layer_set_visible((Layer*)batteryChargingLayer, charge.is_charging);
    layer_set_visible((Layer*)batteryEmptyLayer,   !charge.is_charging && charge.charge_percent <= 20);
}

void bluetoothConnectionHandler(bool connected)
{
    layer_set_visible((Layer*)bluetoothDisconnectedLayer, !connected);
}

///////////////////////////////////////////////////////////////////////////////////////////

static void setupTextLayer(Layer * rootLayer, TextLayer ** layer, GRect location, ResHandle fontHandle, GTextAlignment alignment)
{
    *layer = text_layer_create(location);
    text_layer_set_background_color(*layer, GColorClear);
    text_layer_set_text_color(*layer, GColorWhite);
    text_layer_set_font(*layer, fonts_load_custom_font(fontHandle));
    text_layer_set_text_alignment(*layer, alignment);
    layer_add_child(rootLayer, (Layer*) *layer);
}

static void setupInverterLayer(Layer * rootLayer, InverterLayer ** layer, GRect location)
{
    *layer = inverter_layer_create(location);
    layer_add_child(rootLayer, (Layer*) *layer);
}

static void setupBitmapLayer(Layer * rootLayer, BitmapLayer ** layer, GBitmap ** bitmap,  GRect location, uint32_t resourceId)
{
    *bitmap = gbitmap_create_with_resource(resourceId);
    *layer = bitmap_layer_create(location);
    bitmap_layer_set_bitmap(*layer, *bitmap);
    layer_add_child(rootLayer, bitmap_layer_get_layer(*layer));
}

static void windowLoad(Window * window)
{
    window_set_background_color(window, GColorBlack);

    //Get Font
    const ResHandle fontImagine48 = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_48);
    const ResHandle fontImagine24 = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_24);
    const ResHandle fontImagine18 = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_18);
    const ResHandle fontImagine10 = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_10);

    //Allocate text layers
    Layer * rootLayer = window_get_root_layer(window);
    setupTextLayer(rootLayer, &layerH0,    GRect(H0X, TIMEY,   50, 60), fontImagine48, GTextAlignmentRight);
    setupTextLayer(rootLayer, &layerH1,    GRect(H1X, TIMEY,   50, 60), fontImagine48, GTextAlignmentRight);
    setupTextLayer(rootLayer, &colonLayer, GRect(69,  TIMEY-6, 50, 60), fontImagine48, GTextAlignmentLeft);
    setupTextLayer(rootLayer, &layerM0,    GRect(M0X, TIMEY,   50, 60), fontImagine48, GTextAlignmentRight);
    setupTextLayer(rootLayer, &layerM1,    GRect(M1X, TIMEY,   50, 60), fontImagine48, GTextAlignmentRight);
    setupTextLayer(rootLayer, &weekLayer,  GRect( 0,  DATEY,   30, 30), fontImagine10, GTextAlignmentLeft);
    setupTextLayer(rootLayer, &dateLayer,  GRect(30,  DATEY,  114, 30), fontImagine24, GTextAlignmentRight);
    setupTextLayer(rootLayer, &dayLayer,   GRect(-20, DAYY,  158, 30), fontImagine18, GTextAlignmentRight);

    //Allocate inverter layers
    setupInverterLayer(rootLayer, &invLayerH0,     GRect(0, 0, INV_LAYER_WIDTH, 0));
    setupInverterLayer(rootLayer, &invLayerH1,     GRect(0, 0, INV_LAYER_WIDTH, 0));
    setupInverterLayer(rootLayer, &invLayerM0,     GRect(0, 0, INV_LAYER_WIDTH, 0));
    setupInverterLayer(rootLayer, &invLayerM1,     GRect(0, 0, INV_LAYER_WIDTH, 0));
    setupInverterLayer(rootLayer, &bottomInvLayer, GRect(0, 0, 144, 0));
    setupInverterLayer(rootLayer, &batteryLayer,   GRect(0, 0, 144, 0));

    //Allocate bitmap layers
    setupBitmapLayer(rootLayer, &batteryChargingLayer,       &imgBatteryCharging,       GRect(0,168-10,18,10), RESOURCE_ID_IMAGE_BATTERY_CHARGING);
    setupBitmapLayer(rootLayer, &batteryEmptyLayer,          &imgBatteryEmpty,          GRect(0,168-10,18,10), RESOURCE_ID_IMAGE_BATTERY_EMPTY);
    setupBitmapLayer(rootLayer, &bluetoothDisconnectedLayer, &imgBluetoothDisconnected, GRect(0,168-20,15,10), RESOURCE_ID_IMAGE_BLUETOOTH_DISCONNECTED);

    //Make sure the face is not blank
    const time_t now = time(NULL);
    struct tm * t = localtime(&now);
    ADJUST_TIME_TO_CUSTOM_TIME(t);
    updateTextLayersTime(t);
    updateTextLayersDate(t);

    //Stop 'all change' on first minute
    prevDigits = curDigits;

    // initial update of battery and bluetooth
    batteryStateHandler(battery_state_service_peek());
    bluetoothConnectionHandler(bluetooth_connection_service_peek());
}

static void windowUnload(Window * window)
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

    //Free bitmaps
    gbitmap_destroy(imgBatteryCharging);
    gbitmap_destroy(imgBatteryEmpty);
    gbitmap_destroy(imgBluetoothDisconnected);

    //Free bitmap layers
    bitmap_layer_destroy(batteryChargingLayer);
    bitmap_layer_destroy(batteryEmptyLayer);
    bitmap_layer_destroy(bluetoothDisconnectedLayer);
}

static Window * init(void)
{
    Window * window = window_create();
    window_set_window_handlers(window, createWindowHandlers(windowLoad, windowUnload));

    //Subscribe to events
    tick_timer_service_subscribe(SECOND_UNIT, tickTimerHandler);
    accel_tap_service_subscribe(accelTapHandler);
    battery_state_service_subscribe(batteryStateHandler);
    bluetooth_connection_service_subscribe(bluetoothConnectionHandler);

    //Finally
    window_stack_push(window, true);

    return window;
}

static void deinit(Window * window)
{
    //Unsubscribe from events
    tick_timer_service_unsubscribe();
    accel_tap_service_unsubscribe();
    battery_state_service_unsubscribe();
    bluetooth_connection_service_unsubscribe();

    window_destroy(window);
}

int main(void)
{
    Window * window = init();
    app_event_loop();
    deinit(window);
    return 0;
}
