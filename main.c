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
    unsigned int sel, offset;
	unsigned char* pM;

	if (S < 0 || S > 2) {
		printf("Wrong Memory Size Input\n Only can access with byte(0), halfword(1), or word(2)\n");
		return 0;
	}

	sel = A >> 20;
	offset = A & 0xFFFFF;

	if (sel == 0x004)
		pM = progMEM;         // program memory

	else if (sel == 0x100)
		pM = dataMEM;  // data memory

	else if (sel == 0x7FF)
		pM = stackMEM;  // stack

	else {
		printf("No memory\n");
		exit(1);
	}

	if (S == 0) {  // byte
		if (nRW == 0) { // read
			return pM[offset];
		}
		else if (nRW == 1) { // write
			pM[offset] = V;
		}
	}

	else if (S == 1) { // half word
		if (nRW == 0) { // read
			return (pM[offset] | (pM[offset + 1] << 8));
		}
		else if (nRW == 1) { // write
			pM[offset] = V;
			pM[offset + 1] = V >> 8;
		}
	}

	else if (S == 2) { // word
		if (nRW == 0) { // read
			return (pM[offset] | (pM[offset + 1] << 8) | (pM[offset + 2] << 16) | (pM[offset + 3] << 24));
		}
		else if (nRW == 1) { // write
			pM[offset] = V;
			pM[offset + 1] = V >> 8;
			pM[offset + 2] = V >> 16;
			pM[offset + 3] = V >> 24;
		}
	}

	else {
		printf("Wrong instrustions\n Only can Read(0), or Write(1)\n");
		return 0;
	}
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

