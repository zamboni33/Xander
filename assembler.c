
/*---------------------------------------------------------------------
 *
 * assembler.c
 * 
 * 
 * Alexander Morgan
 * ABM545
 * Computer Architecture Fall 2013
 * LC3b Assembler
 * 
 * Assembler application for the conversion of a human readable LC3b 
 * file int LC3b machine code.
 * 
 *
 ---------------------------------------------------------------------*/
 
/*---------------------------- Includes ------------------------------*/
 
 /* Standard */
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <math.h>
 #include <ctype.h>
 
 /* User */
 /* #include "assembler.h" */
 
/*----------------------------- Globals ------------------------------*/

 int programCounter = 0;

/*------------------------------ Types -------------------------------*/
 
   typedef struct 
 {
	int lineComponentCount;
	int linePosition;
	char** lineComponent;
	unsigned int lineDrop;
	int invalid;
 } brokenLine;
 
   typedef struct 
 {
	char* symbol;
	unsigned int memoryLocation;
 } symbolTable;
 
/*---------------------------- Constants -----------------------------*/

/*--------------------------- Prototypes -----------------------------*/
 
 brokenLine* breakLine (int* lineSize, char* readIn);
 char* fetchLine (int* lineSize, char* readIn, FILE* successfulOpen);
 char* assignMemory(brokenLine* currentLine, unsigned int* memory);
 int isThisALabel (brokenLine* labelPossible, char** opcodeConstants, char** psuedoOpcodes, int linePosition);
 void defineConstants(char** opcodeConstants);
 void definePsuedoOps (char** psuedoOpcodeConstants);
 int decodeCommand (brokenLine* command, char** opcodeConstants, char** psuedoOpcode, int linePosition);
 void analyzeCommand(int commandMatch, brokenLine* currentLine, symbolTable** currentSymbolTable, int symbolTableSize);
 
 void checkRegister (brokenLine* currentStruct, char* whichRegister);
 int checkImmediate (brokenLine* currentStruct, char* immediateString, int max, int min);
 unsigned int checkSymbol (brokenLine* currentStruct, char* symbolString, symbolTable** currentSymbolTable, int symbolTableSize);
 void checkAdd (brokenLine* operands, int immediateFlag, int immediate);
 void checkAnd (brokenLine* operands, int immediateFlag, int immediate);
 void checkXor (brokenLine* operands, int immediateFlag, int immediate);
 void checkJmp(brokenLine* currentLine);
 void checkJsrr(brokenLine* currentLine);
 void checkNot(brokenLine* currentLine);
 void checkRet(brokenLine* currentLine);
 void checkRti(brokenLine* currentLine);
 
