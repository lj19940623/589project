// c++ based server obj
// take byte stream data based on tcp connection from ios app sides
// will implemented as multi thread server with mutex
// adapted from cpsc441 boli-code
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <thread>
#include <iostream>
#include <stdlib.h>

#include "generalHeader.h"

using namespace std;

class Server{

      int portNum;
      struct sockaddr_in server;
      struct sigaction act;

      pthread_t threads[3];   // only 2 camera
      int childsockfd[3];
      int parentsockfd;
      int threadCount = 0;
      pthread_mutex_t mutex[3];
      int xy[6];

      /* This is a signal handler to do graceful exit if needed */
      static void catcher( int sig ) {
            // close(childsockfd[0]);
            // close(childsockfd[1]);
            exit(0);
      }

      static void * subThread(void * ptr){
            Server * p = (Server *)ptr;
            int threadIdx = p->threadCount++;

            cout << "thread #"<< threadIdx <<" running \n";
            /* accept a connection */
            if( (p->childsockfd[threadIdx] = accept(p->parentsockfd, NULL, NULL)) == -1 )
            {
                  cout << "server: accept() call failed!\n";
                  return ptr;
            }
            pthread_create(&p->threads[p->threadCount], NULL, subThread, (void *) ptr);
            /* obtain the message from this client */
            uint8_t x[8];
            while( recv(p->childsockfd[threadIdx], x, 8, 0) > 0 )
            {
                  // cout << threadIdx<<": ";
                  // printf("received 8 bytes: %d  %d  %d  %d,  %d  %d  %d  %d", x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7]);
                  // printf("received 2*int32: %d, %d \n", ((((((x[0]<<8)+x[1])<<8)+x[2])<<8)+x[3]), ((((((x[4]<<8)+x[5])<<8)+x[6])<<8)+x[7]));
                  p->setXY(threadIdx,((((((x[0]<<8)+x[1])<<8)+x[2])<<8)+x[3]), ((((((x[4]<<8)+x[5])<<8)+x[6])<<8)+x[7]) );
            }

            /* when client is no longer sending information to us, */
            /* the socket can be closed and the child process terminated */
            close(p->childsockfd[threadIdx]);
            cout << "thread #"<< threadIdx <<" terminated \n";
            return ptr;
      }
public:
      Server(int port): portNum(port){
            pthread_mutex_init( & mutex[0], NULL);
            pthread_mutex_init( & mutex[1], NULL);
      };
      ~Server(){
            pthread_mutex_destroy( & mutex[0]);
            pthread_mutex_destroy( & mutex[1]);
            cout << "Server decomposed\n";
            close(parentsockfd);
      }

      void start(){
            /* Set up a signal handler to catch some weird termination conditions. */
            act.sa_handler = catcher;
            sigfillset(&(act.sa_mask));
            sigaction(SIGPIPE, &act, NULL);
            /* Initialize server sockaddr structure */
            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_port = htons(portNum);
            server.sin_addr.s_addr = htonl(INADDR_ANY);
            /* set up the transport-level end point to use TCP */
            if( (parentsockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1 ){
                  cout << "Server: socket() call failed!\n";
                  return;
            }
            /* bind a specific address and port to the end point */
            if( ::bind(parentsockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in) ) == -1 ){
                  cout << "Server: bind() call failed!\n";
                  return;
            }
            /* start listening for incoming connections from clients */
            if( listen(parentsockfd, 5) == -1 ){
                  cout << "Server: listen() call failed!\n";
                  return;
            }
            pthread_create(&threads[0], NULL, subThread, (void *) this);
            // pthread_join(threads[0], NULL);
      }
      void setXY(int index, int x, int y){
            pthread_mutex_lock( & mutex[index]);
            xy[index*2] = x;
            xy[index*2+1] = y;
            pthread_mutex_unlock( & mutex[index]);
      }
      int getXY(int index){
            pthread_mutex_lock( & mutex[index/2]);
            int temp = xy[index];
            pthread_mutex_unlock( & mutex[index/2]);
            return temp;
      }
      vec3 get3DCoor(){
            return vec3((float)getXY(1)/1080,(float)(1.0f-getXY(3)/1080.0f),(float)getXY(0)/1920);
      }
};
// int main(){
//       Server sv(2225);
//       sv.start();
//       for(int t = 0; t<200000; t++){
//             usleep(10);
//             cout << "this is main thread t="<<t<<endl;
//             cout <<" : with x = "<<sv.getXY(0)<<" with y = "<<sv.getXY(1)<<endl;
//             cout <<" : with x = "<<sv.getXY(2)<<" with y = "<<sv.getXY(3)<<endl;
//       }
//       cout << "end of main thread and subthread will be terminated\n";
//       return 0;
// }
