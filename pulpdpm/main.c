/* PMSIS includes */
#include "pmsis.h"

#define AXI_BASE 0x10500000
/* Program Entry. */
int main(void)
{
    printf("\n\n\t *** PMSIS HelloWorld ***\n\n");
    printf("Entering main controller\n");

    int *pm_reg = (volatile int *)AXI_BASE;

    // *pm_reg = 1;
    /* Init cluster configuration structure. */
    for (int i = 0; i < 10; i++)
    {
        printf("data from sensor 1: ");
        printf("data from sensor 2: ");
        printf("data from sensor 3: ");
    }
    printf("Bye !\n");

    

    for (int k = 0; k < 500; k++)
    {
        int j = k;
        j += k;
    }

    return 0;
}
