#ifndef _LIBC_ERRNO_H
#define _LIBC_ERRNO_H 1

#define EINVAL		0x00000001
#define ENOMEM		0x00000002
#define ERANGE		0x00000003
#define EOVERFLOW	0x00000004
#define EADDRINUSE	0x00000005

#if defined(__is_libk) || defined(__is_missy_kernel)
extern int _kerrno;
#define errno _kerrno
#else
// TODO: Provide definition for thread local, modifiable lvalue for libc
extern int __errno;
#define errno __errno
#endif

#endif // _LIBC_ERRNO_H