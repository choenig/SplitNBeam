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
#define HTX -12
#define HUX  22
#define MTX  64
#define MUX  98
#define OFFSET 14

//Prototypes
void setTimeDigits(struct tm * t);
void setDate(struct tm * t);
void predictNextDigits(struct tm *t);
void animateLayer(Layer *layer, GRect start, GRect finish, int duration, int delay);

//Globals
static TextLayer *HTLayer, *HULayer, *colonLayer, *MTLayer, *MULayer, *dateLayer;
static InverterLayer *HTInvLayer, *HUInvLayer, *MTInvLayer, *MUInvLayer, *bottomInvLayer;
static int HTDigit = 0, HTprev = 0, 
           HUDigit = 0, HUprev = 0,
           MTDigit = 0, MTprev = 0,
           MUDigit = 0, MUprev = 0;

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
        predictNextDigits(t); //CALLS GETTIMEDIGITS()

        //Only change minutes units if its changed
        if((MUDigit != MUprev) || (DEBUG))
        {
            animateLayer(inverter_layer_get_layer(MUInvLayer), GRect(MUX+OFFSET, 0, INV_LAYER_WIDTH, 0), GRect(MUX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), 600, 0);
            animateLayer(text_layer_get_layer(MULayer), GRect(MUX, TIMEY, 50, 60), GRect(MUX, -50, 50, 60), 200, 700);
        }

        //Only change minutes tens if its changed
        if((MTDigit != MTprev) || (DEBUG))
        {
            animateLayer(inverter_layer_get_layer(MTInvLayer), GRect(MTX+OFFSET, 0, INV_LAYER_WIDTH, 0), GRect(MTX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), 600, 0);
            animateLayer(text_layer_get_layer(MTLayer), GRect(MTX, TIMEY, 50, 60), GRect(MTX, -50, 50, 60), 200, 700);
        }

        //Only change hours units if its changed
        if((HUDigit != HUprev) || (DEBUG))
        {
            animateLayer(inverter_layer_get_layer(HUInvLayer), GRect(HUX+OFFSET, 0, INV_LAYER_WIDTH, 0), GRect(HUX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), 600, 0);
            animateLayer(text_layer_get_layer(HULayer), GRect(HUX, TIMEY, 50, 60), GRect(HUX, -50, 50, 60), 200, 700);
        }

        //Only change hours tens if its changed
        if((HTDigit != HTprev) || (DEBUG))
        {
            animateLayer(inverter_layer_get_layer(HTInvLayer), GRect(HTX+OFFSET, 0, INV_LAYER_WIDTH, 0), GRect(HTX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), 600, 0);
            animateLayer(text_layer_get_layer(HTLayer), GRect(HTX, TIMEY, 50, 60), GRect(HTX, -50, 50, 60), 200, 700);
        }
    }
    else if(seconds == 0)
    {
        //Set the time off screen
        setTimeDigits(t);

        //Animate stuff back into place
        if((MUDigit != MUprev) || (DEBUG))
        {
            animateLayer(text_layer_get_layer(MULayer), GRect(MUX, -50, 50, 60), GRect(MUX, TIMEY, 50, 60), 200, 100);
            animateLayer(inverter_layer_get_layer(MUInvLayer), GRect(MUX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), GRect(MUX+OFFSET, 0, INV_LAYER_WIDTH, 0), 500, 500);
            MUprev = MUDigit;   //reset the thing
        }
        if((MTDigit != MTprev) || (DEBUG))
        {
            animateLayer(text_layer_get_layer(MTLayer), GRect(MTX, -50, 50, 60), GRect(MTX, TIMEY, 50, 60), 200, 100);
            animateLayer(inverter_layer_get_layer(MTInvLayer), GRect(MTX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), GRect(MTX+OFFSET, 0, INV_LAYER_WIDTH, 0), 500, 500);
            MTprev = MTDigit;
        }
        if((HUDigit != HUprev) || (DEBUG))
        {
            animateLayer(text_layer_get_layer(HULayer), GRect(HUX, -50, 50, 60), GRect(HUX, TIMEY, 50, 60), 200, 100);
            animateLayer(inverter_layer_get_layer(HUInvLayer), GRect(HUX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), GRect(HUX+OFFSET, 0, INV_LAYER_WIDTH, 0), 500, 500);
            HUprev = HUDigit;
        }
        if((HTDigit != HTprev) || (DEBUG))
        {
            animateLayer(text_layer_get_layer(HTLayer), GRect(HTX, -50, 50, 60), GRect(HTX, TIMEY, 50, 60), 200, 100);
            animateLayer(inverter_layer_get_layer(HTInvLayer), GRect(HTX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), GRect(HTX+OFFSET, 0, INV_LAYER_WIDTH, 0), 500, 500);
            HTprev = HTDigit;
        }
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
    ResHandle f_handle = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_48);
    ResHandle sm_f_handle = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_24);

    //Allocate text layers
    Layer * rootLayer = window_get_root_layer(window);
    setupTextLayer(rootLayer, &HTLayer,    GRect(HTX, TIMEY,  50, 60), f_handle, GTextAlignmentRight);
    setupTextLayer(rootLayer, &HULayer,    GRect(HUX, TIMEY,  50, 60), f_handle, GTextAlignmentRight);
    setupTextLayer(rootLayer, &colonLayer, GRect(69,  TIMEY,  50, 60), f_handle, GTextAlignmentLeft);
    setupTextLayer(rootLayer, &MTLayer,    GRect(MTX, TIMEY,  50, 60), f_handle, GTextAlignmentRight);
    setupTextLayer(rootLayer, &MULayer,    GRect(MUX, TIMEY,  50, 60), f_handle, GTextAlignmentRight);
    setupTextLayer(rootLayer, &dateLayer,  GRect(45,  DATEY, 100, 30), sm_f_handle, GTextAlignmentRight);

    //Allocate inverter layers
    HTInvLayer = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));
    HUInvLayer = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));
    bottomInvLayer = inverter_layer_create(GRect(0, 0, 144, 0));
    MTInvLayer = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));
    MUInvLayer = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));

    layer_add_child(window_get_root_layer(window), (Layer*) HTInvLayer);
    layer_add_child(window_get_root_layer(window), (Layer*) HUInvLayer);
    layer_add_child(window_get_root_layer(window), (Layer*) bottomInvLayer);
    layer_add_child(window_get_root_layer(window), (Layer*) MTInvLayer);
    layer_add_child(window_get_root_layer(window), (Layer*) MUInvLayer);

    //Make sure the face is not blank
    const time_t now = time(0);
    struct tm * t = localtime(&now);
    setTimeDigits(t);
    setDate(t);

    //Stop 'all change' on first minute
    MUprev = MUDigit;
    MTprev = MTDigit;
    HUprev = HUDigit;
    HTprev = HTDigit;
}

