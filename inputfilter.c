#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

static int allowed_vendor = -1;
static int allowed_product = -1;
static int initialized = 0;

typedef int (*open_func_t)(const char*, int, ...);

static open_func_t real_open = NULL;

static void ensure_real_open() {
    if (!real_open)
        real_open = (open_func_t)dlsym(RTLD_NEXT, "open");
}

static void init_whitelist() {
    if (initialized) return;
    initialized = 1;
    const char *env = getenv("SDL_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT");
    if (!env) return;
    // Expected form: 0xABCD/0x1234
    unsigned vid, pid;
    if (sscanf(env, "0x%x/0x%x", &vid, &pid) == 2) {
        allowed_vendor = vid;
        allowed_product = pid;
    }
}

static int check_device(const char *path) {
    if (allowed_vendor < 0 || allowed_product < 0) return 0; // No whitelist set, allow all
    ensure_real_open();
    int fd = real_open(path, O_RDONLY | O_NONBLOCK);
    if (fd < 0) return 0; // Can't open for checking, allow
    struct input_id id;
    int ok = 0;
    if (ioctl(fd, EVIOCGID, &id) == 0) {
        if (id.vendor == allowed_vendor && id.product == allowed_product) {
            ok = 1;
        }
    }
    close(fd);
    return ok;
}


int open(const char *pathname, int flags, ...) {
    init_whitelist();
    ensure_real_open();

    // Only check regular open on /dev/input devices
    if (strncmp(pathname, "/dev/input", 10) == 0) {
        if (!check_device(pathname)) {
            errno = EACCES;
            return -1;
        }
    }

    va_list args;
    va_start(args, flags);
    int mode = va_arg(args, int);
    va_end(args);
    return real_open(pathname, flags, mode);
}

// For open64
int open64(const char *pathname, int flags, ...) {
    init_whitelist();
    static open_func_t real_open64 = NULL;
    if (!real_open64) real_open64 = dlsym(RTLD_NEXT, "open64");

    if (strncmp(pathname, "/dev/input", 10) == 0) {
        if (!check_device(pathname)) {
            errno = EACCES;
            return -1;
        }
    }

    va_list args;
    va_start(args, flags);
    int mode = va_arg(args, int);
    va_end(args);
    return real_open64(pathname, flags, mode);
}
