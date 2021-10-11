#ifndef _AUTH_H
#define _AUTH_H

#define REASON_STR_OFFSET 5
#define SEC_TYPES_MAX 255

#define VNC_AUTH 2
#define CHALLENGE_SZ    16

int security_phase(int fd);

#endif