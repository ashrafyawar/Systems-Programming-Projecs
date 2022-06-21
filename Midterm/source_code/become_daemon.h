#ifndef BECOME_DAEMON_H
#define BECOME_DAEMON_H

#define BD_NO_CHDIR             01
#define BD_NO_CLOSE_FILES       02
#define BD_NO_REOPEN_STD_FDS    04

#define BD_NO_UMASK0            010
#define BD_MAX_CLOSE            8192

#include <signal.h>

int logFd;
void sighub_handler(int);
void sigterm_handler(int);

int become_daemon();

extern volatile sig_atomic_t sigterm;
extern volatile sig_atomic_t sighub;
#endif