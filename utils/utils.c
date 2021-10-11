//
// Created by urv on 5/19/21.
//
#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>

#define MEMZERO(x)	memset(&(x), 0, sizeof (x));


double stopwatch(char* label, double timebegin) {
    struct timeval tv;

    if( timebegin == 0 ) {
      if (label)
        fprintf(stderr, "%s: Start stopwatch... \n", label);
      else
        fprintf(stderr, "Start stopwatch... \n");


      gettimeofday(&tv, NULL);
      double time_begin = ((double) tv.tv_sec) * 1000 +
                        ((double) tv.tv_usec) / 1000;
      return time_begin;

    } else {
      gettimeofday(&tv, NULL);
      double time_end = ((double)tv.tv_sec) * 1000 + ((double)tv.tv_usec) / 1000 ;

      fprintf(stderr, "%s: Execute time = %f(ms) \n",
              label, time_end - timebegin);

      return 0;
    }
}


// Convert memory dump to hex
void memdump(char *name, uint8_t *buff, size_t len, uint8_t column_n) {
    size_t idx;

    printf("%s: ============== memory dump from 0x%p  [len=%lu]\n", name, buff, len);

    printf("  ");
    for( idx = 0; idx < len; idx++) {
        if( idx > 0 && idx%column_n == 0)
            printf("\n  ");
        printf("%02x ", *(buff + idx));
    }

    printf("\n");
}

char * memdump2str(uint8_t *buff, uint8_t len) {
    size_t idx;

    char *str = (char*)calloc(len * 3 + 1, sizeof(char));

    for( idx = 0; idx < len; idx++) {
        sprintf(str + 3*idx, "%02x ", *(buff + idx));
    }

    return str;
}