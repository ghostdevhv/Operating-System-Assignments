#include <bits/stdc++.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <csignal>
#include <cstdio>
#include <ctime>
#include <sys/time.h>

using namespace std;


class compare
{
public:
    bool operator()(pair<pid_t,pair<int,int> > p1, pair<pid_t,pair<int,int> > p2)
    {
        if(p1.second.first!=p2.second.first) return p1.second.first>p2.second.first;
        return p1.second.second>p2.second.second;
    }
};


key_t key = 270;

#define TYPE 1
#define TYPE2 2


// typedef struct time1{

//     struct timeval it;
//     struct timeval temp;
//     float rt;
//     float wt;
//     float tt;
// }time1 ;
struct timeval tt1;


typedef struct my_msgbuf
{
    long type;
    int priority_info;
    pid_t pid;
    int process_num;
    char text[100];
}my_msgbuf;

int flag = -1;

static void handler(int signo)
{
    printf("I/O request | Signal Received\n");
    flag = 1;
}

static void handler2(int signo)
{
    printf("Signal for Process Termination\n");
    flag = 2;
}

typedef pair<pid_t,pair<int,int> > pp;
queue<pair<pid_t,int> > regular;
queue<pair<pid_t,int> > waiting;
queue<pair<pid_t,int> > temp;

priority_queue<pp,vector<pp>,compare> regular_PR;
priority_queue<pp,vector<pp>,compare> waiting_PR;
priority_queue<pp,vector<pp>,compare> temp_PR;

