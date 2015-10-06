/*
 *  Created on: Mar 2, 2014
 *      Author: '(Yongzhen R.)
 *     Version: v1.0-0426
 * Description: Implement a command line version of a film guessing game.
 *              Before the guess, a masked film title chosen randomly from
 *              an existent file called "filmname.txt" is printed on the screen.
 *              And user can guess letters in the character mode or input
 *              the complete title in the guess mode with 5 chances.
 *              After a single game, user can decide to begin the next turn or
 *              exit the game by inputting "Y/y" or "N/n" respectively.
 */

#include <stdio.h>      /* fgets, sscanf, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <string.h>     /* strcmp, strcpy, strchr */
#include <ctype.h>      /* isalpha, islower, toupper */
#include <stdbool.h>    /* macro: true, false */

#define MAX_GUESSES 5
// For each turn of the game, user has only at most 5 chances.
#define MAX_TITLES 100
// The maximum number of film titles.
#define MASK '*'
// Asterisks mask the film title outputted on the screen.

#define YES_U 'Y'
#define YES_L 'y'
#define NO_U 'N'
#define NO_L 'n'
// Continue the game or exit according to the letter that user inputs.
#define C_U 'C'
#define C_L 'c'
#define F_U 'F'
#define F_L 'f'
// Guess an individual character or the whole film title.
#define CHAR_MODE 0
#define GUESS_MODE 1
#define TERMINATE_MODE -1
// Three different states of the game.

#define FULL_SCREEN
#ifdef FULL_SCREEN
	#define WINDOW_HEIGHT 43
#else
	#define WINDOW_HEIGHT 24
#endif
// Assign the value of WINDOW_HEIGHT used in clear_screen_and_print_welcome() function.
#define LINE_LENGTH 256
// LINE_LENGTH controls the length of each line when the program reads the file.
#define PATH "filmtitles.txt"
#define FILE_MODE "r"
#define MS_NEWLINE "\r\n"
// Microsoft Windows compatibility mode.

struct film_string
{
	char name[LINE_LENGTH];
	/*
	 * name is a character array containing the original name of a file title
	 * used to print on the screen.
	 */
	char upper_name[LINE_LENGTH];
	/*
	 * upper_name is a character array containing the upper case name of a file title
	 * used to be compared with user's input.
	 */
	_Bool alpha[26];
	/*
	 * alpha is a Boolean array with 26 elements
	 * and it stores the output state of every letter.
	 * For example, if letter 'A' is inputed by user and the film title contains
	 * 'A' or 'a', the corresponding slot (alpha[0]) will be marked as true.
	 */
};

// Function declarations.
void clear_screen_and_print_welcome(void);
int read_file(char (*film_library)[LINE_LENGTH]);
struct film_string random_select(char (*film_library)[LINE_LENGTH], int num);
void lower_to_upper(char *string);
void mask_and_print(const struct film_string title);
int get_option(void);
struct film_string char_mode(struct film_string title);
int guess_mode(int guess, const char *upper_name);
void handle_newline(char *keyboard_input, const char *file_input);
_Bool continue_game(void);

int main(void)
{
	struct film_string film_title;
	int game_state, guess_times, total_titles_num = 0;
	char film_library[MAX_TITLES][LINE_LENGTH];
	// film_library is a character storing lines from the file.

	clear_screen_and_print_welcome();
	total_titles_num = read_file(film_library);
	do
	{
		guess_times = 0;
		// Before each turn of the game, guess_times has to be initialised.
		film_title = random_select(film_library, total_titles_num);
		do
		{
			mask_and_print(film_title);
			game_state = get_option();
			if ( game_state == CHAR_MODE )
			{
				film_title = char_mode(film_title);
			}
			if ( game_state == GUESS_MODE )
			{
				guess_times++;
				// Once user enters the guess mode, times of guesses will add 1.
				game_state = guess_mode(guess_times, film_title.upper_name);
			}
		}
		while ( game_state != TERMINATE_MODE );
	}
	while ( continue_game() == true );
	return 0;
}

/*
 * Function: clear_screen_and_print_welcome
 * ----------------------------------------
 * Description: print out blank lines and welcome information.
 * Parameter: N/A.
 * Return: N/A.
 */
void clear_screen_and_print_welcome(void)
{
	int i;
	for ( i = 0; i < WINDOW_HEIGHT; i++ )
	{
		putchar('\n');
	} // Print out WINDOWS_HEIGHT blank lines.
	printf("**************** Welcome to Film Genie ****************\n");
}

