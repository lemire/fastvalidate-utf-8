// https://github.com/WojciechMula/toys/blob/master/000helpers/linux-perf-events.h
#pragma once
#ifdef __linux__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* See feature_test_macros(7) */
#endif
#include <asm/unistd.h> // for __NR_perf_event_open
#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h> // for perf event constants
#include <sys/ioctl.h>        // for ioctl
#include <sys/syscall.h>      /* For SYS_xxx definitions */
#include <unistd.h>
#include <unistd.h> // for syscall

struct LinuxEvents {

  int fd;
  struct perf_event_attr attribs;
};

void LinuxEvents_init(struct LinuxEvents *L) {
  L->fd = 0;
  memset(&L->attribs, 0, sizeof(L->attribs));
  (L->attribs).type = PERF_TYPE_HARDWARE;
  (L->attribs).size = sizeof(L->attribs);
  (L->attribs).config = PERF_COUNT_HW_CPU_CYCLES;
  (L->attribs).disabled = 1;
  (L->attribs).exclude_kernel = 1;
  (L->attribs).exclude_hv = 1;

  const int pid = 0;    // the current process
  const int cpu = -1;   // all CPUs
  const int group = -1; // no group
  const unsigned long flags = 0;
  L->fd = syscall(__NR_perf_event_open, &(L->attribs), pid, cpu, group, flags);
  if (L->fd == -1) {
    printf("perf_event_open");
  }
}
void LinuxEvents_close(struct LinuxEvents *L) { close(L->fd); }

void LinuxEvents_start(struct LinuxEvents *L) {
  if (ioctl(L->fd, PERF_EVENT_IOC_RESET, 0) == -1) {
    printf("ioctl(PERF_EVENT_IOC_RESET)");
  }

  if (ioctl(L->fd, PERF_EVENT_IOC_ENABLE, 0) == -1) {
    printf("ioctl(PERF_EVENT_IOC_ENABLE)");
  }
}

unsigned long LinuxEvents_end(struct LinuxEvents *L) {
  if (ioctl(L->fd, PERF_EVENT_IOC_DISABLE, 0) == -1) {
    printf("ioctl(PERF_EVENT_IOC_DISABLE)");
  }

  unsigned long result;
  if (read(L->fd, &result, sizeof(result)) == -1) {
    printf("read");
  }

  return result;
}

#endif
