/*
 *  Created on: Apr 1, 2014
 *      Author: Yongzhen Ren & Padraig Cunningham
 *     Version: v3.0-0427
 * Description: Implement a command line version of analysing GPS data in
 *              gpx format to extract summary statistics from GPS track information.
 *              The program will read latitude, longitude, elevation and time of every point
 *              and produce general statistics during the whole track.
 *              Also, a form of splits (1 km) will be created.
 *              [Use built-in mktime() instead of self-made timeDiff().]
 */

#define _XOPEN_SOURCE
// Define it in order to use M_PI in math.h and strptime().
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define D2R (M_PI / 180.0)

// Location of GPX file.
//#define GPX_FILE_PATH "./inputFiles/Howth-Cross.gpx"
#define GPX_FILE_PATH "./inputFiles/Run4.9k.gpx"
//#define GPX_FILE_PATH "./inputFiles/Zell75k.gpx"
//#define GPX_FILE_PATH "./inputFiles/Test1.gpx"
//#define GPX_FILE_PATH "./inputFiles/Test2.gpx"
//#define GPX_FILE_PATH "./inputFiles/Test3.gpx"

// The node structure stores paths.
struct node
{
	double lat;
	double lon;
	double ele;
	char timeString[22];
	struct node *next;
};

struct split
{
	int splitNo;
	long int pace;
	/*
	 * time_t, equivalent to long int in gcc,
	 * is not used in the program considering software portability.
	 */
	double speed;
	double elevDiff;
	struct split *next;
};

struct node *head = NULL;
struct node *curr = NULL;
struct split *headSplit = NULL;
struct split *currSplit = NULL;

// Function declaration.
void open_file_and_load_data(void);
double read_double_after_token(char *txtStr, char *tkn, char **endPtr);
char *read_string_after_token(char *txtStr, char *tkn, char *res, int len);
void add_to_list(double lat, double lon, double ele, char *timeStr);
void create_list(double lat, double lon, double ele, char *timeStr);
void calculate_tot_dist(void);
double haversine_m(double lat1, double lon1, double lat2, double lon2);
void add_to_splits_list(int splitNo, long int pace, double speed, double elevDiff);
void create_splits(int splitNo, long int pace, double speed, double elevDiff);
char *sec_to_clock_time(long int sec);

int main(void)
{
	open_file_and_load_data();
	// Function that is called once at the start to read in the character names.
	calculate_tot_dist();
	return 0;
}

/*
 * Function: open_file_and_load_data
 * ---------------------------------
 * Description: open the designated file and parse the whole file.
 * Parameters: N/A.
 * Return: N/A.
 */
void open_file_and_load_data(void)
{
	int lineNum = 0;
	char theLine[120];
	char *endOfHdr = "<trkseg>", *endOfData = "</trkseg>", *currPosition;
	char tempTimeStr[30];
	double lat, lon, ele;
	FILE *fpn = fopen(GPX_FILE_PATH, "r"); // Open for reading.
	if ( fpn == NULL ) // Check does file exist etc.
	{
		perror(GPX_FILE_PATH);
		exit(EXIT_FAILURE);
	}
	else
	{
		// First work through the header in the GPX file.
		// Assume header finishes at "</trkseg>" stored in endOfHdr.
		while ( fgets(theLine, sizeof theLine, fpn)
				&& ((strncmp(theLine, endOfHdr, strlen(endOfHdr)) != 0)) )
		{
			// Skip through the file until "<trkseg>" is reached (endOfHdr).
			lineNum++;
		}
		// Read the text data and store the lat, lon and time data in a linked list.
		while ( fgets(theLine, sizeof theLine, fpn)
				&& ((strncmp(theLine, endOfData, strlen(endOfData)) != 0)) )
		// Finish when "</trkseg>" reached (endOfData).
		{
			lat = read_double_after_token(theLine, "lat=\"", &currPosition);
			lon = read_double_after_token(currPosition, "lon=\"", &currPosition);
			ele = read_double_after_token(currPosition, "<ele>", &currPosition);
			// Speed up the whole process of reading data by shortening the string every time.
			add_to_list(lat, lon, ele, read_string_after_token(currPosition, "<time>", tempTimeStr, 20));
 			// Date and time in are in Univeral Coordinated Time (UTC), not local time.
			lineNum++;
		}
	}
	fclose(fpn);
}

