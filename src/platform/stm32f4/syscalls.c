// 
// STM32F4xx syscallのでっちあげsyscalls テンプレート
// 
// 参考資料：
//	RM0090 Reference manual - STM32F4xx advanced ARM-based 32-bit MCUs
//	PM0056 Programming manual - STM32Fxxx Cortex-M3 programming manual
//	UM1472 Users manual - STM32F4DISCOVERY discovery board
//	http://sourceware.org/newlib/libc.html
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <errno.h>
#undef errno
volatile int errno;

// #define SBRK_DEBUG 1

#define USE_REENTRANT_SYSTEM_CALLS

#include "stm32f4xx.h"
#include "core_cm4.h"
#include "sysclk_config.h"
// #include <pm.h>

#if 1

#define STDIN_FILENO	(0)	/* standard input file descriptor */
#define STDOUT_FILENO	(1)	/* standard output file descriptor */
#define STDERR_FILENO	(2)	/* standard error file descriptor */

#define	LED_GR(x)	{if(x){GPIOD->ODR|=(1 << 12);TIM4->CCR1=0xff;}else{GPIOD->ODR&=~(1 << 12);TIM4->CCR1=0;};}
#define	LED_OR(x)	{if(x){GPIOD->ODR|=(1 << 13);TIM4->CCR2=0xff;}else{GPIOD->ODR&=~(1 << 13);TIM4->CCR2=0;};}
#define	LED_RE(x)	{if(x){GPIOD->ODR|=(1 << 14);TIM4->CCR3=0xff;}else{GPIOD->ODR&=~(1 << 14);TIM4->CCR3=0;};}
#define	LED_BL(x)	{if(x){GPIOD->ODR|=(1 << 15);TIM4->CCR4=0xff;}else{GPIOD->ODR&=~(1 << 15);TIM4->CCR4=0;};}

extern int16_t USART2_getc(void);
extern void USART2_putc(char);
extern void USART2_puts(char *);

#define putstr	USART2_puts
#define putch	USART2_putc
#define getch	USART2_getc

#define MARKS(x) (*(uint32_t *) 0x10000000) |= (1<<(x))

void _exit(int n);                          	//	swi	#0x900001
int  _close(int fd);                        	//	swi	#0x900006
int  _execve(char *name, char **argv, char **env);	//	swi	#0x90000b
int  _fstat(int fd, struct stat * st);      	//	swi	#0x90006c
int  _getpid(void);                         	//	swi	#0x900014
int  _isatty(int fd);
int  _kill(int pid, int sig);               	//	swi	#0x900025
int  _link(char *old, char *new);           	//	swi	#0x900009
int  _lseek(int file, int ptr, int dir);    	//	swi	#0x900013
int  _open(const char *name, int flags, int mode);	//	swi	#0x900005
int  _read(int fd, char *buffer, int len);  	//	swi	#0x900003
caddr_t _sbrk(size_t incr);                 	// uint32_t brk(uint32_t);	swi	#0x90002d
int  _stat(const char *file, struct stat *st);	//	swi	#0x90006a
clock_t _times(struct tms *buf);            	//	swi	#0x90002b
int  _unlink(char *name);                   	//	swi	#0x90000a
int  _wait(int *status);
int  _write(int fd, const char *ptr, int len);	//	swi	#0x900004
void exit(int n);
int  close(int fd);
int  execve(char *name, char **argv, char **env);
int  fstat(int fd, struct stat * st);
int  getpid(void);
int  isatty(int fd);
int  kill(int pid, int sig);
int  link(char *old, char *new);
int  lseek(int file, int ptr, int dir);
int  open(const char *name, int flags, int mode);
int  read(int fd, char *buffer, int len);
caddr_t sbrk(size_t incr);
int  stat(const char *file, struct stat *st);
clock_t times(struct tms *buf);
int  unlink(char *name);
int  wait(int *status);
int  write(int fd, const char *ptr, int len);

