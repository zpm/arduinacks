
#include <stdio.h>


main() {

  char* buffer = "POST /cmd HTTP/1.1";

  if (strncmp(buffer, "POST /", 6) != 0) {
    // if this isn't a post, return false
    return 0;
  }

  // find terminating space
  char* spaceAt = strstr(&buffer[6], " ") + 1;
  printf("Space at = %u\n", spaceAt);

  // find all the slashes
  char* slashAt1 = strstr(buffer, "/") + 1;
  printf("%u\n", slashAt1);

  char* slashAt2 = strstr(slashAt1, "/") + 1;
  printf("%u\n", slashAt2);

  char* slashAt3 = strstr(slashAt2, "/") + 1;
  printf("%u\n", slashAt3);

  char* slashAt4 = strstr(slashAt3, "/") + 1;
  printf("%u\n", slashAt4);




  char* tempString[8];
  tempString[0] = 0;

  // strncpy(tempString, slashAt1, slashAt2-slashAt1-1);






  // strncpy does not automatically add terminating zero,
  // but strncat does! So start with blank string and concatenate.
  int ic = 0;
  int ir = 0;
  int ig = 0;
  int ib = 0;

  return 1;


}