/*------------------------- Implementation ---------------------------*/
 
 int main (int argc, char* argv[])
{	 
 /*
  * Inputs: File containing human readable LC3 code, Output Filename
  * Outputs: LC3b bytecoded assembly
  * Note: If the output filename exists, it's data will be erased. 
  * 
----------------------------------------------------------------------*/
	
	/* Variable List */
	
	FILE* successfulOpen;
	FILE* output;
	
	int i;
	int j;
	int sourceFileLocation = 0;
	int labelFlag = 0;
	int* lineSize;
	int totalLineCount = 0;
	int errorCode = 0;
	unsigned int* memoryInt;
	unsigned int* memoryIntSecondPass;
	int symbolTableSize = 0;
	int currentLineSize = 0;
	int maxSymbolTableSize = 100;
	int maxCurrentLineSize = 100;
	
	char* readIn;
	char* newLine;
	char* newCommand;
	char* memoryBegin;
	char* str;
	char** opcodeConstants;
	char** psuedoOpcodes;
	char* hexDrop;
	
	brokenLine** currentLine;
	brokenLine** tempLinePtr;
	
	symbolTable** currentSymbolTable;
	symbolTable** tempSymbolPtr;
	
	/* Creating Opcode List */
	
	opcodeConstants = (char**) malloc(sizeof(char*) * 32);
	psuedoOpcodes = (char**) malloc(sizeof(char*) * 3);
	hexDrop = (char*) malloc(sizeof(char*) * 7);
	defineConstants(opcodeConstants);
	definePsuedoOps(psuedoOpcodes);
	
	/* Opening assembly input file */
	
	successfulOpen = fopen(argv[1], "r");
	if(!successfulOpen){
		printf("Error opening assembly input file. Please check source file.\n");
		return(1);
	}
	
	output = fopen(argv[2], "w");
	if(!output){
		printf("Error opening assembly input file. Please check source file.\n");
		return(1);
	}
	
/*-------------------- Creating Symbol Table -------------------------*/	
	
	/* Begin parsing assembly input file */
	
	readIn = (char*) malloc(1);
	lineSize = (int*) malloc(1 * sizeof(int));
	memoryInt = (unsigned int*) malloc(1 * sizeof(int));
	currentLine = (brokenLine**) malloc(sizeof(brokenLine**) * maxCurrentLineSize);
	currentSymbolTable = (symbolTable**) malloc(sizeof(symbolTable**) * maxSymbolTableSize);

	/* Handle the .ORIG first */
	
	/* while (strcmp(currentLine[currentLineSize]->lineComponent[0], ".orig")){ */
	do{
		newLine = fetchLine(lineSize, readIn, successfulOpen);
		currentLine[currentLineSize] = breakLine(lineSize, newLine);
		sourceFileLocation += 1;
	} while(!newLine || currentLine[currentLineSize]->lineComponentCount == 0);
		/* currentLine[currentLineSize] = breakLine(lineSize, newLine); */
		printf("Test Line\n");
		currentLine[currentLineSize]->linePosition = sourceFileLocation;
		/* Account for empty lines */
		while(currentLine[currentLineSize]->lineComponent[0] == 0){
			newLine = fetchLine(lineSize, readIn, successfulOpen);
			sourceFileLocation += 1;
			currentLine[currentLineSize] = breakLine(lineSize, newLine);
		}
	memoryBegin = assignMemory(currentLine[currentLineSize], memoryInt);
	totalLineCount += 1;
	/* *memoryIntSecondPass = *memoryInt; */
	programCounter = *memoryInt;
	sprintf(hexDrop, "0x%x\n", programCounter);

	printf("Memory Start: %d.\n", *memoryInt);
	
	
	currentLineSize += 1;
	
	/* Scan the rest of the assembly file now. */
	while (strcmp(currentLine[currentLineSize - 1]->lineComponent[0], ".end")){  /* The subtraction is confusing as shit. */
		do {
			newLine = fetchLine(lineSize, readIn, successfulOpen);
			sourceFileLocation += 1;
		} while (!newLine);
		currentLine[currentLineSize] = breakLine(lineSize, newLine);
		currentLine[currentLineSize]->linePosition = sourceFileLocation;
			if(currentLineSize > maxCurrentLineSize - 10){
				maxCurrentLineSize *= 2;
				tempLinePtr = realloc((symbolTable**) currentLine, maxCurrentLineSize);
				currentLine = tempLinePtr;
			}	
		
		/* Account for empty lines */
		while(!newLine || currentLine[currentLineSize]->lineComponentCount == 0){
			newLine = fetchLine(lineSize, readIn, successfulOpen);
			currentLine[currentLineSize] = breakLine(lineSize, newLine);
			sourceFileLocation += 1;
		}
		currentLine[currentLineSize]->linePosition = sourceFileLocation;

		if(currentLine[currentLineSize]->lineComponentCount){
		
			labelFlag = isThisALabel(currentLine[currentLineSize], opcodeConstants, psuedoOpcodes, currentLine[currentLineSize]->linePosition);

			/* Send in the pointer to the struct and bump it if its an erroneous label. */
			if(labelFlag){
				if(symbolTableSize > maxSymbolTableSize - 10){
					maxSymbolTableSize *= 2;
					tempSymbolPtr = realloc((symbolTable**) currentSymbolTable, maxSymbolTableSize);
					currentSymbolTable = tempSymbolPtr;
				}
				currentSymbolTable[symbolTableSize] = (symbolTable*) malloc (sizeof(symbolTable));
				currentSymbolTable[symbolTableSize]->symbol = currentLine[currentLineSize]->lineComponent[0];
				currentSymbolTable[symbolTableSize]->memoryLocation = *memoryInt;
				symbolTableSize += 1;
				
				/* Removing the label from the lineComponent list. */
				currentLine[currentLineSize]->lineComponent = & (currentLine[currentLineSize]->lineComponent[1]);
				currentLine[currentLineSize]->lineComponentCount -= 1;
			}
			*memoryInt += 2;
		}

		currentLineSize += 1;
	}  

/*-------------------------- Second Pass -----------------------------*/  

/* Need to disregard the first line because it is .orig */

	int commandMatch = 0;

	fwrite( (char*) hexDrop, 1, 7, output);
	for(i = 1; i < currentLineSize; i += 1){
		if(!currentLine[i]->invalid){
			commandMatch = decodeCommand(currentLine[i], opcodeConstants, psuedoOpcodes, currentLine[i]->linePosition);
			if(commandMatch >= 0){
				programCounter += 2;
				analyzeCommand(commandMatch, currentLine[i], currentSymbolTable, symbolTableSize);
				sprintf(hexDrop, "0x%x\n", currentLine[i]->lineDrop);
				fwrite( (char*) hexDrop, 1, 7, output); 
			}
		}
	}

/*------------------- Debug Struct Printing --------------------------*/
	
	for( j = 0; j < currentLineSize; j += 1){	
		if(currentLine[j]->invalid){
			errorCode = currentLine[j]->invalid;
			/* DELETE THE FILE HERE! */
			system("rm -f output.obj");
			break;
		}
		if(!currentLine[j]->invalid){
			for( i = 0; i < currentLine[j]->lineComponentCount; i += 1){
				printf("%d. %s on line: %d  count: %d\n", i , currentLine[j]->lineComponent[i], currentLine[j]->linePosition, currentLine[j]->lineComponentCount);
			}
		}	
	}
		
	for( i = 0; i < symbolTableSize; i += 1){
		printf("%s     and       %d\n", currentSymbolTable[i]->symbol, currentSymbolTable[i]->memoryLocation);
	}
	
	printf("Supposed to return errorCode %d\n", errorCode);
	return(errorCode);

}
 
