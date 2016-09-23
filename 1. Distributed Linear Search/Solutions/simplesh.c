#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <locale.h>
#include <langinfo.h>
#include <stdint.h>

#define Command_length 1000
#define token_delimiters " \t\r\n\a"
#define _OPEN_SYS

char *curr_dir;
extern char **environ;
char *prev_dir;
char date[256];

FILE *ptr;
int initial_count;

struct tm *foo;
struct stat st;
struct group *grp;
struct passwd *pwd;

int shell_cd(char **args);
int shell_exit(char **args);
int shell_clear();
int shell_env(char **args);
int shell_pwd(char **args);
int shell_mkdir(char **args);
int shell_rmdir(char **args);
int shell_ls(char **args);
int shell_history(char **args);

char *builtin_str[] = {		// No problem
	"cd",
	"clear",
	"pwd",
	"env",
	"mkdir",
	"rmdir",
	 "ls",
	"history",
	"exit"
};

int (*builtin_func[]) (char **) = {	// No problem
  &shell_cd,
  &shell_clear,
  &shell_pwd,
  &shell_env,
  &shell_mkdir,
  &shell_rmdir,
  &shell_ls,
  &shell_history,
  &shell_exit
};

int shell_num_builtins() {	// No problem
  return sizeof(builtin_str) / sizeof(char *);
}

int shell_exit(char **args)
{
	return 0;
}

void write_to_file(char *s)
{
	if(strcmp(s,"")==0)
		return ;
	initial_count++;
	char *temp=(char *)malloc(sizeof(char)*Command_length);
	sprintf(temp,"%d",initial_count);
	strcat(temp," ");
	strcat(temp,s);
	strcat(temp,"\n");
	fprintf(ptr,"%s",temp);
	free(temp);
}

int is_valid_int(const char *str)
{
	if(*str==NULL) return 1;
   if (*str == '-') return 0;
   if(*str=='+') str++;
   if (!*str) return 0;
   while (*str)
   {
      if (!isdigit(*str)) return 0;
      else ++str;
   }
   return 1;
}

int shell_history(char **args)
{
	int z;
	char *line=(char *)malloc(sizeof(char)*Command_length);
	if(args[1]!=NULL){
		if(!is_valid_int(args[1])){
			printf("bash: history: %s: positive numeric argument required\n",args[1]);
		} //print error
		else{
			z=atoi(args[1]);
			int total_commands=initial_count;
			fclose(ptr);
			ptr=fopen("/home/himanshu/Downloads/command_history.txt","r+");
			if(z>=total_commands){
				while(fgets(line,Command_length,ptr)!=NULL)
					printf("%s",line);
			}
			else{
				while(fgets(line,Command_length,ptr)!=NULL){
					if(total_commands<=z) printf("%s",line);
					total_commands--;
				}
			}
		}
	}
	else{
		fclose(ptr);
		ptr=fopen("/home/himanshu/Downloads/command_history.txt","r+");
		while(fgets(line,Command_length,ptr)!=NULL)
			printf("%s",line);
		return 1;
	}
	return 1;
}

