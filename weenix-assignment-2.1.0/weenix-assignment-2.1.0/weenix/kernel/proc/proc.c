/******************************************************************************/
/* Important CSCI 402 usage information:                                      */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove this comment block from this file.    */
/******************************************************************************/

#include "kernel.h"
#include "config.h"
#include "globals.h"
#include "errno.h"

#include "util/debug.h"
#include "util/list.h"
#include "util/string.h"
#include "util/printf.h"

#include "proc/kthread.h"
#include "proc/proc.h"
#include "proc/sched.h"
#include "proc/proc.h"

#include "mm/slab.h"
#include "mm/page.h"
#include "mm/mmobj.h"
#include "mm/mm.h"
#include "mm/mman.h"

#include "vm/vmmap.h"

#include "fs/vfs.h"
#include "fs/vfs_syscall.h"
#include "fs/vnode.h"
#include "fs/file.h"

proc_t *curproc = NULL; /* global */
static slab_allocator_t *proc_allocator = NULL;

static list_t _proc_list;
static proc_t *proc_initproc = NULL; /* Pointer to the init process (PID 1) */

void
proc_init()
{
        list_init(&_proc_list);
        proc_allocator = slab_allocator_create("proc", sizeof(proc_t));
        KASSERT(proc_allocator != NULL);
}

proc_t *
proc_lookup(int pid)
{
        proc_t *p;
        list_iterate_begin(&_proc_list, p, proc_t, p_list_link) {
                if (p->p_pid == pid) {
                        return p;
                }
        } list_iterate_end();
        return NULL;
}

list_t *
proc_list()
{
        return &_proc_list;
}

static pid_t next_pid = 0;

/**
 * Returns the next available PID.
 *
 * Note: Where n is the number of running processes, this algorithm is
 * worst case O(n^2). As long as PIDs never wrap around it is O(n).
 *
 * @return the next available PID
 */
static int
_proc_getid()
{
        proc_t *p;
        pid_t pid = next_pid;
        while (1) {
failed:
                list_iterate_begin(&_proc_list, p, proc_t, p_list_link) {
                        if (p->p_pid == pid) {
                                if ((pid = (pid + 1) % PROC_MAX_COUNT) == next_pid) {
                                        return -1;
                                } else {
                                        goto failed;
                                }
                        }
                } list_iterate_end();
                next_pid = (pid + 1) % PROC_MAX_COUNT;
                return pid;
        }
}

/*
 * The new process, although it isn't really running since it has no
 * threads, should be in the PROC_RUNNING state.
 *
 * Don't forget to set proc_initproc when you create the init
 * process. You will need to be able to reference the init process
 * when reparenting processes to the init process.
 */
proc_t *
proc_create(char *name)
{
        /*  NOT_YET_IMPLEMENTED("PROCS: proc_create");*/
        proc_t *proc = slab_obj_alloc(proc_allocator);
        KASSERT(proc != NULL);
        memset(proc, 0, sizeof(proc_t));
        strcpy(proc->p_comm, name);
        list_insert_tail(&_proc_list, &proc->p_list_link);
        if(next_pid == 0){
            proc->p_pid = 0;
            next_pid++;   
        }else 
            proc->p_pid = _proc_getid();
        if(proc->p_pid == PID_INIT)
        proc_initproc = proc;
        KASSERT(proc->p_pid != -1 && "wrong allocating pid for process");
        proc->p_status = PROC_RUNNING;
        proc->p_pagedir = pt_create_pagedir();
        return proc;
        /*  return NULL;*/

}

/**
 * Cleans up as much as the process as can be done from within the
 * process. This involves:
 *    - Closing all open files (VFS)
 *    - Cleaning up VM mappings (VM)
 *    - Waking up its parent if it is waiting
 *    - Reparenting any children to the init process
 *    - Setting its status and state appropriately
 *
 * The parent will finish destroying the process within do_waitpid (make
 * sure you understand why it cannot be done here). Until the parent
 * finishes destroying it, the process is informally called a 'zombie'
 * process.
 *
 * This is also where any children of the current process should be
 * reparented to the init process (unless, of course, the current
 * process is the init process. However, the init process should not
 * have any children at the time it exits).
 *
 * Note: You do _NOT_ have to special case the idle process. It should
 * never exit this way.
 *
 * @param status the status to exit the process with
 */
