/**
 * @harishan_assignment3
 * @author  Harishankar Vishwanathan <harishan@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>

#include "../include/global.h"
#include "../include/logger.h"

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log();

	/*Clear LOGFILE and DUMPFILE*/
	fclose(fopen(LOGFILE, "w"));
	fclose(fopen(DUMPFILE, "wb"));

	/*Start Here*/

	if (argc!=5)
  	{
  		printf("./assignment3 ­-t <path­to­topology­file> -­i <routing­update­interval>\n");
  		exit(1);
  	}

	if (!strcmp(argv[1], "-i"))
	{
		INTERVAL = atoi(argv[2]);
		if (INTERVAL == 0)
		{
			printf("Please enter a valid input\n");
			exit(1);
		}
	}

	else if (!strcmp(argv[3], "-i"))
	{
		INTERVAL = atoi(argv[4]);
		if (INTERVAL == 0)
		{
			printf("Please enter a valid input\n");
			exit(1);
		}
	}

	if (!strcmp(argv[1], "-t"))
	{
		fileopen(argv[2]);
		server();
	}	

	else if (!strcmp(argv[3], "-t"))
	{
		fileopen(argv[4]);
		server();
	}	

	else 
	{
		printf("Please enter a valid input\n");
	}

	return 0;
}
