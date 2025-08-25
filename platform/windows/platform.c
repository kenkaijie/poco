#include <poco/platform.h>

/**< Every coro yields back to the main fiber (where the scheduler resides) */
static void *main_fiber = NULL;

typedef struct _shim {
    void (*function)(void *, void *);
    void *context1;
    void *context2;
} _shim_t;

static void _shim_entry(void *context) {
    _shim_t *shim = (_shim_t *)context;
    void (*function)(void *, void *) = shim->function;
    void *context1 = shim->context1;
    void *context2 = shim->context2;
    free(shim);

    function(context1, context2);
}

int platform_get_context(platform_context_t *context) {
    if (main_fiber == NULL) {
        main_fiber = ConvertThreadToFiber(0);
    }
    context->fiber = GetCurrentFiber();
    return (context->fiber == NULL) ? 1 : 0;
}

int platform_set_context(platform_context_t *context) {
    SwitchToFiber(context->fiber);
    return 0;
}

int platform_swap_context(platform_context_t *old_context,
                          platform_context_t *new_context) {
    if (new_context->fiber != NULL) {
        SwitchToFiber(new_context->fiber);
    } else {
        SwitchToFiber(main_fiber);
    }
    return 0;
}

int platform_make_context(platform_context_t *context, void (*function)(void *, void *),
                          void *coro, void *user_context) {
    _shim_t *shim = malloc(sizeof(_shim_t));

    if (shim == NULL) {
        /* bad malloc */
        return 1;
    }

    shim->function = function;
    shim->context1 = coro;
    shim->context2 = user_context;

    context->fiber = CreateFiber(0, _shim_entry, shim);

    return (context->fiber == NULL) ? 1 : 0;
}

int platform_destroy_context(platform_context_t *context) {
    if (context->fiber != NULL) {
        DeleteFiber(context->fiber);
    }
    return 0;
}
