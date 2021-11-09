#include "effect_deferer.h"

Effect_Deferer_t *queue = NULL;

void Effect_Defer(Effect_t *effect, uint16_t *value, uint16_t mask) {
  Effect_Deferer_t *deferer = calloc(1, sizeof(Effect_Deferer_t));
  deferer->effect = effect,
  deferer->value  = value,
  deferer->mask   = mask;

  deferer->next = queue;
  queue         = deferer;
}

void Effect_QueueDeferred(void) {
  Effect_Deferer_t *deferer = queue;
  while(deferer) {
    if (!deferer->value || (*deferer->value & deferer->mask))
      // Push effect if test is successful
      Effect_Push(deferer->effect);
    else
      // Clear cache if unsuccessful
      deferer->effect->cache = 0;

    deferer = deferer->next;
  }
}