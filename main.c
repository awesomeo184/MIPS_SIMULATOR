#pragma warning(disable : 4996) // _s(언더바s) 함수 말고 기본 함수사용해도 오류 안 나게 
#pragma warning(disable : 6031) 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/errno.h>
#define BYTE 1 //바이트
#define HWORD 2 //하프워드
#define WORD 4 //워드
#define READ 0 //읽기
#define WRITE 1 //쓰기
#define RERUN 1 //다시 실행
#define EXIT 0 //종료
#define INST_SIZE 4 //명령어의 크기 정의, 명령어가 4바이트 단위여서 4로 정의
#define ERROR -1 //오류 발생 메세지
#define UPPER_3BIT 56 // 111 000
#define LOWER_3BIT 7 //000 111
#define JR_SYS 1 //jr syscall
#define MF 2 //mfhi mflo
#define MUL 3 
#define ARITH_LOGIC 4 
#define SLT 5 
#define BRANCH_JUMP_INST 0 
#define IMM_INST 1 
#define LOAD_INST 4 
#define STORE_INST 5 
#define BLTZ 1 
#define R_Format 0 
#define JR 0 
#define J 2 
#define JAL 3 
#define BEQ 4 
#define BNE 5
#define LUI 7
#define MFHI 0
#define ADD 0
#define SUBSTRACT 1
#define AND 0
#define OR 1
#define XOR 2
#define NOR 3
#define SLL 0
#define SRL 2
#define SRA 3
#define SHIFT 0
#define LB 0
#define LW 3
#define LBU 4
#define SB 0
#define SW 3
#define ORIGIN_ADDR 0x400024

typedef union instructionRegister
{
	unsigned int inst;
	struct RFormat
	{
		unsigned int funct : 6;
		unsigned int shamt : 5;
		unsigned int rd : 5;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int opcode : 6;
	}RI;
	struct IFormat
	{
		int offset : 16;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int opcode : 6;
	}II;
	struct JFormat
	{
		unsigned int target : 26;
		unsigned int opcode : 6;
	}JI;

}INST_REG;

typedef struct _INST
{
	INST_REG IR;
	int address;
}INST;

int reg[32];
int HI = 0, LO = 0;
unsigned int PC = 0x400000;
unsigned char progMEM[0x100000], dataMEM[0x100000], stackMEM[0x100000];
unsigned int instructionNumber = 0; //명령어의 개수를 저장할 변수
unsigned int dataNumber = 0; //데이터의 개수를 저장할 변수

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
int readInstruction(const unsigned char MEM[], unsigned int i, const unsigned int n) {
	int result = 0; //명령어를 저장할 변수

	for (; i < n; i++) {
		result |= MEM[i]; //명령어를 비트 OR 연산하여 저장함.

		if (i == n - 1) { //마지막 비트 OR 연산 후 탈출
			break;
		}

		result <<= 8; //그 다음 명령어가 순서 대로 쌓일 수 있게 1바이트만큼 왼쪽으로 비트시프트 연산함.
	}

	return result; //결과 반환

}

