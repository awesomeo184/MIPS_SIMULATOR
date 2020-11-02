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
	unsigned int data;
	char com[5]; //command저장
	int i;	//파일 read시 명령어 라인 수
	char filename[50]; //l command에서 사용. open 할 파일 이름
	char jumpStart[20]; //j command에서 사용. jump할 주소 저장
	char start[20], end[20]; //m command에서 사용. 보여줄 메모리 시작과 끝 위치 저장
	char regNum[20], srVal[20]; //sr command에서 사용. 레지스터와 변경값 저장
	char loc[20], smVal[20]; //sm command에서 사용. 메모리 주소와 변경값 저장.

	printf("<<MIPS Simulator>>\n");
	printf("Made by 강경욱, 노은진, 박승리, 정수현\n\n");
	printf("-------------------------------------------------------\n");
	printf("l <file name>\t\t: Load program\nj <starting address>\t: Jump program\ng\t\t\t: Go program\ns\t\t\t: Step\n");
	printf("m <start> <end>\t\t: View Memory\nr\t\t\t: View register\nx\t\t\t: Program exit\n");
	printf("sr <reg num> <value>\t: 특정 레지스터의 값 설정\nsm <location> <value>\t: 메모리 특정 주소의 값 설정\n");
	printf("cp\t\t\t: command guide print\n"); //사용 가능한 command 다시 출력.
	printf("-------------------------------------------------------\n");

	while (1) {
		printf("\nEnter command: ");
		scanf("%s", com);

		if (com[0] == 'l') {//load program
			scanf("%s", filename);
			i = 0;
			err = fopen_s(&pFile, filename, "rb"); //as_ex01_arith.bin, as_ex02_logic.bin, as_ex03_ifelse.bin, as_ex04_fct.bin
			if (err) {
				printf("Cannot open file\n");
				return 1;
			}

			memset(progMEM, 0, sizeof(progMEM));
			memset(dataMEM, 0, sizeof(dataMEM));
			memset(stackMEM, 0, sizeof(stackMEM));

			while (1) {
				count = fread(&data, sizeof(data), 1, pFile);
				if (count != 1)
					break;
				data = BigEndian(data);

				if (i <= 1) {
					if (i == 0) {
						printf("Number of Instuntions: %d", data);
					}
					else {
						printf(", Number of Data: %d\n", data);
					}
				}
				else {
					printf("%8x\n", data);
				}
				i++;
			}
			fclose(pFile);
		}

		else if (com[0] == 's' && com[1] == 'r') {//레지스터 값 설정
			scanf("%s", regNum);
			scanf("%s", srVal);
			printf("sr, %s, %s\n", regNum, srVal);
		}

		else if (com[0] == 's' && com[1] == 'm') {//메모리 값 설정
			scanf("%s", loc);
			scanf("%s", smVal);
			printf("sm, %s, %s\n", loc, smVal);
		}

		else if (com[0] == 'j') {//jump program
			scanf("%s", jumpStart);
			printf("current PC: %x\n", PC);
			setPC(strtol(jumpStart, NULL, 16));
			printf("PC moved to %x\n", PC);
		}

		else if (com[0] == 'g') {//go program
			printf("go\n");
		}

		else if (com[0] == 's') {//step
			printf("step\n");
		}

		else if (com[0] == 'm') {//view memory
			scanf("%s", start);
			scanf("%s", end);
			printf("memory, %s, %s\n", start, end);
		}

		else if (com[0] == 'r') {//view register
			printf("register\n");
		}

		else if (com[0] == 'x')//program exit
			return 0;

		else if (com[0] == 'c' && com[1] == 'p') { //command guide print
			printf("-------------------------------------------------------\n");
			printf("l <file name>\t\t: Load program\nj <starting address>\t: Jump program\ng\t\t\t: Go program\ns\t\t\t: Step\n");
			printf("m <start> <end>\t\t: View Memory\nr\t\t\t: View register\nx\t\t\t: Program exit\n");
			printf("sr <reg num> <value>\t: 특정 레지스터의 값 설정\nsm <location> <value>\t: 메모리 특정 주소의 값 설정\n");
			printf("cp\t\t\t: command guide print\n");
			printf("-------------------------------------------------------\n");
		}

		else
			printf("wrong command please check\n");
	}
	return 0;
}
