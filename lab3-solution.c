#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define NUM_HIST 10	/* Remember upto 10 command histories. */

char histories[NUM_HIST][MAX_LINE/2+1][MAX_LINE];
int backgrounds[NUM_HIST];
int num_hist = 0;
int latest = -1;

/* copy the command into the history */
void cmdcpy(int idx, char *args[]) {
	int i = 0;
	while (args[i]) {
		strncpy(histories[idx][i], args[i], strlen(args[i])+1);
		++i;
	}
	histories[idx][i][0] = '\0';
}

/* return the index of the given command */
int find_cmd(char *args[]) {
	int i = 0;
	for (; i < num_hist; ++i) {
		int cmd_idx = (latest+NUM_HIST-i) % NUM_HIST;
		if (args[0][0] == histories[cmd_idx][0][0]) {
			return cmd_idx;
		}
	}
	return -1;
}

/* print the command at index idx */
void print_cmd(int idx) {
	int i = 0;
	while (histories[idx][i][0] != '\0') {
		write(STDOUT_FILENO, histories[idx][i], strlen(histories[idx][i]));
		write(STDOUT_FILENO, " ", strlen(" "));
		++i;
	}
	if (backgrounds[idx] == 1) {
		write(STDOUT_FILENO, "&", strlen("&"));
	}
	write(STDOUT_FILENO, "\n", strlen("\n"));
}

/**
 * show_history() changes the default behavior of SIGINT handler. When user
 * sends SIGINT, shows the command histories instead of exiting the process.
 */
void show_history() {
	write(STDOUT_FILENO, "\n", strlen("\n"));
	if (num_hist < NUM_HIST) {
		int i = 0;
		for (; i < num_hist; ++i) {
			print_cmd(i);
		}
	} else {
		int i = 0;
		for (; i < NUM_HIST; ++i) {
			int hist_idx = (latest+1+i) % NUM_HIST;
			print_cmd(hist_idx);
		}
	}
	write(STDOUT_FILENO, "COMMAND->", strlen("COMMAND->"));
	fflush(stdout);
}

/**
 * setup() reads in the next command line, separating it into distinct tokens
 * using whitespace as delimiters. setup() sets the args parameter as a 
 * null-terminated string.
 */
int setup(char inputBuffer[], char *args[],int *background)
{
	int length, /* # of characters in the command line */
			i,      /* loop index for accessing inputBuffer array */
			start,  /* index where beginning of next command parameter is */
			ct;     /* index of where to place the next parameter into args[] */

	ct = 0;

	/* read what the user enters on the command line */
	length = read(STDIN_FILENO, inputBuffer, MAX_LINE);  

	start = -1;
	if (length == 0) {
		exit(0);            /* ctrl+d was entered, end of user command stream */
	}
	/*
	if (length < 0 && errno == EINTR){
		return 0;
	}
	*/
	if (length < 0) {
		printf("Failed read(). Abort.\n");
		exit(-1);
	}

	/* examine every character in the inputBuffer */
	for (i = 0; i < length; i++) { 
		switch (inputBuffer[i]){
			case ' ':
			case '\t' :               /* argument separators */
				if(start != -1){
					args[ct] = &inputBuffer[start];    /* set up pointer of args[ct] */
					ct++;
				}
				inputBuffer[i] = '\0'; /* add a null char; make a C string */
				start = -1;
				break;

			case '\n':                 /* should be the final char examined */
				if (start != -1){
					args[ct] = &inputBuffer[start];     
					ct++;
				}
				inputBuffer[i] = '\0';
				args[ct] = NULL; /* no more arguments to this command */
				break;

			case '&':
				*background = 1;
				inputBuffer[i] = '\0';
				break;

			default :             /* some other character */
				if (start == -1)
					start = i;
		} 
	}    
	args[ct] = NULL; /* just in case the input line was > 80 */
	return 1;
} 


int main(void)
{
	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	char rawInput[MAX_LINE];
	int len;
	int background;             /* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE/2+1];/* command line (of 80) has max of 40 arguments */
	int pid, ret, cmd, cmd_idx;

	/* change handler for SIGINT */
	struct sigaction handler;
	handler.sa_handler = show_history;
	sigemptyset(&handler.sa_mask);
	handler.sa_flags |= SA_RESTART;
	sigaction(SIGINT, &handler, NULL);

	while (1){            /* Program terminates normally inside setup */
		background = 0;
		printf("COMMAND->");
		fflush(0);
		cmd = setup(inputBuffer, args, &background);

		/* user issued <Ctrl><c> */
		if (cmd == 0) {
			continue;
		}

		// collect the zombies
		while (waitpid(-1, &ret, WNOHANG) > 0);

		if (args[0] == NULL) {		/* invalid user input */
			continue;
		}

		// hack for cd
		if (strcmp("cd", args[0]) == 0) {
			if (chdir(args[1]) < 0) {
				printf("can't switch to directory %s.\n", args[1]);
			}
			continue;
		}
		// hack for exit
		if (strcmp("exit", args[0]) == 0) {
			exit(0);
		}

		/* hack for running history */
		if (strcmp("r", args[0]) == 0) {
			int arg_idx = 0;
			if (num_hist == 0) {
				printf("no history.\n");
				continue;
			}

			/* retrieve the index of the corresponding command in the history */
			if (args[1] != NULL) {
				cmd_idx = find_cmd(args+1);
				if (cmd_idx == -1) {
					printf("no matching history.\n");
					continue;
				}
			} else {
				cmd_idx = latest;
			}	
			
			/* retrieve the corresponding command */
			while (histories[cmd_idx][arg_idx][0] != '\0') {
				args[arg_idx] = histories[cmd_idx][arg_idx];
				arg_idx++;
			}
			args[arg_idx] = NULL;
			background = backgrounds[cmd_idx];
		}

		/* remember the commmand */
		if (num_hist < NUM_HIST) {
			cmdcpy(num_hist, args);
			backgrounds[num_hist] = background;
			latest = num_hist;
			num_hist++;
		} else {
			latest = (latest+1) % NUM_HIST;
			cmdcpy(latest, args);
			backgrounds[latest] = background;
		}

		/* execute user command */
		if ((pid = fork()) == 0) {
			ret = execvp(args[0], args);
			if (ret < 0) {
				printf("errors in executing user commands.\n");
				exit(ret);
			}
			exit(0);
		} else {                 
			if (background == 0) {
				waitpid(pid, &ret, WCONTINUED);
				printf("child process return value=%d\n", ret);
			} 
		}
	}
}
