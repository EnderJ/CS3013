/*========Mid Day Commander, V1==========
*=============Itteration: V1=============
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
static int processID;
struct node {//structure for the node
  char *name;
  int id;
  int length;
  int * pIDs;
  bool isBackground;
  struct node *next;
};


// initialize LL
struct node *head = NULL;

/**
addToList function
  pushes an element onto a linked list for adding a command
*/
void addToList(int id, char *name, int length, bool isBackground) {
    struct node * current = head;
    while (current->next != NULL) {
        current = current->next;
    }//end of while
    current->next = malloc(sizeof(struct node));
    current->next->id = id;
    current->next->pIDs = NULL; 
    current->next->isBackground;
    current->next->name = (char *)malloc(sizeof(char *) * length);
    for(int i=0; i<length; i++){
      current->next->name[i] = name[i];
    }//end of for
    current->next->next = NULL;
}//end of addToList

int totalBTaskCount(){
  int total;
  struct node * current = head;
  while(current->next != NULL){
    current = current->next;
    total+=current->bRunCount;
  }
  return total;
}

void deleteList(){
  struct node * temp;
  struct node * current = head;
  while(current != NULL){
    temp = current;
    current = current->next;
    free(temp->name);
    free(temp);
  }//end of while
}
// takes an ID of command + ext... etc.
// executes command
void executeCmd(int commandID){
  //printf("bruh");
  
  struct node * current = head;
  int pos = commandID - 2;
  
  // get the position of the command in the LL
  int i = 0;
  while(i < pos){
    current = current->next;
    i++;
  }
  
  char *str;
  str = current->name;
  //printf("%s", str);
  
  // const char s[128] = " ";
  char *token;
  token = strtok(str, " ");
  //printf("line 80");
  
  // if theres nothing
  if(!token){
    printf("User command isn't vaild."); 
  }
  
  
  // adding tokens to an array
  i = 0;
  char *argv[32];
  while(token != NULL){
      argv[i] = token;  // adds a token to array
      token = strtok(NULL, " "); // advances to the next token
      i++;
  }
  argv[i] = NULL;
  //printf("line 95");
  
  //command
  //printf("line 98");
  if(execvp(argv[0], argv) == -1)
  {
    printf("User Command isn't valid.\n");
    exit(0);
  }
  
}

void logOff(){
  printf("Logging you out, Commander\n");
  deleteList();
  exit(0);
}

void printBRound()
{
      struct node * current = head; //
      while(current->next != NULL) 
      {
        current = current->next;
      }
}

void printStart(){
  //intial statements giving the commander options on what to do 
      printf("===== Mid-Day Commander, v0 =====\n G’day, Commander! What command would you like to run?");
      printf("\n0. whoami : Prints out the result of the whoamicommand \n");
      printf("1. last : Prints out the result of the last command");
      printf("\n2. ls : Prints out the result of a listing on a user-specified path \n");
      struct node * current = head; //
       while(current->next != NULL) 
      {
        current = current->next;
        printf("%d", current->id);
        printf(". ");
        printf("%s", current->name);
        printf(": User added Command \n");
         if(current->isBackground)
           printf("b\n
      }
      printf("a. add command: Adds a new command to the menu.\n");
      printf("c. change directory : Changes process working directory\n");
      printf("e. exit : Leave Mid-Day Commander\n");
      printf("p. pwd : Prints working directory\n");

}


int main(){//main method
  
  head = (struct node*)malloc(sizeof(struct node));
  
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
          printf("--Background Processes--");
          
          printBRound();
          //do shit
          break;
        default:
            printf("This option is currently not available. Please try again.\n");

      }
}

void processNumCommands(int order){
        if(fork() != 0)//checks for parent/child
        {
          //usage data
          struct timeval tstart, tfinish;//struct data for time
          struct rusage pages;//struct data for usage
          gettimeofday(&tstart, 0);
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
        }
        
        else {//else for exectuting commands
             
          
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
                            
              //char ext, path;
              //scanf("%s", &ext);
              //scanf("%s", &path);
              
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
            //{
              // an added command... figure out which one
              // execute command
              //printf("\n line 304\n");
              //printf("%d", order);
              executeCmd(order);

            }//else
      
        }
}