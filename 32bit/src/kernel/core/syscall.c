#include "system.h"

// System calls implemented directly in this file
static void getcwd(char* path);
static bool chdir(const char* path);

static uint32_t syscalls[] = {
	/* System Call Table */
	(uint32_t)&write,               /* 0 */
	(uint32_t)&read,
	(uint32_t)&list_dir,
	(uint32_t)&exec,
	(uint32_t)&getcwd,
	(uint32_t)&chdir,
};
#define NUM_SYSCALLS 6

static void getcwd(char* path) {
	strcpy(cwd, path);
}

static bool chdir(const char* path) {
	strcpy(path, cwd);
    return SUCCESS; // in future, check dir exists
}

void handle_syscall(struct regs *r) {
    __asm__ __volatile__ ("sti"); // so we can get interrupts that are needed to fulfil the call (e.g. keypress)

	fprintf(stddebug, "Handling syscall %d\n", r->eax);

    if (r->eax >= NUM_SYSCALLS) {
        die("Invalid syscall");
    }

    uint32_t location = syscalls[r->eax];
    uint32_t ret;
	// Push the values in our syscall registers onto the stack in the right order for them to be seen as arguments
	// to the relevant function
	asm volatile (
			"push %1\n"
			"push %2\n"
			"push %3\n"
			"push %4\n"
			"push %5\n"
			"call *%6\n"
			"pop %%ebx\n"
			"pop %%ebx\n"
			"pop %%ebx\n"
			"pop %%ebx\n"
			"pop %%ebx\n"
			: "=a" (ret) : "r" (r->edi), "r" (r->esi), "r" (r->edx), "r" (r->ecx), "r" (r->ebx), "r" (location));
    
    r->eax = ret;
}
