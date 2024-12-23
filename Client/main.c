#include "ds18b20.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    float                   temperature;

    while (1)
    {
        if (get_temperature(&temperature) == 0)
        {
            printf("Successfully fetched temperature: %.2f℃", temperature);
        }
        else
        {
            fprintf(stderr, "Failed to fetch temperature\n");
        }

        sleep(60); // 每分钟运行一次
    }


    return 0;
}