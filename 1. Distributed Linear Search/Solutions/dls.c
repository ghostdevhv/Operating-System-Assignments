
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int status1,status2;

void func(int a[],int num,int left, int right)
{
    if((right-left)<5)
    {
        for(int j=left;j<=right;j++)
        {
            if(a[j]==num)
            {
                exit(j);
            }
        }
        exit(-1);
    }

    int b=fork();
    int mid=(left+right)/2;
    if(b==0)
    {
        func(a,num,left,mid);
        exit(0);
    }
    else
    {
        waitpid(b,&status1,0);
        int c=fork();
        if(c==0)
        {
            func(a,num,mid+1,right);
            exit(0);
        }
        else
        {
            waitpid(c,&status2,0);
            if(status1!=255*256) exit(status1>>8);
            else exit(status2>>8);
        }
    }
}

int main(int argc, char *argv[])
{
    if(argc!=3)
        return 1;
    FILE *ptr=fopen(argv[1],"r");
    int i,num,a[1000],n=0;
    while(fscanf(ptr,"%d",&a[n++])!=EOF);
    n-=1;
    num=atoi(argv[2]);
    int a1=fork();
    if(a1==0){
        func(a,num,0,n-1);
        exit(0);
    }
    else{
        waitpid(a1,&status1,0);
        if(status1==255*256) printf("Not Found\n");
        else printf("Number found at %d\n",status1>>8);
    }
    return 0;
}
