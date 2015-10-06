/*
 *  Created on: Apr 22, 2014
 *      Author: '(Yongzhen R.)
 *     Version: v1.0-0501
 * Description: Create a command line version of extracting social networks
 *              from text of Les Miserables written by Victor Hugo.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define RELEASE
#ifdef RELEASE
// If it is the version for release.
	#define INPUT_FILE "./InputFiles/Les-Mis-full-text.txt"
	#define NAME_LIST "./InputFiles/Les-Mis-Names.txt"
#else
	// If it is the version for testing.
	#define INPUT_FILE "./InputFiles/Les-Mis-4lines.txt"
	//#define INPUT_FILE "./InputFiles/Les-Mis-Vol-1.txt"
	//#define INPUT_FILE "./InputFiles/Les-Mis-full-text.txt"

	// The name of character list.
	#define NAME_LIST "./InputFiles/Les-Mis-Names-20.txt"
	// #define NAME_LIST "./InputFiles/Les-Mis-Names.txt"
#endif
#define INPUT_MODE "r"

#define OUTPUT_FILE "./Les-Mis-Co-Occurrence.csv"
#define CSV_FORMAT "%s, %s\n"
#define OUTPUT_MODE "w"

#define BUFFER_SIZE 256
#define NFIELD 1

#define CO_OCCURRENCE 5
// Two names occurring within five lines of each other counts as a co-occurrence.

struct character
{
	char *name;
	struct line
	{
		int lineNo;
		struct line *next;
	} *lineList;
	// A linked list containing line numbers for each name.
};

// Function declarations.
size_t read_names(struct character ***charList);
void get_line_numbers(struct character ***charList, int nameNum);
struct line *add_to_line_list(int lineNo);
void analyse_and_output(struct character ***charList, int nameNum);

int main(void)
{
	struct character **arrayList;
	size_t nameNum;
	nameNum = read_names(&arrayList);
	get_line_numbers(&arrayList, nameNum);
	analyse_and_output(&arrayList, nameNum);
	free(arrayList); // After using allocated memory, free it.
	return 0;
}

/*
 * Function: read_names
 * --------------------
 * Description: open the designated character list and parse the whole file.
 * Parameter: charList: a struct storing the content of the file.
 * Return: num: the number of names in the list.
 */
size_t read_names(struct character ***charList)
{
	FILE *fp = fopen(NAME_LIST, INPUT_MODE);
	if ( fp == NULL )
	{
		perror(NAME_LIST);
		exit(EXIT_FAILURE);
	}
	size_t num = 0;
	char userinput[BUFFER_SIZE];
	*charList = NULL;
	while ( fscanf(fp, "%s", userinput) == NFIELD )
	// Using fscanf() here is a little dangerous because of buffer overflow.
	{
		*charList = realloc(*charList, (num + 1) * sizeof(struct character *));
		// Create a point array for each name.
		if ( charList == NULL )
		{
			perror("charList");
			exit(EXIT_FAILURE);
		}
		(*charList)[num] = malloc(sizeof(struct character));
		if ( (*charList)[num] == NULL )
		{
			perror("(*charList)[num]");
			exit(EXIT_FAILURE);
		}
		(*charList)[num] -> name = malloc(sizeof(char) * (strlen(userinput)+1));
		// Extra space for null character.
		if ( ((*charList)[num] -> name == NULL ) )
		{
			perror("((*charList)[num] -> name");
			exit(EXIT_FAILURE);
		}
		strcpy((*charList)[num] -> name, userinput);
		num++;
	}
	if ( ferror(fp) )
	{
		perror(NAME_LIST);
		exit(EXIT_FAILURE);
	}
	fclose(fp);
	return num;
}

/*
 * Function: get_line_numbers
 * --------------------------
 * Description: parse the whole novel and search for occurrences of names.
 * Parameters: charList: a struct storing the content of the file;
 *             nameNum: the number of names in the list.
 * Return: N/A.
 */
