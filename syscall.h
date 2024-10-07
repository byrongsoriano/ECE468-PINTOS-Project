#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdbool.h>

struct lock filesystem_lock;  // lock for file system operations

struct file_descriptor        // Data structure for the file descriptor
{
    struct file *file;
    int file_descriptor;
    struct list_elem elem;
};

void syscall_init (void);                               // Initialize the system call table
void halt (void);                                       // Halt the operating system
void exit (int status);                                 // Exit the current process
pid_t exec (const char *cmd_line);                      // Create a new process 
int wait (pid_t pid);                                   // Wait for a child process to finish
bool create (const char *file, unsigned initial_size);  // Create a new file
bool remove (const char *file);                         // Remove a file
int open (const char *file);                            // Open a file
int filesize (int fd);
int read (int fd, void *buffer, unsigned size);
int write (int fd, const void *buffer, unsigned size);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);

#endif /* userprog/syscall.h */