void
proc_cleanup(int status)
{
        NOT_YET_IMPLEMENTED("PROCS: proc_cleanup");
}

/*
 * This has nothing to do with signals and kill(1).
 *
 * Calling this on the current process is equivalent to calling
 * do_exit().
 *
 * In Weenix, this is only called from proc_kill_all.
 */
void
proc_kill(proc_t *p, int status)
{
     //   NOT_YET_IMPLEMENTED("PROCS: proc_kill");
      /*NOT_YET_IMPLEMENTED("PROCS: proc_kill");*/
	/* Call proc_cleanup() here to clean the PCB and make it a Zombie*/
	/*clean the PCB expect for p_pid and return value(or status code)*/

	/*KASSERT(1 < p->p_pid);*/
	
        dbg(DBG_PROC,"Killing Process with PID=%d\n",p->p_pid);
	
	/*remove the files*/
	int i;
	for(i=0;i<NFILES;i++)
        {
              if(p->p_files[i]!=NULL)
              {
                 int fd=do_close(i);
                 KASSERT(fd!=-1);
              }
        }
	
	
	/* signalling waiting parent process*/
	KASSERT(NULL != p->p_pproc);
	sched_wakeup_on(&p->p_pproc->p_wait);
	kthread_t *kthr;
	list_iterate_begin(&(p->p_threads), kthr, kthread_t, kt_plink)
        {
               /* while(KT_EXITED !=kthr->kt_state)*/
                {
                        kthread_cancel(kthr,(void*)status);
                        sched_make_runnable(curthr);
        	        sched_switch();
                }
        }list_iterate_end();
	
	p->p_state = PROC_DEAD;
	p->p_status = status;
	sched_wakeup_on(&p->p_wait);/*As Now it has no child left*/
	/*dbg(DBG_PROC,"Killing Process with PID=%d Complete\n",p->p_pid);*/
}
}

/*
 * Remember, proc_kill on the current process will _NOT_ return.
 * Don't kill direct children of the idle process.
 *
 * In Weenix, this is only called by sys_halt.
 */
void
proc_kill_all()
{
      //  NOT_YET_IMPLEMENTED("PROCS: proc_kill_all");
      dbg(DBG_PROC,"In KILL_all\n");	
	list_t *processList = &_proc_list;	
	if(!list_empty(processList)){
		/*iterate over each element and call proc_kill()*/
		proc_t *process1;		
		list_iterate_begin(processList,process1,proc_t,p_list_link){
			/* if parent id == idle process and it is not itself the
			 idle processthen skip that process*/
			if(process1->p_pid != PID_IDLE)
			{
			if(process1->p_pproc->p_pid != PID_IDLE&&process1!=curproc)
			{ 
			dbg(DBG_PROC,"Killing Process with PID=%d\n",process1->p_pid);	
			proc_kill(process1,process1->p_status);
		        }
		        }
		}list_iterate_end();
	}
	if((curproc->p_pid != PID_IDLE) && (curproc->p_pproc->p_pid != PID_IDLE))
                proc_kill(curproc, NULL);
}

/*
 * This function is only called from kthread_exit.
 *
 * Unless you are implementing MTP, this just means that the process
 * needs to be cleaned up and a new thread needs to be scheduled to
 * run. If you are implementing MTP, a single thread exiting does not
 * necessarily mean that the process should be exited.
 */
void
proc_thread_exited(void *retval)
{
     //   NOT_YET_IMPLEMENTED("PROCS: proc_thread_exited");
      proc_cleanup((int)retval);
/*        dbg_print("\n came back from cleanup\n");
        dbg_print("\n calling switch \n");*/
	sched_switch();
}

