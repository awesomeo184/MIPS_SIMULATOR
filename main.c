#pragma warning(disable : 4996) // _s(언더바s) 함수 말고 기본 함수사용해도 오류 안 나게 
#pragma warning(disable : 6031) 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
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
#define ORIGIN_ADDR 0x400000
#define STACK_ADDR 0x80000000
#define UNLOADED 0
#define LOADED 1


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
unsigned int fileLoad = UNLOADED;

void setPC(unsigned int val) {
	PC = val;
	return;

}


// void find_register(unsigned int rn) {
// 	int result = REG(rn, 0, 0);
// 	return result;
// }

// start of ALU
int ALU(int v1, int v2, const INST_REG IR) {
    if (IR.RI.opcode == R_Format) {
        switch (IR.RI.funct) {
            case 0: //sll
                return v1 << v2;
            case 2: //srl
                return (v1 >> v2) & (0xffffffff >> v2);
            case 3: //sra
                return v1 >> v2;
            case 24: //mul
                return v1 * v2;
            case 32: //add
                return v1 + v2;
            case 34: //sub
                return v1 - v2;
            case 36: //and
                return v1 & v2;
            case 37: //or
                return v1 | v2;
            case 42: //slt
                if (v1 < v2)
                    return 1;
                else
                    return 0;
        }
    }
    else {
        switch (IR.II.opcode & LOWER_3BIT) {
            case 0://addi
                return v1 + v2;
            case 2://slti
                if (v1 < v2)
                    return 1;
                else
                    return 0;
            case 4://andi
                return v1 & v2;
            case 5://ori
                return v1 | v2;
            case 6://xori
                return v1 ^ v2;
        }
    }
    printf("wrong ALU function");
}
// end of ALU

//레지스터 접근 함수
int REG(int A, long long v, int nRW) {
	if (nRW == 0) return reg[A];		//READ: 0,WRITE: 1
	if (nRW == 1) reg[A] = v;

}

//메모리 접근 함수
int MEM(unsigned int A, int V, int nRW, int S) {
	unsigned int sel, offset;
	unsigned char* pM;

	if (S != BYTE && S != HWORD && S != WORD) {
		printf("Wrong Memory Size Input\n Only can access with byte(0), halfword(2), or word(4)\n");
		return 0;
	}

	sel = A >> 20;
	offset = A & 0xFFFFF;

	if (sel == 0x004)
		pM = progMEM;  // program memory

	else if (sel == 0x100)
		pM = dataMEM;  // data memory

	else if (sel == 0x7FF)
		pM = stackMEM;  // stack

	else {
		printf("No memory\n");
		exit(1);
	}

	if (S == BYTE) {  // byte
		if (nRW == READ) { // read
			return pM[offset];
		}
		else if (nRW == 1) { // write
			pM[offset] = V;
		}
	}

	else if (S == HWORD) { // half word
		if (nRW == 0) { // read
			return (pM[offset] | (pM[offset + 1] << 8));
		}
		else if (nRW == WRITE) { // write
			for (int i = 0; i < HWORD; i++) {
				pM[offset + i] = V >> (i * 8);
			}
		}
	}

	else if (S == WORD) { // word
		if (nRW == READ) { // read
			return (pM[offset] | (pM[offset + 1] << 8) | (pM[offset + 2] << 16) | (pM[offset + 3] << 24));
		}
		else if (nRW == WRITE) { // write
			for (int i = 0; i < WORD; i++) {
				pM[offset + i] = V >> (i * 8);
			}
		}
	}

	else {
		printf("Wrong instrustions\n Only can Read(0), or Write(1)\n");
		return 0;
	}
}

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