int shell_ls(char **args)
{
	DIR *dp;
	struct dirent *sd;

	if(args[1]== NULL)
	{
		dp = opendir(curr_dir);
		while(sd = readdir(dp))
		{
			if(strcmp(sd->d_name,"..") ==0 || strcmp(sd->d_name,".")==0)
				continue;
			else
				printf("%s\t", sd->d_name);
		}
		printf("\n");
		closedir(dp);
	}

	else if (strcmp(args[1],"-l")!=0 && args[2]==NULL && args[1]!=NULL)
	{
		strcpy(prev_dir,curr_dir);

		if(args[1] == NULL)
		{
			fprintf(stderr, "shell: expected argument to \"cd\"\n");
		}
		else
		{
			strcat(prev_dir,"/");
			strcat(prev_dir,args[1]);
			if(chdir(prev_dir) != 0)
			{
				perror("shell");
				return 1;
			}
			else
			{
				//curr_dir=getcwd(curr_dir,(size_t)Command_length);
				dp = opendir(prev_dir);

				while(sd = readdir(dp))
				{
					if(strcmp(sd->d_name,"..") ==0 || strcmp(sd->d_name,".")==0)
						continue;
					else
						printf("%s\t", sd->d_name);
				}
				printf("\n");
				closedir(dp);
				//strcpy(curr_dir,prev_dir);
			}
		}

	}

	else if (strcmp(args[1],"-l")==0 && args[2]==NULL)
	{
		dp = opendir(curr_dir);

		while(sd = readdir(dp))
		{
			if(strcmp(sd->d_name,"..") ==0 || strcmp(sd->d_name,".")==0)
					continue;
			else
			{
				stat(sd->d_name, &st);
				printf("  ");
				printf( (S_ISDIR(st.st_mode)) ? "d" : "-");
			    printf( (st.st_mode & S_IRUSR) ? "r" : "-");
			    printf( (st.st_mode & S_IWUSR) ? "w" : "-");
			    printf( (st.st_mode & S_IXUSR) ? "x" : "-");
			    printf( (st.st_mode & S_IRGRP) ? "r" : "-");
			    printf( (st.st_mode & S_IWGRP) ? "w" : "-");
			    printf( (st.st_mode & S_IXGRP) ? "x" : "-");
			    printf( (st.st_mode & S_IROTH) ? "r" : "-");
			    printf( (st.st_mode & S_IWOTH) ? "w" : "-");
			    printf( (st.st_mode & S_IXOTH) ? "x" : "-");
				printf("  %lu  ", st.st_nlink);

				pwd = getpwuid(st.st_uid);
				printf("%s  ", pwd->pw_name);

				grp = getgrgid(st.st_gid);
				printf("%s  ", grp->gr_name);

				printf(" %9jd", (intmax_t)st.st_size);

				strftime(date, sizeof(date), "  %b  %d  ", localtime(&(st.st_mtime)));
				printf("%s", date);

				foo = localtime(&st.st_mtime);
				printf("%d:", foo->tm_hour);
				printf("%d  ", foo->tm_min);
				printf("%s\n",sd->d_name);
			}
		}
		closedir(dp);
	}

	else
	{
		strcpy(prev_dir,curr_dir);
		if(args[2] == NULL)
		{
			fprintf(stderr, "shell: expected argument to \"cd\"\n");
		}
		else
		{
			strcat(prev_dir,"/");
			strcat(prev_dir,args[2]);
			if(chdir(prev_dir) != 0)
			{
				perror("shell");
				return 1;
			}
			else
			{
				//curr_dir=getcwd(curr_dir,(size_t)Command_length);
				dp = opendir(curr_dir);

				while(sd = readdir(dp))
				{
					if(strcmp(sd->d_name,"..") ==0 || strcmp(sd->d_name,".")==0)
						continue;
					else
					{
						stat(sd->d_name, &st);
						printf("  ");
						printf( (S_ISDIR(st.st_mode)) ? "d" : "-");
					    printf( (st.st_mode & S_IRUSR) ? "r" : "-");
					    printf( (st.st_mode & S_IWUSR) ? "w" : "-");
					    printf( (st.st_mode & S_IXUSR) ? "x" : "-");
					    printf( (st.st_mode & S_IRGRP) ? "r" : "-");
					    printf( (st.st_mode & S_IWGRP) ? "w" : "-");
					    printf( (st.st_mode & S_IXGRP) ? "x" : "-");
					    printf( (st.st_mode & S_IROTH) ? "r" : "-");
					    printf( (st.st_mode & S_IWOTH) ? "w" : "-");
					    printf( (st.st_mode & S_IXOTH) ? "x" : "-");
						printf("  %lu  ", st.st_nlink);

						pwd = getpwuid(st.st_uid);
						printf("%s  ", pwd->pw_name);

						grp = getgrgid(st.st_gid);
						printf("%s  ", grp->gr_name);

						printf(" %9jd", (intmax_t)st.st_size);

						strftime(date, sizeof(date), "  %b  %d  ", localtime(&(st.st_mtime)));
						printf("%s", date);

						foo = localtime(&st.st_mtime);
						printf("%d:", foo->tm_hour);
						printf("%d  ", foo->tm_min);
						printf("%s\n",sd->d_name);
					}

				}
				//strcpy(curr_dir,prev_dir);
				closedir(dp);
			}
		}
	}
	return 1;
}

int shell_rmdir(char **args)
{
	if(rmdir(args[1])!=0)
		perror("rmdir() error");
	return 1;
}
int shell_mkdir(char **args)
{
	printf("%s\n", curr_dir);
	
	if(mkdir(args[1], S_IRWXU|S_IRGRP|S_IXGRP)!=0)
		perror("mkdir() error");
	return 1;
}

int shell_pwd(char **args)
{
	char * cwd ;
	cwd = (char *)malloc(sizeof(char)*Command_length);
	cwd = getcwd(cwd, (size_t)Command_length);
	if (cwd != NULL)
       fprintf(stdout, "Current working dir: %s\n", cwd);
   else
       perror("getcwd() error");
   return 1;
}

