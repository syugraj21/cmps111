/* This file contains the scheduling policy for SCHED
 *
 * The entry points are:
 *   do_noquantum:        Called on behalf of process' that run out of quantum
 *   do_start_scheduling  Request to start scheduling a proc
 *   do_stop_scheduling   Request to stop scheduling a proc
 *   do_nice		  Request to change the nice level on a proc
 *   init_scheduling      Called from main.c to set up/prepare scheduling
 */
#include "sched.h"
#include "schedproc.h"
#include <assert.h>
#include <minix/com.h>
#include <machine/archtypes.h>
#include "kernel/proc.h" /* for queue constants */

PRIVATE timer_t sched_timer;
PRIVATE unsigned balance_timeout;
PRIVATE int total_tickets = 0;
PRIVATE unsigned index_counter = 0;
PRIVATE int *ticket_array;
PRIVATE struct schedproc *winner_proc;

#define BALANCE_TIMEOUT	5 /* how often to balance queues in seconds */
#define MAX_TICKET 100    /* max tickets a process can have */
#define MIN_TICKET 1      /* min tickets a process can have */
#define STR_TICKET 20     /* starting tickets for each process */
#define MED_PRI 13        /* previous winner priority */
#define MAX_PRI 12        /* max priority level allowed for a process */
#define MIN_PRI 14        /* min priority level allowed for a process */
#define DYNAMIC 1         /* macro to switch between dynamic and static */
#define DEBUG 0           /* macro for debug print statements */

FORWARD _PROTOTYPE( int schedule_process, (struct schedproc * rmp)	);
FORWARD _PROTOTYPE( void balance_queues, (struct timer *tp)		);
FORWARD _PROTOTYPE( void do_lottery, (void)                     );
FORWARD _PROTOTYPE( void double_array, (int *a)                 );
FORWARD _PROTOTYPE( void adjust_tickets,(int adj_tick, struct schedproc *rmp));

#define DEFAULT_USER_TIME_SLICE 200

/*===========================================================================*
 *				do_noquantum				     *
 *===========================================================================*/

PUBLIC int do_noquantum(message *m_ptr)
{
	register struct schedproc *rmp;
	int rv, proc_nr_n, process_pri;

	if (sched_isokendpt(m_ptr->m_source, &proc_nr_n) != OK) {
		printf("SCHED: WARNING: got an invalid endpoint in OOQ msg %u.\n",
		m_ptr->m_source);
		return EBADEPT;
	}

	rmp = &schedproc[proc_nr_n];
	process_pri = (int) rmp->priority;
	rmp->priority = (unsigned)MIN_PRI;
	schedule_process(rmp);
	/* for any process that runs out of quantum, its priority
	 * is lowered by 2 tickets
	 */
    if (DYNAMIC) {
        adjust_tickets(-2, rmp);
    }
	switch (process_pri) {
		/*winner process had no quantum message*/	
        case (12):
            if (DEBUG) {
                printf("Got out of Q12\n");
            }
	        do_lottery();
            break;
		/*previous winner had no quantum msg. This implies that
		 *the winner process blocked.
		 */
        case (13):
            if (DEBUG) {
                printf("Got out of Q13\n");
            }
            winner_proc->priority = MED_PRI; /*lower winner priority to previous winner*/
            schedule_process(winner_proc); 
            do_lottery();
            break;
		/* if the winner process pointer is null, we have no winner and we do_lottery().
		 * if its not null the winner blocked, so we move the winner to the previous winner
		 * queue and select a new winner.
		 */
        case (14):
            if (DEBUG) {
                printf("Got out of Q14\n");
            }
            if (winner_proc != NULL) {
                winner_proc->priority = MED_PRI;
                if (DYNAMIC) {
                    adjust_tickets(5, winner_proc);
                }
                schedule_process(winner_proc);
            }
            do_lottery();
            break;
        default:
            break;
	}
	/*if (rmp->priority < MIN_USER_Q) {
		rmp->priority += 1; /* lower priority
	}

	if ((rv = schedule_process(rmp)) != OK) {
		return rv;
	}*/
	return OK;
}

/*===========================================================================*
 *				do_stop_scheduling			     *
 *===========================================================================*/
PUBLIC int do_stop_scheduling(message *m_ptr)
{
	register struct schedproc *rmp;
	int rv, proc_nr_n, index;

	/* check who can send you requests */
	if (!accept_message(m_ptr))
		return EPERM;

	if (sched_isokendpt(m_ptr->SCHEDULING_ENDPOINT, &proc_nr_n) != OK) {
		printf("SCHED: WARNING: got an invalid endpoint in OOQ msg "
		"%ld\n", m_ptr->SCHEDULING_ENDPOINT);
		return EBADEPT;
	}
    
	rmp = &schedproc[proc_nr_n];
	/*remove the process tickets from the total tickets*/
	total_tickets -= (int) rmp->tickets;
	rmp->tickets = (unsigned) 0;
	rmp->flags = 0; /*&= ~IN_USE;*/
	
	return OK;
}