void _exit(int n){
	MARKS(1);
	loop;
	n++;
}
int _close(int fd){			// this is !! //
	MARKS(2);
	return(-1);
	fd++;
}
int _execve(char *name, char **argv, char **env){
	MARKS(3);
	errno= ENOMEM;
	return(-1);
	*argv=*env=name;
}
int _fork(void) {
	MARKS(4);
	errno= EAGAIN;
	return(-1);
}
int _fstat(int fd, struct stat * st){			// this is !! //
	MARKS(5);
	switch(fd){
	  case(STDIN_FILENO):
	  case(STDOUT_FILENO):
	  case(STDERR_FILENO):
		memset(st, 0, sizeof (* st));
		st->st_mode = S_IFCHR;
		st->st_blksize = 1024;
		return(0);
		break;;
	  default:
		errno= EINVAL;
		errno= EBADF;
		return(-1);
		break;;
	};
}
int _getpid(void) {
	MARKS(6);
	return( 1);
}
int _isatty(int fd){			// this is !! //
	MARKS(7);
	switch(fd){
	  case(STDIN_FILENO):
	  case(STDOUT_FILENO):
	  case(STDERR_FILENO):
		return(1);
		break;;
	  default:
		errno= EBADF;
		return(0);
		break;;
	};
}
int _kill(int pid, int sig) {
	MARKS(8);
	errno= EINVAL;
	return(-1);
	sig++;
	pid++;
}
int _link(char *old, char *new) {
	MARKS(9);
	errno= EMLINK;
	return(-1);
	new++;
	old++;
}
int _lseek(int file, int ptr, int dir) {			// this is !! //
	MARKS(10);
	return(0);
	file= ptr= dir;
	file++;
}
int _open(const char *name, int flags, int mode) {
	MARKS(11);
	return(-1);
	flags= mode= *name;
	flags++;
}
int _read(int fd, char *buffer, int len){			// this is !! //
int cnt;
int16_t c;
	MARKS(12);
	cnt=-1;
//	printf("\r\n_read(%d,%p,%d);\r\n",fd,buffer,len);
	switch(fd){
	  case(STDIN_FILENO):
	  case(STDOUT_FILENO):
	  case(STDERR_FILENO):
//		TIM4->CCR3=0x0f;
//		for (cnt=0;cnt<len;cnt++,buffer++){
			c=getch();
			*buffer= c;
			cnt=1;
//		};
//		TIM4->CCR3=0x0;
		break;;
	  default:
		errno= EBADF;
		return(-1);
		break;;
	};
	return(cnt);
}

extern volatile long CNT_tick;

#ifdef SBRK_DEBUG
static const char hexchar[]="0123456789ABCDEF";
void w10ms(void){
uint16_t x;
	x= ((uint16_t)CNT_tick)+15;
	while(((uint16_t)CNT_tick) < x);
}
void printLONG(uint32_t d){
	putch(hexchar[(d>>28) & 0x0f]);
	putch(hexchar[(d>>24) & 0x0f]);
	putch(hexchar[(d>>20) & 0x0f]);
	putch(hexchar[(d>>16) & 0x0f]);
	putch(hexchar[(d>>12) & 0x0f]);
	putch(hexchar[(d>> 8) & 0x0f]);
	putch(hexchar[(d>> 4) & 0x0f]);
	putch(hexchar[(d    ) & 0x0f]);
	w10ms();
}
#endif

