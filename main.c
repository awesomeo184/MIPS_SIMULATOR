#pragma warning (disable : 4996) // _s(언더바s) 함수 말고 기본 함수사용해도 오류 안 나게 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/errno.h>

int reg[32];
int HI = 0, LO = 0;
unsigned int PC = 0x400000;
unsigned char progMEM[0x100000], dataMEM[0x100000], stackMEM[0x100000];

// fopen_s adaptor for MacOS
errno_t fopen_s(FILE **f, const char *name, const char *mode) {
    errno_t ret = 0;
    assert(f);
    *f = fopen(name, mode);
    /* Can't be sure about 1-to-1 mapping of errno and MS' errno_t */
    if (!*f)
        ret = errno;
    return ret;
}

void setPC(unsigned int val) {

}


void find_register(unsigned int rn) {

}

int ALU(int fct, int v1, int v2) {
}

//레지스터 접근 함수
int REG(int A, int v, int nRW) {

}

//메모리 접근 함수
int MEM(unsigned int A, int V, int nRW, int S) {

}

// 명령어 처리
int find_instructions(unsigned int IR) {

}

unsigned int BigEndian(unsigned int data) {
    unsigned char dataChar[4];

    dataChar[3] = data;
    data = data >> 8;
    dataChar[2] = data;
    data = data >> 8;
    dataChar[1] = data;
    data = data >> 8;
    dataChar[0] = data;

    return *(unsigned int*)dataChar;
}


int main() {
    FILE* pFile = NULL;
    errno_t err;
    int count;
}

