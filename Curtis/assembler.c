/*
	Name  1: Curtis Martin 
	UTEID 1: cm38365
*/

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <limits.h>
#include <stdbool.h>

int maxLines (FILE* input_file);
int parser (FILE* input_file, char* line, char** label, char** opcode, char** arg_1, char** arg_2, char** arg_3, char** arg_4);
int evaluate_opcode (char* line_ptr, char* arg_1, char* arg_2, char* arg_3, int addr);
int determine_opcode (char* line_ptr);
int string_to_number (char* str);
int determine_sr2_register (char* reg);
int determine_sr1_register (char* reg);
int determine_dr_register (char* reg);

#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
#define MAX_LINE_LENGTH 255

/*line 0 is the starting address*/
typedef struct {
  char label[20];
	int address;
}	table_entry;
table_entry symbol_table[MAX_SYMBOLS];

enum {
  DONE, 
  OK, 
  EMPTY_LINE	
};

FILE* input_file = NULL;
FILE* output_file = NULL;
int index = 0;

int main(int argc, char* argv[]) {
  char *program_name   = NULL;
  char *in_file_name   = NULL;
  char *out_file_name  = NULL;
  char line[MAX_LINE_LENGTH + 1];
  char *label;
  char *opcode;
  char *arg_1;
  char *arg_2;
  char *arg_3;
  char *arg_4;
  int value = 0;
  int line_count = 0;
  int i = 0;
  
   
  program_name   = argv[0];
  in_file_name   = argv[1];
  out_file_name  = argv[2];

  /*input line checker*/
  if (in_file_name == NULL || out_file_name == NULL) {
    printf("Incorrect command-line format: assemble <source.asm> <output.obj>\n");
    exit(1);
  }   
  else if (!strstr (in_file_name, ".asm")) {
    printf("Incorrect input file format: %s does not match <source.asm>\n", in_file_name);
    exit(1);
  }
  else if (!strstr (out_file_name, ".obj")) {
    printf("Incorrect output file format: %s does not match <output.obj>\n", out_file_name);
    exit(1);
  }
  else {
    input_file  = fopen(argv[1], "r");
    output_file = fopen(argv[2], "w");
  }

  /*first parse*/
  do {
		value = parser(input_file, line, &label, &opcode, &arg_1, &arg_2, &arg_3, &arg_4);
		if (strcmp(opcode, ".end") == 0) {
			break;
		}    
   	if ((value != DONE && value != EMPTY_LINE)) {
		  if ((strcmp(opcode, ".orig") == 0) && !(line_count == 0)) {
				exit(4);
			}
      else if ((strcmp(opcode, ".orig") == 0) && (line_count == 0)) {
				if (!(string_to_number(arg_1) % 2) && (string_to_number(arg_1) >= 0) && (string_to_number(arg_1) <= 65535)) {
         	strcpy(symbol_table[0].label, "Symbol_table!");
        	symbol_table[0].address = string_to_number(arg_1);
         	index += 1;
				}
				else {
					exit(3);
				}
			}
    	/*label checker*/
    	else if ( *label ) { 
				/*check for valid opcode*/
				if ((determine_opcode(opcode) == -1)) {
					printf("ERROR: Undefined opcode %s\n", label);	
					exit(2);
				}
				if (strlen(label) < 21) {
					for ( i = 0; i < strlen( label ); i++ ) {
						if (!(isalnum(label[i]))) {
							printf("ERROR: Undefined label %s\n", label);	
							exit(1);
						}
					}
					/*duplicate label checker*/
					for (i = 0; i < index; i++) {
						if ((strcmp(symbol_table[i].label, label) == 0) || (strcmp(symbol_table[i].label, "in") == 0) || (strcmp(symbol_table[i].label, "out") == 0) || (strcmp(symbol_table[i].label, "getc") == 0) || (strcmp(symbol_table[i].label, "puts") == 0) ) {
						printf("duplicate symbol checker");	
						exit(4);
						}
					}
					strcpy(symbol_table[index].label, label);
					symbol_table[index].address = symbol_table[0].address + line_count;
					index += 1;
					line_count += 1;
				}
				else {
					printf("ERROR: Undefined label %s\n", label);	
					exit(1);
				}
			}
			else {
     		line_count += 1;
			}
    }
	} while (value != DONE);

  rewind(input_file); 
	line_count = symbol_table[0].address;
	
  /*second parse*/
  do {
    value = parser(input_file, line, &label, &opcode, &arg_1, &arg_2, &arg_3, &arg_4);
    if ( value != DONE && value != EMPTY_LINE ) {
      if ( strcmp(opcode, ".orig") == 0 ) {
				fprintf( output_file, "0x%0.4X\n", string_to_number(arg_1) );
        line_count += 1;
			}
			else if ( strcmp(opcode, ".fill") == 0 ) {
				if ((string_to_number(arg_1) <= 65535) && (string_to_number(arg_1) >= -32768)) {
   				fprintf( output_file, "0x%0.4X\n", string_to_number(arg_1) );
			  	line_count += 1;
				}
				else {
					printf("ERROR: Invalid constant %d\n", string_to_number(arg_1));
					exit(3);
				}
 			}
			else if (strcmp(opcode, ".end") != 0 ) {
        fprintf( output_file, "0x%0.4X\n", evaluate_opcode(opcode, arg_1, arg_2, arg_3, line_count) );
        line_count += 1;
			}
			else {
				fclose( input_file );
  			fclose( output_file );
				exit(0);
	    } 
    }
  } while ( value != DONE );
}