/*--------------------------- Function -------------------------------*/
  brokenLine* breakLine (int* lineSize, char* readIn)
{ 	 
 /*
  * Inputs: Size of and line to read
  * Outputs: The line broken into args
  *  
  --------------------------------------------------------------------*/

	int i = 0;
	int j = 0;
	brokenLine* currentLine = (brokenLine*) malloc (sizeof(brokenLine));
	int errorFlag = 0;
	currentLine->invalid = 0;
	
	/* printf("Inside breakLine\n"); */
	
	currentLine->lineComponentCount = 0;
	/* printf("Assigned my counter\n"); */
	currentLine->lineComponent = (char**) malloc (sizeof(char**) * 10);
	currentLine->lineDrop = 5555;

	for(i = 0; i < *lineSize; i += 1){
		if(readIn[i] != ' ' && readIn[i] != '	'){
			j = 0;
			/* printf("First Letter: %c\n", readIn[i]); */
			currentLine->lineComponent[currentLine->lineComponentCount] = (char*) malloc (sizeof(char) * 20);		/* Asking for memory in the size of the largest possible var. */
			while(readIn[i] != ' ' && i < *lineSize){
				/* printf("Writing %c to file\n", readIn[i]); */
				currentLine->lineComponent[currentLine->lineComponentCount][j] = tolower(readIn[i]);
				i += 1;
				j += 1;
			}
			currentLine->lineComponent[currentLine->lineComponentCount][j] = 0;
			i -= 1;
			currentLine->lineComponentCount += 1;
		}
	}
	return(currentLine);
} 
 
/*--------------------------- Function -------------------------------*/
  char* fetchLine (int* lineSize, char* readIn, FILE* successfulOpen)
{	 
 /*
  * Inputs: Successful file read int, memory to read to, FILE to read from
  * Outputs: Returns a line of code in char* form.
  * Note: This function filters out commas and comments.
  * 
  --------------------------------------------------------------------*/
  
 	fpos_t savedPosition;
	int counter = 0;
	int i = 0;
	int skipFlag = 0;
	int lengthResult = 0;
	
 	fgetpos (successfulOpen, &savedPosition);
 	*lineSize = fread( (char*) readIn, 1, 1, successfulOpen);
 	/* printf("First part of command read: %s\n", readIn); */
 	
	while(*readIn != '\n'){
		if(*readIn == ';'){
			skipFlag = 1;
		}
		if(!skipFlag){
			lengthResult += 1;	
		}
		counter += 1;
		*lineSize = fread( (char*) readIn, 1, 1, successfulOpen);
	}
	fsetpos (successfulOpen, &savedPosition);
	/* printf("Size of command: %d\n", lengthResult); */
	
	/* If nothing useful on the line then let's skip saving it. */
	if(lengthResult == 0){
		for(i = 0; i < counter + 1; i += 1){
			fread( (char*) readIn, 1, 1, successfulOpen);
		}
		return(0);
	}
	
	/* Now we have the length of the new command. */
	char* newLine = (char*) malloc(counter);
	skipFlag = 0;
	
	for(i = 0; i < counter; i += 1){
		fgetpos (successfulOpen, &savedPosition);
		*lineSize = fread( (char*) readIn, 1, 1, successfulOpen);
		if(*readIn == ';'){
			skipFlag = 1;
		}
		
		if(*readIn == ','){
			newLine[i] = ' ';
		} else {
			if(!skipFlag){
				newLine[i] = *readIn;
			}
		}
	}
	
	fread( (char*) readIn, 1, 1, successfulOpen);
	
	*lineSize = lengthResult;
	
	return(newLine);
} 

