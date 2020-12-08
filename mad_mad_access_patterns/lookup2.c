/**
 * Mad Mad Access Patterns
 * CS 241 - Spring 2020
 */
#include "tree.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.

  ./lookup2 <data_file> <word> [<word> ...]
*/

void binSearch(void *addr, char *word, uint32_t subroot);

int main(int argc, char **argv) {
  // check arguments usage
  if (argc < 3) {
    printArgumentUsage();
    exit(1);
  }

  // check open file
  int fd = open(argv[1], O_RDWR);
  if (fd == -1) {
    openFail(argv[1]);
    exit(2);    
  }

  // mmap
  off_t fileSize = lseek(fd, 0, SEEK_END);
  // write(fd, '\0', 1);
  if (fileSize == -1) {
    perror("lseek failed!\n");
    exit(2);
  }

  // printf("file size=%zu\n", fileSize);

  void *addr = mmap(NULL, fileSize, PROT_READ, MAP_SHARED, fd, 0);
  if (addr == (void *) -1) {
    mmapFail(argv[1]);
    exit(2);
  }
  
  // check head format
  char head[5];
  strncpy(head, (char *) addr, 4);
  head[4] = '\0';
  if (strcmp(head, BINTREE_HEADER_STRING) != 0) {
    formatFail(argv[1]);
    exit(2);
  }

  // search words
  for (int i = 2; i < argc; i++) {
    binSearch(addr, argv[i], BINTREE_ROOT_NODE_OFFSET);
  }
  
  munmap(addr, fileSize);
  close(fd);
  return 0;
}


void binSearch(void *addr, char *word, uint32_t subroot) {
  if (subroot == 0) {
    printNotFound(word);
    return;
  } 

  BinaryTreeNode *buffer = (BinaryTreeNode *) (addr + subroot);

  // printFound(buffer->word, buffer->count, buffer->price);

  int cmp = strcmp(word, buffer->word);
  if (cmp == 0) {
    // word found
    printFound(word, buffer->count, buffer->price);
    return;
  }
  else if (cmp > 0) {
    // go right
    binSearch(addr, word, buffer->right_child);
  }
  else {
    // go left
    binSearch(addr, word, buffer->left_child);
  }

}