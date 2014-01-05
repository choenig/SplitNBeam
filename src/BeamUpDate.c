/**
 * Beam Up Date Pebble Watchface
 * Author: Chris Lewis
 * Date: 31st December 2013
 */

#include <pebble.h>

#define DEBUG false

#define INV_LAYER_WIDTH 30
#define INV_LAYER_HEIGHT 101
#define HTX -12 
#define HUX 22
#define MTX 64
#define MUX 98
#define OFFSET 14

//Prototypes
void setTimeDigits(struct tm * t);
void predictNextDigits(struct tm *t);
void animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay);
void setupTextLayer(TextLayer *layer, GRect location, GColor b_colour, GColor t_colour, ResHandle f_handle, GTextAlignment alignment);

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
        animate_layer(inverter_layer_get_layer(bottomInvLayer), GRect(0, 105, 0, 5), GRect(0, 105, tillX, 5), 500, 0);
        firstRun = false;
    }

    if(seconds == 58)
    {
        animate_layer(inverter_layer_get_layer(bottomInvLayer), GRect(0, 105, 108, 5), GRect(0, 105, 144, 5), 500, 1000);
    }

    //Animations and time change
    else if(seconds == 59)
    {
        //Predict next changes
        predictNextDigits(t); //CALLS GETTIMEDIGITS()

        //Only change minutes units if its changed
        if((MUDigit != MUprev) || (DEBUG))
        {
            animate_layer(inverter_layer_get_layer(MUInvLayer), GRect(MUX+OFFSET, 0, INV_LAYER_WIDTH, 0), GRect(MUX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), 600, 0);
            animate_layer(text_layer_get_layer(MULayer), GRect(MUX, 53, 50, 60), GRect(MUX, -50, 50, 60), 200, 700);
        }

        //Only change minutes tens if its changed
        if((MTDigit != MTprev) || (DEBUG))
        {
            animate_layer(inverter_layer_get_layer(MTInvLayer), GRect(MTX+OFFSET, 0, INV_LAYER_WIDTH, 0), GRect(MTX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), 600, 0);
            animate_layer(text_layer_get_layer(MTLayer), GRect(MTX, 53, 50, 60), GRect(MTX, -50, 50, 60), 200, 700);
        }

        //Only change hours units if its changed
        if((HUDigit != HUprev) || (DEBUG))
        {
            animate_layer(inverter_layer_get_layer(HUInvLayer), GRect(HUX+OFFSET, 0, INV_LAYER_WIDTH, 0), GRect(HUX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), 600, 0);
            animate_layer(text_layer_get_layer(HULayer), GRect(HUX, 53, 50, 60), GRect(HUX, -50, 50, 60), 200, 700);
        }

        //Only change hours tens if its changed
        if((HTDigit != HTprev) || (DEBUG))
        {
            animate_layer(inverter_layer_get_layer(HTInvLayer), GRect(HTX+OFFSET, 0, INV_LAYER_WIDTH, 0), GRect(HTX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), 600, 0);
            animate_layer(text_layer_get_layer(HTLayer), GRect(HTX, 53, 50, 60), GRect(HTX, -50, 50, 60), 200, 700);
        }
    }
    else if(seconds == 0)
    {
        //Set the time off screen
        setTimeDigits(t);

        //Animate stuff back into place
        if((MUDigit != MUprev) || (DEBUG))
        {
            animate_layer(text_layer_get_layer(MULayer), GRect(MUX, -50, 50, 60), GRect(MUX, 53, 50, 60), 200, 100);
            animate_layer(inverter_layer_get_layer(MUInvLayer), GRect(MUX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), GRect(MUX+OFFSET, 0, INV_LAYER_WIDTH, 0), 500, 500);
            MUprev = MUDigit;   //reset the thing
        }
        if((MTDigit != MTprev) || (DEBUG))
        {
            animate_layer(text_layer_get_layer(MTLayer), GRect(MTX, -50, 50, 60), GRect(MTX, 53, 50, 60), 200, 100);
            animate_layer(inverter_layer_get_layer(MTInvLayer), GRect(MTX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), GRect(MTX+OFFSET, 0, INV_LAYER_WIDTH, 0), 500, 500);
            MTprev = MTDigit;
        }
        if((HUDigit != HUprev) || (DEBUG))
        {
            animate_layer(text_layer_get_layer(HULayer), GRect(HUX, -50, 50, 60), GRect(HUX, 53, 50, 60), 200, 100);
            animate_layer(inverter_layer_get_layer(HUInvLayer), GRect(HUX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), GRect(HUX+OFFSET, 0, INV_LAYER_WIDTH, 0), 500, 500);
            HUprev = HUDigit;
        }
        if((HTDigit != HTprev) || (DEBUG))
        {
            animate_layer(text_layer_get_layer(HTLayer), GRect(HTX, -50, 50, 60), GRect(HTX, 53, 50, 60), 200, 100);
            animate_layer(inverter_layer_get_layer(HTInvLayer), GRect(HTX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), GRect(HTX+OFFSET, 0, INV_LAYER_WIDTH, 0), 500, 500);
            HTprev = HTDigit;
        }
    }

    //Bottom suface
    if(seconds % 15 == 0)
    {
        const int delay = seconds == 0 ? 500 : 0;
        animate_layer(inverter_layer_get_layer(bottomInvLayer), GRect(0, 105, fromX, 5), GRect(0, 105, tillX, 5), 500, delay);
    }
}

