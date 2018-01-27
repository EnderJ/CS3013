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
static int exitFlag = 0;
static int inputFlag = 0;
static struct cmdNode *lastCalled = NULL;
int fda[2];
int fdn[2];
struct bNode {
  int jobNumber;
  int pid;
  struct cmdNode * cmd; 
  struct timeval start;
  struct bNode *next;
};

struct cmdNode {//structure for the cmdNode
  char *name;
  int id;
  int length;
  int invalid;
  struct cmdNode *next;
  int isBackground;
};

// initialize LL
struct cmdNode *cHead = NULL;
struct bNode *bHead = NULL;

void addBprocess(int pid,struct cmdNode *n,struct timeval start) {
    struct bNode *current = bHead;
    bCount = 1;
    while (current->next != NULL) {
        current = current->next;
        bCount++;
    }//end of while
    current->next = malloc(sizeof(struct bNode));
    current->next->pid = pid;
    current->next->jobNumber = bCount;
    current->next->cmd = n;
    current->next->start = start;
    // set next to NULL
    current->next->next = NULL;
}//end of addToList

struct bNode *getBprocess(int PID){
  struct bNode * current = bHead;
    while (current->next != NULL) {
      current = current->next;
      if(current->pid == PID){
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
      if(current->next->pid == PID){
        trash = current->next;
        if(trash->next != NULL){
          current->next = current->next->next;
        }else{
          current->next = NULL;
        }
        printf("--Job Complete [%d] --\n",trash->jobNumber);
        printf("Command Name:%s\n",trash->cmd->name);
        printf("Process ID: %d\n",PID);
        free(trash);
        found++;
        bCount--;
        if(current==NULL||current->next==NULL)
          return;
      }
      current = current->next;
      
    }
    if(!found)
      printf("PID not in list\n");
} // end of remove


void addToList(int id, char *name, int length) {
    struct cmdNode * current = cHead;
    while (current->next != NULL) {
        current = current->next;
    }//end of while
    current->next = malloc(sizeof(struct cmdNode));
    current->next->id = id;
    current->next->isBackground = 0;
    current->next->length = length;
    //current->next->invalid = 0;
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
      printf("--Command: %s --\n",current->cmd->name);
      printf("[%d]PID: %d\n",current->jobNumber,current->pid);
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
  exitFlag = 1;
  if(bCount>0)
  	printf("processes still running\n");
}

void purgeBList(){
  int status;
  struct bNode *current=bHead;
  while(current->next!=NULL){
    current = current->next;
    int pid = waitpid(current->pid,&status,WNOHANG);
    if(pid>0){
      runBStats(pid);
    }
  }
}

void getCMDs(int *num, char *chr){
	fflush(stdin);
	char *optn = malloc(sizeof(char)*80);
      size_t buffer = 80;
      int na = getline(&optn, &buffer, stdin);
      if(na ==-1){
        *num = -1;
        *chr = 'e';
      }
      optn[na-1] = '\0';
      char order = optn[0];
      int numOrder;
      if(isdigit(order)){
      	numOrder = atoi(optn);
      }else{
      	numOrder = -1;
      }
      *num = numOrder;
      *chr = order;
}

//void inputDaemon();

int main(){
  cHead = (struct cmdNode*)malloc(sizeof(struct cmdNode));
  bHead = (struct bNode*)malloc(sizeof(struct bNode));

	int iPID = 0;
	int status;
	int numCMD;
	char charCMD;
	printStart();
	while(bCount>0 || !exitFlag){
		int cCount = bCount;
		purgeBList();
		if(exitFlag){
      if(cCount!=bCount)
        printf("%d processes still running\n",bCount);
		}else{
		if(!inputFlag){
			inputFlag = 1;
			pipe(fda);
			pipe(fdn);
			iPID=fork();
			if(iPID==0){
				close(fda[0]);
				close(fdn[0]);
				getCMDs(&numCMD,&charCMD);
				write(fda[1],&charCMD,sizeof(charCMD));
				write(fdn[1],&numCMD,sizeof(numCMD));
        exit(0);
			}
		}
    
    if(inputFlag && iPID > 0 && waitpid(iPID,&status,WNOHANG)){
      close(fda[1]);
      close(fdn[1]);
      read(fda[0],&charCMD,sizeof(charCMD));
      read(fdn[0],&numCMD,sizeof(numCMD));
      inputFlag = 0;
      if(numCMD==-1){
        processCharCommands(charCMD);
      }else{
        if(numCMD<3)
          processBasicCmds(numCMD);
        else if(numCMD<numID)
          processCustomCommands(numCMD);
        else{
          printf("command does not exist");
        }
      }
      inputFlag = 0;
      iPID = -1;
      printStart();
    }
  }
	}

	printf("Logging you out Commander\n");
  free(bHead);
  free(cHead);
}


void processCharCommands(char order){
  purgeBList();
  switch(order){
        case 'a':
          printf("Command to Add:");
          char *cmd = NULL;
          size_t buffer = 140;
          int na = getline(&cmd, &buffer, stdin);
          if(na == -1){
            logOff();
            break;
          }
          if(na>129){
            printf("Command invalid maximum of 128 characters\n");
            return;
          }
          cmd[na-1] = '\0';
          int i = 0;
          char *token;
          while(token != NULL){
              token = strtok(NULL, " "); // advances to the next token
              i++;
              if(i>=32){
                printf("Command invalid maximum of 32 arguments\n");
                return;
              }
          }
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
          if(na2 == -1){
            logOff();
            break;
          }
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

//v0 functionality
void processBasicCmds(int command){

   if(fork() != 0)
    {
      struct timeval ts;
      gettimeofday(&ts, 0);
      wait(0);
      runStats(ts);
      
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

void processCustomCommands(int order){
  struct timeval tstart;//struct data for time
  struct cmdNode * current = cHead;
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
  if(cPID)// parent
  {
    if(current->isBackground==0){
      wait(0);
      runStats(tstart);
    }
    else{
      addBprocess(cPID,current,tstart);
      printf("\n\n-- Command: %s --\n",current->name);
      printf("[%d] %d",bCount,cPID);
      printf("\n\n");
    } 
  } else {// child
        executeCmd(current);
    }
  
} 
