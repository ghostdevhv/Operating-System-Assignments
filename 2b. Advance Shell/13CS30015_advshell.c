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
#include <sys/wait.h>
#include <ctype.h>

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
		if((s[0]>='a' && s[0]<='z') || (s[0]>='A' && s[0]<='Z') || (s[0]>='0' && s[0]<='9'))
		{
			if((s[1]>='a' && s[1]<='z') || (s[1]>='A' && s[1]<='Z')|| (s[1]>='0' && s[1]<='9'))
			{
				initial_count++;
				char *temp=(char *)malloc(sizeof(char)*Command_length);
				sprintf(temp,"%d",initial_count);
				strcat(temp," ");
				strcat(temp,s);
				strcat(temp,"\n");
				fprintf(ptr,"%s",temp);
				free(temp);
			}	
			
		}
}

void ls_signal(int signo, siginfo_t *info, void *extra)
{
	int flag;
  	char s[100],ch,s1[100];
  	memset(s,'\0',100);
 
 	fgets(s,100,stdin);
  	//printf("%s\n", s);//("%s",s);
  	int breakpoint=0;
  	FILE *fp;
  	int count=0,i;
  	int indx ;
  	char hist[1000][1000];
  	memset(hist,'\0',1000000);
  	fp = fopen("/home/himanshu/Documents/command_history.txt","r");
  	//printf("Hello\n");
	while((ch=fgetc(fp))!=EOF)
	{
	  if(ch == '\n' )
	  {
	  	  //  hist[count-1][indx] = '\0';
	  	   indx = 0;
	      hist[count++];
	  }
	  else
	  {
	      if(ch>='0' && ch<='9')
	      	continue;
	      else
	      	hist[count][indx++] = ch;

	  }
	}
	// for(int i=0;i<count;i++)
	// 	printf("%s\n",hist[i]);
	
	for(int i=count-1;i>=0;i--)
	{
		int j,k;
		int cnt = 0;
		int cnt1 = 0;
		//printf("%s\n", hist[i]);
		while (hist[i][cnt]!='\0')
		cnt++;

		while (s[cnt1]!='\0')
		cnt1++;

		cnt1 = cnt1-1;

		if(cnt-1>=cnt1)
		{
			for(j=0;j<=cnt-cnt1;j++)
			{
				for(k=j;k<j+cnt1;k++)
				{
					flag=1;
					if (hist[i][k]!=s[k-j])
					{
						flag=0;
					   break;
					}
				}
				if (flag==1)
					break;
			}

			if(flag==1)
			{
				printf("Recommendations : %s\n",hist[i]);
				break;
			}
		}
		else
			continue;
	}

	if(flag==0)
		printf("Recommendations not found\n");
	return ;
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
			ptr=fopen("/home/himanshu/Documents/command_history.txt","r+");
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
		ptr=fopen("/home/himanshu/Documents/command_history.txt","r+");
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
	strcpy(prev_dir,curr_dir);
		strcat(prev_dir,"/");
		strcat(prev_dir,args[1]);
	if(rmdir(prev_dir)!=0)
		perror("rmdir() error");
	return 1;
}
int shell_mkdir(char **args)
{
	//printf("%s\n", curr_dir);
	
	strcpy(prev_dir,curr_dir);
		strcat(prev_dir,"/");
		strcat(prev_dir,args[1]);

	if(mkdir(prev_dir, S_IRWXU|S_IRGRP|S_IXGRP)!=0)
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
	printf("\e[1;1H\e[2J");
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

void piped(char **args,int pos)
{
	// Pos --> No .of arguments except pipe 

	int i,j,k;
	int numPipes;

    numPipes = pos-1;
 
    int p[numPipes][2];
    pid_t child[numPipes + 1];

    for(i = 0; i < numPipes; i++){
        pipe(p[i]);
    }

    memset(child,1,(numPipes + 1)*sizeof(pid_t) );

    for(i = 0; i < numPipes + 1; i++)
    {
        if(child[0] > 0)
        {
            child[i] = fork();
            if(child[i] < 0)
            {

            }
            else if(child[i] == 0)
            {
                for(j = 0; j < numPipes+1;j++)
                {
                	if(j!=i)
                	{
                		child[j] = -1;
                	}
                }
            }
        }
    }

    for(i = 0; i < numPipes + 1; i++)
    {
        if(child[i] == 0)
        {
            if(i == 0)
            {
                dup2(p[i][1],1);	// 1 for STDOUT
                if (execvp(args[i], args) == -1)
    		{
      			perror("shell");
    		}
    		exit(EXIT_FAILURE);
            }
            else if(i < numPipes)
            {
                dup2(p[i-1][0], 0);	// 0 for STDIN
                dup2(p[i][1],1);
                if (execvp(args[i], args) == -1)
    		{
      			perror("shell");
    		}
    		exit(EXIT_FAILURE);
            }
            else
            {
                dup2(p[i-1][0], 0);
                //dup2(p[i][1],1);
                if (execvp(args[i], args) == -1)
    		{
      			perror("shell");
    		}
    		exit(EXIT_FAILURE);
            }

        }
    }

    for (i = 0; i < numPipes + 1; ++i) 
    {
        wait(0);
    }

}

int shell_launch(char **args,int k,int pos)
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
  			if(k==0)
  			{
  				if (execvp(args[0], args) == -1)
	    		{
	      			//perror("shell");
	    		}
    			exit(EXIT_FAILURE);
	  			
  			}
  			else if (k==1)	// "<"
  			{
  				//printf("hII\n");
  				int flag = 0;
  				for(int i=0;i<strlen(args[1]);i++)
  				{
  					if(args[1][i]=='>')
  					{
  						flag = 1;
  						break;
  					}

  				}
  				if(flag==1)
  				{
  					//printf("YOLO\n");
  					int bufsize = Command_length;
					int position = 0;
					char **tokens = malloc(sizeof(char)*bufsize);
					char *token;
					token = strtok(args[1],">");

					while(token!=NULL)
					{
						tokens[position] = token;
						position++;
						token = strtok(NULL,"<");
					}
					tokens[position] = NULL;
					// printf("%s\n",tokens[0]);
					// printf("%s\n",tokens[1]);

					int fd = open(tokens[0],O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
			        int fd1 = open(tokens[1],O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
			        dup2(fd,0); // make stdout go to file
			        dup2(fd1,2); //make stderr go to file
			        dup2(fd1,1); 
			        close(fd);
			        close(fd1);
			        if (execvp(args[0], args) == -1)
			        {
			            perror("lsh");
			        }
        			exit(EXIT_FAILURE);

  				}
  				else
  				{
  					//printf("HIIII");
  					int fd = open(args[1],O_RDONLY,0640);
	  				dup2(fd,0);	// make stdin from the file
	  				close(fd);
	  				if (execvp(args[0], args) == -1)
		    		{
		      			perror("shell");
		    		}
		    		exit(EXIT_FAILURE);
  				}
  				
  			}

  			else if (k==2)	/// '>'  Sign
  			{
  				int fd = open(args[1],O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  				dup2(fd,1);	// make stdout go to file
  				dup2(fd,2);	//make stderr go to file 
  				close(fd);
  				if (execvp(args[0], args) == -1)
	    		{
	      			perror("shell");
	    		}
	    		exit(EXIT_FAILURE);
  			}
  			else
  			{
  				piped(args,pos);
  			}
			
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
	//ptr = fopen("/home/himanshu/Documents/command_history.txt","a");
	int flag = 0;
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

	if(args[0][strlen(args[0])-1] == '&')
	{
		return shell_launch(args,-1,-1);
	}
	else
	{
		int free=0;
		for(int i=0;i<strlen(args[0]);i++)
		{
			if(args[0][i]=='<' || args[0][i]=='>' || args[0][i]=='|')
			{
				free = 1;
				break;
			}
			else
				continue;
		}

		if(free==0)
			return shell_launch(args,0,1);

		for(int i=0;i<strlen(args[0]);i++)
		{
			if(args[0][i]=='<')
			{
				int bufsize = Command_length;
				int position = 0;
				char **tokens = malloc(sizeof(char)*bufsize);
				char *token;
				token = strtok(args[0],"<");

				while(token!=NULL)
				{
					tokens[position] = token;
					position++;
					token = strtok(NULL,"<");
				}
				tokens[position] = NULL;
				return shell_launch(tokens,1,-1);
			}
			else
				continue;
		}

		for(int i=0;i<strlen(args[0]);i++)
		{
			if(args[0][i]=='>')
			{
				int bufsize = Command_length;
				int position = 0;
				char **tokens = malloc(sizeof(char)*bufsize);
				char *token;
				token = strtok(args[0],">");

				while(token!=NULL)
				{
					tokens[position] = token;
					position++;
					token = strtok(NULL,">");
				}
				tokens[position] = NULL;
				return shell_launch(tokens,2,position);
			}
			else
				continue;
		}

		for(int i=0;i<strlen(args[0]);i++)
		{
			if(args[0][i]=='|')
			{
				int bufsize = Command_length;
				int position = 0;
				char **tokens = malloc(sizeof(char)*bufsize);
				char *token;
				token = strtok(args[0],"|");

				while(token!=NULL)
				{
					tokens[position] = token;
					position++;
					token = strtok(NULL,"|");
				}
				tokens[position] = NULL;
				return shell_launch(tokens,3,position);
			}
			else
				continue;
		}

	}

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

		// if(strcmp(s,"")==0)
		// return ;
		// if((s[0]>='a' && s[0]<='z') || (s[0]>='A' && s[0]<='Z') || (s[0]>='0' && s[0]<='9'))
		// {
		// 	if((s[1]>='a' && s[1]<='z') || (s[1]>='A' && s[1]<='Z')|| (s[1]>='0' && s[1]<='9'))
		// 	{
		// 		initial_count++;
		// 		char *temp=(char *)malloc(sizeof(char)*Command_length);
		// 		sprintf(temp,"%d",initial_count);
		// 		strcat(temp," ");
		// 		strcat(temp,s);
		// 		strcat(temp,"\n");
		// 		fprintf(ptr,"%s",temp);
		// 		free(temp);
		// 	}	
			
		// }

		args = shell_split_line(cmd_line);
		status = shell_execute(args);
		free(cmd_line);
		free(args);
	}
	while(status); // Using status variable returned by shell_execute() to determine when to exit
}

int main(int argc, char **argv, char ** envp)
{
	struct sigaction sighandle;
  	sighandle.sa_flags = SA_SIGINFO;
  	sighandle.sa_sigaction = &ls_signal;
  	sigaction(SIGQUIT, &sighandle, NULL);

	FILE *ptr1=fopen("/home/himanshu/Documents/command_count.txt","r");
	if(ptr1==NULL){
		//printf("Hee\n");
		ptr1=fopen("/home/himanshu/Documents/command_count.txt","w");
		fprintf(ptr1,"%d",0);
		initial_count=0;
	}
	else 
	{

		fscanf(ptr1,"%d",&initial_count);
	}
	ptr=fopen("/home/himanshu/Documents/command_history.txt","a");
	shell_loop();
	fclose(ptr1);
	ptr1=fopen("/home/himanshu/Documents/command_count.txt","w");
	fprintf(ptr1, "%d", initial_count);
	fclose(ptr1);
	fclose(ptr);
	return EXIT_SUCCESS;
}
