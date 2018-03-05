// c++ based server
// take tcp based connection from ios app
// will implemented as multi thread server
// port need
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <thread>
#include <iostream>
// #include <stdio.h>
#include <stdlib.h>
// #include <unistd.h>
// #include <netinet/in.h>
// #include <signal.h>
// #include <string.h>

using namespace std;

class Server{

      int portNum;
      struct sockaddr_in server;
      struct sigaction act;


      pthread_t threads[3];   // only 2 camera
      int childsockfd[3];
      int parentsockfd;
      int threadCount = 0;

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
            uint8_t x[4];
            while( recv(p->childsockfd[threadIdx], x, 4, 0) > 0 )
            {
                  cout << threadIdx<<": ";
                  printf("received 4 bytes: %d  %d  %d  %d", x[0], x[1], x[2], x[3]);
                  printf("  received int32: %d \n", ((((((x[0]<<8)+x[1])<<8)+x[2])<<8)+x[3]));
            }

            /* when client is no longer sending information to us, */
            /* the socket can be closed and the child process terminated */
            close(p->childsockfd[threadIdx]);
            cout << "thread #"<< threadIdx <<" terminated \n";
            return ptr;
      }
public:
      Server(int port): portNum(port){};
      ~Server(){
            cout << "server decomposed\n";
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
                  cout << "server: socket() call failed!\n";
                  return;
            }
            /* bind a specific address and port to the end point */
            if( bind(parentsockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in) ) == -1 ){
                  cout << "server: bind() call failed!\n";
                  return;
            }
            /* start listening for incoming connections from clients */
            if( listen(parentsockfd, 5) == -1 ){
                  cout << "server: listen() call failed!\n";
                  return;
            }

            pthread_create(&threads[0], NULL, subThread, (void *) this);
            pthread_join(threads[0], NULL);
            // cout << "server listening on port "<< portNum <<"...\n\n";
            // cout << "server main thread quit\n";
      }
};
int main(){
      Server sv(2225);
      sv.start();
      return 0;
}
