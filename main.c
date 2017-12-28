#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void lsh_loop(void);
char *lsh_read_line(void);
char **lsh_split_line(char*);
int lsh_execute(char **args);

int lsh_cd(char **args);
int lsh_pwd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

char *builtin_str[] = {
	"cd",
	"pwd", 
	"help",
	"exit"
};

int (*builtin_func[]) (char **) = {
	&lsh_cd,
	&lsh_pwd,
	&lsh_help,
	&lsh_exit
};

int lsh_num_builtins(void) {
	return sizeof(builtin_str) / sizeof(char *);
}

int main(int argc, char **argv)
{
	lsh_loop();

	return EXIT_SUCCESS;
}

int lsh_cd(char **args)
{
	if (args[1] == NULL) {
		fprintf(stderr, "lsh: expected argument to \"cd\"\n");
	} else {
		if (chdir(args[1]) != 0) {
			perror("lsh");
		}
	}

	return 1;
}

int lsh_pwd(char **args)
{
	long path_max;
	size_t size;
	char *buf = NULL;
	char *ptr = NULL;

	path_max = pathconf(".", _PC_PATH_MAX);
	if (path_max == -1) {
		size = 1024;
	} else if (path_max > 10240) {
		size = 10240;
	} else {
		size = path_max;
	}
	if ((buf = realloc(buf, size)) == NULL) {
		perror("lsh");
	}

	ptr = getcwd(buf, size);
	if (ptr == NULL) {
		perror("lsh");
	}

	printf("%s\n", buf);

	return 1;
}

int lsh_help(char **args)
{

	return 1;
}

int lsh_exit(char **args)
{

	return 1;
}

void lsh_loop(void)
{
	char *line;
	char **args;
	int status;

	do {
		printf(">");
		line = lsh_read_line();
		args = lsh_split_line(line);
		status = lsh_execute(args);

		free(line);
		free(args);
	} while(status);
}

int lsh_launch(char **args)
{
	pid_t pid, wpid;
	int status;

	pid = fork();
	if (pid == 0) {
		// Child Process
		if (execvp(args[0], args) == -1) {
			perror("lsh");
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		// Error Forking
		perror("lsh");
	} else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));	
	}
	return 1;
}

char *lsh_read_line(void) 
{
	char *line = NULL;
	size_t bufsize = 0;
	getline(&line, &bufsize, stdin);
	return line;
}

int lsh_execute(char **args) {
	int i;

	if (args[0] == NULL) {
		return 1;
	}

	for (i = 0; i < lsh_num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i]) (args);
		}
	}

	return lsh_launch(args);
}


#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **lsh_split_line(char *line)
{
	int bufsize = LSH_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if (!tokens) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, LSH_TOK_DELIM);
	while (token != NULL) {
		tokens[position] = token;
		position++;

		if (position >= bufsize) {
			bufsize += LSH_TOK_BUFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, LSH_TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}






