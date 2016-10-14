/*
Stephen Klein
UNIX shell with history commands
9/30/16
*/



#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAX_HISTORY 10
#define MAX_LINE 80

//This function prints out the last 10 history values that are stored in the history array
int historyOut(char **history, int histNum)
{
  int histMax = MAX_HISTORY + 1; //current size/amount of history
  int currLoc = (histNum - 1)%MAX_HISTORY;//current location in history
  int lineNum = histNum; //what line to print
  int i;//for looping

  if(histNum <=10)
  {
  	for(i = currLoc; i>=0; i--)
  	{
  		printf("%d %s \n", lineNum, history[currLoc]);
  		currLoc--;
  		lineNum--;
  	}
  }

  else
  {
  	int border = 8 - currLoc; //facilitates the wrap around in the array when the values are over 10
  	while(currLoc >= 0)
  	{
  		printf("%d %s \n", lineNum, history[currLoc]);
  		currLoc--;
  		lineNum--;
  	}
  	currLoc = 9;

  	while(border >= 0)
  	{
  		printf("%d %s \n", lineNum, history[currLoc]);
  		currLoc--;
  		lineNum--;
  		border--;
  	}
  }

  return histNum;
}

//This function re-runs the last entered arguments and then outputs the string so it can be readded to the 
//history list.
char* lastHistory(char **history, int histNum, int bg)
{
	int i;
	char **args;
	int index; // where the last entered command is stored
	index  = (histNum - 1)%10;
	args = malloc(MAX_LINE * sizeof(char *));//makes the array to fill with the old arguments
	for (i = 0; i < MAX_LINE; i++)//initializes all of the array's values to null
	{
 		args[i] = malloc(MAX_LINE * sizeof(char));
   		args[i][0] = '\0';
  	}
	printf("%d: %s\n", histNum, history[index]);

  	int w = 0; 
 	int count = 0;
  	i = 0;

  do 
  {
    for(count = 0; isalpha(history[index][i]);i++)//this section parses the chars into commands separated by
    {						  //a space
      args[w][count] = history[index][i];
	count++;
    }
    args[w++][count] = '\0';
    i++;
  } while (history[index][i] == ' ');
  args[w++] = NULL;
  printf("%s\n",args[0]);

  pid_t pid;		//child process creation
  pid = fork();

  if(pid < 0)//if the pid is less than zero it failed to fork the child
  {
	fprintf(stderr, "The Fork Failed");
	return 1;
  }
  if (pid == 0)//the child process executes the command from history
  {
    execvp(args[0], args);
  }

  else
  {
    if(bg == 1) //if bg = 1, it signals to wait instead of running in the background
	wait(0);
    bg = 0;
    pid = getpid();
    histNum = historyUpdate(history, args, histNum);	//adds the reentered last command to history
  }
	return args[0]; //returns the string to be added to history
}


//This function chooses the command corresponding to the number entered next to ! and runs it again
char* chooseFromHistory(char **history, int histNum, int historyIndexNo, int bg)
{
  int index = historyIndexNo%10; //the index of the command to be executed
  int i;
  char **args;
  args = malloc(MAX_LINE * sizeof(char *));
  for (i = 0; i < MAX_LINE; i++)//sets up array
  {
    args[i] = malloc(MAX_LINE * sizeof(char));
    args[i][0] = '\0';
  }
  printf("%d: %s\n", historyIndexNo+1, history[index]); //the values are indexed a value lower than their
							//actual value so history one is stored in bucket 0.

  int w = 0; 
  int count = 0;
  i = 0;

  do 
  {
    for(count = 0; isalpha(history[index][i]);i++)//parses string for execution
    {
      args[w][count] = history[index][i];
      count++;
    }
    args[w++][count] = '\0';
  } while (history[index][i++] == ' ');
  args[w++] = NULL;

  pid_t pid;
  pid = fork();

  if(pid < 0)//fork failed
  {
	fprintf(stderr, "The Fork Failed");
	return 1;
  }
  if (pid == 0)//child executes arguments
  {
    execvp(args[0], args);
  }

  else
  {
    if(bg == 1)
	wait(0);
    bg = 0;
    pid = getpid();
    histNum = historyUpdate(history, args, histNum);	
  }
  return args[0];
}

