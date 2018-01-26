/*========Mid Day Commander, V2==========
*=============Itteration: V2=============
*====
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

void processBasicCmds(int cmd);
void processCustomCommands(int cmd);
void processCharCommands(char cmd);
static int numID = 3;  // id of the command
static int bCount = 0;

struct bNode {
  int jobNumber;
  char *cmdName;
  int ePid;//pid of grandchild "executing" process
  int wPid;//pid of child "waiting" process
  struct timeval start;
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
void addBprocess(int wPid,int ePid, struct cmdNode *n,struct timeval start) {
    struct bNode *current = bHead;
    while (current->next != NULL) {
        current = current->next;
    }//end of while
    current->next = malloc(sizeof(struct bNode));
    current->next->wPid = wPid;
    current->next->ePid = ePid;
    current->next->cmdID = n->id;
    current->next->jobNumber = ++bCount;
    current->next->cmdName = n->name;
    current->next->start = start;
    // set next to NULL
    current->next->next = NULL;
}//end of addToList

struct bNode *getBprocess(int PID){
  struct bNode * current = bHead;
    while (current->next != NULL) {
      current = current->next;
      if(current->wPid == PID){
        return current;
      }
    }
    printf("process not found");
    return NULL;
}

// removes a process to the background process list prints job number and PID stats
void completeBprocess(int PID) {
    int found = 0;
    struct bNode * current = bHead;
    struct bNode * trash = NULL;
    while (current->next != NULL) {
      if(current->next->wPid == PID){
        trash = current->next;
        if(trash->next != NULL){
          current->next = current->next->next;
        }else{
          current->next = NULL;
        }
        free(trash);
        found++;
        bCount--;
        if(current==NULL||current->next==NULL)
          return;
      }
      current = current->next;
      current->jobNumber-=found;
      
    }
    if(!found)
      printf("PID not in list\n");
} // end of remove


/**
addToList function
  pushes an element onto a linked list for adding a command
*/    
void addToList(int id, char *name, int length) {
    struct cmdNode * current = cHead;
    while (current->next != NULL) {
        current = current->next;
    }//end of while
    current->next = malloc(sizeof(struct cmdNode));
    current->next->id = id;
    current->next->isBackground = 0;
    current->next->length = length;
    if(name[length-1]=='&'){
      printf("b\n");
      current->next->isBackground = 1;
    }
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
int executeCmd(struct cmdNode *current){
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
  argv[i-current->isBackground] = NULL;
  //command

  if(execvp(argv[0], argv) == -1)
  {
    printf("User Command isn't valid.\n");
    return -1;
  }
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
    printf("\n");
    while (current->next != NULL) {
      current = current->next;
      printf("--Command: %s --\n",current->cmdName);
      printf("[%d]PID: %d\n",current->jobNumber,current->ePid);
    }
    printf("\n");
}

void runStats(struct timeval tstart){
  struct timeval tfinish;
  struct rusage pages;
  // wait command for process holding
  gettimeofday(&tfinish, 0);
  getrusage(RUSAGE_CHILDREN, &pages);
  long pageFaults = pages.ru_majflt;
  long pageFaultsReclaimed = pages.ru_minflt;
  printf("== Statistics == \n Elapsed time:");
  long time = (tfinish.tv_sec-tstart.tv_sec)*1000 + (tfinish.tv_usec-tstart.tv_usec)/1000;//time value
  printf("%ld", time);
  printf(" milliseconds \n Page Faults: ");
  printf("%ld", pageFaults);
  printf("\n Page Faults (reclaimed): ");
  printf("%ld \n", pageFaultsReclaimed);
  //get reclaimed page faults 
}

void runBStats(int PID){
  struct bNode *process = getBprocess(PID);
  if(process){
    completeBprocess(PID);
    runStats(process->start);
  }
}

void logOff(){
  while(bCount > 0){
    printf("Background Commands are still running\n");
    runBStats(wait(0));
  }
  printf("Logging you out, Commander\n");
  deleteList();
  exit(0);
}

void purgeBList(){
  int status;
  struct bNode *current=bHead;
  while(current->next!=NULL){
    current = current->next;
    int pid = waitpid(current->wPid,&status,WNOHANG);
    if(pid>0){
      completeBprocess(pid);
    }
  }

}

int main(){
  
  cHead = (struct cmdNode*)malloc(sizeof(struct cmdNode));
  bHead = (struct bNode*)malloc(sizeof(struct bNode));
  
  while(1){
    fflush(stdin);
    int status;
    purgeBList();
    printStart();
    char *optn = NULL;
    size_t buffer = 80;
    purgeBList();
    int na = getline(&optn, &buffer, stdin);
    purgeBList();
    if(na ==-1)
        logOff();
    optn[na-1] = '\0';
    char order = optn[0];
    int numOrder;
    if(isdigit(order)){
        numOrder = atoi(optn);
        if(numOrder<numID){
          if(numOrder<3)
            processBasicCmds(numOrder);
          else
            processCustomCommands(numOrder);
        }
        else{
          printf("This option is currently not available. Please try again.\n");
        }
      }
      else{
        processCharCommands(order);
      }
      free(optn);

  } // while loop
  return 0;
} // main

void processCharCommands(char order){
  switch(order){
        case 'a':
          printf("Command to Add:");
          char *cmd = NULL;
          size_t buffer = 140;
          int na = getline(&cmd, &buffer, stdin);
          if(na == -1)
            logOff();
          cmd[na-1] = '\0';
          //add the command to list
          addToList(numID, cmd, strlen(cmd));
          printf("Okay commander, added with ID: %d\n", numID);
          numID++;//increments numID
          free(cmd);
          break;
        case 'c':
           //fflush(stdin);
          printf("--Change Directory--\nNew Directory?:");
          char *c = NULL;
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
          purgeBList();
          printBR();
          break;
        default:
            printf("This option is currently not available. Please try again.\n");

      }
  printf("\n");// seperating midday output
}

//v0 functionality
void processBasicCmds(int command){
   if(fork() != 0)
    {
      struct timeval ts, tf;
      struct rusage pages;
      gettimeofday(&ts, 0);
      wait(0);
      gettimeofday(&tf, 0);
      getrusage(RUSAGE_SELF, &pages);
      long pageFaults = pages.ru_majflt;
      long pageFaultsReclaimed = pages.ru_minflt;
      printf("-- Statistics --- \n Elapsed time:");
        long time = (tf.tv_sec-ts.tv_sec) + tf.tv_usec-ts.tv_usec;
        printf("%ld", time);
        printf(" milliseconds \n Page Faults: ");
        printf("%ld", pageFaults);
        printf("\n Page Faults (reclaimed): ");
        printf("%ld \n", pageFaultsReclaimed);
        //get reclaimed page faults
    } else {
      if(command==0)
      {
        printf("-- Who Am I? -- \n"); 
        char *cmd = "whoami";
        char *argv[2];
        argv[0] = "whoami";
        argv[1] = NULL;
        execvp(cmd, argv);
      }
      else if(command==1)
      {
        printf("-- Last Logins -- \n");
        char *cmd = "last";
        char *argv[2];
        argv[0] = "last";
        argv[1] = NULL;
        execvp(cmd, argv);
      }
      else if(command==2)
      {
        printf("-- Directory Listings -- \n");
        char ext, path;
        printf("\nArguments?: ");
        scanf("%s", &ext);
        printf("\nPath?: ");
        scanf("%s", &path);
        char *cmd = "ls";
        char *argv[4];
        argv[0] = "ls";
        argv[1] = &ext;
        argv[2] = &path;
        argv[3] = NULL;
        execvp(cmd, argv);
      }
    }
}

static int validCMD = 1;

static void invalidCMD(int sign){
  validCMD = 0;
} 

void processCustomCommands(int order){
  struct timeval tstart;//struct data for time
  struct cmdNode * current = cHead;
  signal(SIGINT,invalidCMD);
  int status;
  int fdc[2];
  if(pipe(fdc)==-1){
    fprintf(stderr, "Pipe Failed");
  }
  int gcPID = -1;
  int pos = order - 2;
  // get the position of the command in the LL
  int i = 0;
  while(i < pos){
    current = current->next;
    i++;
  }
  int cPID = fork();
  gettimeofday(&tstart, 0);
   // not backgroud process
  if(cPID==0 && current->isBackground)
    gcPID = fork();

  if((current->isBackground==0&&cPID != 0)||gcPID>0){//watcher process normally main parent
      if(gcPID>0){                                   //if background watcher child
        close(fdc[0]);
        write(fdc[1],&gcPID,sizeof(gcPID));//piping info to main process
      }
      //close(fde[1]);
      if(cPID){
        waitpid(cPID,&status,0);
      }
      else
        waitpid(gcPID,&status,0);

      if(WIFEXITED(status)){
        if(gcPID>0){
          printf("\n--Job Complete [%d] --\n",bCount);
          printf("Command ID: %d\n", current->id);
          printf("Proccess ID: %d\n", gcPID);
          printf("[ Output ]\n");
        }
        runStats(tstart);
      }
      if(cPID==0){
        if(validCMD)
          exit(0);
        else
          raise(SIGINT);
      }
    }else if(cPID != 0){
      close(fdc[1]);
      read(fdc[0],&gcPID,sizeof(gcPID));
      if(validCMD){
        addBprocess(cPID,gcPID,current,tstart);
        printf("--Command: %s --\n",current->name);
        printf("[%d]PID: %d\n",bCount,gcPID);
      }
     }else if(current->isBackground==0||gcPID==0){
      executeCmd(current);
      raise(SIGINT);
    }
    validCMD = 1;
} 
        