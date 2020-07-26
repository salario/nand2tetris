#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

// Prototypes
bool loadpredefined(void);
bool loadlabels(const char *asmfile);
bool loadsymbols(const char *asmfile);
bool checksymbol(const char *symbol);
unsigned symboladdress(const char *symbol);
bool unload(void);

const char* parser(char *line);
const char* dec_to_bin(int i);
const char* dest(const char *instruction);
const char* comp(const char *instruction);
const char* jump(const char *instruction);

unsigned int hash(const char *symbol);
bool insert(const char *symbol, unsigned address);
char* trim(char *str);


char cinst[17];
char reverse[17];
char final_a[17];

// Represents a node in a hash table
typedef struct node
{
    char symbol[100];
    unsigned address;
    struct node *next;
}
node;

// Number of slots in hash table
const unsigned int N = 214639;

// Hash table
node *table[N] = {NULL};

// Set loaded variable to false
bool loaded = false;

int main(int argc, char *argv[])
{
    // Check for correct number of args
    if (argc != 2)
	{
	    printf("Usage: ./assembler FILE.asm\n");
	    return 1;
	}

    // Initiatlize .asm file
    char *asmfile = argv[1];
    int hack_counter = 0;

    // Initialize .hack file
    char hackfile[strlen(asmfile) + 2];
    for (int i = 0; asmfile[i] != 46; i++)
	{
	    hackfile[i] = asmfile[i];
	    hack_counter++;
	}
    hackfile[hack_counter] = '\0';

    strcat(hackfile, ".hack");

    // Load predefined labels to hash table
    loaded = loadpredefined();
    if (!loaded)
	{
	    printf("Could not load predefined from %s.\n", asmfile);
	    unload();
	    return 1;
	} 

    // Load labels into hash table
    loaded = loadlabels(asmfile);
    if (!loaded)
	{
	    printf("Could not load labels from %s.\n", asmfile);
	    unload();
	    return 1;
	}

    // Load symbols into hash table
    loaded = loadsymbols(asmfile);

    // Exit if symbols not loaded
    if (!loaded)
	{
	    printf("Could not load symbols from %s.\n", asmfile);
	    unload();
	    return 1;
	}
    
    // Open .asm file for main translation
    FILE *file = fopen(asmfile, "r");
    if (file == NULL)
	{
	    printf("Could not open %s for main processing.\n", asmfile);
	    unload();
	    return 1;
	}

    // Open .hack file for main translation
    FILE *hack = fopen(hackfile, "w");
    if (hack == NULL)
	{
	    printf("Could not open %s for main processing.\n", hackfile);
	    unload();
	    return 1;
	}
    
    char line[200];
    
    while(fgets(line, sizeof(line), file))
	{
	    char *new_line;
	    new_line = trim(line);
	    if (new_line[0] != '\r' && line[0] != '\n' && line[0] && line[0] != '/' && line[0] != '(')
		{
		    char final_out[17];
		    strcpy(final_out, parser(new_line));

		    fputs(final_out, hack);
		    fputs("\n", hack);
		}
	    else
		{
		    continue;
		}
	}

    // Close files, clear memory, and exit main
    fclose(file);
    fclose(hack);
    unload();
    return 0;
}

// Convert decimal to binary number
const char* dec_to_bin(int n)
{
    if (0 <= n && n <= 65536)
	{
	    for (int i = 0; i < 16; i++)
		{
		    if (n % 2 == 0)
			cinst[i] = '0';
		    else
			cinst[i] = '1';
		    n = n / 2;
		}
	}
    for (int x = 15, y = 0; x >= 0; x--, y++)
	{
	    reverse[y] = cinst[x];
	}
    reverse[16] = '\0';
    return reverse;	
}

// Handle Destination instructions
const char* dest(const char *instruction)
{
    char *res;
    if (strchr(instruction, '=') == NULL)
	return "000";
    res = strstr(instruction, "AMD=");
    if (res)
	return "111";
    res = strstr(instruction, "AD=");
    if (res)
	return "110";
    res = strstr(instruction, "AM=");
    if (res)
	return "101";
    res = strstr(instruction, "A=");
    if (res)
	return "100";
    res = strstr(instruction, "MD=");
    if (res)
	return "011";
    res = strstr(instruction, "D=");
    if (res)
	return "010";
    else
	return "001";
    free(res);
}