/*
 * Function: read_string_after_token
 * ---------------------------------
 * Description: This function will return a double from string txtstr.
 *              The string will be searched for the first occurrence of tkn and
 *				the reading of the double will start steps places after the start of tkn.
 * 				If the token is not found in the string it returns a value of -1.0.
 * 				If there is no number after the token it returns 0.0.
 * Parameters: txtStr: the string which will be parsed;
 *             tkn: the token which will be found;
 *             endPtr: return an address of the next character in txtStr after the numerical value.
 * Return: res: the number read from txtStr.
 */
double read_double_after_token(char *txtStr, char *tkn, char **endPtr)
{
	int steps = strlen(tkn);
	double res = -1.0;
	char *tmpStr = strstr(txtStr, tkn);
	if ( tmpStr )
	// Check to make sure the pointer is not NULL, i.e. strstr returned something.
	{
		res = strtod(tmpStr + steps, endPtr);
	}
	return res;
}

/*
 * Function: read_string_after_token
 * ---------------------------------
 * Description: This function will return a  substring from string txtstr.
 * 				The string will be searched for the first occurrence of tkn
 * 				and then a substring on length len will be returned starting steps
 * 				spaces from the start of tkn.
 * 				If the token is not found in the string it returns a null pointer.
 * Parameters: txtStr: the string which will be parsed;
 *             tkn: the token which will be found;
 *             res: the temp string used to store the content;
 *             len: the length of string which needs to be copied;
 * Return: ret: the address of object string.
 */
char *read_string_after_token(char *txtStr, char *tkn, char *res, int len)
// Target string, token, length of substring and steps beyond start of token.
{
	int steps = strlen(tkn);
	char *ret, *tmpStr = strstr(txtStr, tkn);
	if ( tmpStr )
	{
		// Checking to make sure the pointer is not NULL, i.e. strstr returned something.
		ret = strncpy(res, tmpStr+steps, len);
		*(ret+len) = '\0';
		// Add null character at the end of the string explicitly.
	}
	return ret;
}

/*
 * Function: add_to_list
 * ---------------------
 * Description: add nodes to the main data list.
 * Parameters: lat: the latitude;
 *             lon: the longitude;
 *             ele: the elevation;
 *             timeStr: the time string.
 * Return: N/A.
 */
void add_to_list(double lat, double lon, double ele, char *timeStr)
{
	if ( NULL == head )
	// Yoda expression. "Nice" walkaround.
	{
		create_list(lat, lon, ele, timeStr);
		return; // Terminate the current function.
	}
	struct node *ptr = malloc(sizeof(struct node));
	if ( NULL == ptr )
	{
		perror("Node creation failed");
		exit(EXIT_FAILURE);
	}
	ptr -> lat = lat;
	ptr -> lon = lon;
	ptr -> ele = ele;
	strcpy(ptr -> timeString, timeStr);
	ptr -> next = NULL;
	curr -> next = ptr;
	curr = ptr;
}

/*
 * Function: create_list
 * ---------------------
 * Description: create the list to be used to store the data.
 * Parameters: lat: the latitude;
 *             lon: the longitude;
 *             ele: the elevation;
 *             timeStr: the time string.
 * Return: N/A.
 */
void create_list(double lat, double lon, double ele, char *timeStr)
{
	struct node *ptr = malloc(sizeof(struct node));
	if ( NULL == ptr )
	{
		perror("Node creation failed");
		exit(EXIT_FAILURE);
	}
	ptr -> lat = lat;
	ptr -> lon = lon;
	ptr -> ele = ele;
	strcpy(ptr -> timeString, timeStr);
	ptr -> next = NULL;
	head = curr = ptr;
}

