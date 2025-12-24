#include "../includes/Signal.hpp"

volatile sig_atomic_t g_running = 1;

void handleSignal(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        g_running = 0;
    }
}