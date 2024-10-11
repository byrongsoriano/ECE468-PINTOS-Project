#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "devices/input.h"
#include "devices/shutdown.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/malloc.h"

static void syscall_handler(struct intr_frame *);
void get_args_from_stack(const void *esp, char *argv, int count);
bool verify_ptr(const void *vaddr);

void syscall_init(void)
{
  // Initialize file lock
  lock_init(&filesys_lock);
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_handler(struct intr_frame *f)
{
  // Get stack pointer
  int *esp = f->esp;

  // Verify stack pointer
  if(!verify_ptr((const void*)(esp))) {
    // printf("STACK PTR ERROR\n");
    syscall_exit(-1);
    return;
  }

  switch (*(int *)esp)
  {
  case SYS_HALT:
    syscall_halt();
    break;
    case SYS_EXIT:
      if(!verify_ptr((const void*)(esp + 1))) {
        syscall_exit(-1);
        break;
      }
      syscall_exit((int)*(esp+1));
      break;
    case SYS_EXEC:
      // validate cmd line arguments
      if(!verify_ptr((void*)(esp + 1)) || !verify_ptr((void*)(*(esp + 1))))
        syscall_exit(-1);
      // put result from syscall_exec into return register
      f->eax = syscall_exec((const char*)*(esp+1));
      break;
    case SYS_WAIT:
        if (!verify_ptr((const void *)(esp + 1))) {
            syscall_exit(-1);
        }
        f->eax = syscall_wait((pid_t)*(esp + 1));
        break;
      case SYS_CREATE:
          // validate cmd line arguments
          if (!verify_ptr((void *)(esp + 5)) || !verify_ptr((void *)(*(esp + 4))))
              syscall_exit(-1);
          // put result from syscall_create into return register
          f->eax = syscall_create((const char *)*(esp + 4), (unsigned)*(esp + 5));
          break;
      case SYS_REMOVE:
          if (!verify_ptr((void *)(esp + 1)) || !verify_ptr((void *)(*(esp + 1))))
              syscall_exit(-1);

          f->eax = syscall_remove((const char *)*(esp + 1));
          break;
    case SYS_OPEN:
      if(!verify_ptr((const void*)(esp + 1)) || !verify_ptr((const void*)*(esp + 1))) {
        syscall_exit(-1);
        break;
      }
      f->eax = (uint32_t) syscall_open((char *)*(esp + 1));
      break;
    case SYS_FILESIZE:
      if(!verify_ptr((const void*)(esp + 1))) {
        syscall_exit(-1);
        break;
      }
      f->eax = syscall_filesize((int)*(esp+1));
      break;
    case SYS_SEEK:
        if(!verify_ptr((const void*)(esp+4)) || !verify_ptr((const void*)(esp+5))){
            syscall_exit(-1);
        }
        syscall_seek((int)(*(esp+4)), (unsigned) (*(esp+5)));
    break;
  case SYS_READ:
    if (verify_ptr((const void *)(esp + 5)) && verify_ptr((const void *)(esp + 6)) && verify_ptr((const void *)(esp + 7)) && verify_ptr((const void*)*(esp + 6)))
    {
      f->eax = (uint32_t)syscall_read((int)*(esp + 5), (void *)*(esp + 6), (unsigned int)*(esp + 7));
    }
    else
    {
      syscall_exit(-1);
    }
    break;
  case SYS_WRITE:
    if (verify_ptr((const void *)(esp + 5)) && verify_ptr((const void *)(esp + 6)) && verify_ptr((const void *)(esp + 7)) && verify_ptr((const void*)*(esp + 6)))
    {
      // printf("fd: %d  buf: %d,  size: %d\n", *(esp+5), *(esp+6), *(esp+7));
      f->eax = (uint32_t)syscall_write((int)*(esp + 5), (const void *)*(esp + 6), (unsigned)*(esp + 7));
      // printf("bytes written: %d\n", f->eax);
    }
    else
    {
      syscall_exit(-1);
    }
    break;
  case SYS_TELL:
    if (!verify_ptr((const void *)(esp + 1)))
    {
      syscall_exit(-1);
      break;
    }
    f->eax = syscall_tell((int)*(esp + 1));
    break;
  case SYS_CLOSE:
    if (!verify_ptr((const void *)(esp + 1))) {
      syscall_exit(-1);
      break;
    }

    syscall_close((int)*(esp + 1));
    break;
  }
}
/* Terminates Pintos by calling shutdown_power_off() (declared in threads/init.h).
 * This should be seldom used, because you lose some information about possible deadlock situations, etc.
 */
void syscall_halt(void)
{
  shutdown_power_off();
}

/* Terminates the current user program, returning status to the kernel.
 * If the process's parent waits for it (see below), this is the status that will be returned.
 * Conventionally, a status of 0 indicates success and nonzero values indicate errors.
 */
void syscall_exit(int status)
{
  struct list_elem *e;

  // Loop through children thread of parent thread
  for (e = list_begin(&thread_current()->parent->children); e != list_end(&thread_current()->parent->children); e = list_next(e))
  {
    struct child *f = list_entry(e, struct child, elem);
    // Check if the parent is waiting for this current thread
    if (f->tid == thread_current()->tid)
    {
      sema_up(&thread_current()->parent->child_lock);
      f->used = true;         // parent is currently using this thread
      f->exit_error = status; // return the current status to the parent thread
      sema_down(&thread_current()->parent->child_lock);
    }
  }

  if(thread_current()->parent->waiting_td == thread_current()->tid) {
    sema_up(&thread_current()->parent->child_lock);
  }

  thread_current()->parent->finish = true;
  thread_current()->exit_error = status;
  // printf("%s: exit(%d)\n", thread_current()->name, status); // Needed this for the perl .ck files
  thread_exit();
}

/* Runs the executable whose name is given in cmd_line, passing any given arguments,
 * and returns the new process's program id (pid). Must return pid -1, which otherwise
 * should not be a valid pid, if the program cannot load or run for any reason.
 * Thus, the parent process cannot return from the exec until it knows whether the child
 * process successfully loaded its executable. You must use appropriate synchronization to ensure this.
 */
pid_t syscall_exec(const char *cmd_line) {
  // use synchronization to check the child process
  lock_acquire(&filesys_lock);

  // Get file name from cmd line
  char *cmdline_ptr;
  char *file_name;
  file_name = (char *) malloc(strlen(cmd_line) + 1);
  strlcpy(file_name, cmd_line, strlen(cmd_line) + 1);
  file_name = strtok_r(file_name, " ", &cmdline_ptr);

  // check to see if the file exists
  struct file* f = filesys_open(file_name);

  // if file does not exist, return -1
  if(f==NULL){
    lock_release(&filesys_lock);
    return -1;
  }else{
    file_close(f);
    // call process_execute in process.c to start a new thread, and return program id
    lock_release(&filesys_lock);
    return process_execute(cmd_line);
  }
}

/* Waits for a child process pid and retrieves the child's exit status. */
int syscall_wait(pid_t pid)
{
  return process_wait(pid);
}

/* Creates a new file called file initially initial_size bytes in size.
 * Returns true if successful, false otherwise. Creating a new file does not open it:
 * opening the new file is a separate operation which would require a open system call.
 */
bool syscall_create(const char *file, unsigned initial_size)
{
  lock_acquire(&filesys_lock);
  // call filesys_create from filesys.h
  bool result = filesys_create(file, initial_size);
  lock_release(&filesys_lock);
  return result;
}

/* Deletes the file called file. Returns true if successful, false otherwise.
 * A file may be removed regardless of whether it is open or closed, and removing
 * an open file does not close it.
 */
bool syscall_remove(const char *file)
{
  lock_acquire(&filesys_lock);
  bool ret = filesys_remove(file);
  lock_release(&filesys_lock);
  return ret;
}

/* Opens the file called file. Returns a nonnegative integer handle called a
 * "file descriptor" (fd), or -1 if the file could not be opened.
 */
int syscall_open(const char *file)
{
  lock_acquire(&filesys_lock);
  struct file *fd_struct = filesys_open(file); // open file

  // if file doesn't exist, exit
  if (fd_struct == NULL)
  {
    lock_release(&filesys_lock);
    return -1;
  }

  // Add the file to the list of opened files of the current thread
  struct file_descriptor *new_fd = malloc(sizeof(struct file_descriptor));
  new_fd->file = fd_struct;
  new_fd->fd = thread_current()->fd;
  thread_current()->fd++;
  list_push_back(&thread_current()->file_list, &new_fd->elem);
  lock_release(&filesys_lock);
  return new_fd->fd;
}

/* Returns the size, in bytes, of the file open as fd. */
int syscall_filesize(int fd)
{
  lock_acquire(&filesys_lock);
  struct file *fd_struct = getFile(fd); // get the file within the file list

  // If file doesn't exist in the list, exit
  if (fd_struct == NULL)
  {
    lock_release(&filesys_lock);
    return -1;
  }

  // Get the file length
  int filesize = file_length(fd_struct);
  lock_release(&filesys_lock);

  return filesize;
}

/* Reads size bytes from the file open as fd into buffer. Returns the number
 * of bytes actually read (0 at end of file), or -1 if the file could not be read
 * (due to a condition other than end of file). Fd 0 reads from the keyboard using input_getc().
 */
int syscall_read(int fd, void *buffer, unsigned size)
{

  if (size < 0)
  {
    return -1;
  }
  if (fd == STDIN_FILENO)
  {
    uint8_t *buf = buffer;
    for (size_t i = 0; i < size; i++)
    {
      buf[i] = input_getc(); // get input character from input buffer (key press)
    }
    return size;
  }
  if (fd == STDOUT_FILENO)
  {
    return -1;
  }

  // Read file
  lock_acquire(&filesys_lock);
  struct file *fd_struct = getFile(fd);

  if (fd_struct == NULL)
  {
    lock_release(&filesys_lock);
    return -1;
  }

  int bytes_read = file_read(fd_struct, buffer, size);
  lock_release(&filesys_lock);
  return bytes_read;
}

/* Writes size bytes from buffer to the open file fd. Returns the number of bytes
 * actually written, which may be less than size if some bytes could not be written.
 */
int syscall_write(int fd, const void *buffer, unsigned size)
{

  if (size < 0)
  {
    return -1;
  }

  if (fd == STDIN_FILENO)
  {
    return -1;
  }
  if (fd == STDOUT_FILENO)
  {
    putbuf(buffer, size);
    return size;
  }

  // write to file
  lock_acquire(&filesys_lock);
  struct file *fd_struct = getFile(fd);

  if (fd_struct == NULL)
  {
    lock_release(&filesys_lock);
    return -1;
  }

  int bytes_written = file_write(fd_struct, buffer, size);
  lock_release(&filesys_lock);
  return bytes_written;
}

/* Changes the next byte to be read or written in open file fd to position,
 * expressed in bytes from the beginning of the file. (Thus, a position of 0 is the file's start.)
 */
void syscall_seek(int fd, unsigned position) {
    lock_acquire(&filesys_lock);
    struct file *fd_struct = getFile(fd);

    if (fd_struct == NULL) {
        lock_release(&filesys_lock);
        return;
    }
    file_seek(fd_struct, position);
    lock_release(&filesys_lock);
}
/* Returns the position of the next byte to be read or written in open file fd,
 * expressed in bytes from the beginning of the file.
 */
unsigned syscall_tell(int fd)
{
  lock_acquire(&filesys_lock);
  struct file *fd_struct = getFile(fd); // get file in file list

  // If file doesn't exist, exit
  if (fd_struct == NULL)
  {
    lock_release(&filesys_lock);
    return -1;
  }

  // File tell
  int byte_pos = file_tell(fd_struct);
  lock_release(&filesys_lock);

  return byte_pos;
}

/* Closes file descriptor fd. Exiting or terminating a process implicitly closes all its
 * open file descriptors, as if by calling this function for each one.
 */
void syscall_close(int fd)
{
  // Lock file to prevent issues
  lock_acquire(&filesys_lock);

  struct thread *td = thread_current();
  struct list_elem *list;
  struct file_descriptor *f_descriptor;

  // Traverse through list of files that are currently open
  for (list = list_begin(&td->file_list); list != list_end(&td->file_list); list = list_next(list))
  {
    f_descriptor = list_entry(list, struct file_descriptor, elem);
    // Find target file to close
    if (fd == f_descriptor->fd)
    {
      // Close file
      file_close(f_descriptor->file);

      // Remove file from list
      list_remove(list);
    }
  }

  // Deallocate memory reserved for f_descriptor
  free(f_descriptor);

  // Unlock once done closing file
  lock_release(&filesys_lock);
}

struct file *getFile(int fd)
{
  struct thread *td = thread_current();
  struct list_elem *list;
  struct file_descriptor *f_descriptor;

  // Loop through file list of current thread
  for (list = list_begin(&td->file_list); list != list_end(&td->file_list); list = list_next(list))
  {
    f_descriptor = list_entry(list, struct file_descriptor, elem);
    if (fd == f_descriptor->fd)
    {
      return f_descriptor->file; // found file. return it
    }
  }
  return NULL;
}

bool verify_ptr(const void *vaddr)
{
  // Check to make sure address is not a null pointer
  bool isNullAddr = vaddr == NULL;

  // Check if address is pointer to kernel virtual address space
  bool isKernelSpace = is_kernel_vaddr(vaddr);

  if (isNullAddr)
  {
    // printf("INVALID POINTER: Null Pointer\n");
    thread_current()->exit_error = -1;
    return false;
  }
  else if (isKernelSpace)
  {
    // printf("INVALID POINTER: Pointer to kernel virtual address space\n");
    thread_current()->exit_error = -1;
    return false;
  }
  else
  {
    // Check if address is pointer to unmapped virtual memory
    struct thread *td = thread_current();
    bool isUaddrMapped = pagedir_get_page(td->pagedir, vaddr) != NULL; // pagedir_get_page returns NULL if UADDR is unmapped
    if (!isUaddrMapped)
    {
      // printf("INVALID POINTER: Unmapped to virtual memory\n");
      thread_current()->exit_error = -1;
      return false;
    }
  }
  return true;
}

void close_all_files(struct list *files)
{
  struct list_elem *list;
  struct file_descriptor *f_descriptor;

  // While the file list is not empty
  while (!list_empty(files))
  {
    list = list_pop_front(files); // Pop it off the list
    f_descriptor = list_entry(list, struct file_descriptor, elem);
    file_close(f_descriptor->file); // Close the file that was popped off the list
    free(f_descriptor);
  }
}