/*--------------------------- Function -------------------------------*/ 
  char* assignMemory (brokenLine* origLine, unsigned int* startPos)
{	 
 /*
  * Inputs: orig Line memory information
  * Outputs: Variable for where memory starts
  * 
  --------------------------------------------------------------------*/
  
	  char* hexString = (char*) malloc(sizeof(char) * 6);
	  
	  /* BIG ERROR HERE: WHAT IF HE SENDS IN AN INVALID START MEMORY */
	  
	  /* Code for an invalid constant. */
	  
	  
	  if(origLine->lineComponent[1][0] == '#'){
		  *startPos = atoi(&origLine->lineComponent[1][1]);
		  snprintf (hexString, 7, "0x%x", *startPos);
	  }
	  else{
		  hexString = &origLine->lineComponent[1][1];
		  *startPos = atoi(&origLine->lineComponent[1][1]);
	  }
	  
	  if(*startPos % 2 || *startPos > 65535){
		  origLine->invalid = 3;
	  }
	  /*printf("Memory in dec format: %d\n", tempInt + 2);
	  printf("Memory starts at: %s\n", hexString);*/
	  return(hexString);
}  
  
  
/*--------------------------- Function -------------------------------*/
  int isThisALabel (brokenLine* labelPossible, char** opcodeConstants, char** psuedoOpcode, int linePosition)
{	 
 /*
  * Inputs: 1st argument of a line of code
  * Outputs: Bool specifying whether or not it is a label
  * 
  --------------------------------------------------------------------*/

	int i = 0;
	int counter = 0;
	
	if(labelPossible->lineComponent[0][0] == 'x'){
		printf("Error with label on line %d\n", linePosition);
		labelPossible->invalid = 4;
		return(0);
	}
	
	/* Put a comparison to psuedo ops here. */
	
	for(i = 0; i < 3; i += 1){
		if(!strcmp(labelPossible->lineComponent[0], psuedoOpcode[i])){
		/* This is a psuedo op. */	
			return(0);
		}
	}	
	
	while(labelPossible->lineComponent[0][i] != 0){
		if(!isalnum(labelPossible->lineComponent[0][i])){
		/* This is an error situation here that needs to be dropped to console with the line number. */	
			printf("Error with label on line %d\n", linePosition);
			labelPossible->invalid = 4;
			return(0);
		}
		i += 1;
		counter += 1;
	}
	
	if(counter > 19){
		/* This is an error situation here that needs to be dropped to console with the line number. */	
		printf("Error with label on line %d\n", linePosition);
		/* Assigning zero here is doing nothing." */
		labelPossible->invalid = 4;
		return(0);
	}
	
	for(i = 0; i < 32; i += 1){
		if(!strcmp(labelPossible->lineComponent[0], opcodeConstants[i])){
		/* This is an opcode. */	
			return(0);
		}
	}
	
	if(labelPossible->lineComponentCount < 2){
		printf("Error with label on line %d\n", linePosition);
		labelPossible->invalid = 4;
		return(0);
	}
	
	/*printf("Length of the potential label is: %d\n", counter);*/
	
	return(1);
}