const char* comp(const char *instruction)
{
    char *res;
    res = strstr(instruction, "D|M");
    if (res)
	return "1010101";
    res = strstr(instruction, "D|A");
    if (res)
	return "0010101";
    res = strstr(instruction, "D&M");
    if (res)
	return "1000000";
    res = strstr(instruction, "D&A");
    if (res)
	return "0000000";
    res = strstr(instruction, "M-D");
    if (res)
	return "1000111";
    res = strstr(instruction, "A-D");
    if (res)
	return "0000111";
    res = strstr(instruction, "D-M");
    if (res)
	return "1010011";
    res = strstr(instruction, "D-A");
    if (res)
	return "0010011";
    res = strstr(instruction, "D+M");
    if (res)
	return "1000010";
    res = strstr(instruction, "D+A");
    if (res)
	return "0000010";
    res = strstr(instruction, "M-1");
    if (res)
	return "1110010";
    res = strstr(instruction, "A-1");
    if (res)
	return "0110010";
    res = strstr(instruction, "D-1");
    if (res)
	return "0001110";
    res = strstr(instruction, "M+1");
    if (res)
	return "1110111";
    res = strstr(instruction, "A+1");
    if (res)
	return "0110111";
    res = strstr(instruction, "D+1");
    if (res)
	return "0011111";
    res = strstr(instruction, "-M");
    if (res)
	return "1110011";
    res = strstr(instruction, "-A");
    if (res)
	return "0110011";
    res = strstr(instruction, "-D");
    if (res)
	return "0001111";
    res = strstr(instruction, "!M");
    if (res)
	return "1110001";
    res = strstr(instruction, "!A");
    if (res)
	return "0110001";
    res = strstr(instruction, "!D");
    if (res)
	return "0001101";
    res = strstr(instruction, "-1");
    if (res)
	return "0111010";
    res = strstr(instruction, "=M");
    if (res)
	return "1110000";
    if (strstr(instruction, "=D") || strstr(instruction, "D;"))
	return "0001100";
    if (strstr(instruction, "=A") || strstr(instruction, "A;"))
	return "0110000";
    if (strstr(instruction, "=1") || strstr(instruction, "1;"))
	return "0111111";
    else
	return "0101010";
    free(res);
}

// Handle Jump instruction
const char* jump(const char *instruction)
{
    char *res;
    if (strchr(instruction, ';') == NULL)
	return "000";
    res = strstr(instruction, "JGT");
    if (res)
	return "001";
    res = strstr(instruction, "JEQ");
    if (res)
	return "010";
    res = strstr(instruction, "JGE");
    if (res)
	return "011";
    res = strstr(instruction, "JLT");
    if (res)
	return "100";
    res = strstr(instruction, "JNE");
    if (res)
	return "101";
    res = strstr(instruction, "JLE");
    if (res)
	return "110";
    else
	return "111";
    free(res);
}
 
// Parse each instruction line and return instruction code
const char* parser(char *line)
{
    char *code_line;
    unsigned i;
    code_line = line;
    for (i = 0; !(isspace(code_line[i])); i++)
	{
	    code_line[i] = code_line[i];
	}
    code_line[i] = '\0';
    if (code_line[0] == '@')
	{
	    code_line++;
	    if (isalpha(code_line[0]))
		{
		    strcpy(final_a, dec_to_bin(symboladdress(code_line)));
		    return final_a;
		}
	    else
		{
		    strcpy(final_a, dec_to_bin(atoi(code_line)));
		    return final_a;
		}
	}
    else
	{
	    strcpy(final_a, "111");
	    strcat(final_a, comp(code_line));
	    strcat(final_a, dest(code_line));
	    strcat(final_a, jump(code_line));
	    return final_a;
	}
    free(code_line);
}		    

// Check if symbol is already in hash table
unsigned symboladdress(const char *symbol)
{
    unsigned index = hash(symbol);

    if (table[index] == NULL)
	return 0;

    node *tmp = table[index];

    while (tmp != NULL)
	{
	    if (strcmp(tmp->symbol, symbol) == 0)
		return tmp->address;
		
	    tmp = tmp->next;
	}
    free(tmp);
    return 0;
}

