#include <pthread.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <queue>
#include <stdio.h>
#include <iostream>
#include <fstream>
#define NUMTHRDS 4
#define BUFFERSIZE 3

#define DEBUG 0
#define IFBUG if(DEBUG==1){
#define ENDBUG }           

using namespace std;
typedef struct 
{
    int sender;
    int receiver;
    char msg[100];
} msg_container;

typedef struct
{
    int tid;
    string tcontrol;
} thread_arg;

thread_arg targ_array[4];

pthread_t threads[4];
void *status;

/** buffer **/
pthread_mutex_t mutex_buffer; //block everything associated with buffer
queue<msg_container> buffer[4]; //one buff asso to each of the 4 thread(0-3), which is a queue
int count = 0;//count on total no of msg currently in the buffer as a whole

int running_threads = NUMTHRDS;
int blocked_threads = 0;

char thread_status[4];
/** end buffer **/

pthread_cond_t cond_sender; //used when buffer is full and sender has to be blocked so sender waits on it
pthread_cond_t cond_recv[4];

bool check_deadlock(){
    IFBUG cout << "check_deadlock " << blocked_threads << running_threads <<endl; ENDBUG
    if(running_threads==0 || blocked_threads < running_threads) return false;

    int flag = 0;
    for(int i=0;i<NUMTHRDS;i++){
        if(thread_status[i] == 'r'){
            if(buffer[i].empty()) flag = 1;
        }
        if(thread_status[i] == 's'){
            if(count>=BUFFERSIZE) flag = 1;
        }
    }

    if(flag ==1){
        printf("****** deadlock detected ********\n");
        exit(0);
    }
    return false;
}


msg_container recv(int id){
    pthread_mutex_lock (&mutex_buffer);
    IFBUG cout << "*** recv called by " << id <<endl; ENDBUG
    if(buffer[id].empty()){
        blocked_threads++;
        thread_status[id]='r';
        check_deadlock();
        IFBUG cout << "*** wait recv START : " << id << endl; ENDBUG
        //here mutex released
        pthread_cond_wait(&cond_recv[id], &mutex_buffer);
        //here mutex grabbed
        blocked_threads--;
        thread_status[id]='0';
        IFBUG cout << "+++++++ target deadlock : " << id << endl; ENDBUG
        check_deadlock();
        IFBUG cout << "*** wait recv OVER : " << id << endl; ENDBUG
    }
    //get the msg
    msg_container msg = buffer[id].front();
    buffer[id].pop();
    count--;

    printf("*** Message received: %d, %d, %s\n", msg.sender, msg.receiver, msg.msg);

    //free sender(in any)
    pthread_cond_signal(&cond_sender);
    pthread_mutex_unlock (&mutex_buffer);

    return msg;
}

void send(char *message,int receiver, int id){
    pthread_mutex_lock (&mutex_buffer);
    IFBUG cout << "*** " << count <<" send(" << receiver << ") called by " << id <<endl; ENDBUG
    if(count == BUFFERSIZE){
        blocked_threads++;
        thread_status[id]='s';
        check_deadlock();
        IFBUG cout << "*** wait send START : " << id << endl; ENDBUG
        //here mutex released
        pthread_cond_wait(&cond_sender, &mutex_buffer);
        //here mutex grabbed
        thread_status[id]='0';
        blocked_threads--;
        IFBUG cout << "*** wait send OVER : " << id << endl; ENDBUG
    }
    //cout << "*** sent the message from "<< id << "to" << receiver << endl;
    //
    //create and add the msg to buffer
    msg_container msg ;
    strcpy(msg.msg, message);
    msg.sender = id;
    msg.receiver = receiver;
    buffer[receiver].push(msg);
    count++; //stored

    printf("*** Message sent: %d, %d, %s\n", msg.sender, msg.receiver, msg.msg);


    //free receiver concerned (if it was blocked for this)
    pthread_cond_signal(&cond_recv[receiver]);
    pthread_mutex_unlock (&mutex_buffer);
}

void * runner(void *id){ //thread_arg type

    int tid = (int) id;
    char cfile[100];
    sprintf(cfile, "c%d.txt", tid);
    //cout << "tid " << tid << " tcontrol" << cfile <<endl;

    msg_container msg;

    ifstream infile;
    infile.open(cfile);
    int op = -1;
    int rid;
    char message[100];
    while(infile.good()){
        infile>> op;
        if(op == 1){
            infile>>rid;
            infile.getline(message, 100);

            send(message, rid, tid);
            IFBUG printf("###thread %d : Send message to: %d, msg: %s\n", tid, rid, message); ENDBUG
        }
        else if(op == 0){
            msg = recv(tid);
            IFBUG printf("###thread %d : Message received from: %d, msg: %s\n", tid, msg.sender, msg.msg); ENDBUG

            if(msg.receiver != tid) printf("Wrong delivery of msg intended for %d, deliverd to %d\n", msg.receiver, tid);
        }
        op = -1;
    }

    infile.close();


    pthread_mutex_lock (&mutex_buffer);
    running_threads--; //increment no of running threads
    //cout << "decrementing running_threads "<<running_threads << endl;
    check_deadlock();
    pthread_mutex_unlock (&mutex_buffer);

    //pthread_mutex_unlock (&mutex_buffer);
    //cout << " Thread " << tid << "Exits \n"; 
    pthread_exit(0);
}


int main(){
    pthread_mutex_init(&mutex_buffer, NULL);
    pthread_cond_init (&cond_sender, NULL);
    for(int i=0;i<4;i++){
        pthread_cond_init (&cond_recv[i], NULL);
    }
    for(int i=0;i<4;i++){
        targ_array[i].tid = i;
        thread_status[i]='0';
        pthread_create(&threads[i], NULL, runner, (void *) i);
    }

    /* Wait on the other threads */
    for(int i=0;i<4;i++){
      pthread_join(threads[i], &status);
    }
    return 0;
}