/**
 * Unload window members
 */
static void window_unload(Window *window)
{
    //Free text layers
    text_layer_destroy(HTLayer);
    text_layer_destroy(HULayer);
    text_layer_destroy(colonLayer);
    text_layer_destroy(MTLayer);
    text_layer_destroy(MULayer);
    text_layer_destroy(dateLayer);

    //Free inverter layers
    inverter_layer_destroy(HTInvLayer);
    inverter_layer_destroy(HUInvLayer);
    inverter_layer_destroy(bottomInvLayer);
    inverter_layer_destroy(MTInvLayer);
    inverter_layer_destroy(MUInvLayer);

    //Unsubscribe from events
    tick_timer_service_unsubscribe();
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

    return window;
}

/**
 * De-init app
 */
static void deinit(Window * window)
{
    window_destroy(window);
}

/**
 * Entry point
 */
int main(void) {
    Window * window = init();
    app_event_loop();
    deinit(window);
}

/**
 * Function definitions
 ***************************************************************************8
 */

/**
 * Function to get time digits
 */
void getTimeDigits(struct tm * t)
{
    char txt[] = "00:00";
    strftime(txt, sizeof(txt), clock_is_24h_style() ? "%H:%M" : "%I:%M", t);

    //Get digits
    HTDigit = txt[0] - '0';
    HUDigit = txt[1] - '0';
    MTDigit = txt[3] - '0';
    MUDigit = txt[4] - '0';
}

/**
 * Function to set the time and date digits on the TextLayers
 */
void setTimeDigits(struct tm * t)
{
    getTimeDigits(t);

    static char HTText[] = "0";
    static char HUText[] = "0";
    static char colonText[] = ":";
    static char MTText[] = "0";
    static char MUText[] = "0";

    //Copy digits
    HTText[0] = '0' + HTDigit;
    HUText[0] = '0' + HUDigit;
    MTText[0] = '0' + MTDigit;
    MUText[0] = '0' + MUDigit;

    //Fix digits for debugging purposes
    if(DEBUG)
    {
        HTText[0] = '2';
        HUText[0] = '3';
        MTText[0] = '5';
        MUText[0] = '9';
    }
    
    //Set digits in TextLayers
    text_layer_set_text(HTLayer, HTText);
    text_layer_set_text(HULayer, HUText);
    text_layer_set_text(colonLayer, colonText);
    text_layer_set_text(MTLayer, MTText);
    text_layer_set_text(MULayer, MUText);
}

void setDate(struct tm * t)
{
    static char dateText[] = "Mon xx";
    strftime(dateText, sizeof(dateText), "%a %d", t);	//Sun 01

    //Set date to TextLayer
    text_layer_set_text(dateLayer, dateText);
}

/**
 * Function to predict digit changes to make the digit change mechanic fire correctly.
 * If the values change at seconds == 0, then the animations depending on MUDigit != MUprev
 * will not fire!
 * The solution is to advance the ones about to change pre-emptively
 */
void predictNextDigits(struct tm *t)
{
    struct tm nextTime = *t;
    nextTime.tm_min += 1;
    mktime(&nextTime);

    getTimeDigits(&nextTime);
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

