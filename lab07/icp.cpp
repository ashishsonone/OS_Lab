#include <pthread.h>
#include <cstring>
#include <string>
#include <queue>
#include <stdio.h>
#include <iostream>
#include <fstream>
#define NUMTHRDS 4
#define BUFFERSIZE 20 
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
/** end buffer **/

pthread_cond_t cond_sender; //used when buffer is full and sender has to be blocked so sender waits on it
pthread_cond_t cond_recv[4];


msg_container recv(int id){
    pthread_mutex_lock (&mutex_buffer);
    cout << "*** recv called by " << id <<endl;
    if(buffer[id].empty()){
        cout << "*** wait recv START : " << id << endl;
        pthread_cond_wait(&cond_recv[id], &mutex_buffer);
        cout << "*** wait recv OVER : " << id << endl;
    }
    //get the msg
    msg_container msg = buffer[id].front();
    buffer[id].pop();
    count--;

    printf("*** Message received: %d, %d, %s\n", msg.sender, msg.receiver, msg.msg);
    pthread_mutex_unlock (&mutex_buffer);

    //free sender(in any)
    pthread_cond_signal(&cond_sender);

    return msg;
}

void send(char *message,int receiver, int id){
    pthread_mutex_lock (&mutex_buffer);
    cout << "*** send(" << receiver << ") called by " << id <<endl;
    if(count == BUFFERSIZE){
        cout << "*** wait send START : " << id << endl;
        pthread_cond_wait(&cond_sender, &mutex_buffer);
        cout << "*** wait send OVER : " << id << endl;
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

    pthread_mutex_unlock (&mutex_buffer);

    //free receiver concerned (if it was blocked for this)
    pthread_cond_signal(&cond_recv[receiver]);
}

void * runner(void *id){ //thread_arg type
    //pthread_mutex_lock (&mutex_buffer);
    int tid = (int) id;
    char cfile[100];
    sprintf(cfile, "c%d.txt", tid);
    cout << "tid " << tid << " tcontrol" << cfile <<endl;

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
            printf("###thread %d : Send message to: %d, msg: %s\n", tid, rid, message);

            send(message, rid, tid);
        }
        else if(op == 0){
            msg = recv(tid);
            printf("###thread %d : Message received from: %d, msg: %s\n", tid, msg.sender, msg.msg);

            if(msg.receiver != tid) printf("Wrong delivery of msg intended for %d, deliverd to %d\n", msg.receiver, tid);
        }
        op = -1;
    }

    infile.close();

    //pthread_mutex_unlock (&mutex_buffer);
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
        pthread_create(&threads[i], NULL, runner, (void *) i);
    }

    /* Wait on the other threads */
    for(int i=0;i<4;i++){
      pthread_join(threads[i], &status);
    }
    return 0;
}
