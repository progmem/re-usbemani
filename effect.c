#include "effect.h"
/* Effects */
// Single, calls the color provider once
void _call_Effect_Single(Effect_t *self) {
  RGB_Color_t color = ColorProvider_Get(self->provider);
  RGB_SetRange(color, self->index, self->size);
}
Effect_t *Effect_Single(ColorProvider_t *color, uint8_t index, uint8_t size) {
  Effect_t *result = calloc(1, sizeof(Effect_t));

  result->draw      = _call_Effect_Single;
  result->provider  = color;
  result->index     = index;
  result->size      = size;

  return result;
}

// Multi, calls the color provider for every pixel
void _call_Effect_Multi(Effect_t *self) {
  uint8_t index = self->index;
  uint8_t size  = self->size;

  while(size) {
    RGB_Color_t color = ColorProvider_Get(self->provider);
    RGB_SetRange(color, index, 1);
    index++;
    size--;
  }
}
Effect_t *Effect_Multi(ColorProvider_t *color, uint8_t index, uint8_t size) {
  Effect_t *result = calloc(1, sizeof(Effect_t));

  result->draw      = _call_Effect_Multi;
  result->provider  = color;
  result->index     = index;
  result->size      = size;

  return result;
}

// Press: Call the color provider once when pressed. Use the result while holding.
void _call_Effect_Press(Effect_t *self) {
  if (!self->cache8[0]) {
    // Call the color provider to grab the next color
    ColorProvider_Get(self->provider);
    ++self->cache8[0];
  }

  // Use the cached color
  RGB_Color_t color = HSV_ToRGB(self->provider->color);
  RGB_SetRange(color, self->index, self->size);
}
Effect_t *Effect_Press(ColorProvider_t *color, uint8_t index, uint8_t size) {
  Effect_t *result = calloc(1, sizeof(Effect_t));

  result->draw      = _call_Effect_Press;
  result->provider  = color;
  result->index     = index;
  result->size      = size;

  return result;
}

// Splash: Calls the color provider once. Draws when held. Spawns an effect on press.
//// Child: Echo outwards, dimming until no longer bright
uint8_t splash_fade_rate = 5;
uint8_t splash_bounds_left  = 0;
uint8_t splash_bounds_right = 255;

void _call_Effect_SplashChild(Effect_t *self) {
  uint8_t  *left_index = &(self->index);
  uint8_t *right_index = &(self->cache8[0]);

  // Decrement both indexes unless it would cause it to go out of bounds
  RGB_Color_t color = HSV_ToRGB(self->provider->color);

  if (*left_index > splash_bounds_left) {
    (*left_index)--;
    RGB_AddRange(color, *left_index, self->size);
  }
  if (*right_index < splash_bounds_right) {
    (*right_index)++;
    RGB_AddRange(color, *right_index, self->size);
  }
  // Dim after every draw
  HSV_Dim(&self->provider->color, splash_fade_rate);
  // If no longer lit, cleanup.
  if (!self->provider->color.val) {
    free(self->provider);
    free(self);
    return;
  }
  // Queue as long as the effect needs to persist
  Effect_Push(self);
}
Effect_t *Effect_SplashChild(ColorProvider_t *color, uint8_t index, uint8_t size) {
  Effect_t *result = calloc(1, sizeof(Effect_t));

  result->draw      = _call_Effect_SplashChild;
  result->provider  = color;
  result->index     = index;
  result->size      = 1;

  // Setup the right index as well
  uint8_t *right_index = &(result->cache8[0]);
  *right_index = (index + size) - result->size;

  return result;
}
void Effect_SetSplashFadeRate(uint8_t rate) {
  splash_fade_rate = rate;
}
void Effect_SetSplashBounds(uint8_t start, uint8_t end) {
  splash_bounds_left  = start;
  splash_bounds_right = end;
}

//// Parent: Spawn child on press. Light on hold.
void _call_Effect_Splash(Effect_t *self) {
  if (!self->cache8[0]) {
    // Grab color
    ColorProvider_Get(self->provider);
    // Spawn the effect
    ColorProvider_t *child_cp = ColorProvider_Hue(self->provider->color.hue);
    Effect_t        *child    = Effect_SplashChild(child_cp, self->index, self->size);
    Effect_Shift(child);
    // Flag the cache so we don't run this again
    ++self->cache8[0];
  }
  // Draw the hold
  RGB_Color_t color = HSV_ToRGB(self->provider->color);
  RGB_SetRange(color, self->index, self->size);
}
Effect_t *Effect_Splash(ColorProvider_t *color, uint8_t index, uint8_t size) {
  Effect_t *result = calloc(1, sizeof(Effect_t));

  result->draw      = _call_Effect_Splash;
  result->provider  = color;
  result->index     = index;
  result->size      = size;

  return result;
}

// SingleShot: Display the color only once until reset.
void _call_Effect_SingleShot(Effect_t *self) {
  if (!self->cache8[0]) {
    RGB_Color_t color = ColorProvider_Get(self->provider);
    RGB_SetRange(color, self->index, self->size);
    ++self->cache8[0];
  }
}
Effect_t *Effect_SingleShot(ColorProvider_t *color, uint8_t index, uint8_t size) {
  Effect_t *result = calloc(1, sizeof(Effect_t));

  result->draw      = _call_Effect_SingleShot;
  result->provider  = color;
  result->index     = index;
  result->size      = size;

  return result;
}


/* Effect Queue */
// Start and end of the queue
Effect_t *head = NULL, *tail = NULL;

inline bool Effect_Queued(Effect_t *effect) {
  // If the effect has an effect after it, it's queued
  if (effect->next) return true;
  // Otherwise, check the end of the queue
  return (tail == effect);
}

void Effect_Shift(Effect_t *effect) {
  if (!effect) return;
  if (Effect_Queued(effect)) return;

  effect->next = head;
  head = effect;

  if (!tail) tail = head;
}

void Effect_Push(Effect_t *effect) {
  if (!effect) return;
  if (Effect_Queued(effect)) return;

  if (!head) {
    head = tail = effect;
    return;
  }
  tail->next = effect;
  tail = effect;
}

void Effect_Run(void) {
  Effect_t *effect = head;
  head = tail = NULL;

  while (effect) {
    Effect_t *next = effect->next;
    effect->next = NULL;
    // Draw the effect
    if (effect->draw)
      effect->draw(effect);
    // Advance to the next effect
    effect = next;
  }
}