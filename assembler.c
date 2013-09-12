
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
 
/*------------------------------ Types -------------------------------*/
 
   typedef struct 
 {
	int lineComponentCount;
	int linePosition;
	char** lineComponent;
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
 int isThisALabel (char* labelPossible, char** opcodeConstants, char** psuedoOpcodes, int linePosition);
 void defineConstants(char** opcodeConstants);
 void definePsuedoOps (char** psuedoOpcodeConstants);
 int decodeCommand (char* command, char** opcodeConstants, int linePosition);
 void analyzeCommand(int commandMatch, brokenLine* currentLine /* Dereferenced */);
 
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
	
	int i;
	int j;
	int sourceFileLocation = 0;
	int labelFlag = 0;
	int* lineSize;
	int totalLineCount = 0;
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
	
	brokenLine** currentLine;
	brokenLine** tempLinePtr;
	
	symbolTable** currentSymbolTable;
	symbolTable** tempSymbolPtr;
	
	/* Creating Opcode List */
	
	opcodeConstants = (char**) malloc(sizeof(char*) * 32);
	psuedoOpcodes = (char**) malloc(sizeof(char*) * 3);
	defineConstants(opcodeConstants);
	definePsuedoOps(psuedoOpcodes);
	
	/* Opening assembly input file */
	
	successfulOpen = fopen(argv[1], "r");
	if(!successfulOpen){
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
		sourceFileLocation += 1;
	} while(!newLine);
		currentLine[currentLineSize] = breakLine(lineSize, newLine);
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
		while(currentLine[currentLineSize]->lineComponent[0] == 0){
			newLine = fetchLine(lineSize, readIn, successfulOpen);
			sourceFileLocation += 1;
			currentLine[currentLineSize] = breakLine(lineSize, newLine);
			currentLine[currentLineSize]->linePosition = sourceFileLocation;
				if(currentLineSize > maxCurrentLineSize - 10){
					maxCurrentLineSize *= 2;
					tempLinePtr = realloc((symbolTable**) currentLine, maxCurrentLineSize);
					currentLine = tempLinePtr;
				}			
			
		}
		labelFlag = isThisALabel(currentLine[currentLineSize]->lineComponent[0], opcodeConstants, psuedoOpcodes, currentLine[currentLineSize]->linePosition);
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
	
		currentLineSize += 1;
	}
	
/*------------------- Debug Struct Printing --------------------------*/
	
	for( j = 0; j < currentLineSize; j += 1){	
		for( i = 0; i < currentLine[j]->lineComponentCount; i += 1){
			printf("%d. %s on line: %d  count: %d\n", i , currentLine[j]->lineComponent[i], currentLine[j]->linePosition, currentLine[j]->lineComponentCount);
		}	
	}
		
	for( i = 0; i < symbolTableSize; i += 1){
		printf("%s     and       %d\n", currentSymbolTable[i]->symbol, currentSymbolTable[i]->memoryLocation);
	}
  

/*-------------------------- Second Pass -----------------------------*/  