int evaluate_opcode (char* line_ptr, char* arg_1, char* arg_2, char* arg_3, int addr) {
	/*Add instruction*/
  if (strcmp(line_ptr, "add") == 0) {
		if (determine_sr2_register( arg_3 ) != -1) {
			return ( 4096 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + determine_sr2_register( arg_3 ) );
		}
		else if ((string_to_number( arg_3 ) <= 15) && (string_to_number( arg_3 ) >= -16)) {
			if ( string_to_number( arg_3 ) < 0 ) {
				return ( 4160 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ));
			}
			else {
 		   	return ( 4128 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ) );
			}
		}
		else {
    	printf("ERROR: Invalid constant %d", string_to_number( arg_3 ));
			exit(3);
		}
	}
  /*And instruction*/
  else if (strcmp(line_ptr, "and") == 0) {
		if (determine_sr2_register( arg_3 ) != -1) {
			return ( 20480 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + determine_sr2_register( arg_3 ) );
		}
		else if ((string_to_number( arg_3 ) <= 15) && (string_to_number( arg_3 ) >= -16)) {
			if ( string_to_number( arg_3 ) < 0 ) {
				return ( 20544 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ));
			}
			else {
 		   	return ( 20512 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ) );
			}
		}
		else {
    	printf("ERROR: Invalid constant %d", string_to_number( arg_3 ));
			exit(3);
		}
  }
	/*Branch instruction*/
  else if ((strcmp(line_ptr, "br") == 0) || (strcmp(line_ptr, "brnzp") == 0)) {
    if ((addr - string_to_number( arg_1 )) < 0) {
		  return (3584 + string_to_number( arg_1 ) - addr);
		}
		else {
			return (4096 - (addr - string_to_number( arg_1 )) );
		}	
  }
  else if (strcmp(line_ptr, "brn") == 0) {
    if ((addr - string_to_number( arg_1 )) < 0) {
		  return (2048 + string_to_number( arg_1 ) - addr);
		}
		else {
			return (2560 - (addr - string_to_number( arg_1 )));
		}
  }
  else if (strcmp(line_ptr, "brp") == 0) {
    if ((addr - string_to_number( arg_1 )) < 0) {
		  return (512 + string_to_number( arg_1 ) - addr);
		}
		else {
			return (1024 - (addr - string_to_number( arg_1 )));
		}
	}
  else if (strcmp(line_ptr, "brnp") == 0) {
    if ((addr - string_to_number( arg_1 )) < 0) {
		  return (2560 + string_to_number( arg_1 ) - addr);
		}
		else {
			return (3072 - (addr - string_to_number( arg_1 )));
		}
  }
  else if (strcmp(line_ptr, "brz") == 0) {
    if ((addr - string_to_number( arg_1 )) < 0) {
		  return (1024 + string_to_number( arg_1 ) - addr);
		}
		else {
			return (1536 - (addr - string_to_number( arg_1 )));
		}
  }
  else if (strcmp(line_ptr, "brnz") == 0) {
    if ((addr - string_to_number( arg_1 )) < 0) {
		  return (3072 + string_to_number( arg_1 ) - addr);
		}
		else {
			return (3584 - (addr - string_to_number( arg_1 )));
		}
  }
  else if (strcmp(line_ptr, "brzp") == 0) {
    if ((addr - string_to_number( arg_1 )) < 0) {
		  return (1536 + string_to_number( arg_1 ) - addr);
		}
		else {
			return (2048 - (addr - string_to_number( arg_1 )));
		}
	}
	/*Jmp*/
  else if (strcmp(line_ptr, "jmp") == 0) {
    if ((strcmp(arg_1, "") != 0) && (strcmp(arg_2, "") == 0)){
			return ( 49152 + determine_sr1_register( arg_1 ) );
		}
		else {
			printf("ERROR: Invalid constant %s\n", arg_2);
			exit(4);
		}
  }
	/*Jsr*/
  else if (strcmp(line_ptr, "jsr") == 0) {
    if ((addr - string_to_number( arg_1 )) < 0) {
		  return (18432 + string_to_number( arg_1 ) - addr);
		}
		else {
			return (20480 - (addr - string_to_number( arg_1 )));
		}
  }
	/*Jsrr*/
  else if (strcmp(line_ptr, "jsrr") == 0) {
		if ((strcmp(arg_1, "") != 0) && (strcmp(arg_2, "") == 0)) {
			return ( 16384 + determine_sr1_register( arg_1 ) );
		}
		else {
			printf("ERROR: Invalid constant %s\n", arg_2);
			exit(4);
		}
  }
	/*Loads*/
  else if (strcmp(line_ptr, "ldb") == 0) {
		if ((string_to_number( arg_3 ) <= 31) && (string_to_number( arg_3 ) >= -32)) {
			if ( string_to_number( arg_3 ) < 0 ) {
				return ( 8256 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ));
			}
			else {
 		   	return ( 8192 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ) );
			}
		}
		else {
    	printf("ERROR: Invalid constant %d", string_to_number( arg_3 ));
			exit(3);
		}
  }
	/*ldw*/
  else if (strcmp(line_ptr, "ldw") == 0) {
		if ((string_to_number( arg_3 ) <= 31) && (string_to_number( arg_3 ) >= -32)) {
			if ( string_to_number( arg_3 ) < 0 ) {
				return (24640 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ));
			}
			else {
 		   	return (24576 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ) );
			}
		}
		else {
    	printf("ERROR: Invalid constant %d", string_to_number( arg_3 ));
			exit(3);
		}
	}
	/*Lea*/
  else if (strcmp(line_ptr, "lea") == 0) {
    if ((addr - string_to_number( arg_2 )) < 0) {
		  return ( 57344 + determine_dr_register( arg_1 ) + string_to_number( arg_2 ) - addr);
		}
		else {
			return ( 57856 + determine_dr_register( arg_1 ) - (addr - string_to_number( arg_2 )));
		}
	}
	/*Nop*/
  else if (strcmp(line_ptr, "nop") == 0) {
		if ( strcmp(arg_1, "") == 0) {
			return ( 0 );
		}
		else {
			printf("ERROR: Invalid constant %s\n", arg_1);
			exit(4);
		}
  }
	/*not*/
  else if (strcmp(line_ptr, "not") == 0) {
		if ( strcmp(arg_3, "") == 0 ) {
  		return ( 36927 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) );
		}
		else {
			printf("ERROR: Invalid constant %s\n", arg_1);
			exit(4);
		}
  }
	/*Ret*/
  else if (strcmp(line_ptr, "ret") == 0) {
		if ( strcmp(arg_1, "") == 0) {
			return ( 49600 );
		}
		else {
			printf("ERROR: Invalid constant %s\n", arg_1);
			exit(4);
		}
  }
	/*Rti*/
  else if (strcmp(line_ptr, "rti") == 0) {
		if ( strcmp(arg_1, "") == 0) {
			return ( 32768 );
		}
		else {
			printf("ERROR: Invalid constant %s\n", arg_1);
			exit(4);
		}
  }
	/*Shifts*/
  else if (strcmp(line_ptr, "lshf") == 0) {
    if ((string_to_number( arg_3 ) >= 0) && (string_to_number( arg_3 ) <= 15)){
			return ( 53248 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ) );
		}
		else {
			printf("ERROR: Invalid contsant %d\n", string_to_number( arg_3 ));
			exit(3);
		}
  }
  else if (strcmp(line_ptr, "rshfl") == 0) {
    if ((string_to_number( arg_3 ) >= 0) && (string_to_number( arg_3 ) <= 15)){
			return ( 53264 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ) );
		}
		else {
			printf("ERROR: Invalid contsant %d\n", string_to_number( arg_3 ));
			exit(3);
		}
  }
  else if (strcmp(line_ptr, "rshfa") == 0) {
    if ((string_to_number( arg_3 ) >= 0) && (string_to_number( arg_3 ) <= 15)){
			return ( 53296 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ) );
		}
		else {
			printf("ERROR: Invalid contsant %d\n", string_to_number( arg_3 ));
			exit(3);
		}
  }
	/*Stores*/
  else if (strcmp(line_ptr, "stb") == 0) {
		if ((string_to_number( arg_3 ) <= 31) && (string_to_number( arg_3 ) >= -32)) {
			if ( string_to_number( arg_3 ) < 0 ) {
				return ( 12352 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ));
			}
			else {
 		   	return ( 12288 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ) );
			}
		}
		else {
    	printf("ERROR: Invalid constant %d", string_to_number( arg_3 ));
			exit(3);
		}
  }
	/*Stw*/
  else if (strcmp(line_ptr, "stw") == 0) {
		if ((string_to_number( arg_3 ) <= 31) && (string_to_number( arg_3 ) >= -32)) {
			if ( string_to_number( arg_3 ) < 0 ) {
				return (28736 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ));
			}
			else {
 		   	return (28672 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ) );
			}
		}
		else {
    	printf("ERROR: Invalid constant %d", string_to_number( arg_3 ));
			exit(3);
		}
	}
	/*Trap*/
  else if ((strcmp(line_ptr, "trap") == 0))  {
    if ((string_to_number( arg_1 ) >= 0) && (*arg_1 == 'x') && (strcmp(arg_2, "") == 0) && (string_to_number( arg_1) <= 255) ) {
			return ( 61440 + string_to_number( arg_1 ));
		}
		else {
			printf("ERROR: Invalid contsant %s\n", arg_2);
			exit(3);
		}
  }
	/*Halt*/
	else if (strcmp(line_ptr, "halt") == 0) {
		if ( strcmp(arg_1, "") == 0) {
			return ( 61477 );
		}
		else {
			printf("ERROR: Invalid constant %s\n", arg_1);
			exit(4);
		}
	}
	/*XOR*/
  else if (strcmp(line_ptr, "xor") == 0) {
		if (determine_sr2_register( arg_3 ) != -1) {
			return ( 36864 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + determine_sr2_register( arg_3 ) );
		}
		else if ((string_to_number( arg_3 ) <= 15) && (string_to_number( arg_3 ) >= -16)) {
			if ( string_to_number( arg_3 ) < 0 ) {
				return ( 36928 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ));
			}
			else {
 		   	return ( 36896 + determine_dr_register( arg_1 ) + determine_sr1_register( arg_2 ) + string_to_number( arg_3 ) );
			}
		}
		else {
    	printf("ERROR: Invalid constant %d", string_to_number( arg_3 ));
			exit(3);
		}
	}
  else {
    return ( -1 );
  }
}

