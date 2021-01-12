#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "functions.h"

int globalint = 1;
char currentaddress[256]; // To keep track of what node we're currently at.

// FUNCTION TO SAVE THE GAME AT ANY GIVEN POINT.
void saveGame() {
  printf("Saved Game!\n");
  int fd = open("savefile.txt", O_WRONLY);
  write(fd, currentaddress, sizeof(currentaddress));
  close(fd);
}

// function which makes the choice (takes in stdin int)
int makeChoice(int numChoice) {
    printf("Input choice #: ");
    char choice [10];
    fgets(choice, sizeof(choice), stdin);
    if (!strcmp(choice, "save\n")) {
      saveGame();
      printf("Input choice #: ");
      fgets(choice, sizeof(choice), stdin);
    }
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
    char *x = strstr(buffer, address);
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

// makes node and links to next node recursively
struct Node makeNode(char str [256], char * buffer, char * buffer2) {
    struct Node node;
    strcpy(node.address, str);
    strcpy(currentaddress, node.address); // Sets the "currentaddress" (global String) to this node's address.
    reader(node.address, buffer);
    int len = strlen(str);
    if (len == 10) exit(0); // for now it's exit, but we can add a special function (endgame()) that ends the game
    char add[256], choice[10];
    strcpy(add, str);
    // reads from buffer2 the number of choices with address
    int numChoice = reader2(node.address, buffer2);
    sprintf(choice, "%d", makeChoice(numChoice));
    strcat(add, choice);
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
    //----------------------------------------
    // at the start, check with user whether to load a save file
    while (globalint) {
      printf("Would you like to load in a saved file? (y/n)\n");
      char buffer3[3];
      fgets(buffer3, sizeof(buffer3), stdin);

      // IF THE USER INPUTS 'n' AND DOES NOT WISH TO LOAD A SAVE FILE.
      if (buffer3[0]=='n') { // start at root (new game)
        globalint=0;
        char address[10] = "0";
        struct Node root = makeNode(address, buffer, buffer2);
      }

      // IF THE USER INPUTS 'y' AND DOES WISH THE LOAD A SAVE FILE.
      else if (buffer3[0]=='y') { // start at the checkpoint
        printf("Loading game from saved checkpoint...\n");
        globalint=0;
        char address[10];
        int fd3 = open("savefile.txt", O_RDONLY);
        read(fd3, address, sizeof(address));
        close(fd);
        struct Node save = makeNode(address, buffer, buffer2); // address is imported from save file function
      }

      else printf("Invalid input. Type 'y' or 'n'.\n");

    }

    return 0;
}