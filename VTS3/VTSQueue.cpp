/* test app for VTSQueue template */

#ifdef LOCAL_TEST

#include "VTSQueue.h"
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

char *strings[] = {{"One"},{"Two"},{"Three"},{"Four"},{NULL}};

/* note - only tests the FIFO part, not the multithread safe part */
void main(void)
{
  int index;
  VTSQueue<char *> queue;
  char **pWord;

  index = 0;
  while (strings[index]) {
    queue.Write(&strings[index]);
    index++;
  }

  index = 0;
  do {
    pWord = queue.Read();
    if (pWord)
      assert(strcmp(*pWord,strings[index]) == 0);
    else
      assert(strings[index] == NULL);
    index++;
  } while (pWord);

  return;
}

#endif