/*--------------------------- Function -------------------------------*/
  int decodeCommand (brokenLine* command, char** opcodeConstants, char** psuedoOpcode, int linePosition)
{
 /*
  * Inputs: char* of potential command, char** of opcodes as strings
  * Outputs: None
  * 
  --------------------------------------------------------------------*/ 
	int i;
	
	/* printf("Inside decode.\n"); */
	
	for(i = 0; i < 3; i += 1){
		if(!strcmp(command->lineComponent[0], psuedoOpcode[i])){
		/* This is a psuedo op. */	
		/* Need to do something with the psuedo op here. */
			return(i + 32);
		}
	}
	
	for(i = 0; i < 32; i += 1){
		if(!strcmp(command->lineComponent[0], opcodeConstants[i])){
			return(i);
		}
	}
	/* Throw an error before returning negative one. */
	printf("Problem with opcode on line %d.\n", linePosition);
	command->invalid = 2;
	return(-1);  
}

/*--------------------------- Function -------------------------------*/
	void analyzeCommand(int commandMatch, brokenLine* currentLine, symbolTable** currentSymbolTable, int symbolTableSize)
{
 /*
  * Inputs: Command matched, Line of command and operands
  * Outputs: Boolean of correct format or no
  * 
  --------------------------------------------------------------------*/ 
  
  /* Take the command and operands and decide if it is in correct format.
   * Then send it to a switch function to be handled. */
   /* NOTE: You really need to pass the associated struct. */
   
   /* printf("Inside analyze.\n"); */
   int i = 0;
   int immediate = 0;
   int immediateFlag = 0;
   int tempInt = 0;
   int max = 0;
   int min = 0;
   unsigned int offsetAddress = 0;
   
   switch(commandMatch)
   {
	   
		case 0:
			printf("Checking ADD.\n");
			if(currentLine->lineComponentCount != 4){currentLine->invalid = 4;}
			printf("String %s\n", currentLine->lineComponent[1]);
			checkRegister(currentLine, currentLine->lineComponent[1]);
			checkRegister(currentLine, currentLine->lineComponent[2]);
			if(currentLine->lineComponent[3][0] == 'r'){
				/* printf("Checking this register: %s\n", currentLine->lineComponent[3]); */
				checkRegister(currentLine, currentLine->lineComponent[3]);
			} else{
				max = 15;
				min = -16;
				immediate = checkImmediate(currentLine, currentLine->lineComponent[3], max, min); 
				immediateFlag = 1;
			}
			if(!currentLine->invalid){
				checkAdd(currentLine, immediateFlag, immediate);
			}
			break;		
			
		case 1:
			printf("Checking AND.\n");
			if(currentLine->lineComponentCount != 4){currentLine->invalid = 4;}
			printf("String %s\n", currentLine->lineComponent[1]);
			checkRegister(currentLine, currentLine->lineComponent[1]);
			checkRegister(currentLine, currentLine->lineComponent[2]);
			if(currentLine->lineComponent[3][0] == 'r'){
				/* printf("Checking this register: %s\n", currentLine->lineComponent[3]); */
				checkRegister(currentLine, currentLine->lineComponent[3]);
			} else{
				max = 15;
				min = -16;
				immediate = checkImmediate(currentLine, currentLine->lineComponent[3], max, min); 
				immediateFlag = 1;
			}
			if(!currentLine->invalid){
				checkAnd(currentLine, immediateFlag, immediate);
			}
			break;	
			
		case 2:
			printf("Checking XOR.\n");
			if(currentLine->lineComponentCount != 4){currentLine->invalid = 4;}
			checkRegister(currentLine, currentLine->lineComponent[1]);
			checkRegister(currentLine, currentLine->lineComponent[2]);
			if(currentLine->lineComponent[3][0] == 'r'){
				/* printf("Checking this register: %s\n", currentLine->lineComponent[3]); */
				checkRegister(currentLine, currentLine->lineComponent[3]);
			} else{
				max = 15;
				min = -16;
				immediate = checkImmediate(currentLine, currentLine->lineComponent[3], max, min); 
				immediateFlag = 1;
			}
			if(!currentLine->invalid){
				checkXor(currentLine, immediateFlag, immediate);
			}
			break;					

		case 3:
			printf("Received opcode jmp\n");
			if(currentLine->lineComponentCount != 2){currentLine->invalid = 4;}
			checkRegister(currentLine, currentLine->lineComponent[1]);
			if(!currentLine->invalid){
				checkJmp(currentLine);
			}
			break;
		case 4:
			printf("Received opcode jsrr\n");
			if(currentLine->lineComponentCount != 2){currentLine->invalid = 4;}
			checkRegister(currentLine, currentLine->lineComponent[1]);
			if(!currentLine->invalid){
				checkJsrr(currentLine);
			}
			break;
		
		case 5:
			printf("Received opcode ldb\n");
			break;
		case 6:
			printf("Received opcode ldw\n");
			break;
		
		case 7:
			printf("Received opcode stb\n");
			break;
		case 8:
			printf("Received opcode stw\n");
			break;
		
		case 9:
			printf("Received opcode lshf\n");
			break;
		case 10:
			printf("Received opcode rshfl\n");
			break;
		case 11:
			printf("Received opcode rshfa\n");
			break;
		
		case 12:
			printf("Received opcode lea\n");
			break;
		case 13:
			printf("Received opcode nop\n");
			break;
		case 14:
			printf("Received opcode not\n");
			if(currentLine->lineComponentCount != 3){currentLine->invalid = 4;}
			checkRegister(currentLine, currentLine->lineComponent[1]);
			checkRegister(currentLine, currentLine->lineComponent[2]);
			if(!currentLine->invalid){
				checkNot(currentLine);
			}			
			break;
		case 15:
			printf("Received opcode ret\n");
			if(currentLine->lineComponentCount != 1){currentLine->invalid = 4;}
			if(!currentLine->invalid){
				checkRet(currentLine);
			}			
			break;
		case 16:
			printf("Received opcode rti\n");
			if(currentLine->lineComponentCount != 1){currentLine->invalid = 4;}
			if(!currentLine->invalid){
				currentLine->lineDrop = atoi("28672");
				printf("Value of the lineDrop is: %d\n", currentLine->lineDrop);
			}			
			break;
		case 17:
			printf("Received opcode halt\n");
			break;
		case 18:
			printf("Received opcode jsr\n");
			break;
		case 19:
			printf("Received opcode trap\n");
			break;
		case 20:
			printf("Received opcode puts\n");
			break;
		case 21:
			printf("Received opcode in\n");
			break;
		case 22:
			printf("Received opcode out\n");
			break;
		case 23:
			printf("Received opcode getc\n");
			break;
		case 24:
			printf("Received opcode brn\n");
			offsetAddress = checkSymbol(currentLine, currentLine->lineComponent[1], currentSymbolTable, symbolTableSize);
			printf("The offset address is: %d and the line address is %d\n", offsetAddress, programCounter);
			break;
		case 25:
			printf("Received opcode brp\n");
			offsetAddress = checkSymbol(currentLine, currentLine->lineComponent[1], currentSymbolTable, symbolTableSize);
			printf("The offset address is: %d and the line address is %d\n", offsetAddress, programCounter);
			break;
		case 26:
			printf("Received opcode brnp\n");
			offsetAddress = checkSymbol(currentLine, currentLine->lineComponent[1], currentSymbolTable, symbolTableSize);
			printf("The offset address is: %d and the line address is %d\n", offsetAddress, programCounter);
			break;
		case 27:
			printf("Received opcode br\n");
			offsetAddress = checkSymbol(currentLine, currentLine->lineComponent[1], currentSymbolTable, symbolTableSize);
			printf("The offset address is: %d and the line address is %d\n", offsetAddress, programCounter);			
			break;
		case 28:
			printf("Received opcode br\n");
			offsetAddress = checkSymbol(currentLine, currentLine->lineComponent[1], currentSymbolTable, symbolTableSize);
			printf("The offset address is: %d and the line address is %d\n", offsetAddress, programCounter);
			break;
		case 29:
			printf("Received opcode br\n");
			offsetAddress = checkSymbol(currentLine, currentLine->lineComponent[1], currentSymbolTable, symbolTableSize);
			printf("The offset address is: %d and the line address is %d\n", offsetAddress, programCounter);
			break;
		case 30:
			printf("Received opcode br\n");
			offsetAddress = checkSymbol(currentLine, currentLine->lineComponent[1], currentSymbolTable, symbolTableSize);
			printf("The offset address is: %d and the line address is %d\n", offsetAddress, programCounter);
			break;
		case 31:
			printf("Received opcode br\n");
			offsetAddress = checkSymbol(currentLine, currentLine->lineComponent[1], currentSymbolTable, symbolTableSize);
			printf("The offset address is: %d and the line address is %d\n", offsetAddress, programCounter);
			break;
		case 32:
			printf("Received psuedo op .orig\n");
			/* Why the fuck are we in here? */
			break;
		case 33:
			printf("Received psuedo op .fill\n");
			break;
		case 34:
			printf("Received psuedo op .end\n");
			break;
		
		
		default: 
			printf("How the fuck? Why did we not hit a case?");
		}
  
}
 
 /*--------------------------- Function -------------------------------*/
  void checkRegister (brokenLine* currentStruct, char* whichRegister)
{
 /*
  * Inputs: char** array to hold opcodes as strings
  * Outputs: Initialization
  * 
  --------------------------------------------------------------------*/ 
 
 	int i = 0;
	int regSize = 0;
	char* tempPtr;
	
	if(!currentStruct->invalid){
		if(whichRegister[0] != 'r'){currentStruct->invalid = 4;}
		tempPtr = &whichRegister[1];
		regSize = atoi(tempPtr);
		/* printf("RegSize = %d\n", regSize); */
		if(regSize == 0 && whichRegister[1] != '0'){
			/* printf("Fucked operand\n"); */
			currentStruct->invalid = 4;
			return;
		}
		
		if(regSize < 0 || regSize > 7){
			/* printf("Register number too big\n"); */
			currentStruct->invalid = 4;
			return;
		}	
	}

	if(currentStruct->invalid){
		printf("Error within operands on line %d\n", currentStruct->linePosition);
	}
}