/**
 * Load window members
 */
static void window_load(Window *window) {
    window_set_background_color(window, GColorBlack);

    //Get Font
    ResHandle f_handle = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_48);
    ResHandle sm_f_handle = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_24);

    //Allocate text layers
    HTLayer = text_layer_create(GRect(HTX, 53, 50, 60));
    text_layer_set_background_color(HTLayer, GColorClear);
    text_layer_set_text_color(HTLayer, GColorWhite);
    text_layer_set_font(HTLayer, fonts_load_custom_font(f_handle));
    text_layer_set_text_alignment(HTLayer, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), (Layer*) HTLayer);

    HULayer = text_layer_create(GRect(HUX, 53, 50, 60));
    text_layer_set_background_color(HULayer, GColorClear);
    text_layer_set_text_color(HULayer, GColorWhite);
    text_layer_set_font(HULayer, fonts_load_custom_font(f_handle));
    text_layer_set_text_alignment(HULayer, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), (Layer*) HULayer);

    colonLayer = text_layer_create(GRect(69, 53, 50, 60));
    text_layer_set_background_color(colonLayer, GColorClear);
    text_layer_set_text_color(colonLayer, GColorWhite);
    text_layer_set_font(colonLayer, fonts_load_custom_font(f_handle));
    text_layer_set_text_alignment(colonLayer, GTextAlignmentLeft);
    layer_add_child(window_get_root_layer(window), (Layer*) colonLayer);

    MTLayer = text_layer_create(GRect(MTX, 53, 50, 60));
    text_layer_set_background_color(MTLayer, GColorClear);
    text_layer_set_text_color(MTLayer, GColorWhite);
    text_layer_set_font(MTLayer, fonts_load_custom_font(f_handle));
    text_layer_set_text_alignment(MTLayer, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), (Layer*) MTLayer);

    MULayer = text_layer_create(GRect(MUX, 53, 50, 60));
    text_layer_set_background_color(MULayer, GColorClear);
    text_layer_set_text_color(MULayer, GColorWhite);
    text_layer_set_font(MULayer, fonts_load_custom_font(f_handle));
    text_layer_set_text_alignment(MULayer, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), (Layer*) MULayer);

    dateLayer = text_layer_create(GRect(45, 105, 100, 30));
    text_layer_set_background_color(dateLayer, GColorClear);
    text_layer_set_text_color(dateLayer, GColorWhite);
    text_layer_set_font(dateLayer, fonts_load_custom_font(sm_f_handle));
    text_layer_set_text_alignment(dateLayer, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), (Layer*) dateLayer);

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

    //Stop 'all change' on first minute
    MUprev = MUDigit;
    MTprev = MTDigit;
    HUprev = HUDigit;
    HTprev = HTDigit;
}

/**
 * Unload window members
 */
static void window_unload(Window *window) {	
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

static char dateText[] = "Mon 01";

/**
 * Function to get time digits
 */
void getTimeDigits(struct tm *t) 
{
    char txt[] = "00:00";
    strftime(txt, sizeof(txt), clock_is_24h_style() ? "%H:%M" : "%I:%M", t);

    //Get digits
    HTDigit = txt[0] - '0';
    HUDigit = txt[1] - '0';
    MTDigit = txt[3] - '0';
    MUDigit = txt[4] - '0';

    //Get date
    strftime(dateText, sizeof(dateText), "%a %d", t);	//Sun 01
}

/**
 * Function to set the time and date digits on the TextLayers
 */
void setTimeDigits(struct tm *t)
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
    
    //Set date
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

void animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay)
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

