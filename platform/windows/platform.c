#include <poco/platform.h>

/**< Every coro yields back to the main fiber (where the scheduler resides) */
static LPVOID main_fiber = NULL;

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

int platform_get_context(PlatformContext *context) {
    if (main_fiber == NULL) {
        main_fiber = ConvertThreadToFiber(0);
    }
    context->fiber = GetCurrentFiber();
    return ((context->fiber == NULL) || (main_fiber == NULL)) ? 1 : 0;
}

int platform_set_context(PlatformContext *context) {
    SwitchToFiber(context->fiber);
    return 0;
}

int platform_swap_context(PlatformContext *old_context, PlatformContext *new_context) {
    /* Inject the null fiber to become the main fiber */
    if (platform_get_context(old_context) != 0) {
        return -1;
    }

    return platform_set_context(new_context);
}

int platform_make_context(PlatformContext *context, void (*function)(void *, void *),
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

int platform_destroy_context(PlatformContext *context) {
    if ((context->fiber != NULL) && (context->fiber != main_fiber)) {
        DeleteFiber(context->fiber);
    }
    return 0;
}