/*--------------------------- Function -------------------------------*/
  int checkImmediate (brokenLine* currentStruct, char*immediateString, int max, int min)
{
 /*
  * Inputs: brokenLine* containing operands to check for validity
  * Outputs: ErrorCode
  * 
  --------------------------------------------------------------------*/ 
	int result = 0;
	int longResult = 0;
	char* tempPtr;
	
	printf("Checking immediate.\n");
	
	if(immediateString[0] != '#' && immediateString[0] != 'x'){
		currentStruct->invalid = 4;
		/* printf("WTF?\n"); */
		return(0);
	}
	
	if(immediateString[0] == '#'){
		if(immediateString[1] == '-'){
			tempPtr = &immediateString[2];
			result = atoi(tempPtr);
			if(result == 0 && immediateString[2] != '0'){currentStruct->invalid = 3;}
			result = -result;
			if(result < min){currentStruct->invalid = 3;}
			/* printf("Result: %d\n", result); */
			return(result);
		} else{
			tempPtr = &immediateString[1];
			result = atoi(tempPtr);
			if(result == 0 && immediateString[1] != '0'){currentStruct->invalid = 3;}
			if(result > max){currentStruct->invalid = 3;}
			/* printf("Result: %d\n", result); */
			return(result);
		}
	} else if(immediateString[0] == 'x'){
		if(immediateString[2] == '-'){	
			tempPtr = &immediateString[2];
			longResult = strtol(tempPtr, NULL, 16);
			if(longResult == 0 && immediateString[2] != '0'){currentStruct->invalid = 3;}
			longResult = -longResult;
			if(longResult < min){currentStruct->invalid = 3;}
			/* printf("Result: %d\n", longResult); */
			return(longResult);
		} else{
			tempPtr = &immediateString[1];
			longResult = strtol(tempPtr, NULL, 16);
			if(longResult == 0 && immediateString[1] != '0'){currentStruct->invalid = 3;}
			if(longResult > max){currentStruct->invalid = 3;}
			/* printf("Result: %d\n", longResult); */
			return(longResult);		
		}
	}
			
}	
 
