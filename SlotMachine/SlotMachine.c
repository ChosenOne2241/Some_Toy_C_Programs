/*
 *  Created on: Feb 10, 2014
 *      Author: '(Yongzhen R.)
 *     Version: v1.0-0213
 * Description: Implement a command line version of a slot machine.
 *              User can input their bet (MIN_BET <= bet <= available credits
 *              [at first, INIT_CREDIT]) after a welcome title is printed out.
 *              The output of the slot machine is random and user can win or
 *              lose credits according to the state of three faces.
 *              After a single game, user can decide to begin the next turn or
 *              exit the game by inputting "Y/y" or "N/n" respectively.
 */

#include <stdio.h>      /* fgets, sscanf, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#define INIT_CREDIT 10
// The initial credits assigned by system.
#define MIN_BET 2
// The minimum credits that user can use to bet.

#define APPLE 1
#define ORANGE 2
#define PEAR 3
// Use three numbers to represent different faces on the column.
#define APPLE_STR "APPLE"
#define ORANGE_STR "ORANGE"
#define PEAR_STR "PEAR"
// Face contents.
#define ERROR_STR "ERROR"
// ERROR_STR will be showed instead of normal faces only if output error happens.

#define YES_U 'Y'
#define YES_L 'y'
#define NO_U 'N'
#define NO_L 'n'
// Control the process of the game according to the letter that user inputs.

// #define FULL_SCREEN
#ifdef FULL_SCREEN
	#define WINDOW_HEIGHT 43
#else
	#define WINDOW_HEIGHT 24
#endif
// Assign the value of WINDOW_HEIGHT used in clear_screen_and_print_welcome() function.
#define LINE_LENGTH 80
// The standard length of a line for a terminal, used to limit user's input.

struct column
{
	int face;
};
/*
 * Technically, we do not need this structure because it just contains one entry.
 * But considering possible future expansion of functions, it becomes necessary.
 */

struct slot
{
	struct column col_1;
	struct column col_2;
	struct column col_3;
};
// Each slot machine has three columns.

void clear_screen_and_print_welcome(void);
int get_bet(int credit);
struct slot pull_handle(void);
void display_faces(struct slot s);
char *print_column_face(struct column c);
int calculate_reward(struct slot s, int bet);
void continue_or_exit(int credit);
// Function declarations.

int main(void)
{
	struct slot game_slot;
	int game_credit = INIT_CREDIT, game_bet;
	// All variables in main() function begin with "game_".
	clear_screen_and_print_welcome();
	while ( 1 ) // It is an infinite loop.
	{
		game_bet = get_bet(game_credit);
		game_slot = pull_handle();
		display_faces(game_slot);
		game_credit += calculate_reward(game_slot, game_bet);
		// Add reward to the credits that user owns.
		continue_or_exit(game_credit);
	}
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
	printf("**************** Welcome to My Slot Machine ****************\n");
}

/*
 * Function: get_bet
 * -----------------
 * Description: get user's bet from keyboard.
 * Parameter: credit: user's current credits.
 * Return: bet: the number of user's bet.
 */
int get_bet(int credit)
{
	int bet = 0;
	char user_input[LINE_LENGTH];
	printf("Your available credit is %d\n", credit);
	do
	{
		printf("Please input how much you wish to bet: ");
		fgets(user_input, LINE_LENGTH, stdin);
		sscanf(user_input, "%d", &bet);
		if ( (bet >= MIN_BET) && (bet <= credit) )
		{
			break;
			// That means that bet gets a valid input and jump out of the loop.
		}
		else
		{
			printf("Invalid bet: must be >= %d credits or <= %d credits.\n", \
					MIN_BET, credit);
			// That means that it is invalid and continue the loop.
		}
	}
	while ( 1 ); // It is an infinite loop.
	return bet;
}

/*
 * Function: pull_handle
 * ---------------------
 * Description: generate states of three faces randomly.
 * Parameter: N/A.
 * Return: s: states of three faces.
 */
struct slot pull_handle(void)
{
	struct slot s;
	srand(time(NULL));
	// Seed the random number generator from system clock.
	s.col_1.face = 1 + (rand() % 3);
	s.col_2.face = 1 + (rand() % 3);
	s.col_3.face = 1 + (rand() % 3);
	// Generate random numbers ranging from 1 to 3.
	return s;
}

