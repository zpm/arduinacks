
#include <stdio.h>


main() {

  char* buffer = "POST /basis/100/1000000/ HTTP/1.1";

  if (strncmp(buffer, "POST /", 6) != 0) {
    // if this isn't a post, return false
    return 0;
  }

  // set all the values to zero
  int ic = 0;
  int ir = 0;
  int ig = 0;
  int ib = 0;

  // find terminating space
  char* spaceAt = strstr(&buffer[6], " ") + 1;

  // setup temp variable
  char* tempString[8];

  // find all the slashes
  char* slashAt1 = strstr(&buffer[6], "/") + 1;
  if (slashAt1 < spaceAt) {
    char* slashAt2 = strstr(slashAt1, "/") + 1;
    if (slashAt2 < spaceAt) {
      char* slashAt3 = strstr(slashAt2, "/") + 1;
      if (slashAt3 < spaceAt) {
        char* slashAt4 = strstr(slashAt3, "/") + 1;
        if (slashAt4 < spaceAt) {
          tempString[0] = 0;
          strncpy(tempString, slashAt3, slashAt4-slashAt3-1);
        } else {
          tempString[0] = 0;
          strncpy(tempString, slashAt3, spaceAt-slashAt3-1);
        }
        ib = atoi(tempString);
        tempString[0] = 0;
        strncpy(tempString, slashAt2, slashAt3-slashAt2-1);
      } else {
        tempString[0] = 0;
        strncpy(tempString, slashAt2, spaceAt-slashAt2-1);
      }
      ig = atoi(tempString);
      tempString[0] = 0;
      strncpy(tempString, slashAt1, slashAt2-slashAt1-1);
    } else {
      tempString[0] = 0;
      strncpy(tempString, slashAt1, spaceAt-slashAt1-1);
    }
    ir = atoi(tempString);
    tempString[0] = 0;
    strncpy(tempString, &buffer[6], slashAt1-&buffer[6]-1);
  } else {
    tempString[0] = 0;
    strncpy(tempString, &buffer[6], spaceAt-&buffer[6]-1);
  }

  // command set for ic bit
  if (!strncmp(tempString, "off", 6) != 0) {
    ic = 0;
  } else if (!strncmp(tempString, "basis", 6) != 0) {
    ic = 1;
  } else if (!strncmp(tempString, "schizm", 6) != 0) {
    ic = 2;
  } else if (!strncmp(tempString, "blinder", 6) != 0) {
    ic = 3;
  } else if (!strncmp(tempString, "pulsar", 6) != 0) {
    ic = 4;
  }

  printf("ic = %u\n", ic);
  printf("ir = %u\n", ir);
  printf("ig = %u\n", ig);
  printf("ib = %u\n", ib);

  return 1;

}