bool checksymbol(const char *symbol)
{
    unsigned index = hash(symbol);

    if (table[index] == NULL)
	return false;

    node *tmp = table[index];

    while (tmp != NULL)
	{
	    if (strcmp(tmp->symbol, symbol) == 0)
	        return true;

	    tmp = tmp->next;
	}
    free(tmp);
    return false;
}


// Insert symbols to hash table
bool loadsymbols(const char *asmfile)
{
    char line[200];  
    int counter = 16;
    bool check_sym = false;
    bool insert_success = false;
    FILE *file = fopen(asmfile, "r");
    if (file == NULL)
	{
	    printf("Could not open %s for symbol processing.\n", asmfile);
	    return false;
	}
    
    while(fgets(line, sizeof(line), file))
	{
	    char *new_line;
	    new_line = trim(line);

	    if (new_line[0] == '@' && isalpha(new_line[1]))
		{
		    //unsigned i;
		    // for (i = 1; new_line[i] != '\0' && !isspace(new_line[i]); i++)
		    //{
		    //	    symbol[i-1] = new_line[i];
		    //	}
		    //symbol[i] = '\0';
		    new_line++;
		    unsigned i;
		    for (i = 0; !(isspace(new_line[i])); i++)
			{
			    new_line[i] = new_line[i];
			}
		    new_line[i] = '\0';

		    check_sym = checksymbol(new_line);
		    if (!check_sym)
			{
			    insert_success = insert(new_line, counter);
			    if (!insert_success)
				{
				    printf("Could not load symbol to table.\n");
				    free(new_line);
				    return false;
				}
			    counter++;
			}
		}
	}
    // Close files, clear memory, and exit main
    fclose(file);
    return true;
}

// Insert labels to hash table
bool loadlabels(const char *asmfile)
{
    char line[200];
    int counter = 0;
    bool insert_success = false;
    FILE *file = fopen(asmfile, "r");
    if (file == NULL)
	{
	    printf("Could not open %s for label processing.\n", asmfile);
	    return false;
	}
   
    while(fgets(line, sizeof(line), file))
	{
	    char *new_line;
	    new_line = trim(line);
	    if (new_line[0] == '(')
		{
		    new_line++;
		    unsigned i;
		    for (i = 0; new_line[i] != ')'; i++)
			{
			    new_line[i] = new_line[i];
			}
		    new_line[i] = '\0';

		    insert_success = insert(new_line, counter);

		    if (!insert_success)
			{
			    printf("Could not load %s label to table.\n", new_line);
			    free(new_line);
			    return false;
			}
		}
	    else if (new_line[0] != '\n' && new_line[0] != '\r' && new_line[0] != '/')
		{
		    counter++;
		}
	}
   // Close files, clear memory, and exit main
    fclose(file);
    return true;
}