int main(int argc, char* argv[])
{
    signal(SIGUSR1,handler);
    signal(SIGUSR2,handler2);
    if(argc!=2)
        perror("Error 86: ");

    string check = argv[1];

    struct my_msgbuf sched,sched1;
    int msqid;

    if ((msqid = msgget(key, 0666 | IPC_CREAT)) == -1)
    {
        perror("msgget");
        exit(1);
    }
    int check_process = 0;
    int running_pid,running_procnum;
    int time_quant=0;
    int running_priority;

    if(check == "RR")
    {
        double RT[1000];
        int b_RT[1000];
        struct timeval start_RT_TT[1000];

        double TT[1000];
        int b_TT[1000];

        double WT[1000];
        int b_WT[1000];
        struct timeval start_WT[1000];

        memset(b_RT,-1,sizeof(b_RT));
        memset(b_TT,-1,sizeof(b_RT));
        memset(b_WT,-1,sizeof(b_RT));
        memset(WT,0,sizeof(WT));

        if (msgrcv(msqid, &sched, sizeof(my_msgbuf), TYPE2, 0) == -1)
        {
           perror("Receive Error 109: ");
        }

        if(b_RT[sched.process_num] == -1)   // Response and Turn Around Initial Time
        {
            gettimeofday(&start_RT_TT[sched.process_num],NULL);
            //start_RT_TT[sched.process_num] = clock();
            b_RT[sched.process_num] = 1;
            b_TT[sched.process_num] = 1;
        }

        check_process++;

        regular.push(make_pair(sched.pid,sched.process_num));

        if(b_WT[sched.process_num] == -1)
        {
            gettimeofday(&start_WT[sched.process_num],NULL);
            //start_WT[sched.process_num] = clock();
            b_WT[sched.process_num] = 1;

        }

        sched.type = sched.pid;
        sched.pid = getpid();
        if(msgsnd(msqid,&sched,sizeof(my_msgbuf),0)== -1)
        {
            perror("Send Error 134: ");
        }

        while(check_process>0)
        {
            running_pid = regular.front().first;
            running_procnum = regular.front().second;

            if(b_RT[running_procnum]==1)
            {
                gettimeofday(&tt1,NULL);
                RT[running_procnum]=(tt1.tv_sec-start_RT_TT[running_procnum].tv_sec)+1.0*(tt1.tv_usec-start_RT_TT[running_procnum].tv_usec)/1000000;
                //RT[running_procnum] = (clock()-start_RT_TT[running_procnum])/(double) CLOCKS_PER_SEC;
                b_RT[running_procnum] = 2;
            }

            if(b_WT[running_procnum] == 1)
            {
                gettimeofday(&tt1,NULL);
                WT[running_procnum] += (tt1.tv_sec-start_WT[running_procnum].tv_sec)+1.0*(tt1.tv_usec-start_WT[running_procnum].tv_usec)/1000000;
                //WT[running_procnum] += (clock() - start_WT[running_procnum])/(double) CLOCKS_PER_SEC;
                b_WT[running_procnum] = -1;
            }            

            printf("P%d <%d> is running\n",running_procnum,running_pid);

            regular.pop();
            usleep(100);
            kill(running_pid,SIGUSR1);

            for(time_quant=0;time_quant<1000;time_quant++)
            {
                usleep(100);
                int k = msgrcv(msqid,&sched1,sizeof(my_msgbuf),TYPE2,IPC_NOWAIT);
                if(k>0)
                {
                    
                    if(b_RT[sched1.process_num] == -1)
                    {
                        gettimeofday(&start_RT_TT[sched1.process_num] ,NULL);
                        //start_RT_TT[sched1.process_num] = clock();
                        b_RT[sched1.process_num] = 1;
                        b_TT[sched1.process_num] = 1;
                    }

                    if(sched1.pid!=running_pid)
                    {
                        check_process++;
                        regular.push(make_pair(sched1.pid,sched1.process_num));
                        if(b_WT[sched1.process_num] == -1)
                        {
                            gettimeofday(&start_WT[sched1.process_num],NULL);
                            //start_WT[sched1.process_num] = clock();
                            b_WT[sched1.process_num] = 1;
                        }

                        sched1.type = regular.back().first;
                        sched1.pid = getpid();
                        if(msgsnd(msqid,&sched1,sizeof(my_msgbuf),0)==-1)
                        {
                            perror("Send Error 190: ");
                        }
                    }
                }

                if(flag==1)
                {
                    printf("P%d <%d> requests I/O\n",running_procnum,running_pid);
                    flag = -1;
                    waiting.push(make_pair(running_pid,running_procnum));
                    running_pid = -1;
                    running_procnum = -1;
                    break;
                }

                else if(flag ==2)
                {
                    printf("P%d <%d> terminated\n",running_procnum,running_pid);
                    if(b_TT[running_procnum]==1)
                    {
                        gettimeofday(&tt1,NULL);
                        TT[running_procnum]=(tt1.tv_sec-start_RT_TT[running_procnum].tv_sec)+1.0*(tt1.tv_usec-start_RT_TT[running_procnum].tv_usec)/1000000;
                        //TT[running_procnum] = (clock()-start_RT_TT[running_procnum])/(double) CLOCKS_PER_SEC;
                        //printf("%ld    %ld\n", clock(),start_RT_TT[running_procnum]);
                        //printf("%ld\n", CLOCKS_PER_SEC);
                        b_TT[running_procnum] = 2;
                    }

                    flag = -1;
                    running_pid = -1;
                    running_procnum = -1;
                    check_process--;
                    break;
                }
            }

            if(running_pid!= -1)
            {
                if(time_quant==1000)
                {
                    regular.push(make_pair(running_pid,running_procnum));
                    if(b_WT[running_procnum] == -1)
                    {
                        gettimeofday(&start_WT[running_procnum],NULL);
                        //start_WT[running_procnum] = clock();
                        b_WT[running_procnum] = 1;
                    }

                    kill(running_pid,SIGUSR2);
                }
            }

            printf("%lu\n",waiting.size());
            if(regular.empty() and waiting.empty())
            {
                printf("No process found | Scheduler exiting\n");
               
                FILE *ptr;
                ptr = fopen("a.txt","a");
                fprintf(ptr, "****** Regular Round Robin Manner ******\n\n");

                fprintf(ptr, "Process No.\tResponse Time\tTurn-Around Time\tWaiting Time\n");
                for(int i=1;i<1000;i++)
                {
                    if(b_RT[i]==2)
                    {
                        fprintf(ptr, "\t%d\t\t%lf\t\t%lf\t\t\t%lf\n",i,RT[i],TT[i],WT[i]);
                    }
                    else
                        break;

                }
                fprintf(ptr, "\n");
                fclose(ptr);
                return 0;
            }

            if(!regular.empty())
            {
                if (msgrcv(msqid, &sched, sizeof(my_msgbuf),1, IPC_NOWAIT) != -1)
                {
                    if(strcmp(sched.text,"I/O Complete")==0)
                    {
                        while(!waiting.empty())
                        {
                            if(waiting.front().first==sched.pid)
                            {
                                printf("P%d <%d> completed I/O\n",waiting.front().second,sched.pid);
                                regular.push(make_pair(waiting.front().first,waiting.front().second));
                                if(b_WT[waiting.front().second] == -1)
                                {
                                    gettimeofday(&start_WT[waiting.front().second],NULL);
                                    //start_WT[waiting.front().second] = clock();
                                    b_WT[waiting.front().second] = 1;

                                }
                                waiting.pop();
                            }
                            else
                            {
                                temp.push(waiting.front());
                                waiting.pop();
                            }
                        }

                        while(!temp.empty())
                        {
                            waiting.push(temp.front());
                            temp.pop();
                        }
                    }
                }
            }

            else
            {
                
                int flag2=0;

                while(1)
                {
                    
                    if(flag2==1) break;

                    if (msgrcv(msqid, &sched, sizeof(my_msgbuf),1, IPC_NOWAIT) != -1)
                    {
                        
                        if(strcmp(sched.text,"I/O Complete")==0)
                        {
                            while(!waiting.empty())
                            {
                                if(waiting.front().first==sched.pid)
                                {
                                    printf("P%d <%d> completed I/O\n",waiting.front().second,sched.pid);
                                    regular.push(make_pair(waiting.front().first,waiting.front().second));
                                    if(b_WT[waiting.front().second] == -1)
                                    {
                                        gettimeofday(&start_WT[waiting.front().second],NULL);
                                        //start_WT[waiting.front().second] = clock();
                                        b_WT[waiting.front().second] = 1;

                                    }
                                    waiting.pop();
                                }
                                else
                                {
                                    temp.push(waiting.front());
                                    waiting.pop();
                                }
                            }

                            while(!temp.empty())
                            {
                                waiting.push(temp.front());
                                temp.pop();
                            }

                            flag2=1;
                        }
                    }
                }
            }
        }
    }
    else if (check == "PR")
    {
        double RT[1000];
        int b_RT[1000];
        struct timeval start_RT_TT[1000];

        double TT[1000];
        int b_TT[1000];

        double WT[1000];
        int b_WT[1000];
        struct timeval start_WT[1000];

        memset(b_RT,-1,sizeof(b_RT));
        memset(b_TT,-1,sizeof(b_RT));
        memset(b_WT,-1,sizeof(b_RT));
        memset(WT,0,sizeof(WT));


        if (msgrcv(msqid, &sched, sizeof(my_msgbuf), TYPE2, 0) == -1)
        {
           perror("Receive Error 334: ");
        }

        if(b_RT[sched.process_num] == -1 && b_TT[sched.process_num] == -1)   // Response & Turn Around Initial Time
        {
            gettimeofday(&start_RT_TT[sched.process_num],NULL);
            //start_RT_TT[sched.process_num] = clock();
            b_RT[sched.process_num] = 1;
            b_TT[sched.process_num] = 1;
        }

        check_process++;

        regular_PR.push(make_pair(sched.pid,make_pair(sched.priority_info,sched.process_num)));

        if(b_WT[sched.process_num] == -1)
        {
            gettimeofday(&start_WT[sched.process_num],NULL);
            //start_WT[sched.process_num] = clock();
            b_WT[sched.process_num] = 1;

        }

        sched.type = sched.pid;
        sched.pid = getpid();
        if(msgsnd(msqid,&sched,sizeof(my_msgbuf),0)== -1)
        {
            perror("Send Error 344: ");
        }

        while(check_process>0)
        {
            running_pid = regular_PR.top().first;
            running_procnum = regular_PR.top().second.second;
            running_priority = regular_PR.top().second.first;

            if(b_RT[running_procnum] == 1)
            {
                gettimeofday(&tt1,NULL);
                RT[running_procnum]=(tt1.tv_sec-start_RT_TT[running_procnum].tv_sec)+1.0*(tt1.tv_usec-start_RT_TT[running_procnum].tv_usec)/1000000;
                //RT[running_procnum] = (clock()-start_RT_TT[running_procnum])/(double) CLOCKS_PER_SEC;
                b_RT[running_procnum] = 2;
            }

            if(b_WT[running_procnum] == 1)
            {
                gettimeofday(&tt1,NULL);
                WT[running_procnum] += (tt1.tv_sec-start_WT[running_procnum].tv_sec)+1.0*(tt1.tv_usec-start_WT[running_procnum].tv_usec)/1000000;
                //WT[running_procnum] += (clock()-start_WT[running_procnum])/(double) CLOCKS_PER_SEC;
                b_WT[running_procnum] = -1;
            }
            printf("P%d <%d> is running\n",running_procnum,running_pid);

            regular_PR.pop();
            usleep(100);
            kill(running_pid,SIGUSR1);

            for(time_quant=0;time_quant<2000;time_quant++)
            {
                usleep(100);
                int k = msgrcv(msqid,&sched1,sizeof(my_msgbuf),TYPE2,IPC_NOWAIT);

                if(k>0)
                {
                    

                    if(b_RT[sched1.process_num] == -1 && b_TT[sched1.process_num] == -1)   // Response & Turn Around Initial Time
                    {
                        gettimeofday(&start_RT_TT[sched1.process_num],NULL);
                        //start_RT_TT[sched.process_num] = clock();
                        b_RT[sched1.process_num] = 1;
                        b_TT[sched1.process_num] = 1;
                    }

                    if(sched1.pid!=running_pid)
                    {
                        check_process++;
                        regular_PR.push(make_pair(sched1.pid,make_pair(sched1.priority_info,sched1.process_num)));
                        if(b_WT[sched1.process_num] == -1)
                        {
                            gettimeofday(&start_WT[sched1.process_num],NULL);
                            //start_WT[sched1.process_num] = clock();
                            b_WT[sched1.process_num] = 1;

                        }
                        sched1.type = sched1.pid;
                        sched1.pid = getpid();
                        if(msgsnd(msqid,&sched1,sizeof(my_msgbuf),0)==-1)
                        {
                            perror("Send Error 265: ");
                        }
                    }
                }

                if(flag==1)
                {
                    printf("P%d <%d> requests I/O\n",running_procnum,running_pid);
                    flag = -1;
                    waiting_PR.push(make_pair(running_pid,make_pair(running_priority,running_procnum)));
                    running_pid = -1;
                    running_procnum = -1;
                    break;
                }

                else if(flag ==2)
                {
                    if(b_TT[running_procnum] == 1)
                    {
                        gettimeofday(&tt1,NULL);
                        TT[running_procnum]=(tt1.tv_sec-start_RT_TT[running_procnum].tv_sec)+1.0*(tt1.tv_usec-start_RT_TT[running_procnum].tv_usec)/1000000;
                        //TT[running_procnum] = (clock()-start_RT_TT[running_procnum])/(double) CLOCKS_PER_SEC;
                        b_TT[running_procnum] = 2;
                    }
                    printf("P%d <%d> terminated\n",running_procnum,running_pid);
                    flag = -1;
                    running_pid = -1;
                    running_procnum = -1;
                    check_process--;
                    break;
                }
            }

            if(running_pid!= -1)
            {
                if(time_quant==2000)
                {
                    
                    regular_PR.push(make_pair(running_pid,make_pair(running_priority,running_procnum)));
                    if(b_WT[running_procnum] == -1)
                    {
                        gettimeofday(&start_WT[running_procnum],NULL);
                        //start_WT[running_procnum] = clock();
                        b_WT[running_procnum] = 1;

                    }
                    kill(running_pid,SIGUSR2);
                }
            }

            if(regular_PR.empty() and waiting_PR.empty())
            {
                printf("No process found | Scheduler exiting\n");
               
                FILE *ptr;
                ptr = fopen("a.txt","a");
                fprintf(ptr, "****** Priority Round Robin Manner ******\n\n");

                fprintf(ptr, "Process No.\tResponse Time\tTurn-Around Time\tWaiting Time\n");
                for(int i=1;i<1000;i++)
                {
                    if(b_RT[i]==2)
                    {
                        fprintf(ptr, "\t%d\t\t%lf\t\t%lf\t\t\t%lf\n",i,RT[i],TT[i],WT[i]);
                    }
                    else
                        break;

                }
                fprintf(ptr, "\n");
                fclose(ptr);
                return 0;
            }

            if(!regular_PR.empty())
            {
                if (msgrcv(msqid, &sched, sizeof(my_msgbuf),1, IPC_NOWAIT) != -1)
                {
                    if(strcmp(sched.text,"I/O Complete")==0)
                    {
                        while(!waiting_PR.empty())
                        {
                            if(waiting_PR.top().first==sched.pid)
                            {
                                printf("P%d <%d> completed I/O\n",waiting.front().second,sched.pid);
                                regular_PR.push(waiting_PR.top());
                                if(b_WT[waiting_PR.top().second.second] == -1)
                                {
                                    gettimeofday(&start_WT[waiting_PR.top().second.second],NULL);
                                    //start_WT[waiting_PR.top().second.second] = clock();
                                    b_WT[waiting_PR.top().second.second] = 1;

                                }
                                waiting_PR.pop();
                            }
                            else
                            {
                                temp_PR.push(waiting_PR.top());
                                waiting_PR.pop();
                            }
                        }

                        while(!temp_PR.empty())
                        {
                            waiting_PR.push(temp_PR.top());
                            temp_PR.pop();
                        }
                    }
                }
            }

            else
            {
                int flag2=0;

                while(1)
                {
                    if(flag2==1) break;

                    if (msgrcv(msqid, &sched, sizeof(my_msgbuf),1, IPC_NOWAIT) != -1)
                    {
                        if(strcmp(sched.text,"I/O Complete")==0)
                        {
                            while(!waiting_PR.empty())
                            {
                                if(waiting_PR.top().first==sched.pid)
                                {
                                    printf("P%d <%d> completed I/O\n",waiting_PR.top().second.second,sched.pid);
                                    regular_PR.push(waiting_PR.top());
                                    if(b_WT[waiting_PR.top().second.second] == -1)
                                    {
                                        gettimeofday(&start_WT[waiting_PR.top().second.second],NULL);
                                        //start_WT[waiting_PR.top().second.second] = clock();
                                        b_WT[waiting_PR.top().second.second] = 1;

                                    }
                                    waiting_PR.pop();
                                }
                                else
                                {
                                    temp_PR.push(waiting_PR.top());
                                    waiting_PR.pop();
                                }
                            }

                            while(!temp_PR.empty())
                            {
                                waiting_PR.push(temp_PR.top());
                                temp_PR.pop();
                            }

                            flag2=1;
                        }
                    }
                }
            }
        }
    }
}
