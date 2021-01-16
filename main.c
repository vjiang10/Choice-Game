#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "functions.h"

char currentaddress[256]; // To keep track of what node we're currently at.
int autosave = 0; // If 0, then autosave is off. If 1, then autosave is on.

// Function to save the game.
void saveGame() {
  // If we're at the beginning of the game, don't save.
  if (!strcmp(currentaddress, "0")) printf("No need to save. You're at the beginning!\n");

  // Otherwise, save.
  else {
    printf("Saved Game!\n");
    int fd = open("savefile.txt", O_WRONLY);
    write(fd, currentaddress, sizeof(currentaddress));
    close(fd);
  }
  printf("Input choice #: ");
}

// Function to autosave. Basically saveGame() but without the text prompts. (It happens in the background).
void autoSave() {
  // If we're not at the beginning of the game, autosave.
  if (strcmp(currentaddress, "0")) {
    int fd = open("savefile.txt", O_WRONLY);
    write(fd, currentaddress, sizeof(currentaddress));
    close(fd);
  }
}

// Function to prompt the player if they want to enable autosave. Enables autosave if desired.
void promptAutosave() {
  char promptResponse[256];
  fgets(promptResponse, sizeof(promptResponse), stdin);

  // If the player types anything other than 'y' or 'n'
  if (strcmp(promptResponse, "y\n")&&strcmp(promptResponse, "n\n")) {
    while (strcmp(promptResponse, "y\n")&&strcmp(promptResponse, "n\n")) {
      printf("Invalid input. Type 'y' or 'n'.\n");
      fgets(promptResponse, sizeof(promptResponse), stdin);
    }
  }

  // If the player types 'y', enable autosave.
  if (promptResponse[0]=='y') {
    autosave = 1;
    printf("Autosave enabled.\n\n");
  }

  // If the player types 'n', disable autosave.
  if (promptResponse[0]=='n') {
    printf("Autosave disabled.\n\n");
  }
}

// Function to prompt the player if they want to load a savefile.
void promptLoadfile(char *buffer, char *buffer2) {
  char promptResponse[256];
  fgets(promptResponse, sizeof(promptResponse), stdin);

  // If the player types anything other than 'y' or 'n'
  if (strcmp(promptResponse, "y\n")&&strcmp(promptResponse, "n\n")) {
    while (strcmp(promptResponse, "y\n")&&strcmp(promptResponse, "n\n")) {
      printf("Invalid input. Type 'y' or 'n'.\n");
      fgets(promptResponse, sizeof(promptResponse), stdin);
    }
  }

  // If the player types 'y', attempt to load the savefile.
  if (promptResponse[0]=='y') { // start at the most recently saved checkpoint
    printf("Looking for save file...\n");
    char address[10];
    int fd = open("savefile.txt", O_RDONLY);
    read(fd, address, sizeof(address));
    close(fd);
    if (!strcmp(address, "0")) printf("You have no save file. Loading from beginning of the game...\n");
    else printf("Found save file. Loading from last saved checkpoint...\n");
    struct Node save = makeNode(address, buffer, buffer2);
  }

  // If the player types 'n', start the game at the beginning.
  if (promptResponse[0]=='n') {
    char address[10] = "0";
    struct Node root = makeNode(address, buffer, buffer2);
  }
}

// function which makes the choice (takes in stdin int)
int makeChoice(int numChoice) {
    printf("Input choice #: ");
    char choice [256];
    fgets(choice, sizeof(choice), stdin);

    // If the player types 'save'
    if (!strcmp(choice, "save\n")) {
      while (!strcmp(choice, "save\n")) {
        saveGame();
        fgets(choice, sizeof(choice), stdin);
      }
    }

    // If the player types 'back'
    if (!strcmp(choice, "back\n")){
      if (!strcmp(currentaddress, "0")) {
        printf("Can't go back. You're at the beginning!\n");
      }
      else return atoi("10");
    }

    // If the player types 'quit'
    if (!strcmp(choice, "quit\n")) exit(0);
    else {
      while (atoi(choice) < 1 || atoi(choice) > numChoice) {
          printf("Input invalid. Please try again: ");
          fgets(choice, sizeof(choice), stdin);
      }
    }
    return atoi(choice);
}

// prints the text from story.txt at address
void reader(char * address, char * buffer){
    char add [256];
    strcpy(add, address);
    strcat(add, " ");
    char *x = strstr(buffer, add);
    x+=strlen(address);
    int i; for (i = 0; x[i] != '|'; x++); x++;
    int k; for (k = 0; x[k] != '|'; k++) printf("%c", x[k]);
    printf("\n");
}

// second reader; returns the number of choices at address
int reader2(char * address, char * buffer) {
    char *y = strstr(buffer, address);
    y+=strlen(address);
    int i; for (i = 0; y[i] != ':'; y++); y++;
    return atoi(y);
}

static void sighandler(int signo){
    if (signo == SIGQUIT) {
      printf("\n");
      saveGame();  // Ctrl + "\"
    }
}

// makes node and links to next node recursively
struct Node makeNode(char str [256], char * buffer, char * buffer2) {
    struct Node node;
    strcpy(node.address, str);
    strcpy(currentaddress, node.address); // Sets the "currentaddress" (global String) to this node's address.
    if (autosave) autoSave();
    reader(node.address, buffer);
    int len = strlen(str);
    // in the future, not using len but actually checking last char in string
    // if particular char, say T, terminate and initiate end game function
    if (len == 10) exit(0); // for now it's exit, but we can add a special function (endgame()) that ends the game
    char add[256], choice[10];
    strcpy(add, str);

    signal(SIGQUIT, sighandler);

    // reads from buffer2 the number of choices with address
    int numChoice = reader2(node.address, buffer2);
    sprintf(choice, "%d", makeChoice(numChoice));
    if (!strcmp(choice, "10")){
      add[strlen(add)-1] = 0;
    }
    else{
    strcat(add, choice);}
    makeNode(add, buffer, buffer2);
}

int main() {
    // loads in story.txt into a buffer
    char buffer[5000];
    int fd = open("story.txt", O_RDONLY);
    read(fd, buffer, sizeof(buffer));
    close(fd);
    // loads numChoice.txt into another buffer
    char buffer2[256];
    int fd2 = open("numChoice.txt", O_RDONLY);
    read(fd2, buffer2, sizeof(buffer2));
    close(fd2);

    // at the start, check with user whether to load a save file
    while (1) {
      // Autosave Prompt:
      printf("Would you like to enable autosave? (y/n)\n");
      promptAutosave();

      // Loadfile Prompt: (If they don't want to load, the game starts at the beginning.)
      printf("Would you like to load in a saved file? (y/n)\n");
      promptLoadfile(buffer, buffer2);
    }

    return 0;
}