int shell_env(char **args)
{
	int i = 1;
  char *s = *environ;

  for (; s; i++) {
    printf("%s\n", s);
    s = *(environ+i);
  }
  return 1;
}

int shell_clear()
{
	const char* CLEAR_SCREE_ANSI = "\e[1;1H\e[2J";
  	write(STDOUT_FILENO,CLEAR_SCREE_ANSI,12);
  	return 1;
}

int shell_cd(char **args)
{
	if(args[1] == NULL)
	{
		fprintf(stderr, "shell: expected argument to \"cd\"\n");
	}
	else
	{
		strcpy(prev_dir,curr_dir);
		strcat(prev_dir,"/");
		strcat(prev_dir,args[1]);
		 if(chdir(prev_dir) != 0){
		 	perror("shell");
			return 2;
		 }
		else{
			curr_dir=getcwd(curr_dir,(size_t)Command_length);
		}
	}
	return 1;
}

int shell_launch(char **args)
{
  	if(args[0][strlen(args[0])-1] == '&')
  	{
  		args[0][strlen(args[0])-1] = '\0';
  		pid_t pid, wpid;
  		int status;
		pid = fork();
  		if (pid == 0) // Child process
  		{
    		if (execvp(args[0], args) == -1)
    		{
      			perror("shell");
    		}
    		setpgid(0, 0);

    		exit(EXIT_FAILURE);
  		}

  		else if (pid < 0)
  		{
    		perror("shell");
  		}

  		else 	// Parent process
  		{
    		if (!tcsetpgrp(STDIN_FILENO, getpid()))
    		{
    			perror("tcsetpgrp failed");
			}
  		}
  	}
  	else
  	{
  		pid_t pid, wpid;
  		int status;
		pid = fork();
  		if (pid == 0) 
  		{
    		if (execvp(args[0], args) == -1)
    		{
      			perror("shell");
    		}
    		exit(EXIT_FAILURE);
  		}

  		else if (pid < 0)
  		{
    		perror("lsh");
  		}

  		else 	
  		{
    		do
    		{
      			wpid = waitpid(pid, &status, WUNTRACED);
    		}
    		while (!WIFEXITED(status) && !WIFSIGNALED(status));
  		}
  	}

  	return 1;
}

int shell_execute(char **args)	
{
	if(args[0] == NULL)
	{
		return 1;
	}
	for(int i=0 ;i<shell_num_builtins();i++)
	{
		if(strcmp(args[0], builtin_str[i])==0)
		{
			return (*builtin_func[i])(args);
		}
	}
	return shell_launch(args);

}

char **shell_split_line(char *cmd_line)	
{
	int bufsize = Command_length;
	int position = 0;
	char **tokens = malloc(sizeof(char)*bufsize);
	char *token;
	token = strtok(cmd_line, token_delimiters);

	while(token!=NULL)
	{
		tokens[position] = token;
		position++;
		token = strtok(NULL, token_delimiters);
	}
	tokens[position] = NULL;
	return tokens;
}

char *shell_read_line()	
{
	int bufsize = Command_length;
	char *line = malloc(sizeof(char)*bufsize);
	gets(line);
	return line;
}

void shell_loop()	
{
	char *cmd_line;
	char **args;
	int status;
	curr_dir=(char *)(malloc(1000*sizeof(char)));
	prev_dir=(char *)(malloc(1000*sizeof(char)));
	curr_dir=getcwd(curr_dir,(size_t)Command_length);
	do{
		printf("%s",curr_dir);
		printf("> ");
		cmd_line = shell_read_line();
		write_to_file(cmd_line);
		args = shell_split_line(cmd_line);
		status = shell_execute(args);
		free(cmd_line);
		free(args);
	}
	while(status); // Using status variable returned by shell_execute() to determine when to exit
}

int main(int argc, char **argv, char ** envp)
{
	FILE *ptr1=fopen("/home/himanshu/Downloads/command_count.txt","r");
	if(ptr1==NULL){
		printf("Hee\n");
		ptr1=fopen("/home/himanshu/Downloads/command_count.txt","w");
		fprintf(ptr1,"%d",0);
		initial_count=0;
	}
	else 
	{
		printf("Hell");
		fscanf(ptr1,"%d",&initial_count);
	}
	ptr=fopen("/home/himanshu/Downloads/command_history.txt","a");
	shell_loop();
	fclose(ptr1);
	ptr1=fopen("/home/himanshu/Downloads/command_count.txt","w");
	fprintf(ptr1, "%d", initial_count);
	fclose(ptr1);
	fclose(ptr);
	return EXIT_SUCCESS;
}