int parser (FILE* input_file, char* line, char** label, char** opcode, char** arg_1, char** arg_2, char** arg_3, char** arg_4) {
  int i;
  char* ptr_line;
  char* ret;
       
  if (!fgets(line, 250, input_file)) {
    return ( DONE );
  }

  for ( i = 0; i < strlen(line); i++ ) {
    line[i] = tolower(line[i]);
  }
  
  *label = *opcode = *arg_1 = *arg_2 = *arg_3 = *arg_4 = line + strlen(line);
  ptr_line = line;

  while (*ptr_line != ';' && *ptr_line != '\0' && *ptr_line != '\n') {
   ptr_line++;
  }
  
  *ptr_line = '\0';
 
  if (!(ptr_line = strtok( line, "\t\n ,"))) {
    return (EMPTY_LINE );
  }
  
  if ( (determine_opcode( ptr_line) == -1) && (ptr_line[0] != '.') ) {
    *label = ptr_line;
    if (!(ptr_line = strtok( NULL, "\t\n ,"))) {
      return ( OK );
    }
  } 
  
  *opcode = ptr_line;

	if( !( ptr_line = strtok( NULL, "\t\n ," ) ) ) {
    return( OK );
  }
	   
  *arg_1 = ptr_line;

  if( !( ptr_line = strtok( NULL, "\t\n ," ) ) ) {
    return( OK );
  }
	
  *arg_2 = ptr_line;

	if( !( ptr_line = strtok( NULL, "\t\n ," ) ) ){
    return( OK );
  }
 
	*arg_3 = ptr_line;

	if( !( ptr_line = strtok( NULL, "\t\n ," ) ) ) {
    return( OK );
  }
	
  *arg_4 = ptr_line;

  return( OK );   
} 

