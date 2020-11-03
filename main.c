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
    PC = val;
    return;
}

void find_register(unsigned int rn) {

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

}

unsigned int getOpcode(unsigned int IR) {
    unsigned int opcode = IR >> 26;
    return opcode;
}

unsigned int getFunct(unsigned int IR) {
    unsigned int funct = (IR << 26) >> 26;
    return funct
}

int find_instructions(unsigned int IR) {
    unsigned int opc;
    unsigned int funct;
    char inst[8] = ""; //instruction name

    opc = getOpcode(IR);
    funct = getFunct(IR);
    // keep working
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
	int linecount;	//파일 read시 명령어 라인 수
	char filename[50]; //l command에서 사용. open 할 파일 이름
	unsigned int jumpStart = 0; //j command에서 사용. jump할 주소 저장
	unsigned int start = 0, end = 0; //m command에서 사용. 보여줄 메모리 시작과 끝 위치 저장
	unsigned int regNum = 0;
	int srVal = 0; //sr command에서 사용. 레지스터와 변경값 저장
	unsigned int loc = 0;
	int smVal = 0; //sm command에서 사용. 메모리 주소와 변경값 저장.
	char check[2]; //y/n check

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
			linecount = 0;
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

				if (linecount <= 1) {
					if (linecount == 0) {
						printf("Number of Instuntions: %d", data);
					}
					else {
						printf(", Number of Data: %d\n", data);
					}
				}
				else {
					printf("%8x\n", data);
				}
				linecount++;
			}
			fclose(pFile);
		}

		else if (com[0] == 's' && com[1] == 'r') {//레지스터 값 설정
			scanf("%d", &regNum);
			scanf("%d", &srVal);

			if (regNum < 0 || regNum >= 32) {
				printf("Wrong register number\n");
				continue;
			}

			find_register(regNum);
			printf(" = %x to ", reg[regNum]);
			find_register(regNum);
			printf(" = %x Are you sure? [y/n]: ", srVal);
			scanf("%s", check);

			if (check[0] == 'y') {
				reg[regNum] = srVal;
				printf("Change Successful!\n");
			}
			else{
				printf("Change Failed!\n");
			}
		}

		else if (com[0] == 's' && com[1] == 'm') {//메모리 값 설정
			scanf("%x", &loc);
			scanf("%d", &smVal);
			printf("sm, %x, %x\n", loc, smVal);
		}

		else if (com[0] == 'j') {//jump program
			scanf("%x", &jumpStart);
			printf("current PC: %x\n", PC);
			setPC(jumpStart);
			printf("PC moved to %x\n", PC);
		}

		else if (com[0] == 'g') {//go program
			printf("go\n");
		}

		else if (com[0] == 's') {//step
			printf("step\n");
		}

		else if (com[0] == 'm') {//view memory
			scanf("%x", &start);
			scanf("%x", &end);
			printf("memory, %s, %s\n", start, end);
		}

		else if (com[0] == 'r') {//view register
			for (int i = 0; i < 32; i++) {
				find_register(i);
				printf(": %x\n", reg[i]);
			}
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
