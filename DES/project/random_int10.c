// 
// Program:  random.c
// Name:  James E. Stine, Jr.
// Purpose: to create some random hex digits
// 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>

#define STR_LEN 2

int main(void) {

    int temp;
    int i, j, p;
    int16_t number1;
    int16_t number2;
    int NUMB[3];
    int SECTION[3];
    
    unsigned char str1[STR_LEN + 1] = {0};
    unsigned char str2[STR_LEN + 1] = {0};    
    const char *hex_digits16 = "0123456789abcdef";
    const char *hex_digits8 = "01234567";
    const char *hex_digits4 = "0123";
    const char *hex_digits2 = "01";
    
    srand(time(NULL));
    NUMB[0] = 17/2;
    NUMB[1] = 21/2;
    NUMB[2] = 24/2;
    SECTION[0] = 60712;
    SECTION[1] = 60714;
    SECTION[2] = 60716;

    for (i = 0; i < 3; i++) {
      for (j = 0; j < NUMB[i]; j++) {
	
	str1[0] = hex_digits4[ rand() % 4 ];
	
	str2[1] = hex_digits16[ rand() % 16 ];
	str2[0] = hex_digits16[ rand() % 16 ];

	printf("Section %d : %d = %s_%s\n", SECTION[i], j, str1, str2);
      }
      printf("\n");
    }
 
    return EXIT_SUCCESS;
}