/*
 * Function: display_faces
 * -----------------------
 * Description: display different faces according to different numbers.
 * Parameter: s: states of three faces.
 * Return: N/A.
 */
void display_faces(struct slot s)
{
	printf("Your selection: |%s| |%s| |%s|\n", print_column_face(s.col_1), \
print_column_face(s.col_2), print_column_face(s.col_3));
}

/*
 * Function: print_column_face
 * ---------------------------
 * Description: return different strings according to different numbers.
 * Parameter: c: states of three faces.
 * Return: one of constant strings (APPLE_STR, ORANGE_STR, or PEAR_STR).
 */
char *print_column_face(struct column c)
{
	switch ( c.face )
	{
		case APPLE:
			return APPLE_STR;
			break;
		case ORANGE:
			return ORANGE_STR;
			break;
		case PEAR:
			return PEAR_STR;
			break;
		default:
			return ERROR_STR;
			// When the error happens, ERROR_STR will be showed instead of normal faces.
			break;
			// All break statements will never be run.
	}
	return ERROR_STR;
	// When the error happens, ERROR_STR will be showed instead of normal faces.
}

/*
 * Function: calculate_reward
 * --------------------------
 * Description: calculate the reward depending on the value of variable s.
 * Parameters: s: states of three faces;
 *             bet: the number of user's bet.
 * Return: reward: the money that user wins or loses
 *                 (full house, half house or empty house).
 */
int calculate_reward(struct slot s, int bet)
{
	int reward;
	if ( (s.col_1.face == s.col_2.face) && (s.col_2.face == s.col_3.face) )
	// All faces are the same: full house.
	{
		printf("Full house - You won %d credits.\n", bet);
		reward = bet;
	}
	else
	{
		if ( (s.col_1.face != s.col_2.face) && (s.col_2.face != s.col_3.face) \
				&& (s.col_1.face != s.col_3.face) )
			// Faces are not the same to each other: empty house.
		{
			printf("Empty house - You lost %d credits.\n", bet);
			reward = -(bet);
		}
		else
		// Rest of permutations is half house.
		{
			reward = bet / 2;
			printf("Half house - You won %d credits.\n", reward);
		}
	}
	return reward;
}

/*
 * Function: continue_or_exit
 * --------------------------
 * Description: continue or exit the program by reading user's input from keyboard.
 * Parameter: credit: user's current credits.
 * Return: N/A.
 */
void continue_or_exit(int credit)
{
	char user_input[LINE_LENGTH], yn;
	do
	{
		printf("Play again? ('Y/N'): ");
		fgets(user_input, LINE_LENGTH, stdin);
		sscanf(user_input, "%c", &yn);
		if ( (yn == NO_L) || (yn == NO_U) )
		// User input 'N' or 'n'. (User does not want to play any more.)
		{
			if ( credit < INIT_CREDIT )
			// User get less credits now than at beginning before the program exits.
			{
				printf("***** End of Game: total amount LOST %d credits *******\n", \
						INIT_CREDIT - credit);
			}
			else
			// User get more credits now than at beginning before the program exits.
			{
				printf("***** End of Game: total amount WON %d credits *******\n", \
						credit - INIT_CREDIT);
			}
			exit(EXIT_SUCCESS); // Terminate the whole program.
		}
		else
		{
			if ( (yn == YES_L) || (yn == YES_U) )
			// User input 'Y' or 'y'. (User wants to play.)
			{
				if ( credit < MIN_BET )
				/* The program is forced to exit,
				 * for user does not have enough credits to bet.
				 */
				{
					printf("Sorry, but you don't have enough credits - Bye!!\n");
					exit(EXIT_SUCCESS); // Terminate the whole program.
				}
				else
				{
					break; // Jump out of the loop and continue the game.
				}
			}
			else
			// Otherwise, go on the loop until get the valid input.
			{
				printf("Incorrect Input (must be 'Y/N').\n");
			}
		}
	}
	while ( 1 ); // It is an infinite loop.
}
