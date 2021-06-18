#include "message_handling.h"

#include <stdio.h>
#include <SPIv1.h> // necessary, otherwise CC1200 prototype are not available
#include <unistd.h>
#include <stdbool.h>

struct RX_data{
    int x;
    int y;
    int z;
}

struct RX_data test;

void tester(int a, int b){
    test.x = a;
    test.y = b;
    test.z = b;
    printf("done...\n");
}

int print_values(){
    printf("%d\n", test.x);
    printf("%d\n", test.y);
    printf("%d\n", test.z);
}