
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
int available=30001, busy=30002, maxx=1000000, stop=30003, sending=30004,isprime[1000005];

void sieve()
{
    for(int i=2;i<1000000;i++) isprime[i]=1;
    for(int i=2;i<1000000;i++){
        if(isprime[i]){
            for(int j=i+i;j<1000000;j+=i) isprime[j]=0;
        }
    }
}

int main(int argc, char *argv[])
{
    if(argc!=3) return 1;
    srand(time(NULL));
    int n,k;
    n=atoi(argv[1]);
    k=atoi(argv[2]);
    int i,array_p[k][k],array_c[k][k],j,result[k][k],prime_cnt=0,x;
    sieve();
    int pivot,parent_to_child[k][2],child_to_parent[k][2],result_prime[n],array_temp[k][k];
    pid_t pid[k];
    for(i=0;i<k;i++){
        pipe(parent_to_child[i]);
        pipe(child_to_parent[i]);
        pid[i]=fork();
        if(pid[i]==0){
            write(child_to_parent[i][1],&available,sizeof(available));
            while(1){
                read(parent_to_child[i][0],array_c[i],sizeof(array_c[i]));
                write(child_to_parent[i][1],&busy,sizeof(busy));
                printf("child->%d\n",i);
                for(j=0;j<k;j++){
                    if(isprime[array_c[i][j]]) result[i][j]=1;
                    else result[i][j]=0;
                    printf("%d--%d\n",array_c[i][j],result[i][j]);
                }
                printf("\n");
                write(child_to_parent[i][1],result[i],sizeof(result[i]));
                write(child_to_parent[i][1],&available,sizeof(available));
            }
        }
    }
    while(1){
        if(prime_cnt>=n){
            for(x=0;x<n;x++) printf("-----------%d-----------\n",result_prime[x]);
            for(x=0;x<k;x++) kill(pid[x],SIGKILL);
            exit(0);
            break;
        }
        while(1){
            int flag=0,kk;
            for(i=0;i<k;i++){
                printf("reading status\n");
                kk=read(child_to_parent[i][0],&flag,sizeof(flag));
                printf("%d\n",flag);
                if(flag==available){
                    printf("child number %d is available\n",i);
                    for(x=0;x<k;x++) array_p[i][x]=rand()%30000+1;
                    write(parent_to_child[i][1],array_p[i],sizeof(array_p[i]));
                    pivot=i;
                    flag=1;
                    break;
                }
                else if(flag==busy){
                    printf("child number %d is busy\n",i);
                    read(child_to_parent[i][0],array_temp[i],sizeof(array_temp[i]));
                    for(int x=0;x<k;x++){
                        int flag2=0;
                        if(array_temp[pivot][x]==1){
                            for(int y=0;y<prime_cnt;y++)
                                if(result_prime[y]==array_p[pivot][x]) flag2=1;
                            if(flag2==0){
                                result_prime[prime_cnt++]=array_p[pivot][x];
                                if(prime_cnt>=n){
                                    for(x=0;x<n;x++) printf("Prime %d----------->%d\n",x+1,result_prime[x]);
                                    for(x=0;x<k;x++) kill(pid[x],SIGKILL);
                                    exit(0);
                                    break;
                                }
                            }
                        }
                    }
                }
                else printf("Found an error!!\n");
            }
            if(flag==1) break;
        }
    }
    exit(0);
}