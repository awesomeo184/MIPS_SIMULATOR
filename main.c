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
    const char* register_table[] = { "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1",
    "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra" };

    if (rn >= 0 && rn <= 31) {
        printf("%s", register_table[rn]);
    }
    else {
        printf("잘못된 레지스터 값이 입력되었습니다.\n");
    }  
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
