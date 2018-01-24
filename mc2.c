/*========Mid Day Commander, V2==========
*=============Itteration: V2=============
*==Authors: Matthew Hagan & Trevor Dowd==
*/
//include statements

#include <stdio.h> 
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>

void processNumCommands(int cmd);
void processCharCommands(char cmd);
static int numID = 3;  // id of the command
static int bCount = 0;

struct bNode {
  struct timeval start;
  int pid;
  int cmdID;
  struct bNode *next;
};

struct cmdNode {//structure for the cmdNode
  char *name;
  int id;
  int length;
  struct cmdNode *next;
  int isBackground;
};


// initialize LL
struct cmdNode *cHead = NULL;
struct bNode *bHead = NULL;

// adds a process to the background process list
void addBprocess(int pid, struct timeval start, int cmdID) {
    struct bNode * current = bHead;
    while (current->next != NULL) {
        current = current->next;
    }//end of while
    current->next = malloc(sizeof(struct bNode));
    current->next->pid = pid;
    current->next->cmdID = cmdID;
    current->next->start = start;
    // set next to NULL
    current->next->next = NULL;
}//end of addToList


// removes a process to the background process list
void removeBprocess(int gcPID) {
    int found = 0;
    struct bNode * current = bHead;
    while (current->next != NULL) {
      current = current->next;
      if(current->pid == gcPID){
        found = 1;
      }
      // its in the middle
      if(found && current->next != NULL){
        current->pid = current->next->pid;
        current->cmdID = current->next->cmdID;
        current->start = current->next->start;
        // current->next = current->next->next;
      }
      // its at the end of the list
      else if(found){
        free(current);
        current = NULL;
      }
    }//end of while
} // end of remove


/**
addToList function
  pushes an element onto a linked list for adding a command
*/    
void addToList(int id, char *name, int length, int isBackground) {
    struct cmdNode * current = cHead;
    while (current->next != NULL) {
        current = current->next;
    }//end of while
    current->next = malloc(sizeof(struct cmdNode));
    current->next->id = id;
    current->next->isBackground = isBackground;
    current->next->name = (char *)malloc(sizeof(char *) * length);
    for(int i=0; i<length; i++){
      current->next->name[i] = name[i];
    }//end of for
    current->next->next = NULL;
}//end of addToList

void deleteList(){
  struct cmdNode * temp;
  struct cmdNode * current = cHead;
  while(current != NULL){
    temp = current;
    current = current->next;
    free(temp->name);
    free(temp);
  }//end of while
}

// executes command
void executeCmd(struct cmdNode *current){
  char *str;
  str = current->name;
  char *token;
  token = strtok(str, " ");
  
  // if theres nothing
  if(!token){
    printf("User command isn't vaild.\n"); 
  }  
  // adding tokens to an array
  int i = 0;
  char *argv[32];
  while(token != NULL){
      argv[i] = token;  // adds a token to array
      token = strtok(NULL, " "); // advances to the next token
      i++;
  }
  
  if(current->isBackground == 1){
    argv[i-1] = NULL;
  } else{
    argv[i] = NULL;
  }
  
  //command
  if(execvp(argv[0], argv) == -1)
  {
    printf("User Command isn't valid.\n");
    exit(0);
  }
}

void logOff(){
  while(bCount > 0){
    printf("Background Commands are still running\n");
    wait(0);
  }
    printf("Logging you out, Commander\n");
    deleteList();
    exit(0);
}

int isBackGround(int cmd){
  // returns boolean is command
  struct cmdNode * current = cHead; //
  while(current->next != NULL) 
  {
    if(current->next->isBackground){
      return 1;
    } else{
      current = current->next;
    }
  } // end while
  return 0;
}



