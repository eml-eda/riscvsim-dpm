#include <stdio.h>
#include <stdint.h>

int main()
{
    // read sensor
    for(int i=0; i<10; i++){
    printf("data from sensor 1: %x\n", *((uint32_t*) 0x20000000));
    printf("data from sensor 2: %x\n", *((uint32_t*) 0x20000100));
    printf("data from sensor 3: %x\n\n", *((uint32_t*) 0x20000200));
    // compute something

    //go to sleep
    
    }
    return 0;
}