// register char *stack_ptr asm ("sp");
// extern char stack_ptr asm ("_endof_sram"); // Defined by the linker
// extern char _end       asm ("_end"); // Defined by the linker
extern char _endof_sram;
extern char _end;
static char *heap_end=0;
caddr_t _sbrk(size_t incr){			// this is !! //
	char *prev_heap_end;
	char *stack;
//	stack= (char *)__get_MSP(); 
	stack=  &_endof_sram; 

	MARKS(13);
#ifdef SBRK_DEBUG
	TIM4->CCR4=0xff;
	putstr("\r\nheap_end=");
	printLONG((uint32_t)heap_end);
#endif
	if( heap_end == NULL )heap_end = & _end;
	prev_heap_end = heap_end;

#ifdef SBRK_DEBUG
	putstr("...");
	printLONG((uint32_t)heap_end);
#endif

	if(( heap_end + incr ) > stack){
		return (caddr_t)(-1);	// errno= ENOMEM;
	};
	heap_end += incr;

#ifdef SBRK_DEBUG
	putstr(" ... heap_return=");
	printLONG((uint32_t)prev_heap_end);
#endif
	return( (caddr_t) prev_heap_end );
}
int _stat(const char *file, struct stat *st) {
	memset(st, 0, sizeof (* st));
	st->st_mode = S_IFCHR;
	st->st_blksize = 1024;
	return(0);
	file++;
}
clock_t _times(struct tms *buf){
	MARKS(14);
	return(-1);
	memset(buf, 0, sizeof (* buf));
}
int _unlink(char *name) {
	MARKS(15);
	errno= ENOENT;
	return -1;
	name++;
}
int _wait(int *status) {
	MARKS(16);
	errno= ECHILD;
	return(-1);
	(*status)=0;
}
int _write(int fd, const char *ptr, int len){			// this is !! //
int cnt;
	MARKS(17);
	cnt=-1;
//	TIM4->CCR4=0x0f;
	switch(fd){
	  case(STDIN_FILENO):
	  case(STDOUT_FILENO):
	  case(STDERR_FILENO):
		for (cnt=0;cnt<len;cnt++,ptr++)putch(*ptr);
		break;;
	  default:
		errno= EBADF;
		return(-1);
		break;;
	};
	return(cnt);
}

void exit(int n){
	_exit(n);
}
int close(int fd){
	return(_close(fd));
}
int execve(char *name, char **argv, char **env){
	return(_execve(name, argv, env));
}
int fork(void){
	return(_fork());
}
int fstat(int fd, struct stat * st){
	return(_fstat(fd, st));
}
int getpid(void){
	return(_getpid());
}
int isatty(int fd){
	return(_isatty(fd));
}
int kill(int pid, int sig){
	return(_kill(pid, sig));
}
int link(char *old, char *new){
	return(_link(old, new));
}
int lseek(int file, int ptr, int dir){
	return(_lseek(file, ptr, dir));
}
int open(const char *name, int flags, int mode){
	return(_open(name, flags, mode));
}
int read(int fd, char *buffer, int len){
	return(_read(fd, buffer, len));
}
caddr_t sbrk(size_t incr){
	return(_sbrk( incr ));
}
int stat(const char *file, struct stat *st) {
	return(_stat(file, st));
}
clock_t times(struct tms *buf){
	return(_times(buf));
}
int unlink(char *name){
	return(_unlink(name));
}
int wait(int *status){
	return(_wait(status));
}
int write(int fd, const char *ptr, int len){
	return(_write(fd, ptr, len));
}








#ifdef USE_REENTRANT_SYSTEM_CALLS
int _close_r(void *reent, int fd){
	return(_close(fd));
	reent++;
}
int _fork_r(void *reent){
	return(_fork());
	reent++;
}
int _fstat_r(void *reent, int fd, struct stat * st){
	return(_fstat(fd, st));
	reent++;
}
int _getpid_r(void *reent){
	return(_getpid());
	reent++;
}
int _isatty_r(void *reent, int fd){
	return(_isatty(fd));
	reent++;
}
int _kill_r(void *reent, int pid, int sig){
	return(_kill(pid, sig));
	reent++;
}
int _link_r(void *reent, char *old, char *new){
	return(_link(old, new));
	reent++;
}
int _lseek_r(void *reent, int file, int ptr, int dir){
	return(_lseek(file, ptr, dir));
	reent++;
}
int _open_r(void *reent, const char *name, int flags, int mode){
	return(_open(name, flags, mode));
	reent++;
}
int _read_r(void *reent, int fd, char *buffer, int len){
	return(_read(fd, buffer, len));
	reent++;
}
caddr_t _sbrk_r(void *reent, size_t incr){
	return(_sbrk( incr ));
	reent++;
}
int _stat_r(void *reent, const char *file, struct stat *st){
	return(_stat(file, st));
	reent++;
}
int _unlink_r(void *reent, char *name){
	return(_unlink(name));
	reent++;
}
int _wait_r(void *reent, int *status){
	return(_wait(status));
	reent++;
}
int _write_r(void *reent, int fd, char *ptr, int len){
	return(_write(fd, ptr, len));
	reent++;
}

#endif /* USE_REENTRANT_SYSTEM_CALLS */


#endif