/*
 * Function: calculate_tot_dist
 * ----------------------------
 * Description: calculate the total length of the track and print out the statistics.
 * Parameter: N/A.
 * Return: N/A.
 * Bug: mktime function only accepts local time as argument instead of UTC and
 *      it may cause potential problems (time calculated by mktime is smaller
 *      than (3600 seconds) what user expects due to daylight savings switch and time zone.
 *      Without considering portability, replacing mktime() with timegm() is a better option.
 *      See also: https://sourceware.org/bugzilla/show_bug.cgi?id=4033
 */
void calculate_tot_dist(void)
{
	enum { FIRST, NOTFIRST } firstNodeFlag = FIRST;
	double distBetwPoints;

	// Variables used in "Overall Statistics".
	double pathLen = 0.0, latPrev, lonPrev, averagePace;
	long int elapsedTime;
	struct tm startTime, finishTime;
	// Hold the start and end time.
	struct node *ptr = head;

	// Variables used in "Splits Statistics".
	int splitNo = 0;
	long int averagePaceSplit;
	double splitLen = 0.0, startElevationSplit, finishElevationSplit;
	struct tm startTimeSplit, finishTimeSplit;
	// Hold the start and end time of each split.
	struct split *ptrSplit;

    while ( ptr != NULL )
	{
		if ( firstNodeFlag == FIRST )
    	// First node.
		{
			strptime(ptr -> timeString, "%Y-%m-%dT%TZ", &startTime);
			/*
			 * There is no strptime function in ISO C (just in POSIX).
			 * strptime is a function to populate a tm time structure from an GPX time string.
			 * e.g. "2013-09-12T15:59:18Z"
			 */
			startTimeSplit = startTime;
			startElevationSplit = ptr -> ele;
			firstNodeFlag = NOTFIRST;
    	}
		else
    	{
			distBetwPoints = haversine_m(latPrev, lonPrev, ptr -> lat, ptr -> lon);
			pathLen += distBetwPoints;
			splitLen += distBetwPoints;
			if ( (splitLen >= 1000.0) || (ptr -> next == NULL) )
			/*
			 * Create a new split when splitLen reached 1000.0 or
			 * the current point is the last one in the linked list.
			 */
			{
				splitNo++;
				strptime(ptr -> timeString, "%Y-%m-%dT%TZ", &finishTimeSplit);
				// %T: Equivalent to %H:%M:%S.
				averagePaceSplit = (long int) difftime(mktime(&finishTimeSplit), mktime(&startTimeSplit));
				// Return the time difference in seconds between two tm time structures.
				finishElevationSplit = ptr -> ele;
				add_to_splits_list(splitNo,
				                   averagePaceSplit,
				                   splitLen * 3.6 / (double) averagePaceSplit,
				                   // (splitLen / 1000.0) / ((double) averagePaceSplit / 3600.0),
				                   finishElevationSplit - startElevationSplit);
				splitLen = 0.0; // Clear the variable and begin a new split.
				startElevationSplit = finishElevationSplit;
				startTimeSplit = finishTimeSplit;
			}
    	}
		// Update the location information and go to next node.
		latPrev = ptr -> lat;
		lonPrev = ptr -> lon;
		ptr = ptr -> next;
    }
	// Print results on the screen.
	printf("\n-------Overall Statistics-------\n");
    printf("Path Length: %5.0f m\n", pathLen);
	strptime(curr -> timeString, "%Y-%m-%dT%TZ", &finishTime);
	// The structure pointed by curr still store the data of the last node.
	elapsedTime = (long int) difftime(mktime(&finishTime), mktime(&startTime));
	printf("Elapsed Time: %ld sec\n", elapsedTime);
	averagePace = (double) elapsedTime * 50.0 / pathLen / 3.0;
	// averagePace = (double) elapsedTime / (pathLen / 1000.0) / 60.0;
	printf("Average Pace: %4.2f m/km\n", averagePace);
	printf("\n-------Splits Statistics-------\n");
	printf("--------------------------------------------------\n");
	printf(" Split No. | Pace m:s | Speed km/h | Elevation m\n");
	printf("--------------------------------------------------\n");
	ptrSplit = headSplit;
	while ( ptrSplit != NULL )
	{
		printf("%6d %12s %11.2f %11.0f\n", ptrSplit -> splitNo,
	                                       sec_to_clock_time(ptrSplit -> pace),
	                                       ptrSplit -> speed,
	                                       ptrSplit -> elevDiff);
		ptrSplit = ptrSplit -> next;
	}
	printf("--------------------------------------------------\n");
	printf("-------Splits Statistics End-------\n\n");
}