/*
 * Function: read_file
 * -------------------
 * Description: read film titles from a designated file.
 * Parameter: film_library: an array containing containing lines in the file.
 * Return: num: the number of lines read from the file.
 */
int read_file(char (*film_library)[LINE_LENGTH])
{
	FILE *fp;
	int num = 0;
	fp = fopen(PATH, FILE_MODE);
	if ( fp == NULL )
	{
		perror(PATH); // Print out the error message.
		exit(EXIT_FAILURE); // Terminate the program.
	}
	else
	{
		while ( (num < MAX_TITLES) && (fgets(film_library[num], LINE_LENGTH, fp) != NULL) )
		/*
		 * Read lines from lines until num is greater than MAX_TITLES or
		 * the program reaches EOF. The order of two expressions is unchangeable.
		 */
		{
			num++;
		}
	}
	printf("We have %d films.\n\n", num);
	fclose(fp);
	return num;
}

/*
 * Function: random_select
 * -----------------------
 * Description: select one film title from film_library.
 * Parameters: film_library: an array containing containing lines in the file;
 *             num: the number of lines in the array.
 * Return: selected: a selected film title.
 */
struct film_string random_select(char (*film_library)[LINE_LENGTH], int num)
{
	int i;
	struct film_string selected = { .alpha[0] = false };
	srand(time(NULL));
	// Seed the random number generator from system clock.
	i = rand() % num;
	// Generate random numbers ranging from 0 to (num-1).
	strcpy(selected.name, film_library[i]);
	// Copy (i+1)th item to the variable selected.name.
	strcpy(selected.upper_name, selected.name);
	lower_to_upper(selected.upper_name);
	// Make all letters in upper_name become upper case ones.
	return selected;
}

/*
 * Function: lower_to_upper
 * ----------------------
 * Description: convert lower case letters to upper case ones in the string.
 * Parameter: string: a string ending with a null character ('\0').
 * Return: N/A.
 */
void lower_to_upper(char *string)
{
	int i = 0;
	while ( string[i] != '\0' )
	{
		if ( islower(string[i]) )
		// If the current letter is lower case.
		{
			string[i] = toupper(string[i]);
		}
		i++;
	}
}

/*
 * Function: mask_and_print
 * ------------------------
 * Description: print out the masked film title.
 * Parameter: title: selected film title.
 * Return: N/A.
 */
void mask_and_print(const struct film_string title)
{
	int i = 0;
	printf("Your film title to guess:\n");
	while ( title.name[i] != '\0' )
	{
		if ( isalpha(title.name[i]) && (title.alpha[title.upper_name[i] - 'A'] == false) )
		// When the character is a letter which never is inputted by user in the previous game.
		{
			putchar(MASK);
			// Mask the letter by an asterisk.
		}
		else
		{
			putchar(title.name[i]);
			// Print the character in a normal way.
		}
		i++;
	}
}

/*
 * Function: get_option
 * --------------------
 * Description: decide what to do next depending on the option from keyboard.
 * Parameter: N/A.
 * Return: flag: the indicator of the game state.
 */
int get_option(void)
{
	char user_input[LINE_LENGTH], option;
	int flag;
	while ( 1 ) // It is an infinite loop.
	{
		printf("\nWould you like to guess a character (enter 'c') OR guess the film (enter 'f'):\n");
		fgets(user_input, LINE_LENGTH, stdin);
		sscanf(user_input, "%c", &option);
		if ( (option == C_U) || (option == C_L) )
		// If user inputs 'C' or 'c'.
		{
			flag = CHAR_MODE;
			break;
		}
		else
		{
			if ( (option == F_U) || (option == F_L) )
			// If user inputs 'F' or 'f'.
			{
				flag = GUESS_MODE;
				break;
			}
			else
			{
				printf("Invalid input. Please try again.\n");
			}
		}
	}
	return flag;
}

/*
 * Function: char_mode
 * -------------------
 * Description: get the character user wants to guess.
 * Parameter: title: selected film title.
 * Return: title: updated film title.
 */