// Puts the string of the entered argument into the bucket corresponded to by index
int historyUpdate(char **history, char *enterredArgs, int histNum)
{
  int index = histNum++ % MAX_HISTORY;
  strcpy (history[index], enterredArgs);
  return histNum;
}

int main(int w, char *argv[])
{
  int runInBackground = 0; //always run in background unless '&' entered
	  char last[80];
	  char **args; 
	  char **history; 
	  args = malloc(MAX_LINE * sizeof(char *)); //allocating memory for the string arrays
	  history = malloc(MAX_HISTORY * sizeof(char *));
	  int i;
	  for (i = 0; i < MAX_LINE; i++)//fill args array with null
	  {
	    args[i] = malloc(MAX_LINE * sizeof(char));
	    args[i][0] = '\0';
	  }
	  for (i = 0; i < MAX_HISTORY; i++)//fill history array with null to initialize
	  {
	    history[i] = malloc(MAX_HISTORY * sizeof(char));
	    history[i][0] = '\0';
	  }

	  int should_run = 1; //acts as a boolean telling us when to run or close the shell
	  int count = 0; //used to count the chars in an argument
	  int histNum = 0; //keeps track of the numbered entries for history

	  while (should_run)//runs endlessly until a user enters "quit"
	  {
	    printf("osh> ");
	    fflush(stdout);

	    int commandLength = 0;
	    int w = 0;
	    char c;

	    do
	    {
	      c = getchar();
	      for(count = 0; !isspace(c); count++)//refills the arrays with args in each iteration of loop
	      {
	        args[w][count] = c;
	        last[commandLength] = c;
		commandLength++;
		c = getchar();
	      }
	      args[w][count] = '\0';
		w++;
	      last[commandLength] = '\0';
	    } while (c == ' ');

	    int tempcount = 0;
	    while(args[tempcount]!= '\0')
	    {
	    	if(args[tempcount]== "&") //checks for '&' character in parsed argument list
	    		runInBackground = 1; //tells parent to wait for child to finish
	    	tempcount++;
	    }

	    if(strcmp(args[0],"exit") == 0) //will break while loop
	    {
	      should_run = 0;
	    } 
	    else if(strcmp(args[0], "cd")==0)
	    {
		int tec = chdir(args[1]); //changes directory
		if(tec == 0)
			printf("success!\n"); //signifies that the directory changed
		 histNum = historyUpdate(history, last, histNum); //updates history with args		
	    }

	    else if(strcmp (args[0],"history") == 0)//prints history list
	    {
	      historyOut(history, histNum);
	    } 
	    else if(strcmp (args[0],"!!") == 0)//tells to execute last command
	    {
	      if(histNum == 0) //if there is no history yet, it can't execute anything
	      	printf("No commands in history\n");
	      else
	    	{
	    		char* re_add; //contains the last command (to be reentered in history)
	      		re_add = lastHistory(history, histNum, runInBackground);
	      		histNum = historyUpdate(history, re_add, histNum);
				runInBackground =0;//sets run in background to 0 (stop waiting)
			}
	    }
	    else if(args[0][0] == '!' && isdigit (args[0][1]))//detects exclamation and number to be used
	    {
	      int temp = 0;
	      if(sscanf(args[0], "!%d", &temp) == 1)
	      {
	        if(temp > 0 && temp <= histNum && temp > histNum -10)
	        {
	        	char* re_add;
	        	re_add = chooseFromHistory(history, histNum, temp-1, runInBackground);
	        	histNum = historyUpdate(history, re_add, histNum);
				runInBackground = 0;

	        }
	        else //if command number is out of range
	        {
	        	printf("No such command in history.\n");
	        }
	      }
	    }
	    else
	    {
	      free(args[w]);//frees up memory space for next loop
	      args[w] = NULL;
	      pid_t pid;
	      pid = fork();

		  if (pid < 0)
		  {
		    fprintf(stderr, "The fork failed");
		    return -1;
		  }
		  else if (pid == 0)
		  {
		    execvp(args[0], args);
		  }
		
		  else
		  {
		    if(runInBackground == 1)
			wait(0);
		    runInBackground = 0; //don't wait on next iteration
		  //  pid = getpid();
		  }
	      args[w] = malloc(MAX_LINE * sizeof(char));
	      args[w][0] = '\0';
	      histNum = historyUpdate(history, last, histNum); //adds command to history
	    }
	 }
  
  return 0;
}
