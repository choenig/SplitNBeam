#ifndef PEBBLEAPI_H_
#define PEBBLEAPI_H_

#include <pebble.h>

void layer_set_visible(Layer *layer, bool visible)
{
    layer_set_hidden(layer, !visible);
}

static WindowHandlers createWindowHandlers(WindowHandler load, WindowHandler unload)
{
    WindowHandlers wh;
    wh.load   = load;
    wh.unload = unload;
    wh.appear = 0;
    wh.disappear = 0;
    return wh;
}

static AnimationHandlers createAnimationHandlers(AnimationStoppedHandler stopped)
{
    AnimationHandlers ah;
    ah.started = 0;
    ah.stopped = stopped;
    return ah;
}

static void onAnimationStopped(Animation * anim, bool finished, void * context)
{
    (void)finished;
    (void)context;

    //Free the memory used by the Animation
    property_animation_destroy((PropertyAnimation*) anim);
}

static void animateLayer(Layer * layer, GRect start, GRect finish, int duration, int delay)
{
    //Declare animation
    PropertyAnimation * anim = property_animation_create_layer_frame(layer, &start, &finish);

    //Set characteristics
    animation_set_duration((Animation*) anim, duration);
    animation_set_delay((Animation*) anim, delay);
    animation_set_curve((Animation*) anim, AnimationCurveEaseInOut);

    animation_set_handlers((Animation*) anim, createAnimationHandlers(onAnimationStopped), NULL);

    //Start animation!
    animation_schedule((Animation*) anim);
}

#endif
