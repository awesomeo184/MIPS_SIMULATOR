#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int regi[32];
int HI = 0, LO = 0;
unsigned int PC = 0x400000;
unsigned char progMEM[0x100000], dataMEM[0x100000], stackMEM[0x100000];

void setPC(unsigned int val) {
    PC = val;
    return;
}

void find_register(unsigned int rn) {

}

int ALU(unsigned int funct, int a, int b) {

}

int REG(int A, int v, int nRW) {

}

int MEM(unsigned int A, int v, int nRW, int S) {

}


int find_instructions(unsigned int IR) {

}


int main() {
    printf("Hello, World!\n");
    return 0;
}