void printInstruction(const INST IR) { //명령어들은 
	const char* opcode_table[] = //opcode 선택을 위한 룩업테이블
	{ "R", "bltz", "j", "jal", "beq", "bne", 0, 0, "addi", 0, "slti", 0, "andi", "ori", "xori", "lui", 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "lb", 0, 0, "lw", "lbu", 0, 0, 0, "sb", 0, 0, "sw", 0, 0, 0, 0 };

	const char* funct_table[] = //funct 선택을 위한 룩업테이블
	{ "sll", 0, "srl", "sra", 0, 0, 0, 0, "jr", 0, 0, 0, "syscall", 0, 0, 0, "mfhi", 0, "mflo", 0, 0, 0, 0, 0,
	"mul", 0, 0, 0, 0, 0, 0, 0, "add", 0, "sub", 0, "and", "or", "xor", "nor", 0, 0, "slt", 0, 0, 0, 0, 0 };

	if (IR.address == PC) {
		printf("▶");
	}
	else {
		printf("  ");
	}


	if (IR.IR.RI.opcode == R_Format) { //RFormat
		switch (((IR.IR.RI.funct) & UPPER_3BIT) >> 3) {
		case SHIFT: //shift 명령어들
			printf("%s $%d, $%d, %d\n", funct_table[IR.IR.RI.funct], IR.IR.RI.rd, IR.IR.RI.rt, IR.IR.RI.shamt);
			break;

		case JR_SYS:
			if (((IR.IR.RI.funct) & LOWER_3BIT) == JR) { //jr
				printf("%s $%d\n", funct_table[IR.IR.RI.funct], IR.IR.RI.rs);
			}
			else { //syscall
				printf("%s\n", funct_table[IR.IR.RI.funct]);
			}
			break;

		case MF: //mhfi, mflo
			printf("%s %d\n", funct_table[IR.IR.RI.funct], IR.IR.RI.rd);
			break;

		default: //그외 R타입 명령여들
			printf("%s $%d, $%d, $%d\n", funct_table[IR.IR.RI.funct], IR.IR.RI.rd, IR.IR.RI.rs, IR.IR.RI.rt);
			break;

		}
	}
	else { //IFormat, JFormat
		switch (((IR.IR.II.opcode) & UPPER_3BIT) >> 3) {
		case BRANCH_JUMP_INST:

			switch (IR.IR.II.opcode & LOWER_3BIT) {
			case BLTZ: //bltz
				printf("%s $%d, %d\n", opcode_table[IR.IR.II.opcode], IR.IR.II.rs, IR.IR.II.offset * INST_SIZE);
				break;

			case J: //J-Format 명령어들
			case JAL:
				printf("%s 0x%08X\n", opcode_table[IR.IR.JI.opcode], (IR.IR.JI.target << 2) | ((PC + 4) & 0xF0000000));
				break;

			case BEQ: //그 외 분기 명령어들
			case BNE:
				printf("%s $%d, $%d, %d\n", opcode_table[IR.IR.II.opcode], IR.IR.II.rs, IR.IR.II.rt, IR.IR.II.offset * INST_SIZE);
				break;
			}
			break;

		case IMM_INST:
			if ((IR.IR.II.opcode & LOWER_3BIT) == LUI) { //lui			
				printf("%s $%d, %d\n", opcode_table[IR.IR.II.opcode], IR.IR.II.rt, IR.IR.II.offset);
			}
			else { //그외 imm 사용하는 명령어들
				printf("%s $%d, $%d, %d\n", opcode_table[IR.IR.II.opcode], IR.IR.II.rt, IR.IR.II.rs, IR.IR.II.offset);
			}
			break;

		case LOAD_INST: //메모리 접근 명령어들 출력
		case STORE_INST:
			printf("%s $%d, %d($%d)\n", opcode_table[IR.IR.II.opcode], IR.IR.II.rt, IR.IR.II.offset, IR.IR.II.rs);
			break;
		}
	}
}