/*--------------------------- Function -------------------------------*/
  unsigned int checkSymbol (brokenLine* currentStruct, char* symbolString, symbolTable** currentSymbolTable, int symbolTableSize)
{
 /*
  * Inputs: brokenLine* containing operands to check for validity
  * Outputs: ErrorCode
  * 
  --------------------------------------------------------------------*/ 
 int i = 0;
 
 for ( i = 0; i < symbolTableSize; i += 1){
	 if(!strcmp(symbolString, currentSymbolTable[i]->symbol)){
		 return(currentSymbolTable[i]->memoryLocation);
	 }
 }
 
 currentStruct->invalid = 1;
 return(0); 
}
 
 
 
 
 
/*--------------------------- Function -------------------------------*/
  void checkAdd (brokenLine* operands, int immediateFlag, int immediate)
{
 /*
  * Inputs: brokenLine* containing operands to check for validity
  * Outputs: ErrorCode
  * 
  --------------------------------------------------------------------*/ 
	printf("Inside the actual check with immediate %d.\n", immediate);
}	

/*--------------------------- Function -------------------------------*/
  void checkAnd (brokenLine* operands, int immediateFlag, int immediate)
{
 /*
  * Inputs: brokenLine* containing operands to check for validity
  * Outputs: ErrorCode
  * 
  --------------------------------------------------------------------*/ 
	printf("Inside the actual check with immediate %d.\n", immediate);
}	
 
