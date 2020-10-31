#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define PROG 0x004
#define DATA 0x100
#define STACK 0x7FF
#define BYTE 1
#define HWORD 2
#define WORD 4
#define READ 0
#define WRITE 1
#define NONE 0
#define RERUN 1
#define EXIT 0
#define ERROR -1

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

int ALU(int fct, int v1, int v2) {
	if (fct == 0)//sll
		return v1 << v2;
	else if (fct == 2) {//srl
		return (v1 >> v2) & (0xffffffff >> v2);
	}
	else if (fct == 3)//sra
		return v1 >> v2;
	else if (fct == 24)//mul
		return v1 * v2;
	else if (fct == 32)//add
		return v1 + v2;
	else if (fct == 34)//sub
		return v1 - v2;
	else if (fct == 36)//and
		return v1 & v2;
	else if (fct == 37)//or
		return v1 | v2;
	else if (fct == 38)//xor
		return v1 ^ v2;
	else if (fct == 39)//nor
		return ~(v1 | v2);
	else if (fct == 42) {//slt
		if (v1 < v2)
			return 1;
		else
			return 0;
	}
	else {
		printf("wrong ALU function");
	}
}

int REG(int A, int v, int nRW) {

}

int MEM(unsigned int A, int V, int nRW, int S) {
	unsigned int memory_access_unit[3] = { BYTE, HWORD , WORD }; //선택한 메모리에 따른 크기 결정할 룩업테이블
	unsigned int sel = 0, offset = 0; //메모리 주소 판단하여 저장 변수들
	unsigned char* pM = NULL; //선택한 메모리를 가리키게 할 포인터
	unsigned int memory_access_size = 0; //메모리 접근 사이즈를 저장할 변수
	unsigned int result = 0; //결과 저장할 변수
	unsigned int i = 0, tmp = 0;
	unsigned int extraction = 0;

	if (S > 2 || S < 0) { //BYTE ~ WORD의 명령어 선택을 안 했을 시 오류 출력 후 반환
		printf("Memory access size error");
		return ERROR;
	}


	sel = A >> 20; //어떤 메모리인지 추출
	offset = A & 0xFFFFF; //어떤 장소에 저장하는지 추출
	memory_access_size = memory_access_unit[S]; //메모리 접근 단위 추출

	switch (sel)
	{
	case PROG: //프로그램 메모리
		pM = progMEM;
		break;
	case DATA: //데이터 메모리
		pM = dataMEM;
		break;
	case STACK: //스택 메모리
		pM = stackMEM;
		break;
	default: //잘못 입력한 경우
		printf("No Memory\n");
		return ERROR;
	}

	switch (nRW) { //Read / Wirte 작업
	case READ:
		if (offset % memory_access_size != 0) //메모리 접근 단위가 메모리와 aligned 되어있지 않은 경우 
		{
			printf("Memory Access Error!\n"); //오류 문구 출력 후 반환
			return ERROR;
		}

		for (i = 0; i < memory_access_size; i++) { //메모리 접근 단위만큼 반복
			result |= pM[offset + i]; //결과에  메모리에 있는 값 옮김.

			if (i == memory_access_size - 1) { //마지막에 탈출
				break;
			}

			result <<= 8; //결과를 한 바이트씩 왼쪽으로 시프트하면서 루프 돎.
		}

		return result;
	case WRITE:
		if (offset % memory_access_size != 0) { //메모리 접근 단위가 메모리와 aligned 되어있지 않은 경우 
			printf("Memory Access Error!\n"); //오류 문구 출력 후 반환
			return ERROR;
		}

		switch (memory_access_size) { //메모리 접근 단위에 따라 추출하는 범위 설정
		case BYTE:
			extraction = 0xFF;
			break;
		case HWORD:
			extraction = 0xFF00;
			break;
		case WORD:
			extraction = 0xFF000000;
			break;
		}


		for (i = 0; i < memory_access_size; i++) {
			tmp = (V << (8 * i)) & extraction; //추출 후,
			pM[offset + i] = tmp >> (8 * i); //저장
		}

		return NONE;
	default:
		printf("Error!\n");
		return ERROR;
	}
}

int find_instructions(unsigned int IR) {

}


int main() {
    printf("Hello, World!\n");
    return 0;
}
