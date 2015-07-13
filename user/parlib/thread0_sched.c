/* Copyright (c) 2015 Google, Inc.
 * Barret Rhoden <brho@cs.berkeley.edu>
 * See LICENSE for details.
 *
 * thread0_sched: a basic scheduler for thread0, used by SCPs without a
 * multithreaded 2LS linked in.
 *
 * This is closely coupled with uthread.c */

#include <ros/arch/membar.h>
#include <parlib/arch/atomic.h>
#include <parlib/parlib.h>
#include <parlib/vcore.h>
#include <parlib/uthread.h>
#include <parlib/event.h>
#include <stdlib.h>

static void thread0_sched_entry(void);
static void thread0_thread_blockon_sysc(struct uthread *uthread, void *sysc);

/* externed into uthread.c */
struct schedule_ops thread0_2ls_ops = {
	.sched_entry = thread0_sched_entry,
	.thread_blockon_sysc = thread0_thread_blockon_sysc,
};

/* externed into uthread.c */
struct uthread *thread0_uth;

/* Thread0 scheduler ops (for processes that haven't linked in a full 2LS) */
static void thread0_sched_entry(void)
{
	if (current_uthread)
		run_current_uthread();
	else
		run_uthread(thread0_uth);
}

static void thread0_thread_blockon_sysc(struct uthread *uthread, void *arg)
{
	struct syscall *sysc = (struct syscall*)arg;
	/* We're in vcore context.  Regardless of what we do here, we'll pop back in
	 * to vcore entry, just like with any uthread_yield.  We don't have a 2LS,
	 * but we always have one uthread: the SCP's thread0.  Note that at this
	 * point, current_uthread is still set, but will be cleared as soon as the
	 * callback returns (and before we start over in vcore_entry).
	 *
	 * If notif_pending is already set (due to a concurrent signal), we'll fail
	 * to yield.  Once in VC ctx, we'll handle any other signals/events that
	 * arrived, then restart the uthread that issued the syscall, which if the
	 * syscall isn't done yet, will just blockon again.
	 *
	 * The one trick is that we don't want to register the evq twice.  The way
	 * register_evq currently works, if a SC completed (SC_DONE) while we were
	 * registering, we could end up clearing sysc->ev_q before the kernel sees
	 * it.  We'll use u_data to track whether we registered or not. */
	#define U_DATA_BLOB ((void*)0x55555555)
	if ((sysc->u_data == U_DATA_BLOB)
	    || register_evq(sysc, &__ros_scp_simple_evq)) {
		sysc->u_data = U_DATA_BLOB;
		/* Sending false for now - we want to signal proc code that we want to
		 * wait (piggybacking on the MCP meaning of this variable).  If
		 * notif_pending is set, the kernel will immediately return us. */
		__ros_syscall_noerrno(SYS_yield, FALSE, 0, 0, 0, 0, 0);
	}
}
