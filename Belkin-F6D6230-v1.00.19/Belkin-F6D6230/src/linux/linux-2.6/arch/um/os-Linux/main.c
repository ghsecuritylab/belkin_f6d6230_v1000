/*
 * Copyright (C) 2000, 2001 Jeff Dike (jdike@karaya.com)
 * Licensed under the GPL
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <asm/page.h>
#include "kern_util.h"
#include "as-layout.h"
#include "mem_user.h"
#include "irq_user.h"
#include "user.h"
#include "init.h"
#include "mode.h"
#include "choose-mode.h"
#include "uml-config.h"
#include "os.h"
#include "um_malloc.h"
#include "kern_constants.h"

/* Set in main, unchanged thereafter */
char *linux_prog;

#define PGD_BOUND (4 * 1024 * 1024)
#define STACKSIZE (8 * 1024 * 1024)
#define THREAD_NAME_LEN (256)

static void set_stklim(void)
{
	struct rlimit lim;

	if(getrlimit(RLIMIT_STACK, &lim) < 0){
		perror("getrlimit");
		exit(1);
	}
	if((lim.rlim_cur == RLIM_INFINITY) || (lim.rlim_cur > STACKSIZE)){
		lim.rlim_cur = STACKSIZE;
		if(setrlimit(RLIMIT_STACK, &lim) < 0){
			perror("setrlimit");
			exit(1);
		}
	}
}

static __init void do_uml_initcalls(void)
{
	initcall_t *call;

	call = &__uml_initcall_start;
	while (call < &__uml_initcall_end){
		(*call)();
		call++;
	}
}

static void last_ditch_exit(int sig)
{
	uml_cleanup();
	exit(1);
}

static void install_fatal_handler(int sig)
{
	struct sigaction action;

	/* All signals are enabled in this handler ... */
	sigemptyset(&action.sa_mask);

	/* ... including the signal being handled, plus we want the
	 * handler reset to the default behavior, so that if an exit
	 * handler is hanging for some reason, the UML will just die
	 * after this signal is sent a second time.
	 */
	action.sa_flags = SA_RESETHAND | SA_NODEFER;
	action.sa_restorer = NULL;
	action.sa_handler = last_ditch_exit;
	if(sigaction(sig, &action, NULL) < 0){
		printf("failed to install handler for signal %d - errno = %d\n",
		       errno);
		exit(1);
	}
}

#define UML_LIB_PATH	":/usr/lib/uml"

static void setup_env_path(void)
{
	char *new_path = NULL;
	char *old_path = NULL;
	int path_len = 0;

	old_path = getenv("PATH");
	/* if no PATH variable is set or it has an empty value
	 * just use the default + /usr/lib/uml
	 */
	if (!old_path || (path_len = strlen(old_path)) == 0) {
		putenv("PATH=:/bin:/usr/bin/" UML_LIB_PATH);
		return;
	}

	/* append /usr/lib/uml to the existing path */
	path_len += strlen("PATH=" UML_LIB_PATH) + 1;
	new_path = malloc(path_len);
	if (!new_path) {
		perror("coudn't malloc to set a new PATH");
		return;
	}
	snprintf(new_path, path_len, "PATH=%s" UML_LIB_PATH, old_path);
	putenv(new_path);
}

extern int uml_exitcode;

extern void scan_elf_aux( char **envp);

int __init main(int argc, char **argv, char **envp)
{
	char **new_argv;
	int ret, i, err;

#ifdef UML_CONFIG_CMDLINE_ON_HOST
	/* Allocate memory for thread command lines */
	if(argc < 2 || strlen(argv[1]) < THREAD_NAME_LEN - 1){

		char padding[THREAD_NAME_LEN] = {
			[ 0 ...  THREAD_NAME_LEN - 2] = ' ', '\0'
		};

		new_argv = malloc((argc + 2) * sizeof(char*));
		if(!new_argv) {
			perror("Allocating extended argv");
			exit(1);
		}

		new_argv[0] = argv[0];
		new_argv[1] = padding;

		for(i = 2; i <= argc; i++)
			new_argv[i] = argv[i - 1];
		new_argv[argc + 1] = NULL;

		execvp(new_argv[0], new_argv);
		perror("execing with extended args");
		exit(1);
	}
#endif

	linux_prog = argv[0];

	set_stklim();

	setup_env_path();

	new_argv = malloc((argc + 1) * sizeof(char *));
	if(new_argv == NULL){
		perror("Mallocing argv");
		exit(1);
	}
	for(i=0;i<argc;i++){
		new_argv[i] = strdup(argv[i]);
		if(new_argv[i] == NULL){
			perror("Mallocing an arg");
			exit(1);
		}
	}
	new_argv[argc] = NULL;

	/* Allow these signals to bring down a UML if all other
	 * methods of control fail.
	 */
	install_fatal_handler(SIGINT);
	install_fatal_handler(SIGTERM);
	install_fatal_handler(SIGHUP);

	scan_elf_aux( envp);

	do_uml_initcalls();
	ret = linux_main(argc, argv);

	/* Disable SIGPROF - I have no idea why libc doesn't do this or turn
	 * off the profiling time, but UML dies with a SIGPROF just before
	 * exiting when profiling is active.
	 */
	change_sig(SIGPROF, 0);

	/* This signal stuff used to be in the reboot case.  However,
	 * sometimes a SIGVTALRM can come in when we're halting (reproducably
	 * when writing out gcov information, presumably because that takes
	 * some time) and cause a segfault.
	 */

	/* stop timers and set SIG*ALRM to be ignored */
	disable_timer();

	/* disable SIGIO for the fds and set SIGIO to be ignored */
	err = deactivate_all_fds();
	if(err)
		printf("deactivate_all_fds failed, errno = %d\n", -err);

	/* Let any pending signals fire now.  This ensures
	 * that they won't be delivered after the exec, when
	 * they are definitely not expected.
	 */
	unblock_signals();

	/* Reboot */
	if(ret){
		printf("\n");
		execvp(new_argv[0], new_argv);
		perror("Failed to exec kernel");
		ret = 1;
	}
	printf("\n");
	return uml_exitcode;
}

#define CAN_KMALLOC() \
	(kmalloc_ok && CHOOSE_MODE((os_getpid() != tracing_pid), 1))

extern void *__real_malloc(int);

void *__wrap_malloc(int size)
{
	void *ret;

	if(!CAN_KMALLOC())
		return __real_malloc(size);
	else if(size <= UM_KERN_PAGE_SIZE)
		/* finding contiguous pages can be hard*/
		ret = um_kmalloc(size);
	else ret = um_vmalloc(size);

	/* glibc people insist that if malloc fails, errno should be
	 * set by malloc as well. So we do.
	 */
	if(ret == NULL)
		errno = ENOMEM;

	return ret;
}

void *__wrap_calloc(int n, int size)
{
	void *ptr = __wrap_malloc(n * size);

	if(ptr == NULL)
		return NULL;
	memset(ptr, 0, n * size);
	return ptr;
}

extern void __real_free(void *);

extern unsigned long high_physmem;

void __wrap_free(void *ptr)
{
	unsigned long addr = (unsigned long) ptr;


	if((addr >= uml_physmem) && (addr < high_physmem)){
		if(CAN_KMALLOC())
			kfree(ptr);
	}
	else if((addr >= start_vm) && (addr < end_vm)){
		if(CAN_KMALLOC())
			vfree(ptr);
	}
	else __real_free(ptr);
}