/*--------------------------- Function -------------------------------*/
  void checkXor (brokenLine* operands, int immediateFlag, int immediate)
{
 /*
  * Inputs: brokenLine* containing operands to check for validity
  * Outputs: ErrorCode
  * 
  --------------------------------------------------------------------*/ 
	printf("Inside the actual check with immediate %d.\n", immediate);
}	

/*--------------------------- Function -------------------------------*/
  void checkJmp (brokenLine* operands)
{
 /*
  * Inputs: brokenLine* containing operands to check for validity
  * Outputs: ErrorCode
  * 
  --------------------------------------------------------------------*/ 
	
}

/*--------------------------- Function -------------------------------*/
  void checkJsrr (brokenLine* operands)
{
 /*
  * Inputs: brokenLine* containing operands to check for validity
  * Outputs: ErrorCode
  * 
  --------------------------------------------------------------------*/ 
	
}
  
/*--------------------------- Function -------------------------------*/
  void checkNot (brokenLine* operands)
{
 /*
  * Inputs: brokenLine* containing operands to check for validity
  * Outputs: ErrorCode
  * 
  --------------------------------------------------------------------*/ 
	
}

/*--------------------------- Function -------------------------------*/
  void checkRet (brokenLine* operands)
{
 /*
  * Inputs: brokenLine* containing operands to check for validity
  * Outputs: ErrorCode
  * 
  --------------------------------------------------------------------*/ 
	
}

/*--------------------------- Function -------------------------------*/
  void checkRti (brokenLine* operands)
{
 /*
  * Inputs: brokenLine* containing operands to check for validity
  * Outputs: ErrorCode
  * 
  --------------------------------------------------------------------*/ 
	
} 
 
 
 
 
 
 
 
 
 
 
 
  
  
/*--------------------------- Function -------------------------------*/
  void defineConstants (char** opcodeConstants)
{
 /*
  * Inputs: char** array to hold opcodes as strings
  * Outputs: Initialization
  * 
  --------------------------------------------------------------------*/ 
  
	opcodeConstants[0] = "add";
	opcodeConstants[1] = "and";
	opcodeConstants[2] = "xor";
	
	opcodeConstants[3] = "jmp";
	opcodeConstants[4] = "jsrr";
	
	opcodeConstants[5] = "ldb";
	opcodeConstants[6] = "ldw";
	
	opcodeConstants[7] = "stb";
	opcodeConstants[8] = "stw";
	
	opcodeConstants[9] = "lshf";
	opcodeConstants[10] = "rshfl";
	opcodeConstants[11] = "rshfa";
	
	opcodeConstants[12] = "lea";
	opcodeConstants[13] = "nop";
	opcodeConstants[14] = "not";
	opcodeConstants[15] = "ret";	
	opcodeConstants[16] = "rti";
	opcodeConstants[17] = "halt";
	opcodeConstants[18] = "jsr";
	opcodeConstants[19]	 = "trap";
	opcodeConstants[20]	 = "puts";
	opcodeConstants[21] = "in";
	opcodeConstants[22] = "out";
	opcodeConstants[23] = "getc";
	opcodeConstants[24] = "brn";
	opcodeConstants[25] = "brp";
	opcodeConstants[26] = "brnp";
	opcodeConstants[27] = "br";
	opcodeConstants[28] = "brz";
	opcodeConstants[29] = "brnz";
	opcodeConstants[30] = "brzp";
	opcodeConstants[31] = "brnzp";
}
	
/*--------------------------- Function -------------------------------*/
  void definePsuedoOps (char** psuedoOpcodes)
{
 /*
  * Inputs: char** array to hold pseudo opcodes as strings
  * Outputs: Initialization
  * 
  --------------------------------------------------------------------*/ 
	psuedoOpcodes[0] = ".orig";
	/* THIS PROBABLY SHOULDNT BE IN HERE!!!!!! */
  	psuedoOpcodes[1] = ".fill";
  	psuedoOpcodes[2] = ".end";
	
}
	
 
 
