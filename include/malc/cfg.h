#ifndef __MALC_CFG_H__
#define __MALC_CFG_H__

#include <bl/base/platform.h>
#include <bl/base/integer.h>

/*------------------------------------------------------------------------------
idle_task_period_us:
  The IDLE task will be run with this periodicity. This means that calls to
  "malc_run_idle_task" won't do nothing until the period is expired.
start_own_thread:
  When this is set the "malc" will launch and own a dedicated thread for the
  consumer. If this is unset the logger is run from an user thread  by using
  "malc_run_consume_task" and "malc_run_idle_task".
------------------------------------------------------------------------------*/
typedef struct malc_worker_cfg {
  u32  idle_task_period_us;
  bool start_own_thread;
}
malc_worker_cfg;
/*------------------------------------------------------------------------------
timestamp:
  Timestamp at the producer side. It's slower but more precise. In general if
  you can't tolerate ~10ms jitter on the logging timestamp you should set this
  at the expense of performance.
can_use_heap:
  Each producer has a buffer in TLS, when it's exhausted and this variable is
  set the heap is used until TLS buffers start to be available.
block_on_empty_tls_buffer:
  When this is set and "can_use_heap" is not set the call will block until TLS
  buffers are available. If both "can_use_heap" and "block_on_empty_tls_buffer"
  are unset the logging call won't block and will return "bl_would_overflow".
------------------------------------------------------------------------------*/
typedef struct malc_producer_cfg {
  bool timestamp;
  bool can_use_heap;
  bool block_on_empty_tls_buffer;
}
malc_producer_cfg;
/*------------------------------------------------------------------------------
sanitize_log_entries:
  The log entries are removed from any character that may make them to be
  confused with another log line, e.g. newline, so log injection isn't possible.

log_rate_filter_time_us:
log_rate_filter_max:
log_rate_filter_watch_count:
  These three parameters are better explained all in once.

  A hacked application may try to erase data logs by running a legitimate piece
  of code of the application in a loop, either to make the server disk to be
  full (no rotation in place) or to erase attack traces. These parameters try to
  limit messages with an excessive data rate.

  A buffer with the last log entries and its timestamps is kept. If one of these
  entries repeats too often it will be silently dropped.

  "log_rate_filter_time_us" and "log_rate_filter_max" control how many log
  entries can happen in a unit of time before the protection being activated.

  "log_rate_filter_watch_count" controls how many of the last log entries are
  watched (circular buffer).
------------------------------------------------------------------------------*/
typedef struct malc_security {
  bool sanitize_log_entries;
  u32  log_rate_filter_time_us;
  u32  log_rate_filter_max;
  u32  log_rate_filter_watch_count;
}
malc_security;
/*----------------------------------------------------------------------------*/
typedef struct malc_cfg {
  malc_worker_cfg   worker;
  malc_security     sec;
  malc_producer_cfg producer;
}
malc_cfg;
/*----------------------------------------------------------------------------*/

#endif /* __MALC_CFG_H__ */
