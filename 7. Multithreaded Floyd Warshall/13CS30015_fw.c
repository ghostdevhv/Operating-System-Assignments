#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int graph[100][100];
int dist[100][100];

#define INF 99999

int n,m;	// Nodes and Undirected Edges
int k;


pthread_mutex_t read_mutex;
pthread_mutex_t write_mutex;


int readers = 0;
int checker;

void *check_func(void *t)
{
  	long i = (long)t;
  	//printf("Creating Thread Inside k = %d, i = %ld\n",k,i);
  	 for(int j=0;j<n;j++)
  	 {
        pthread_mutex_lock(&read_mutex);
        readers++;
        if(readers == 1)
          pthread_mutex_lock(&write_mutex);

        pthread_mutex_unlock(&read_mutex);

		    checker = dist[i][k] + dist[k][j] < dist[i][j];
		
        pthread_mutex_lock(&read_mutex);
        
        readers--;
        if(readers == 0)
        {  
            pthread_mutex_unlock(&write_mutex);
        }
        
        pthread_mutex_unlock(&read_mutex);

		if(checker)
		{
			pthread_mutex_lock(&write_mutex);
			dist[i][j]=dist[i][k]+dist[k][j];
			pthread_mutex_unlock(&write_mutex);
		}
          
  	}
  	pthread_exit(NULL);
}

int main()
{
    printf("Enter the no. of Nodes and no. of Undirected Edges\n");
    scanf("%d %d",&n,&m);

    int u,v,w;

    for(int i=0;i<m;i++)  // Creating graph that stores adjacency matrix
    {
	    scanf("%d %d %d",&u,&v,&w);
      	graph[u-1][v-1] = w;
      	graph[v-1][u-1] = w;
    }

    for(int i=0;i<n;i++)
    {
      	for(int j=0;j<n;j++)
      	{
	        if(i!=j && graph[i][j]==0)
	        {
	          	graph[i][j] = INF;
	        }
      	}
    }

	printf("\n------- Initial Matrix --------\n\n");
  	for (int i = 0; i < n; i++)
    {
      	for (int j = 0; j < n; j++)
      	{
          	if (graph[i][j] == INF)
          	{	
              	printf("%7s", "INF");
          	}
          	else
          	{	
              	printf ("%7d", graph[i][j]);
          	}
      	}
      	printf("\n");
  	}

    for(int i=0;i<n;i++)
    {
  		for(int j=0;j<n;j++)
        	dist[i][j] = graph[i][j];
    }

   	pthread_t mythread[100];
   	pthread_mutex_init(&read_mutex, NULL);
   	pthread_mutex_init(&write_mutex, NULL);
   	

  	for(k=0;k<n;k++)
  	{
  		//printf("==================== k = %d ==============\n", k);
  		for(long i=0;i<n;i++)
  		{
  			//printf("Creating Thread k = %d, i = %ld\n",k,i);
  			if(pthread_create(&mythread[i], NULL, check_func,(void *)i))
  			{
  				perror("Create Thread Failed:");
  				exit(0);
  			}
  		}

  		for(int i=0;i<n;i++)
  		{
  			if(pthread_join(mythread[i], NULL))

  			{
  				perror("Joining Thread Failed:");
  				exit(0);
  			}
  		}
  		//printf("====================================\n");

  	}

  	pthread_mutex_destroy(&read_mutex);
  	pthread_mutex_destroy(&read_mutex);
  	
  	
  	printf("\n------- Final Distance Matrix --------\n\n");
  	for (int i = 0; i < n; i++)
    {
      	for (int j = 0; j < n; j++)
      	{
          	if (dist[i][j] == INF)
          	{	
              	printf("%7s", "INF");
          	}
          	else
          	{	
              	printf ("%7d", dist[i][j]);
          	}
      	}
      	printf("\n");
  	}
  	pthread_exit(NULL);
}