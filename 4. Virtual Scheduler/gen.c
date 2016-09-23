#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

int main()
{
	int n,t;
	char temp[20],temp1[50];
	int priority,no_itr,sleep_t;
	double prob;
	int priorityI,no_itrI,sleep_tI;
	double probI;
	pid_t pid;
	
	printf("Please enter the number of Processes and the time-interval between two processes\n");
	scanf("%d%d",&n,&t);
	
	printf("CPU Bound Process | Please enter the following details\n");
	printf("1. Priority \n2. No. of iterations\n3. Sleep Time\n4. Sleep Probability\n\n");
	scanf("%d %d %d %lf",&priority,&no_itr,&sleep_t,&prob);
	
	printf("\nI/O Bound Process | Please enter the following details\n");
	printf("1. Priority \n2. No. of iterations\n3. Sleep Time\n4. Sleep Probability\n\n");
	scanf("%d %d %d %lf",&priorityI,&no_itrI,&sleep_tI,&probI);

	for(int i=1;i<=n/2;i++)		/* CPU Bound Processes */
	{
		sleep(t);
		sprintf(temp,"./process ");
		strcpy(temp1,temp);
		sprintf(temp,"%d ",priority);
		strcat(temp1,temp);
		sprintf(temp,"%d ",no_itr);
		strcat(temp1,temp);
		sprintf(temp,"%d ",sleep_t);
		strcat(temp1,temp);
		sprintf(temp,"%lf ",prob);
		strcat(temp1,temp);
		sprintf(temp,"%d ",i);
		strcat(temp1,temp);

		if((pid = fork())==0)
		{
			execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c",temp1, (void*)NULL);
		}
		else
		{

		}
	}

	for(int i=n/2+1;i<=n;i++)	/* I/O Bound Processes */
	{
		sleep(t);
		sprintf(temp,"./process ");
		strcpy(temp1,temp);
		sprintf(temp,"%d ",priorityI);
		strcat(temp1,temp);
		sprintf(temp,"%d ",no_itrI);
		strcat(temp1,temp);
		sprintf(temp,"%d ",sleep_tI);
		strcat(temp1,temp);
		sprintf(temp,"%lf ",probI);
		strcat(temp1,temp);
		sprintf(temp,"%d",i);
		strcat(temp1,temp);

		if((pid = fork())==0)
		{
			execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c", temp1, (void*)NULL);
		}
		else
		{

		}
	}
}