/*
 * Function: haversine_m
 * ---------------------
 * Description: calculate distance between two points expressed as lat and long.
 * Parameters: lat1: the latitude of point 1;
 *             lon1: the longitude of point 1;
 *             lat2: the latitude of point 2;
 *             lon2: the longitude of point 2.
 * Return: d: the distance between two points.
 */
double haversine_m(double lat1, double lon1, double lat2, double lon2)
{
	// Haversine formula.
	double dlong = (lon2 - lon1) * D2R;
	double dlat = (lat2 - lat1) * D2R;
	double a = pow(sin(dlat / 2.0), 2.0) + cos(lat1 * D2R) * cos(lat2 * D2R) * pow(sin(dlong / 2.0), 2.0);
	double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
	double d = 6367137.0 * c;
	return d;
}

/*
 * Function: create_splits
 * -----------------------
 * Description: add nodes to the main data list.
 * Parameters: splitNo: the number of the current split;
 *             pace: the duration in this split;
 *             speed: the average speed;
 *             eleDiff: the difference between current point and the last point in previous split.
 * Return: N/A.
 */
void add_to_splits_list(int splitNo, long int pace, double speed, double elevDiff)
{
	if ( NULL == headSplit )
	{
		create_splits(splitNo, pace, speed, elevDiff);
		return; // Terminate the current function.
	}
	struct split *splitPtr = malloc(sizeof(struct split));
	if ( NULL == splitPtr )
	{
		perror("Node creation failed");
		exit(EXIT_FAILURE);
	}
	splitPtr -> splitNo = splitNo;
	splitPtr -> pace = pace;
	splitPtr -> speed = speed;
	splitPtr -> elevDiff = elevDiff;
	splitPtr -> next = NULL;
	currSplit -> next = splitPtr;
	currSplit = splitPtr;
}

/*
 * Function: create_splits
 * -----------------------
 * Description: create the list to be used to store the data.
 * Parameters: splitNo: the number of the current split;
 *             pace: the duration in this split;
 *             speed: the average speed;
 *             eleDiff: the difference between current point and the last point in previous split.
 * Return: N/A.
 */
void create_splits(int splitNo, long int pace, double speed, double elevDiff)
{
	struct split *splitPtr = malloc(sizeof(struct split));
	if ( NULL == splitPtr )
	{
		perror("Node creation failed");
		exit(EXIT_FAILURE);
	}
	splitPtr -> splitNo = splitNo;
	splitPtr -> pace = pace;
	splitPtr -> speed = speed;
	splitPtr -> elevDiff = elevDiff;
	splitPtr -> next = NULL;
	headSplit = currSplit = splitPtr;
}

/*
 * Function: sec_to_clocktime
 * --------------------------
 * Description: convert seconds to readable clock time.
 * Parameter: sec: seconds which need to be converted.
 * Return: time: a string containing a readable clock time.
 */
char *sec_to_clock_time(long int sec)
{
	static char time[10] = { '\0' }; // Note that it is a static array.
	int minute, second;
	minute = (int) (sec / 60L);
	second = (int) (sec % 60L);
	sprintf(time, "%d:%02d", minute, second);
	// Output the formatted data to the string.
	return time;
}