// Insert predefined symbols to hash table
bool loadpredefined(void)
{
    bool insert_success = false;
    // Insert SP, 0 to table
    insert_success = insert("SP", 0);
    if (!insert_success)
	{
	    printf("Could not load SP to table.\n");
	    return false;
	}
    // Insert LCL, 1 to table
    insert_success = insert("LCL", 1);
    if (!insert_success)
	{
	    printf("Could not load LCL to table.\n");
	    return false;
	}  

    // Insert ARG, 2 to table
    insert_success = insert("ARG", 2);
    if (!insert_success)
	{
	    printf("Could not load ARG to table.\n");
	    return false;
	}  

    // Insert THIS, 3 to table
    insert_success = insert("THIS", 3);
    if (!insert_success)
	{
	    printf("Could not load THIS to table.\n");
	    return false;
	}  

    // Insert THAT, 4 to table
    insert_success = insert("THAT", 4);
    if (!insert_success)
	{
	    printf("Could not load THAT to table.\n");
	    return false;
	}  
   
    // Insert R0, 0 to table
    insert_success = insert("R0", 0);
    if (!insert_success)
	{
	    printf("Could not load R0 to table.\n");
	    return false;
	}  

    // Insert R1, 1 to table
    insert_success = insert("R1", 1);
    if (!insert_success)
	{
	    printf("Could not load R1 to table.\n");
	    return false;
	}  

    // Insert R2, 2 to table
    insert_success = insert("R2", 2);
    if (!insert_success)
	{
	    printf("Could not load R2 to table.\n");
	    return false;
	}  

    // Insert R3, 3 to table
    insert_success = insert("R3", 3);
    if (!insert_success)
	{
	    printf("Could not load R3 to table.\n");
	    return false;
	}  

    // Insert R4, 4 to table
    insert_success = insert("R4", 4);
    if (!insert_success)
	{
	    printf("Could not load R4 to table.\n");
	    return false;
	}  

    // Insert R5, 5 to table
    insert_success = insert("R5", 5);
    if (!insert_success)
	{
	    printf("Could not load R5 to table.\n");
	    return false;
	}  

    // Insert R6, 6 to table
    insert_success = insert("R6", 6);
    if (!insert_success)
	{
	    printf("Could not load R6 to table.\n");
	    return false;
	}  

    // Insert R7, 7 to table
    insert_success = insert("R7", 7);
    if (!insert_success)
	{
	    printf("Could not load R7 to table.\n");
	    return false;
	}  

    // Insert R8, 8 to table
    insert_success = insert("R8", 8);
    if (!insert_success)
	{
	    printf("Could not load R8 to table.\n");
	    return false;
	}  

    // Insert R9, 9 to table
    insert_success = insert("R9", 9);
    if (!insert_success)
	{
	    printf("Could not load R9 to table.\n");
	    return false;
	}  

    // Insert R10, 10 to table
    insert_success = insert("R10", 10);
    if (!insert_success)
	{
	    printf("Could not load R10 to table.\n");
	    return false;
	}  

    // Insert R11, 11 to table
    insert_success = insert("R11", 11);
    if (!insert_success)
	{
	    printf("Could not load R11 to table.\n");
	    return false;
	}  

    // Insert R12, 12 to table
    insert_success = insert("R12", 12);
    if (!insert_success)
	{
	    printf("Could not load R12 to table.\n");
	    return false;
	}  

    // Insert R13, 13 to table
    insert_success = insert("R13", 13);
    if (!insert_success)
	{
	    printf("Could not load R13 to table.\n");
	    return false;
	}  

    // Insert R14, 14 to table
    insert_success = insert("R14", 14);
    if (!insert_success)
	{
	    printf("Could not load R14 to table.\n");
	    return false;
	}  

    // Insert R15, 15 to table
    insert_success = insert("R15", 15);
    if (!insert_success)
	{
	    printf("Could not load R15 to table.\n");
	    return false;
	}  
 
    // Insert SCREEN, 16384 to table
    insert_success = insert("SCREEN", 16384);
    if (!insert_success)
	{
	    printf("Could not load SCREEN to table.\n");
	    return false;
	}  

    // Insert KBD, 24576 to table
    insert_success = insert("KBD", 24576);
    if (!insert_success)
	{
	    printf("Could not load KBD to table.\n");
	    return false;
	}
    return true;
}


// Inserts symbol address pair into hash table
bool insert (const char *symbol, unsigned address)
{
    // Allocate memory for new symbol
    node *new_node = malloc(sizeof(node));
    if (new_node == NULL)
	{
	    printf("insert node malloc error.\n");
	    free(new_node);
	    return false;
	}

    // Initialize new node
    strcpy(new_node->symbol, symbol);
    new_node->address = address;

    // Run symbol through hash function
    unsigned index = hash(symbol);

    // Insert node into hash table
    if (table[index] == NULL)
	{
	    table[index] = new_node;
	    new_node->next = NULL;
	}
    else
	{
	    new_node->next = table[index];
	    table[index] = new_node;
	}
    return true;
}

// Hashes symbol to a number using Dan Bernstein's hash function
unsigned int hash(const char *symbol)
{
    unsigned int hash = 5381;
    int c;

    while ((c = *symbol++))
	{
	    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
    
    return hash % N;
}
    
// Unloads symbol table from memory, returning true if successful else false
bool unload(void)
{
    // create a variable to go through index
    int index = 0;
    
    // iterate through entire hashtable array
    while (index < N)
    {
        // if hashtable is empty at index, go to next index
        if (table[index] == NULL)
        {
            index++;
        }
        
        // if hashtable is not empty, iterate through nodes and start freeing
        else
        {
            while(table[index] != NULL)
            {
                node* cursor = table[index];
                table[index] = cursor->next;
                free(cursor);
            }
            
            // once hashtable is empty at index, go to next index
            index++;
        }
    }
    // return true if successful
    return true;
}


char* trim(char *str)
{
    for (int i = 0; str[0] == ' ' || str[0] == '\t'; str++);
    
    return str;
}