/* If pid is -1 dispose of one of the exited children of the current
 * process and return its exit status in the status argument, or if
 * all children of this process are still running, then this function
 * blocks on its own p_wait queue until one exits.
 *
 * If pid is greater than 0 and the given pid is a child of the
 * current process then wait for the given pid to exit and dispose
 * of it.
 *
 * If the current process has no children, or the given pid is not
 * a child of the current process return -ECHILD.
 *
 * Pids other than -1 and positive numbers are not supported.
 * Options other than 0 are not supported.
 */
pid_t
do_waitpid(pid_t pid, int options, int *status)
{
  //      NOT_YET_IMPLEMENTED("PROCS: do_waitpid");
    int i=0;
        int closed_pid=-ECHILD;
        proc_t *p;
        kthread_t *cur_proc_thd;
        KASSERT(options == 0);
        KASSERT(curproc!=NULL);
        /*dbg(DBG_PROC,"Process with PID=%d waiting for childs to die\n",curproc->p_pid);*/
        if(list_empty(&(curproc->p_children)))
        { 
               
                return -ECHILD;
        }
        else
        {
                if(pid==-1)
                {
                wait_to_die1:
                        if(list_empty(&(curproc->p_children)))
                                goto END;
                        list_iterate_begin(&(curproc->p_children), p,proc_t,p_child_link)
                        {
                                KASSERT(p!=NULL);
                                if(p->p_state==PROC_DEAD)
                                {
                                        KASSERT(-1 == pid || p->p_pid == pid);                          
                                       
                                        if(curproc->p_pid==0)
                                        {
                                                if(p->p_pid==2)
                                                {
                                                        closed_pid=p->p_pid;
                                                        goto END;
                                                }
                                        }
                                        i=1;
                                       
                                        closed_pid=p->p_pid;
                                        *status=(p->p_status);
                                        
                                        list_remove(&(p->p_list_link));
                                        list_iterate_begin(&(p->p_threads), cur_proc_thd, kthread_t, kt_plink)
                                        {
                                                KASSERT(KT_EXITED == cur_proc_thd->kt_state);
                                                kthread_destroy(cur_proc_thd);
                                        } list_iterate_end();
					KASSERT(NULL != p->p_pagedir);
                                        pt_destroy_pagedir(p->p_pagedir);
                                        list_remove(&(p->p_child_link));
                                        slab_obj_free(proc_allocator,p);
                                        goto END;
                                }
                        }list_iterate_end();
                        if(i==0)
                        {
                               sched_sleep_on(&(curproc->p_wait));
                               goto wait_to_die1;
                        }
                }
                else if(pid>0)
                {
                       list_iterate_begin(&(curproc->p_children), p,proc_t,p_child_link)
                       {
                                if(p->p_pid==pid) 
                                {       
                                        i=1;
                                
                                wait_to_die2:                    
                                        if (p->p_state == PROC_DEAD)
                                        {
                                                KASSERT(-1 == pid || p->p_pid == pid);
                                                closed_pid=p->p_pid;
                                                *status=(p->p_status);
                                                list_remove(&(p->p_list_link));
                                                list_iterate_begin(&(p->p_threads), cur_proc_thd, kthread_t, kt_plink)
                                                {
                                                        KASSERT(KT_EXITED == cur_proc_thd->kt_state);
							kthread_destroy(cur_proc_thd);
                                                } list_iterate_end();
						KASSERT(NULL != p->p_pagedir);
						
                                                
                                                pt_destroy_pagedir(p->p_pagedir);
                                                list_remove(&(p->p_child_link));
                                                slab_obj_free(proc_allocator,p);
                                                goto END;  
                                        }
                                        else
                                        {
                                                sched_sleep_on(&(curproc->p_wait));
                                                goto wait_to_die2;
                                        }
                                }
                      }list_iterate_end();
                      if(i==0)
                                return -ECHILD;    
                 }
                 else
                        KASSERT((int)pid!=0);
         }
                                       
         /*NOT_YET_IMPLEMENTED("PROCS: do_waitpid");*/
     END:    return closed_pid;
        return 0;
}