/*===========================================================================*
 *				do_start_scheduling			     *
 *===========================================================================*/
PUBLIC int do_start_scheduling(message *m_ptr)
{
	register struct schedproc *rmp;
	int rv, proc_nr_n, parent_nr_n, nice;
	
	/* we can handle two kinds of messages here */
	assert(m_ptr->m_type == SCHEDULING_START || 
		m_ptr->m_type == SCHEDULING_INHERIT);

	/* check who can send you requests */
	if (!accept_message(m_ptr))
		return EPERM;

	/* Resolve endpoint to proc slot. */
	if ((rv = sched_isemtyendpt(m_ptr->SCHEDULING_ENDPOINT, &proc_nr_n))
			!= OK) {
		return rv;
	}
	rmp = &schedproc[proc_nr_n];

	/* Populate process slot */
	rmp->endpoint     = m_ptr->SCHEDULING_ENDPOINT;
	rmp->parent       = m_ptr->SCHEDULING_PARENT;
	/*rmp->max_priority = (unsigned) m_ptr->SCHEDULING_MAXPRIO;*/
	rmp->max_priority = (unsigned) MAX_PRI; /*set the priorities*/
    rmp->priority = (unsigned)MIN_PRI;
	if (rmp->max_priority >= NR_SCHED_QUEUES) {
		return EINVAL;
	}
	
	switch (m_ptr->m_type) {
	/*
	 * Change max and min priority level
	 */
	case SCHEDULING_START:
		/* We have a special case here for system processes, for which
		 * quanum and priority are set explicitly rather than inherited 
		 * from the parent */
        rmp->max_priority = (unsigned) m_ptr->SCHEDULING_MAXPRIO;
		rmp->priority   = rmp->max_priority;  /*rmp->max_priority;*/
		rmp->time_slice = (unsigned) m_ptr->SCHEDULING_QUANTUM;
		/*rmp->tickets = (unsigned) STR_TICKET;*/
		break;
		
	case SCHEDULING_INHERIT:
		/* Inherit current priority and time slice from parent. Since there
		 * is currently only one scheduler scheduling the whole system, this
		 * value is local and we assert that the parent endpoint is valid */
		if ((rv = sched_isokendpt(m_ptr->SCHEDULING_PARENT,
				&parent_nr_n)) != OK)
			return rv;
        
        /*rmp->max_priority =(unsigned) MAX_PRI;*/
		
		rmp->time_slice = schedproc[parent_nr_n].time_slice;
        rmp->tickets = (unsigned)STR_TICKET;
        total_tickets += STR_TICKET;
		break;
		
	default: 
		/* not reachable */
		assert(0);
	}
    
	/* Take over scheduling the process. The kernel reply message populates
	 * the processes current priority and its time slice */
	if ((rv = sys_schedctl(0, rmp->endpoint, 0, 0)) != OK) {
		printf("Sched: Error taking over scheduling for %d, kernel said %d\n",
			rmp->endpoint, rv);
		return rv;
	}
	rmp->flags = IN_USE;

	/* Schedule the process, giving it some quantum */
	if ((rv = schedule_process(rmp)) != OK) {
		printf("Sched: Error while scheduling process, kernel replied %d\n",
			rv);
		return rv;
	}

	/* Mark ourselves as the new scheduler.
	 * By default, processes are scheduled by the parents scheduler. In case
	 * this scheduler would want to delegate scheduling to another
	 * scheduler, it could do so and then write the endpoint of that
	 * scheduler into SCHEDULING_SCHEDULER
	 */

	m_ptr->SCHEDULING_SCHEDULER = SCHED_PROC_NR;

	return OK;
}

/*===========================================================================*
 *				do_nice					     *
 *===========================================================================*/