void conductInstruction(const INST IR) { //실제 명령어들을 실행시키기 위한 함수
	const char* opcode_table[] = //opcode 선택을 위한 룩업테이블
	{ "R", "bltz", "j", "jal", "beq", "bne", 0, 0, "addi", 0, "slti", 0, "andi", "ori", "xori", "lui", 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "lb", 0, 0, "lw", "lbu", 0, 0, 0, "sb", 0, 0, "sw", 0, 0, 0, 0 };

	const char* funct_table[] = //funct 선택을 위한 룩업테이블
	{ "sll", 0, "srl", "sra", 0, 0, 0, 0, "jr", 0, 0, 0, "syscall", 0, 0, 0, "mfhi", 0, "mflo", 0, 0, 0, 0, 0,
		"mul", 0, 0, 0, 0, 0, 0, 0, "add", 0, "sub", 0, "and", "or", "xor", "nor", 0, 0, "slt", 0, 0, 0, 0, 0 };

	int access_size = 0;

	if (IR.IR.RI.opcode == R_Format) { //RFormat
		switch (((IR.IR.RI.funct) & UPPER_3BIT) >> 3) { //상위 3bit로 명령어 종류 파악
		case SHIFT: //shift
			REG(IR.IR.RI.rd, ALU(IR.IR.RI.rt, IR.IR.RI.shamt, IR.IR.RI.funct), WRITE);
			setPC(PC + 4);
			break;

		case JR_SYS: //jr와 syscall
			if (((IR.IR.RI.funct) & LOWER_3BIT) == JR) { //jr
				setPC(REG(IR.IR.RI.rs, 0, READ)); //rs에 저장된 주소를 읽어서 점프
			} 
			else { //syscall
				setPC(PC + 4);
			}
			break;

		case MF: //mfhi와 mflo
			if (((IR.IR.RI.funct) & LOWER_3BIT) == MFHI) { //mfhi
				REG(IR.IR.RI.rd, HI, WRITE); //HI의 내용을 읽어서 저장
			} 
			else { //mflo
				REG(IR.IR.RI.rd, LO, WRITE); //LO의 내용을 읽어서 저장
			}			
			setPC(PC + 4);
			break;

		default: //나머지 명령어들
			REG(IR.IR.RI.rd, ALU(IR.IR.RI.rs, IR.IR.RI.rt, IR.IR.RI.funct), WRITE); 
			setPC(PC + 4);
		}		
	}
	else { //IFormat, JFormat
		switch (((IR.IR.II.opcode) & UPPER_3BIT) >> 3)	{ //상위 3bit로 명령어 종류 파악
		case BRANCH_JUMP_INST:
			switch(IR.IR.II.opcode & LOWER_3BIT) {
			case BLTZ: //bltz
				if (REG(IR.IR.II.rs, 0, READ) < 0) { //값이 0보다 작으면
					setPC(PC + IR.IR.II.offset * INST_SIZE); //지정된 곳으로 브런치.
				} 
				else {
					setPC(PC + 4); //아니면 PC를 4만 증가시킴.
				}
					
				break;
			case J: //J-Format
			case JAL:
				setPC((IR.IR.JI.target << 2) | ((PC + 4) & 0xF0000000)); //다음 PC에서 상위 4bit를 추출한 것을 offset을 2bit sll한 것과 bitwise or하여 PC 설정
				break;
			case BEQ: //beq
				if (REG(IR.IR.II.rs, 0, READ) == REG(IR.IR.II.rt, 0, READ)) { //레지스터의 내용이 같으면
					setPC(PC + IR.IR.II.offset * INST_SIZE); //분기하고
				}
				else { //아니면
					setPC(PC + 4); //PC를 4증가
				}
				break;
			case BNE: //bne
				if (REG(IR.IR.II.rs, 0, READ) != REG(IR.IR.II.rt, 0, READ)) { //레지스터의 내용이 다르면
					setPC(PC + IR.IR.II.offset * INST_SIZE); //분기하고
				}
				else { //아니면
					setPC(PC + 4); //PC를 4증가
				}
				break;
			}
			break;

		case IMM_INST: //뒤에 i가 붙는 명령어들
			if ((IR.IR.II.opcode & LOWER_3BIT) == LUI) { //lui는 포맷이 다르기 때문에 따로 설정
				REG(IR.IR.II.rt, IR.IR.II.offset & 0xFFFF0000, WRITE); //상위 16bit를 읽어서 저장
			}
			else { //그외 imm 사용하는 명령어들
				REG(IR.IR.II.rt, ALU(IR.IR.II.rs, IR.IR.II.offset, IR.IR.II.opcode), WRITE); 
			}

			setPC(PC + 4);
			break;
		
		case LOAD_INST:	//load 계열 명령어
			switch (IR.IR.II.opcode & LOWER_3BIT) { //하위 3bit로 명령어 세부 판단
			case LB: 
			case LBU: //byte 단위 명령어면 바이트로 설정
				access_size = BYTE;
				break;
			case LW: //word 단위명 워드로 설정
				access_size = WORD;
				break;
			}

			REG(IR.IR.II.rt, MEM(IR.IR.II.rs + IR.IR.II.offset * access_size, 0, READ, access_size), WRITE);			
			setPC(PC + 4);
			break;
		case STORE_INST: //store 계열 명령어
			switch (IR.IR.II.opcode & LOWER_3BIT) { //하위 3bit로 명령어 세부 판단
			case SB: //byte 단위명 바이트로 설정
				access_size = BYTE;
				break;
			case SW: //word 단위면 워드로 설정
				access_size = WORD;
				break;
			}

			MEM(IR.IR.II.rs + IR.IR.II.offset, REG(IR.IR.II.rs + (IR.IR.II.offset * access_size), IR.IR.II.rt, READ), WRITE, access_size);
			setPC(PC + 4);
			break;
		}			
	}
}


