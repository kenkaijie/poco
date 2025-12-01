#include <poco/platform.h>

/**< Every coro yields back to the main fiber (where the scheduler resides) */
static LPVOID main_fiber = NULL;

typedef struct shim {
    void (*entrypoint)(void *, void *);
    void *coro;
    void *user_context;
} Shim;

static void shim_entry(void *context) {
    Shim *shim = context;
    void (*entrypoint)(void *, void *) = shim->entrypoint;
    void *coro = shim->coro;
    void *user_context = shim->user_context;
    free(shim);

    entrypoint(coro, user_context);
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

int platform_make_context(PlatformContext *context, void (*entrypoint)(void *, void *),
                          void *coro, void *user_context) {
    /* This is freed in the shim entrypoint*/
    Shim *shim = malloc(sizeof(Shim));

    if (shim == NULL) {
        /* bad malloc */
        return 1;
    }

    shim->entrypoint = entrypoint;
    shim->coro = coro;
    shim->user_context = user_context;

    context->fiber = CreateFiber(0, shim_entry, shim);

    return (context->fiber == NULL) ? 1 : 0;
}

int platform_destroy_context(PlatformContext *context) {
    if ((context->fiber != NULL) && (context->fiber != main_fiber)) {
        DeleteFiber(context->fiber);
    }
    return 0;
}
