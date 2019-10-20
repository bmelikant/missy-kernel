#ifndef _LIBC_STRING_H
#define _LIBC_STRING_H

#include <sys/cdefs.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** memory functions */
void 	*memcpy(void *destination, const void *source, size_t length);		// movecopy.c, done
void 	*memmove(void *destination, const void *source, size_t length);		// movecopy.c, may need malloc
void 	*memset(void *destination, int initial, size_t length);				// other.c
int		memcmp(const void *mem1, const void *mem2, size_t length);			// compare.c, done
void 	*memchr(const void *mem, int c, size_t len);						// search.c

/** string functions */
char 	*strcpy(char *destination, const char *source);						// movecopy.c, done
char 	*strncpy(char *destination, const char *source, size_t len);		// movecopy.c, done
char 	*strcat(char *destination, const char *source);						// concat.c
char 	*strncat(char *destination, const char *source, size_t len);		// concat.c
int  	strcmp(const char *s1, const char *s2);								// compare.c, done
int  	strncmp(const char *s1, const char *s2, size_t len);				// compare.c, done
// char 	*strcoll(const char *s1, const char *s2);						// compare.c, locale-specific; not implemented yet
// size_t	strxfrm(char *destination, const char *source, size_t len);		// compare.c, locale-specific; not implemented yet
char 	*strchr(const char *str, int c);									// search.c
size_t	strcspn(const char *str1, const char *str2);						// search.c
char	*strbprk(const char *str1, const char *str2);						// search.c
char	*strrchr(const char *str, int c);									// search.c
size_t	strspn(const char *str1, const char *str2);							// search.c
char	*strstr(const char *str1, const char *str2);						// search.c
char	*strtok(char *str, const char *delimiters);							// other.c
size_t	strlen(const char *str);											// other.c

#ifdef __cplusplus
}
#endif

#endif // _LIBC_STRING_H