void waitInput() { //화면 지우기 전에 입력을 기다리는 함수
	printf("Press Enter to continue...\n");
	rewind(stdin);
	getchar();
}

int loadFile(const char* file_name) { //파일을 읽어오는 
	FILE* fp = NULL; //파일을 읽어올 파일 포인터
	unsigned int i = 0; //반복문에 쓸 변수
	INST IR;
	static unsigned char MEM[16] = { 0 }; //파일에서 불러오는 명령어들을 저장할 변수

	fp = fopen(file_name, "rb"); //파일 열기

	if (fp == NULL) { //파일 열기 오류 발생시 처리
		printf("파일 열기 오류발생\n"); //문구 출력 후 종료
		return ERROR;
	}

	while (i < 2) { //명령어와 데이터의 개수를 가지고 있는 데이터들만
		fseek(fp, i * INST_SIZE, SEEK_SET); //파일포인터의 위치를 4바이트씩 증가시킴.
		fread(&MEM[i * INST_SIZE], INST_SIZE, 1, fp); //파일포인터의 현 위치에서 메모리에 명령어를 4바이트씩 MEM에 저장함.
		i++; //인덱스 증가
	}

	instructionNumber = readInstruction(MEM, 0, INST_SIZE); //명령어의 개수를 불러옴. (처음 4바이트는 명령어의 개수)
	dataNumber = readInstruction(MEM, INST_SIZE, INST_SIZE * 2); //데이터의 개수를 불러옴. (그 다음 4바이트는 데이터의 개수)


	while (i < instructionNumber + 2) { //명령어의 개수를 가진만큼
		fseek(fp, i * INST_SIZE, SEEK_SET); //파일포인터의 위치를 4바이트씩 증가시킴.
		fread(&progMEM[i * INST_SIZE], INST_SIZE, 1, fp); //파일포인터의 현 위치에서 메모리에 명령어를 4바이트씩 프로그램 메모리에 저장함.
		i++; //인덱스 증가
	}

	while (feof(fp) == 0) { //파일포인터가 파일의 끝에 도달할 때까지
		fseek(fp, i * INST_SIZE, SEEK_SET); //파일포인터의 위치를 4바이트씩 증가시킴.
		fread(&dataMEM[i * INST_SIZE], INST_SIZE, 1, fp); //파일포인터의 현 위치에서 메모리에 명령어를 4바이트씩 데이터 메모리에 저장함.
		i++; //인덱스 증가
	}
	
	for (i = 8; i < instructionNumber * INST_SIZE + INST_SIZE * 2; i += INST_SIZE) { //명령어의 개수에 명령어의 크기 단위를 곱한만큼 루프를 돔.
		IR.IR.inst = readInstruction(progMEM, i, i + INST_SIZE); //명령어를 차례대로 IR에 읽어온 후
		IR.address = ORIGIN_ADDR + i - 8;
		printInstruction(IR); //IR를 해석하여 출력
	}

	fclose(fp);

	return 1;
}