PUBLIC int do_nice(message *m_ptr)
{
	struct schedproc *rmp;
	int rv;
	int proc_nr_n;
	unsigned change_ticket, old_q, old_max_q, old_ticket;

	/* check who can send you requests */
	if (!accept_message(m_ptr))
		return EPERM;

	if (sched_isokendpt(m_ptr->SCHEDULING_ENDPOINT, &proc_nr_n) != OK) {
		printf("SCHED: WARNING: got an invalid endpoint in OOQ msg "
		"%ld\n", m_ptr->SCHEDULING_ENDPOINT);
		return EBADEPT;
	}

	rmp = &schedproc[proc_nr_n];
	change_ticket = (unsigned) m_ptr->SCHEDULING_MAXPRIO;
	/*if (new_q >= NR_SCHED_QUEUES) {
		return EINVAL;
	}*/

    /* adjust the ticket values according to the value given by the
	 * msg pointer.*/
    adjust_tickets(change_ticket,rmp);
	
    if (DEBUG) {
        printf("This process has %d tickets: \n", rmp->tickets);
    }
	/* Store old values, in case we need to roll back the changes */
	old_q     = rmp->priority;
	old_max_q = rmp->max_priority;
    old_ticket = rmp->tickets;

	/* Update the proc entry and reschedule the process */
	/*rmp->max_priority = rmp->priority = change_ticket;*/

	if ((rv = schedule_process(rmp)) != OK) {
		/* Something went wrong when rescheduling the process, roll
		 * back the changes to proc struct */
		rmp->priority     = old_q;
		rmp->max_priority = old_max_q;
        rmp->tickets = old_ticket;
	}

	return rv;
}

/*===========================================================================*
 *				schedule_process			     *
 *===========================================================================*/
PRIVATE int schedule_process(struct schedproc * rmp)
{
	int rv;

	if ((rv = sys_schedule(rmp->endpoint, rmp->priority,
			rmp->time_slice)) != OK) {
		printf("SCHED: An error occurred when trying to schedule %d: %d\n",
		rmp->endpoint, rv);
	}

	return rv;
}


/*===========================================================================*
 *				start_scheduling			     *
 *===========================================================================*/

PUBLIC void init_scheduling(void)
{
    winner_proc = NULL; 
	balance_timeout = BALANCE_TIMEOUT * sys_hz();
	init_timer(&sched_timer);
	set_timer(&sched_timer, balance_timeout, balance_queues, 0);
}

/*===========================================================================*
 *				balance_queues				     *
 *===========================================================================*/

/* This function in called every 100 ticks to rebalance the queues. The current
 * scheduler bumps processes down one priority when ever they run out of
 * quantum. This function will find all proccesses that have been bumped down,
 * and pulls them back up. This default policy will soon be changed.
 */
PRIVATE void balance_queues(struct timer *tp)
{
	struct schedproc *rmp;
	int proc_nr;
	int rv;
	/*since this a lottery scheduler, the balance_queues method is not need*/
	 
	 
	/*for (proc_nr=0, rmp=schedproc; proc_nr < NR_PROCS; proc_nr++, rmp++) {
		if (rmp->flags & IN_USE) {
			if (rmp->priority > rmp->max_priority) {
				rmp->priority -= 1; 
				schedule_process(rmp);
			}
		}
	}*/

	set_timer(&sched_timer, balance_timeout, balance_queues, 0);
}

/*==========================================================================*
 *                              do_lottery                                  *
 *==========================================================================*/

/* This function generates a random ticket number from 0 to n-1, and picks
 * a winnign process. The process that wins the lotter is sent to the priority 
 * level 12 (aka the winner queue).
 */
PRIVATE void do_lottery (void) 
{
    struct schedproc *rmp;
	int proc_nr, rv;
	int sum = 0;
    int win_ticket = random() % total_tickets;
    if (DEBUG) {
        printf("Winning ticket: %d\n", win_ticket);
    }
	/* loop over the process's and pick the winning process and start
	 * that winning process
	 */
	for (proc_nr = 0, rmp=schedproc; proc_nr < NR_PROCS; proc_nr++, rmp++) {
	       if (rmp->flags & IN_USE) {
	           sum += (int) rmp->tickets;
	           if (sum > win_ticket) {
			       rmp->priority = (unsigned) MAX_PRI;
			       winner_proc = rmp;
			       schedule_process(rmp);
			       break;
               }
	       }
	}
}


/*==========================================================================*
 *                              adjust_ticket                               *
 *==========================================================================*/
 
/* This function adjusts the amount of tickets for a process.
 * The number of tickets to adjust by is given by adj_tik
 */
PRIVATE void adjust_tickets (int adj_tik, struct schedproc *rmp) 
{
	int ticket_nr = (int)rmp->tickets;      
	int new_ticket = ticket_nr + adj_tik;   
	int temp;
	/* check specail case to make sure the added ticket
	 * value is never below 1 or greater than 100. This
	 * also changes the total_ticket variable. 
	 */
	if (new_ticket <= 0) {                  
	     total_tickets -= ticket_nr;
	     ++total_tickets;
	    rmp->tickets = (unsigned) MIN_TICKET; 
	} else if (new_ticket > 100) {
	    temp = 100 - ticket_nr;
	    total_tickets += temp;
	    rmp->tickets = (unsigned) MAX_TICKET;
	} else {
	    total_tickets += adj_tik;
	    rmp->tickets = (unsigned) new_ticket;
	}
     
}