void printInstruction(const INST IR) {
	const char* opcodeTable[] = //opcode 선택을 위한 룩업테이블
	{ "R", "bltz", "j", "jal", "beq", "bne", 0, 0, "addi", 0, "slti", 0, "andi", "ori", "xori", "lui", 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "lb", 0, 0, "lw", "lbu", 0, 0, 0, "sb", 0, 0, "sw", 0, 0, 0, 0 };

	const char* functTable[] = //funct 선택을 위한 룩업테이블
	{ "sll", 0, "srl", "sra", 0, 0, 0, 0, "jr", 0, 0, 0, "syscall", 0, 0, 0, "mfhi", 0, "mflo", 0, 0, 0, 0, 0,
	"mul", 0, 0, 0, 0, 0, 0, 0, "add", 0, "sub", 0, "and", "or", "xor", "nor", 0, 0, "slt", 0, 0, 0, 0, 0 };

	if (IR.address == PC) {
		printf("▶");
	}
	else {
		printf("  ");
	}

	printf("  [%08X]", IR.address);


	if (IR.IR.RI.opcode == R_Format) { //RFormat
		switch (((IR.IR.RI.funct) & UPPER_3BIT) >> 3) {
		case SHIFT: //shift 명령어들
			printf("%s $%d, $%d, %d\n", functTable[IR.IR.RI.funct], IR.IR.RI.rd, IR.IR.RI.rt, IR.IR.RI.shamt);
			break;

		case JR_SYS:
			if (((IR.IR.RI.funct) & LOWER_3BIT) == JR) { //jr
				printf("%s $%d\n", functTable[IR.IR.RI.funct], IR.IR.RI.rs);
			}
			else { //syscall
				printf("%s\n", functTable[IR.IR.RI.funct]);
			}
			break;

		case MF: //mhfi, mflo
			printf("%s %d\n", functTable[IR.IR.RI.funct], IR.IR.RI.rd);
			break;

		default: //그외 R타입 명령여들
			printf("%s $%d, $%d, $%d\n", functTable[IR.IR.RI.funct], IR.IR.RI.rd, IR.IR.RI.rs, IR.IR.RI.rt);
			break;

		}
	}
	else { //IFormat, JFormat
		switch (((IR.IR.II.opcode) & UPPER_3BIT) >> 3) {
		case BRANCH_JUMP_INST:

			switch (IR.IR.II.opcode & LOWER_3BIT) {
			case BLTZ: //bltz
				printf("%s $%d, %d\n", opcodeTable[IR.IR.II.opcode], IR.IR.II.rs, IR.IR.II.offset * INST_SIZE);
				break;

			case J: //J-Format 명령어들
			case JAL:
				printf("%s 0x%08X\n", opcodeTable[IR.IR.JI.opcode], (IR.IR.JI.target << 2) | ((PC + 4) & 0xF0000000));
				break;

			case BEQ: //그 외 분기 명령어들
			case BNE:
				printf("%s $%d, $%d, %d\n", opcodeTable[IR.IR.II.opcode], IR.IR.II.rs, IR.IR.II.rt, IR.IR.II.offset * INST_SIZE);
				break;
			}
			break;

		case IMM_INST:
			if ((IR.IR.II.opcode & LOWER_3BIT) == LUI) { //lui			
				printf("%s $%d, %d\n", opcodeTable[IR.IR.II.opcode], IR.IR.II.rt, IR.IR.II.offset);
			}
			else { //그외 imm 사용하는 명령어들
				printf("%s $%d, $%d, %d\n", opcodeTable[IR.IR.II.opcode], IR.IR.II.rt, IR.IR.II.rs, IR.IR.II.offset);
			}
			break;

		case LOAD_INST: //메모리 접근 명령어들 출력
		case STORE_INST:
			printf("%s $%d, %d($%d)\n", opcodeTable[IR.IR.II.opcode], IR.IR.II.rt, IR.IR.II.offset, IR.IR.II.rs);
			break;
		}
	}
}