/*
 * Cancel all threads, join with them, and exit from the current
 * thread.
 *
 * @param status the exit status of the process
 */
void
do_exit(int status)
{
    //    NOT_YET_IMPLEMENTED("PROCS: do_exit");
            proc_kill(curproc,status);

}

size_t
proc_info(const void *arg, char *buf, size_t osize)
{
        const proc_t *p = (proc_t *) arg;
        size_t size = osize;
        proc_t *child;

        KASSERT(NULL != p);
        KASSERT(NULL != buf);

        iprintf(&buf, &size, "pid:          %i\n", p->p_pid);
        iprintf(&buf, &size, "name:         %s\n", p->p_comm);
        if (NULL != p->p_pproc) {
                iprintf(&buf, &size, "parent:       %i (%s)\n",
                        p->p_pproc->p_pid, p->p_pproc->p_comm);
        } else {
                iprintf(&buf, &size, "parent:       -\n");
        }

#ifdef __MTP__
        int count = 0;
        kthread_t *kthr;
        list_iterate_begin(&p->p_threads, kthr, kthread_t, kt_plink) {
                ++count;
        } list_iterate_end();
        iprintf(&buf, &size, "thread count: %i\n", count);
#endif

        if (list_empty(&p->p_children)) {
                iprintf(&buf, &size, "children:     -\n");
        } else {
                iprintf(&buf, &size, "children:\n");
        }
        list_iterate_begin(&p->p_children, child, proc_t, p_child_link) {
                iprintf(&buf, &size, "     %i (%s)\n", child->p_pid, child->p_comm);
        } list_iterate_end();

        iprintf(&buf, &size, "status:       %i\n", p->p_status);
        iprintf(&buf, &size, "state:        %i\n", p->p_state);

#ifdef __VFS__
#ifdef __GETCWD__
        if (NULL != p->p_cwd) {
                char cwd[256];
                lookup_dirpath(p->p_cwd, cwd, sizeof(cwd));
                iprintf(&buf, &size, "cwd:          %-s\n", cwd);
        } else {
                iprintf(&buf, &size, "cwd:          -\n");
        }
#endif /* __GETCWD__ */
#endif

#ifdef __VM__
        iprintf(&buf, &size, "start brk:    0x%p\n", p->p_start_brk);
        iprintf(&buf, &size, "brk:          0x%p\n", p->p_brk);
#endif

        return size;
}

size_t
proc_list_info(const void *arg, char *buf, size_t osize)
{
        size_t size = osize;
        proc_t *p;

        KASSERT(NULL == arg);
        KASSERT(NULL != buf);

#if defined(__VFS__) && defined(__GETCWD__)
        iprintf(&buf, &size, "%5s %-13s %-18s %-s\n", "PID", "NAME", "PARENT", "CWD");
#else
        iprintf(&buf, &size, "%5s %-13s %-s\n", "PID", "NAME", "PARENT");
#endif

        list_iterate_begin(&_proc_list, p, proc_t, p_list_link) {
                char parent[64];
                if (NULL != p->p_pproc) {
                        snprintf(parent, sizeof(parent),
                                 "%3i (%s)", p->p_pproc->p_pid, p->p_pproc->p_comm);
                } else {
                        snprintf(parent, sizeof(parent), "  -");
                }

#if defined(__VFS__) && defined(__GETCWD__)
                if (NULL != p->p_cwd) {
                        char cwd[256];
                        lookup_dirpath(p->p_cwd, cwd, sizeof(cwd));
                        iprintf(&buf, &size, " %3i  %-13s %-18s %-s\n",
                                p->p_pid, p->p_comm, parent, cwd);
                } else {
                        iprintf(&buf, &size, " %3i  %-13s %-18s -\n",
                                p->p_pid, p->p_comm, parent);
                }
#else
                iprintf(&buf, &size, " %3i  %-13s %-s\n",
                        p->p_pid, p->p_comm, parent);
#endif
        } list_iterate_end();
        return size;
}