void printStart(){
  //intial statements giving the commander options on what to do 
      printf("===== Mid-Day Commander, v2 =====\n G’day, Commander! What command would you like to run?");
      printf("\n0. whoami : Prints out the result of the whoamicommand \n");
      printf("1. last : Prints out the result of the last command");
      printf("\n2. ls : Prints out the result of a listing on a user-specified path \n");
      struct cmdNode * current = cHead; //
       while(current->next != NULL) 
      {
        current = current->next;
        printf("%d", current->id);
        printf(". ");
        printf("%s", current->name);
        printf(": User added Command \n");
      }
      printf("a. add command: Adds a new command to the menu.\n");
      printf("c. change directory : Changes process working directory\n");
      printf("e. exit : Leave Mid-Day Commander\n");
      printf("p. pwd : Prints working directory\n");
      printf("r. running processes : Prints a list of running processes\n");

}

void printBR(){
    struct bNode * current = bHead;
    while (current->next != NULL) {
      printf("fuckbois\n");
      current = current->next;
      printf("\nPID: ");
      printf("%d", current->pid);
    }
}

void printBRComplete(int gcPID, struct timeval bpstart){
  printf("Background Process Complete: \n");
  printf("\nPID: ");
  printf("%d\n", gcPID);
  struct timeval bpfinish;
  struct rusage bpPages;
  gettimeofday(&bpfinish, 0);
  getrusage(RUSAGE_SELF, &bpPages);
  long bpPageFaults = bpPages.ru_majflt;
  long bpPageFaultsReclaimed = bpPages.ru_minflt;
  printf("-- Statistics --- \n Elapsed time:");
  long time = (bpfinish.tv_sec-bpstart.tv_sec) + bpfinish.tv_usec-bpstart.tv_usec;//time value
  printf("%ld", time);
  printf(" milliseconds \n Page Faults: ");
  printf("%ld", bpPageFaults);
  printf("\n Page Faults (reclaimed): ");
  printf("%ld \n", bpPageFaultsReclaimed);
}

int main(){
  
  cHead = (struct cmdNode*)malloc(sizeof(struct cmdNode));
  bHead = (struct bNode*)malloc(sizeof(struct bNode));
  
  while(1){
    fflush(stdin);
    printStart();
    char *optn = malloc(sizeof(char)*80);
    size_t buffer = 80;
    int na = getline(&optn, &buffer, stdin);
    if(na ==-1)
        logOff();
      optn[na-1] = '\0';
      char order = optn[0];
      int numOrder;
      if(isdigit(order)){
        numOrder = atoi(optn);
        free(optn);
        if(numOrder<numID)
          processNumCommands(numOrder);
        else
          printf("This option is currently not available. Please try again.\n");
      }
      else{
        free(optn);
        processCharCommands(order);
      }
      

      
    
  } // while loop
  return 0;

} // main

void processCharCommands(char order){
  switch(order){
        case 'a':
          printf("Command to Add:");
          char *cmd = malloc(sizeof(char)*128);
          size_t buffer = 128;
          int na = getline(&cmd, &buffer, stdin);
          if(na == -1)
            logOff();
          cmd[na-1] = '\0';
          //add the command to list
          addToList(numID, cmd, strlen(cmd),cmd[na-2]=='&');
          printf("Okay commander, added with ID: %d\n", numID);
          numID++;//increments numID
          free(cmd);
          break;
        case 'c':
           //fflush(stdin);
          printf("--Change Directory--\nNew Directory?:");
          char *c = malloc(sizeof(char)*80);
          size_t buf = 80;
          int na2 = getline(&c, &buf, stdin);
          if(na2 == -1)
            logOff();
          c[na2-1] = '\0';
          chdir(c);
          free(c);
          break;
        case 'e':
          logOff();
          break;
        case 'p':
          printf("--CurrentDirectory--\nDirectory: ");
          char currentDirectory[2048];
          getcwd(currentDirectory, 2048);
          printf("%s \n", currentDirectory);
          break;
        case 'r':
          printf("--Background Processes Running--\n");
          printBR();
          break;
        default:
            printf("This option is currently not available. Please try again.\n");

      }
  printf("\n");// seperating midday output
}

