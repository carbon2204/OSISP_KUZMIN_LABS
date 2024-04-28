#pragma once
#include <signal.h>
#include <stddef.h>

void onSignal(int signum, void (*handler)(int)) {
  struct sigaction act = {.sa_flags = 0, .sa_handler = handler};
  sigaction(signum, &act, NULL);
}
