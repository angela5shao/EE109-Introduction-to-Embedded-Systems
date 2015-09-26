void ds1631_init(void); // initializes I2C & DS1631
void ds1631_conv(void); // sets DS1631 to do continuous temp conversions
void ds1631_temp(unsigned char *); // read 2 bytes with temperature in Celsius


/*

#include <stdio.h>

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    return 0;
} */