int string_to_number (char* str) { 
   char* t_ptr;
   char* original_str;
   int t_length;
   int k;
   int number = 0; 
   int negative = 0;
   long int long_number;

   original_str = str;
   /* decimal */
   if ( *str == '#' ) 
   { 
     str++;
     /* dec is negative */
     if ( *str == '-' )
     {
       negative = 1;
       str++;
     }

     t_ptr = str;
     t_length = strlen(t_ptr);

     for( k = 0; k < t_length; k++ )
     {
       if ( !isdigit(*t_ptr) ) 
       {
	       printf("Error: invalid decimal operand, %s\n",original_str);
	       exit(4);
       }
       t_ptr++;
     }

     number = atoi(str);
     if ( negative ) 
     {
       number = -number;
     }
       return number;
  }
  /* hex */
  else if( *str == 'x' ) {	
       str++;

       /* hex is negative */
       if( *str == '-' ) {
       negative = 1;
       str++;
       }
       t_ptr = str;
       t_length = strlen(t_ptr);
       for ( k = 0; k < t_length; k++ )
       {
         if ( !isxdigit(*t_ptr) ) 
         {
	         printf("Error: invalid hex operand, %s\n",original_str);
	         exit(4);
         }
         t_ptr++;
      }
     
      /* convert hex string into integer */
      long_number = strtol(str, NULL, 16);    
      number = (long_number > INT_MAX)? INT_MAX : long_number;
 
     if ( negative ) { 
       number = -number;
     }
       return number;
  }
  else 
  {
		for (k = 1; k < index; k++) {
			if (strcmp(symbol_table[k].label, str) == 0) {
				return (symbol_table[k].address);
      } 
		}
    	printf( "Error: invalid operand, %s\n", original_str);
	    exit(4);  
  }
}