void step() //
{
	unsigned int i = 0, j = 0;
	INST IR;
	char selection = 0;
	int input = 1;
	int address = 0;
		

	for (i = 8; i < instructionNumber * INST_SIZE + INST_SIZE * 2; i += INST_SIZE) { //명령어의 개수에 명령어의 크기 단위를 곱한만큼 루프를 돔.
		
		for (j = 8; j < instructionNumber * INST_SIZE + INST_SIZE * 2; j += INST_SIZE) //명령어의 개수에 명령어의 크기 단위를 곱한만큼 루프를 돔.
		{
			IR.IR.inst = readInstruction(progMEM, j, j + INST_SIZE); //명령어를 차례대로 IR에 읽어온 후
			IR.address = ORIGIN_ADDR + j - 8;
			printInstruction(IR); //IR를 해석하여 출력
		}

		IR.IR.inst = readInstruction(progMEM, i, i + INST_SIZE); //명령어를 차례대로 IR에 읽어온 후
		conductInstruction(IR);

		if (input == 1) {
			printf("계속하려면 s를 누르세요.");
			rewind(stdin);
			scanf("%c", &selection);
		}

		switch (selection) {
		case 'g':
			input = 0;
			selection = 0;
			break;
		case 's':
			break;
		case 'j':
			scanf("%x", &address);
			i += (address - PC);
			setPC(address);
			break;
		}

		if (i < instructionNumber * INST_SIZE + INST_SIZE) {
			system("cls");
		}

	}
}

void go() { //g 
	unsigned int i = 0, j = 0;
	INST IR;
	char selection = 0;

	setPC(ORIGIN_ADDR);

	for (i = 8; i < instructionNumber * INST_SIZE + INST_SIZE * 2; i += INST_SIZE) { //명령어의 개수에 명령어의 크기 단위를 곱한만큼 루프를 돔.

		for (j = 8; j < instructionNumber * INST_SIZE + INST_SIZE * 2; j += INST_SIZE) //명령어의 개수에 명령어의 크기 단위를 곱한만큼 루프를 돔.
		{
			IR.IR.inst = readInstruction(progMEM, j, j + INST_SIZE); //명령어를 차례대로 IR에 읽어온 후
			IR.address = ORIGIN_ADDR + j - 8;
			printInstruction(IR); //IR를 해석하여 출력
		}

		IR.IR.inst = readInstruction(progMEM, i, i + INST_SIZE); //명령어를 차례대로 IR에 읽어온 후
		conductInstruction(IR);

		if (i < instructionNumber * INST_SIZE + INST_SIZE) {
			system("cls");
		}
	}
}

void clearMemory() { //메모리 비워주는 함수
	memset(progMEM, 0, sizeof(progMEM));
	memset(stackMEM, 0, sizeof(stackMEM));
	memset(dataMEM, 0, sizeof(dataMEM));
	instructionNumber = 0;
	dataNumber = 0;
}

int printMenu() { //UI 함수
	char selection = '\0';
	char fileName[32] = { 0 };

	printf("명령어 입력: "); 
	scanf("%c", &selection);

	switch (selection) {
	case 's':
		step();
		setPC(ORIGIN_ADDR);
		return RERUN;
	case 'l':
		clearMemory();
		scanf("%s", fileName);

		loadFile(fileName);		
		return RERUN;
	case 'g':
		go();
		return RERUN;
	case 'x':
		return EXIT;
	case 'r':
		//구현 필요
		return RERUN;
	case 'v':
		//구현 필요
		return RERUN;
	default:
		printf("잘못된 명령어를 입력하셨습니다.\n");
		return RERUN;
	}
}


int main() {
    setPC(ORIGIN_ADDR);

	while (printMenu()) {
 		waitInput();
		system("cls");
	}
}