void get_line_numbers(struct character ***charList, int nameNum)
{
	FILE *fp = fopen(INPUT_FILE, INPUT_MODE);
	if ( fp == NULL )
	{
		perror(INPUT_FILE);
		exit(EXIT_FAILURE);
	}
	size_t num;
	int line;
	char userinput[BUFFER_SIZE], *strPtr;
	struct line *prev, *curr;
	for ( num = 0; num < nameNum; num++ )
	// Parse the whole file (nameNum) times.
	{
		(*charList)[num] -> lineList = NULL; // Initialise the head of the list.
		line = 0;
		// Every time parse from the first line of file.
		while ( fgets(userinput, BUFFER_SIZE, fp) != NULL )
		{
			line++;
			strPtr = userinput;
			// Point to the beginning of userinput.
			while ( (strPtr = strstr(strPtr, (*charList)[num] -> name)) )
			// N.B.: "=" instead of "==".
			{
				strPtr += strlen((*charList)[num] -> name);
				// If the same name occurs multiple times in a line.
				if ( (*charList)[num] -> lineList == NULL )
				// If it is the first node in the list.
				{
					(*charList)[num] -> lineList = add_to_line_list(line);
					prev = (*charList)[num] -> lineList;
				}
				else
				{
					curr = add_to_line_list(line);
					prev -> next = curr;
					prev = curr;
				}
			}
		}
		if ( ferror(fp) )
		{
			perror(INPUT_FILE);
			// In this case, if an error occurs, the program will keep running.
		}
		rewind(fp);
		/*
		 * EOF and error internal indicators of fp are cleared.
		 * The statement is equivalent to `(void) fseek(fp, 0L, SEEK_SET);`, 
		 * but does not check if the operation succeeded.
		 */
	}
	fclose(fp);
}

/*
 * Function: add_to_line_list
 * --------------------------
 * Description: add line number to lists for each name.
 * Parameter: lineNo: line number.
 * Return: cp: a pointer to newly allocated memory.
 */
struct line *add_to_line_list(int lineNo)
{
	struct line *structPtr = malloc(sizeof(struct line));
	if ( structPtr == NULL )
	{
		perror("structPtr");
		exit(EXIT_FAILURE);
	}
	structPtr -> lineNo = lineNo;
	structPtr -> next = NULL;
	return structPtr;
}

/*
 * Function: analyse_and_output
 * ----------------------------
 * Description: analyse lists for each name and output the result into the designated file.
 * Parameters: charList: a struct storing the content of the file.
 *             nameNum: the number of names in the list.
 * Return: N/A.
 */
void analyse_and_output(struct character ***charList, int nameNum)
{
	FILE *fp = fopen(OUTPUT_FILE, OUTPUT_MODE);
	if ( fp == NULL )
	{
		perror(OUTPUT_FILE);
		exit(EXIT_FAILURE);
	}
	size_t i, j;
	struct line *first, *second, *secondFirstPair;
	_Bool firstPairForFirst;
	// The flag variable is used to check if current pair is the first one for current node in first.
	// A variant of bubble sorting.
	for ( i = 0; i < nameNum - 1; i++ )
	{
		if ( (*charList)[i] -> lineList == NULL )
		{
			continue;
			// End current for-loop (i) and make first point to next name.
		}
		for ( j = i + 1; j < nameNum; j++ )
		{
			secondFirstPair = (*charList)[j] -> lineList;
			/*
			 * secondFirstPair points to the position of the first node in second
			 * which counts as a co-occurrence with previous node in first.
			 * It is initialised by the first node in second.
			 */
			if ( secondFirstPair == NULL )
			{
				continue;
				// End current for-loop (j) and make second point to next name.
			}
			first = (*charList)[i] -> lineList;
			while ( first )
			// If first does not reach the end, continue the loop.
			{
				second = secondFirstPair;
				firstPairForFirst = false;
				// Before scanning second for next node in first, variables have to be initialised.
				while ( second && (first -> lineNo + CO_OCCURRENCE > second -> lineNo) )
				/*
				 * If second does not reach the end or line numbers of second are
				 * still possibly within five lines, continue the loop.
				 */
				{
					if ( abs((first -> lineNo) - (second -> lineNo)) < CO_OCCURRENCE )
					// Find the required pair.
					{
						fprintf(fp, CSV_FORMAT, (*charList)[i] -> name, (*charList)[j] -> name);
						if ( firstPairForFirst == false )
						// If it is the first pair for current node in first.
						{
							firstPairForFirst = true;
							secondFirstPair = second;
						}
					}
					second = second -> next;
				}
				first = first -> next;
			}
		}
	}
	fclose(fp);
}