/* Need to disregard the first line because it is .orig */

	int commandMatch = 0;

	for(i = 1; i < currentLineSize; i += 1){
		commandMatch = decodeCommand(currentLine[i]->lineComponent[0], opcodeConstants, currentLine[i]->linePosition);
		if(commandMatch >= 0){
			analyzeCommand(commandMatch, currentLine[i]);
		}
	}

	return(0);

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
	
	/* printf("Inside breakLine\n"); */
	
	currentLine->lineComponentCount = 0;
	/* printf("Assigned my counter\n"); */
	currentLine->lineComponent = (char**) malloc (sizeof(char**) * 10);

	for(i = 0; i < *lineSize; i += 1){
		if(readIn[i] != ' '){
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
	  
	  
	  if(origLine->lineComponent[1][0] == '#'){
		  *startPos = atoi(&origLine->lineComponent[1][1]);
		  snprintf (hexString, 7, "0x%x", *startPos);
	  }
	  else{
		  hexString = &origLine->lineComponent[1][1];
		  *startPos = atoi(&origLine->lineComponent[1][1]);
	  }
	  /*printf("Memory in dec format: %d\n", tempInt + 2);
	  printf("Memory starts at: %s\n", hexString);*/
	  return(hexString);
}  
  
  
/*--------------------------- Function -------------------------------*/
 
  int isThisALabel (char* labelPossible, char** opcodeConstants, char** psuedoOpcode, int linePosition)
{	 
 /*
  * Inputs: 1st argument of a line of code
  * Outputs: Bool specifying whether or not it is a label
  * 
  --------------------------------------------------------------------*/

	int i = 0;
	int counter = 0;
	
	if(labelPossible[0] == 'x'){return(0);}
	
	/* Put a comparison to psuedo ops here. */
	
	for(i = 0; i < 3; i += 1){
		if(!strcmp(labelPossible, psuedoOpcode[i])){
		/* This is a psuedo op. */	
			return(0);
		}
	}	
	
	while(labelPossible[i] != 0){
		if(!isalnum(labelPossible[i])){
		/* This is an error situation here that needs to be dropped to console with the line number. */	
			printf("Error with label on line %d\n", linePosition);
			return(0);
		}
		i += 1;
		counter += 1;
	}
	
	if(counter > 19){
		/* This is an error situation here that needs to be dropped to console with the line number. */	
		printf("Error with label on line %d\n", linePosition);
		labelPossible = 0;
		return(0);
	}
	
	for(i = 0; i < 32; i += 1){
		if(!strcmp(labelPossible, opcodeConstants[i])){
		/* This is an opcode. */	
		labelPossible = 0;
			return(0);
		}
	}
	
	/*printf("Length of the potential label is: %d\n", counter);*/
	
	return(1);
}

/*--------------------------- Function -------------------------------*/
 
  int decodeCommand (char* command, char** opcodeConstants, int linePosition)
{
 /*
  * Inputs: char* of potential command, char** of opcodes as strings
  * Outputs: None
  * 
  --------------------------------------------------------------------*/ 
	int i;
	
	/* printf("Inside decode.\n"); */
	
	/* If I send in a null, it was a bad label. Return -1. */
	
	for(i = 0; i < 32; i += 1){
		if(!strcmp(command, opcodeConstants[i])){
			return(i);
		}
	}
	/* Throw an error before returning negative one. */
	return(-1);  
}

/*--------------------------- Function -------------------------------*/

	void analyzeCommand(int commandMatch, brokenLine* currentLine)
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
   switch(commandMatch)
   {
	   
		case 0:
			printf("Received opcode add\n");
			break;		
		case 1:
			printf("Received opcode and\n");
			break;
		case 2:
			printf("Received opcode puts\n");
			break;
		case 3:
			printf("Received opcode halt\n");
			break;
		case 4:
			printf("Received opcode jump\n");
			break;
		case 5:
			printf("Received opcode jsr\n");
			break;
		case 6:
			printf("Received opcode jsrr\n");
			break;
		case 7:
			printf("Received opcode ldb\n");
			break;
		case 8:
			printf("Received opcode ldw\n");
			break;
		case 9:
			printf("Received opcode lea\n");
			break;
		case 10:
			printf("Received opcode nop\n");
			break;
		case 11:
			printf("Received opcode not\n");
			break;
		case 12:
			printf("Received opcode ret\n");
			break;
		case 13:
			printf("Received opcode lshf\n");
			break;
		case 14:
			printf("Received opcode rshfl\n");
			break;
		case 15:
			printf("Received opcode rshfa\n");
			break;
		case 16:
			printf("Received opcode rti\n");
			break;
		case 17:
			printf("Received opcode stb\n");
			break;
		case 18:
			printf("Received opcode stw\n");
			break;
		case 19:
			printf("Received opcode trap\n");
			break;
		case 20:
			printf("Received opcode xor\n");
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
			printf("Received opcode br\n");
			break;
		case 25:
			printf("Received opcode br\n");
			break;
		case 26:
			printf("Received opcode br\n");
			break;
		case 27:
			printf("Received opcode br\n");
			break;
		case 28:
			printf("Received opcode br\n");
			break;
		case 29:
			printf("Received opcode br\n");
			break;
		case 30:
			printf("Received opcode br\n");
			break;
		case 31:
			printf("Received opcode br\n");
			break;
		
		
		default: 
			printf("How the fuck?");
		}
  
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
	opcodeConstants[2]	 = "puts";
	opcodeConstants[3] = "halt";
	opcodeConstants[4] = "jmp";
	opcodeConstants[5] = "jsr";
	opcodeConstants[6] = "jsrr";
	opcodeConstants[7] = "ldb";
	opcodeConstants[8] = "ldw";
	opcodeConstants[9] = "lea";
	opcodeConstants[10] = "nop";
	opcodeConstants[11] = "not";
	opcodeConstants[12] = "ret";
	opcodeConstants[13] = "lshf";
	opcodeConstants[14] = "rshfl";
	opcodeConstants[15] = "rshfa";
	opcodeConstants[16] = "rti";
	opcodeConstants[17] = "stb";
	opcodeConstants[18] = "stw";
	opcodeConstants[19]	 = "trap";
	opcodeConstants[20] = "xor";
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
  	psuedoOpcodes[1] = ".fill";
  	psuedoOpcodes[2] = ".end";
	
}
	
	
	
 
 