void conductInstruction(const INST IR) { //실제 명령어들을 실행시키기 위한 함수
	int accessSize = 0;

	if (IR.IR.RI.opcode == R_Format) { //RFormat
		switch (((IR.IR.RI.funct) & UPPER_3BIT) >> 3) { //상위 3bit로 명령어 종류 파악
		case SHIFT: //shift
			REG(IR.IR.RI.rd, ALU(IR.IR.RI.rt, IR.IR.RI.shamt, IR.IR), WRITE);
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
			REG(IR.IR.RI.rd, ALU(IR.IR.RI.rs, IR.IR.RI.rt, IR.IR), WRITE);
			setPC(PC + 4);
		}
	}
	else { //IFormat, JFormat
		switch (((IR.IR.II.opcode) & UPPER_3BIT) >> 3) { //상위 3bit로 명령어 종류 파악
		case BRANCH_JUMP_INST:
			switch (IR.IR.II.opcode & LOWER_3BIT) {
			case BLTZ: //bltz
				if (REG(IR.IR.II.rs, 0, READ) < 0) { //값이 0보다 작으면
					setPC(PC + IR.IR.II.offset * INST_SIZE); //지정된 곳으로 브런치.
				}
				else {
					setPC(PC + 4); //아니면 PC를 4만 증가시킴.
				}

				break;
			case J: //J-Format
				setPC((IR.IR.JI.target << 2) | ((PC + 4) & 0xF0000000)); //다음 PC에서 상위 4bit를 추출한 것을 offset을 2bit sll한 것과 bitwise or하여 PC 설정
				break;
			case JAL:
				REG(31, PC + 4, WRITE); //돌아올 주소를 $ra에 저장
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
				REG(IR.IR.II.rt, IR.IR.II.offset << 16, WRITE); //상위 16bit를 읽어서 저장
			}
			else { //그외 imm 사용하는 명령어들
				REG(IR.IR.II.rt, ALU(IR.IR.II.rs, IR.IR.II.offset, IR.IR), WRITE);
			}

			setPC(PC + 4);
			break;

		case LOAD_INST:	//load 계열 명령어
			switch (IR.IR.II.opcode & LOWER_3BIT) { //하위 3bit로 명령어 세부 판단
			case LB:
			case LBU: //byte 단위 명령어면 바이트로 설정
				accessSize = BYTE;
				break;
			case LW: //word 단위명 워드로 설정
				accessSize = WORD;
				break;
			}

			REG(IR.IR.II.rt, MEM(REG(IR.IR.II.rs, 0, READ) + IR.IR.II.offset, 0, READ, accessSize), WRITE);
			setPC(PC + 4);
			break;
		case STORE_INST: //store 계열 명령어
			switch (IR.IR.II.opcode & LOWER_3BIT) { //하위 3bit로 명령어 세부 판단
			case SB: //byte 단위명 바이트로 설정
				accessSize = BYTE;
				break;
			case SW: //word 단위면 워드로 설정
				accessSize = WORD;
				break;
			}

			MEM(REG(IR.IR.II.rs, 0, READ) + IR.IR.II.offset, REG(IR.IR.II.rs + IR.IR.II.offset, IR.IR.II.rt, READ), WRITE, accessSize);
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

int loadFile(const char* file_name) {
	FILE* fp = NULL; //파일을 읽어올 파일 포인터
	unsigned int i = 0; //반복문에 쓸 변수
	INST IR;
	static unsigned char MEM[16] = { 0 }; //파일에서 불러오는 명령어들을 저장할 변수

	fp = fopen(file_name, "rb"); //파일 열기

	if (fp == NULL) { //파일 열기 오류 발생시 처리
		printf("File Open Error!\n"); //문구 출력 후 종료
		return ERROR;
	}

	fileLoad = LOADED; //파일이 로드됨.

	while (i < 2) { //명령어와 데이터의 개수를 가지고 있는 데이터들만
		fseek(fp, i * INST_SIZE, SEEK_SET); //파일포인터의 위치를 4바이트씩 증가시킴.
		fread(&MEM[i * INST_SIZE], INST_SIZE, 1, fp); //파일포인터의 현 위치에서 메모리에 명령어를 4바이트씩 MEM에 저장함.
		i++; //인덱스 증가
	}

	instructionNumber = readInstruction(MEM, 0, INST_SIZE); //명령어의 개수를 불러옴. (처음 4바이트는 명령어의 개수)
	dataNumber = readInstruction(MEM, INST_SIZE, INST_SIZE * 2); //데이터의 개수를 불러옴. (그 다음 4바이트는 데이터의 개수)

	i = 0;

	while (i < instructionNumber) { //명령어의 개수를 가진만큼
		fread(&progMEM[i * INST_SIZE], INST_SIZE, 1, fp); //파일포인터의 현 위치에서 메모리에 명령어를 4바이트씩 프로그램 메모리에 저장함.
		i++; //인덱스 증가
	}

	i = 0;

	while (feof(fp) == 0) { //파일포인터가 파일의 끝에 도달할 때까지
		fread(&dataMEM[i * INST_SIZE], INST_SIZE, 1, fp); //파일포인터의 현 위치에서 메모리에 명령어를 4바이트씩 데이터 메모리에 저장함.
		i++; //인덱스 증가
	}
	
	for (i = 0; i < instructionNumber * INST_SIZE; i += INST_SIZE) { //명령어의 개수에 명령어의 크기 단위를 곱한만큼 루프를 돔.
		IR.IR.inst = readInstruction(progMEM, i, i + INST_SIZE); //명령어를 차례대로 IR에 읽어온 후
		IR.address = ORIGIN_ADDR + i;
		printInstruction(IR); //IR를 해석하여 출력
	}

	fclose(fp);

	return 1;
}


void go() {
	unsigned int i = 0, j = 0;
	INST IR;
	char selection = 0;

	setPC(ORIGIN_ADDR);

	for (i = 0; i < instructionNumber * INST_SIZE; i += INST_SIZE) { //명령어의 개수에 명령어의 크기 단위를 곱한만큼 루프를 돔.

		for (j = 0; j < instructionNumber * INST_SIZE; j += INST_SIZE) //명령어의 개수에 명령어의 크기 단위를 곱한만큼 루프를 돔.
		{
			IR.IR.inst = readInstruction(progMEM, j, j + INST_SIZE); //명령어를 차례대로 IR에 읽어온 후
			IR.address = ORIGIN_ADDR + j;
			printInstruction(IR); //IR를 해석하여 출력
		}

		IR.IR.inst = readInstruction(progMEM, i, i + INST_SIZE); //명령어를 차례대로 IR에 읽어온 후
		conductInstruction(IR);

		if (i < instructionNumber * INST_SIZE - INST_SIZE) {
			system("cls");
		}
	}
}

void clearMemory() {
	memset(progMEM, 0, sizeof(progMEM));
	memset(stackMEM, 0, sizeof(stackMEM));
	memset(dataMEM, 0, sizeof(dataMEM));
	instructionNumber = 0;
	dataNumber = 0;
}

void viewRegister() {
	int i = 0;

	for (i = 0; i < 32; i++) {
		printf("[$%d] : %X\n", i, reg[i]);
	}


}
// 나중에 최종 때 지우겠습니다!!
// void printMemory(unsigned int start, unsigned int end){
// 	unsigned int last, sel, offset;
// 	unsigned char* pM;
// 	int address;

// 	// sel = start >> 20;
// 	offset = start & 0xFFFFF;
// 	last = end & 0xFFFFF;
// 	address = start;

	// if (sel == 0x004)
	// 	pM = progMEM;         

	// else if (sel == 0x100)
	// 	pM = dataMEM;  

	// else if (sel == 0x7FF)
	// 	pM = stackMEM; 

	// else {
	// 	printf("No memory\n");
	// 	exit(1);
	// }
// 	for (int i = offset; i <= last; i += 1){
// 		printf("%x = %x\n", address, *pM);
// 		address +=1;
// 		pM += 1;	
// 	}
// }

void viewMemory(){
	unsigned int start, end, value;
	printf("Range of Memory Address ex) 0x100000 0x100010\n>> ");
	scanf("%x %x", &start, &end);
	for (unsigned int i = start; i <= end ; i += 1 ){
		value = MEM(i, 0, 0, 1);
		printf("%x = %x\n", i, value);
		
	}
}

void setReg(){
	int regAddress;
	int value;

	printf("Register address (0~31) \n");
	scanf("%d", &regAddress);
	printf("Value to be set");
	scanf("%x", &value);

	REG(regAddress, value, 1);
	viewRegister();
}

void setMem(){
	unsigned int memAddress;
	int value, result;

	printf("Memory address\n>> ");
	scanf("%x", &memAddress);
	printf("Value to be set\n>> ");
	scanf("%x", &value);

	MEM(memAddress,value,1,1);
	result = MEM(memAddress,0, 0, 1); // 바꾸고 읽기
	printf("%x = %x\n", memAddress, result);
}

void step()
{
	unsigned int i = 0, j = 0;
	INST IR;
	char selection = 0;
	char selection2 = 0;
	int input = 1;
	int address = 0;

	setPC(ORIGIN_ADDR);

	for (i = 0; i < instructionNumber * INST_SIZE; i += INST_SIZE) { //명령어의 개수에 명령어의 크기 단위를 곱한만큼 루프를 돔.

		for (j = 0; j < instructionNumber * INST_SIZE; j += INST_SIZE) //명령어의 개수에 명령어의 크기 단위를 곱한만큼 루프를 돔.
		{
			IR.IR.inst = readInstruction(progMEM, j, j + INST_SIZE); //명령어를 차례대로 IR에 읽어온 후
			IR.address = ORIGIN_ADDR + j;
			printInstruction(IR); //IR를 해석하여 출력
		}

		IR.IR.inst = readInstruction(progMEM, i, i + INST_SIZE); //명령어를 차례대로 IR에 읽어온 후
		conductInstruction(IR);

		if (input == 1) {
			printf("Command Input (Enter to step!) : ");
			rewind(stdin);
			scanf("%c", &selection);
		}

		switch (selection) {
		case 'g':
			input = 0;
			selection = 0;
			break;
		case 'j':
			scanf("%x", &address);
			i += (address - PC);
			setPC(address);
			break;
		case 'r':
			system("cls");
			viewRegister();
			waitInput();
			break;
		case 'm':
			system("cls");
			viewMemory();
			waitInput();
			break;
		case 'x':
			exit(0);
			break;
		case 's':
			scanf("%c", &selection2);
			switch (selection2) {
			case 'r':
				system("cls");
				setReg();
				waitInput();
				break;
			case 'm':
				system("cls");
				setMem();
				waitInput();
				break;
			}
		}

		if (i < instructionNumber * INST_SIZE - INST_SIZE) {

			system("cls");
		}

	}
}

int printMenu() {
	char selection = '\0';
	char selection2 = '\0';
	char fileName[32] = { 0 };

	printf("Command Input: "); 
	scanf("%c", &selection);

	switch (selection) {
	case 's':
		scanf("%c", &selection2);
		switch (selection2) {
		case '\n':
			if (fileLoad == UNLOADED) {
				printf("File should be loaded first!\n");
				break;
			}

			step();
			break;
		case 'r': 
			setReg();
			break;
		case 'm': 
			setMem();
			break;
		}
		
		return RERUN;
	case 'l':
		clearMemory();
		scanf("%s", fileName);

		loadFile(fileName);		
		return RERUN;
	case 'g':
		if (fileLoad == UNLOADED) {
			printf("File should be loaded first!\n");
			return RERUN;
		}	
			
		go();
		return RERUN;
	case 'x':
		return EXIT;
	case 'r':
		viewRegister();
		return RERUN;
	case 'm':
		viewMemory();
		return RERUN;		
	default:
		printf("Invalid Command!\n");
		return RERUN;
	}
}


int main() {

	setPC(ORIGIN_ADDR);
	REG(29, STACK_ADDR, WRITE); //처음에 스택포인터를 스택메모리 시작 주소로 설정

	while (printMenu()) {
 		waitInput();
		system("cls");
	}

    return 0;
}
