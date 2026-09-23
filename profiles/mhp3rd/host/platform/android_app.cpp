// The entry point of the Android app. SDLActivity loads libmain.so and calls
// SDL_main on a Java thread whose stack is far too small for chained AOT
// calls, and an app cannot restart itself with a larger one the way the Linux
// executable does. So the game runs on a thread of its own with the 64 MiB
// stack the other platforms give their main thread.

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <pthread.h>

#include <cstdio>
#include <string>

int yakumo_main(int argc, char **argv);

namespace {

constexpr std::size_t kGameStackBytes = 64u * 1024u * 1024u;

struct Launch {
    int argc;
    char **argv;
    int result;
};

void *run_game(void *argument) {
    auto *launch = static_cast<Launch *>(argument);
    launch->result = yakumo_main(launch->argc, launch->argv);
    return nullptr;
}

// An app has no terminal: what the program prints goes to yakumo.log in its
// internal storage, where `adb shell run-as` can read it.
void redirect_output() {
    const char *storage = SDL_GetAndroidInternalStoragePath();
    if (storage == nullptr) return;
    const std::string log = std::string(storage) + "/yakumo.log";
    if (std::freopen(log.c_str(), "w", stdout) != nullptr) std::setvbuf(stdout, nullptr, _IOLBF, 0);
    if (std::freopen(log.c_str(), "a", stderr) != nullptr) std::setvbuf(stderr, nullptr, _IONBF, 0);
}

} // namespace

int main(int argc, char *argv[]) {
    redirect_output();
    Launch launch{argc, argv, 1};
    pthread_attr_t attributes;
    pthread_attr_init(&attributes);
    pthread_attr_setstacksize(&attributes, kGameStackBytes);
    pthread_t thread;
    if (pthread_create(&thread, &attributes, run_game, &launch) != 0) {
        std::fprintf(stderr, "Yakumo: cannot start the game thread with a %zu MiB stack\n", kGameStackBytes >> 20);
        pthread_attr_destroy(&attributes);
        return 1;
    }
    pthread_attr_destroy(&attributes);
    pthread_join(thread, nullptr);
    std::fflush(stdout);
    return launch.result;
}
