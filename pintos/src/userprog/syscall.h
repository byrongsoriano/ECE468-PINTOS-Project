#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <user/syscall.h>
#include "threads/synch.h"
#include "threads/thread.h"
#include "stdbool.h"
#include <stdio.h>

struct lock filesys_lock;

struct file_descriptor
{
    struct file *file;
    int fd;
    struct list_elem elem;
};

struct file *getFile(int fd);

void syscall_init(void);
void syscall_halt(void);
void syscall_exit(int status);
pid_t syscall_exec(const char *cmd_line);
int syscall_wait(pid_t pid);
bool syscall_create(const char *file, unsigned intitlal_size);
bool syscall_remove(const char *file);
int syscall_open(const char *file);
int syscall_filesize(int fd);
int syscall_read(int fd, void *buffer, unsigned size);
int syscall_write(int fd, const void *buffer, unsigned size);
void syscall_seek(int fd, unsigned position);
unsigned syscall_tell(int fd);
void syscall_close(int fd);

void close_all_files(struct list *files);

#endif /* userprog/syscall.h */