int determine_sr2_register (char* reg) {
	if (strcmp(reg, "r0") == 0) {
		return ( 0 );
 	}
  else if (strcmp(reg, "r1") == 0) {
		return ( 1 );
 	}
  else if (strcmp(reg, "r2") == 0) {
		return ( 2 );
 	}
  else if (strcmp(reg, "r3") == 0) {
		return ( 3 );
 	}
  else if (strcmp(reg, "r4") == 0) {
		return ( 4 );
 	}
  else if (strcmp(reg, "r5") == 0) {
		return ( 5 );
 	}
  else if (strcmp(reg, "r6") == 0) {
		return ( 6 );
 	}
  else if (strcmp(reg, "r7") == 0) {
		return ( 7 );
 	}
	else {
		return ( -1 );
	}
}

int determine_sr1_register (char* reg) {
	if (strcmp(reg, "r0") == 0) {
		return ( 0 );
 	}
  else if (strcmp(reg, "r1") == 0) {
		return ( 64 );
 	}
  else if (strcmp(reg, "r2") == 0) {
		return ( 128 );
 	}
  else if (strcmp(reg, "r3") == 0) {
		return ( 192 );
 	}
  else if (strcmp(reg, "r4") == 0) {
		return ( 256 );
 	}
  else if (strcmp(reg, "r5") == 0) {
		return ( 320 );
 	}
  else if (strcmp(reg, "r6") == 0) {
		return ( 384 );
 	}
  else if (strcmp(reg, "r7") == 0) {
		return ( 448 );
 	}
	else {
printf("sr1_register");
		exit(4);
	}
}

int determine_dr_register (char* reg) {
	if (strcmp(reg, "r0") == 0) {
		return ( 0 );
 	}
  else if (strcmp(reg, "r1") == 0) {
		return ( 512 );
 	}
  else if (strcmp(reg, "r2") == 0) {
		return ( 1024 );
 	}
  else if (strcmp(reg, "r3") == 0) {
		return ( 1536 );
 	}
  else if (strcmp(reg, "r4") == 0) {
		return ( 2048 );
 	}
  else if (strcmp(reg, "r5") == 0) {
		return ( 2560 );
 	}
  else if (strcmp(reg, "r6") == 0) {
		return ( 3072 );
 	}
  else if (strcmp(reg, "r7") == 0) {
		return ( 3584 );
 	}
	else {
printf("dr_register");
		exit(4);
	}
}

int determine_opcode (char* line_ptr) {
  if (strcmp(line_ptr, "add") == 0) {
   	return ( 1 );
	}
  else if (strcmp(line_ptr, "and") == 0) {
	 	return ( 1 );
	}
  else if (strcmp(line_ptr, "br") == 0) {
		return ( 1 );	
  }
  else if (strcmp(line_ptr, "brn") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "brp") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "brnp") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "brz") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "brnz") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "brzp") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "brnzp") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "halt") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "jmp") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "jsr") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "jsrr") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "ldb") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "ldw") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "lea") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "nop") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "not") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "ret") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "rti") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "lshf") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "rshfl") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "rshfa") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "stb") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "stw") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "trap") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, "xor") == 0) {
		return ( 1 );
  }
  else if (strcmp(line_ptr, ".fill") == 0) {
		return ( 1 );
  }
  else {
    return ( -1 );
  }
}