struct film_string char_mode(struct film_string title)
{
	char user_input[LINE_LENGTH], letter;
	printf("Please enter a character: ");
	fgets(user_input, LINE_LENGTH, stdin);
	sscanf(user_input, "%c", &letter);
	if ( islower(letter) )
	// Convert the letter to capital one.
	{
		letter = toupper(letter);
	}
	if ( strchr(title.upper_name, letter) == NULL )
	/*
	 * The strchr() function returns a pointer to the first occurrence of letter
	 * in the upper_name, or NULL if letter is not found.
	 */
	{
		printf("Your character doesn’t exist! Please continue playing.\n");
	}
	else
	{
		printf("Your character exists! Well done. Please continue playing.\n");
		title.alpha[letter - 'A'] = true;
		// Update the state of the letter entered by user.
	}
	return title;
}

/*
 * Function: guess_mode
 * --------------------
 * Description: get the final answer that user inputs.
 * Parameters: guess: the number of current guesses;
 *             upper_name: a string with all capital letters.
 * Return: flag: the indicator of the game state.
 */
int guess_mode(int guess, const char *upper_name)
{
	char guess_input[LINE_LENGTH];
	int flag;
	if ( guess > MAX_GUESSES )
	// Given guesses run out.
	{
		printf("Sorry! You have no more guesses!!!!\n");
		flag = TERMINATE_MODE;
		return flag;
	}
	printf("Please your guess: ");
	fgets(guess_input, LINE_LENGTH, stdin);
	handle_newline(guess_input, upper_name);
	// Cope with possible newline problem.
	lower_to_upper(guess_input);
	if ( strcmp(guess_input, upper_name) == 0 )
	// If two strings are identical.
	{
		switch ( guess )
		{
			case 1:
			// If user gets the answer in just one guess.
				printf("Woohoo! You are a genius! You got it in %d guess.\n", guess);
				break;
			case MAX_GUESSES:
			// If user get the answer in MAX_GUESSES guesses.
				printf("Phew! You got it on your last guess.\n");
				break;
			default:
				printf("You are a true film genie! You got it in %d guesses.\n", guess);
				break;
		}
		flag = TERMINATE_MODE;
		// When user get the answer, the current turn of the game will end.
	}
	else
	// If user does not get it.
	{
		printf("Incorrect guess. Please continue playing.\n");
		flag = GUESS_MODE;
	}
	return flag;
}

/*
 * Function: handle_newline
 * ------------------------
 * Description: deal with newline problem between Windows and Unix-like systems.
 * Parameters: keyboard_input: character string from keyboard;
 *             file_input: character string from the file.
 * Return: N/A.
 */
void handle_newline(char *keyboard_input, const char *file_input)
{
	char *pointer = NULL;
	if ( (strstr(file_input, MS_NEWLINE) != NULL) && (strstr(keyboard_input, MS_NEWLINE) == NULL) )
	// If the file is created in Microsoft Windows and user's OS is Unix-like.
	{
		pointer = strstr(keyboard_input, "\n");
		// The pointer points to the position of "\n".
		sprintf(pointer, "\r\n");
		// Cover "\n\0" with "\r\n\0".
	}
	if ( (strstr(file_input, MS_NEWLINE) == NULL) && (strstr(keyboard_input, MS_NEWLINE) != NULL) )
	// If the file is created in Unix-like OS and user's OS is Microsoft Windows.
	{
		pointer = strstr(keyboard_input, "\r\n");
		// The pointer points to the position of "\r\n".
		sprintf(pointer, "\n");
		// Cover "\r\n\0" with "\n\0".
	}
}

/*
 * Function: continue_game
 * -----------------------
 * Description: check whether user wants to continue the game.
 * Parameter: N/A.
 * Return: play_or_not: an indicator whether the game goes on or not.
 */
_Bool continue_game(void)
{
	_Bool play_or_not;
	// play_or_not only has two possible values: true (1) and false (0).
	char user_input[LINE_LENGTH], option;
	while ( 1 ) // It is an infinite loop.
	{
		printf("Would you like to play again? Yes (enter 'y') OR No (enter 'n'): ");
		fgets(user_input, LINE_LENGTH, stdin);
		sscanf(user_input, "%c", &option);
		if ( (option == YES_U) || (option == YES_L) )
		// Continue the game.
		{
			play_or_not = true;
			break;
		}
		else
		{
			if ( (option == NO_U) || (option == NO_L) )
			// Exit the game.
			{
				printf("Game is Over. GoodBye!\n");
				play_or_not = false;
				break;
			}
			else
			{
				printf("Invalid input. Please try again.\n");
			}
		}
	}
	return play_or_not;
}