void processNumCommands(int order){
  struct cmdNode * current = cHead;
  int pos = order - 2;
  // get the position of the command in the LL
  int i = 0;
  while(i < pos){
    current = current->next;
    i++;
  }
  struct timeval tstart, tfinish;//struct data for time
  struct rusage pages;//struct data for usage
  int cPID;
  gettimeofday(&tstart, 0);
  if((current->isBackground==0)  && ((cPID = fork()) != 0)){ // not backgroud process
        if(cPID)// parent
        {
          
          
          wait(0);// wait command for process holding
          gettimeofday(&tfinish, 0);
          getrusage(RUSAGE_SELF, &pages);
          long pageFaults = pages.ru_majflt;
          long pageFaultsReclaimed = pages.ru_minflt;
          printf("-- Statistics --- \n Elapsed time:");
          long time = (tfinish.tv_sec-tstart.tv_sec) + tfinish.tv_usec-tstart.tv_usec;//time value
          printf("%ld", time);
          printf(" milliseconds \n Page Faults: ");
          printf("%ld", pageFaults);
          printf("\n Page Faults (reclaimed): ");
          printf("%ld \n", pageFaultsReclaimed);
          //get reclaimed page faults
        } else {// child
             
            if(order== 0)//who am i command
            {
              
              printf("-- Who Am I? -- \n");

              //command parts
              char *cmd = "whoami";
              char *argv[2];
              argv[0] = "whoami";
              argv[1] = NULL;

              //command
              execvp(cmd, argv);

            }
            else if(order== 1)//last command
            {
              printf("-- Last Logins -- \n");

              //command parts
              char *cmd = "last";
              char *argv[2];
              argv[0] = "last";
              argv[1] = NULL;

              //command
              execvp(cmd, argv); 
            }
            else if(order==2)//directory listing command
            {
              printf("-- Directory Listings -- \n");

              //command parts
              char *ext = malloc(sizeof(char)*80);
              char *path = malloc(sizeof(char)*80);
              
              printf("\nArguments?: ");
              size_t buffer = 80;
              int na = getline(&ext, &buffer, stdin);
              if(na == -1)
                logOff();
              ext[na-1] = '\0';
              
              printf("\nPath?: ");
              size_t buffer2 = 80;
              int na2 = getline(&path, &buffer2, stdin);
              if(na2 == -1)
                logOff();
              path[na2-1] = '\0';
                          
              
              char *cmd = "ls";
              char *argv[4];
              argv[0] = "ls";
              argv[1] = ext;
              argv[2] = path;
              argv[3] = NULL;
              
              //command
              execvp(cmd, argv);
              free(ext);
              free(path);
            }//else if 
            else{ 
              // execute command
              executeCmd(current);
            }//else
        }
 } else{ // background process
      if(cPID)
      {
        int statusParent;
        struct rusage brusageParent;
        wait3(&statusParent, WNOHANG, &brusageParent);
      } // if
      else if(cPID == 0) // child
      {
        int status;
        struct rusage brusage;
        struct timeval bpstartTime;
        int gcPID;
        if((gcPID = fork()) != 0){  // child
          addBprocess(gcPID,tstart,current->id);
          gettimeofday(&bpstartTime, 0);
          bCount++;
          printf("-- Command: %s --", current->name);
          printf("[%d] %d /* indicate background task #1 and process id */", bCount, gcPID);
          printf("\n first motherfuckers\n");
          wait3(&status, 0, &brusage);  // wait for grandchild process to finish executing
          printf("\n third better this be\n");
          printBRComplete(gcPID, bpstartTime);
          bCount = bCount - 1;
          removeBprocess(gcPID);
          exit(0);

        } // end if
        else { // grandchild
          printf("\nthis better be second\n");
          executeCmd(current);
   // running in background
        } // end else
      } // else if
    } // end else
}
        