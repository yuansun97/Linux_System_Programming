/**
 * Mad Mad Access Patterns
 * CS 241 - Spring 2020
 */
#include "tree.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/

int binSearch(FILE *fp, char *word, uint32_t subroot, BinaryTreeNode *buffer);

int main(int argc, char **argv) {
  // check arguments usage
  if (argc < 3) {
    printArgumentUsage();
    exit(1);
  }

  // check file open
  FILE *fp = fopen(argv[1], "r");
  char *buffer = (char *) malloc(sizeof(BinaryTreeNode));
  size_t result;
  if (fp == NULL) {
    openFail(argv[1]);
    exit(2);
  }

  // check file format
  result = fread(buffer, sizeof(char), 4, fp);
  buffer[4] = '\0';
  if ( result != 4 || strcmp(buffer, BINTREE_HEADER_STRING) != 0 ) {
    formatFail(argv[1]);
    exit(2);
  }

  // search words
  BinaryTreeNode *treeNode = (BinaryTreeNode *) buffer;
  for (int i = 2; i < argc; i++) {
    if (binSearch(fp, argv[i], BINTREE_ROOT_NODE_OFFSET, treeNode)) {
      // word found
      printFound(argv[i], treeNode->count, treeNode->price);
    } else {
      // no such word
      printNotFound(argv[i]);
    }
  }

  fclose(fp);
  free(buffer);

  return 0;
}

/**
 * Search word in an open file, set the TreeNode metadata in buffer.
 * 
 * @param fp -- file where to search
 * @param word -- word to search
 * @param buffer -- buffer to store the metadata of treenode
 * 
 * @return 0 if no such word
 *         1 if word is in the file
 * */
int binSearch(FILE *fp, char *word, uint32_t subroot, BinaryTreeNode *buffer) {
  if (subroot == 0) return 0;
  
  fseek(fp, subroot, SEEK_SET);
  fread(buffer, sizeof(BinaryTreeNode), 1, fp);
  if (ferror(fp)) {
    perror("fread error!\n");
    exit(1);
  }

  // Read word on the tree node
  size_t len = 256;
  char nodeWord[len];
  fread(nodeWord, sizeof(char), len, fp);
  // printFound(nodeWord, buffer->count, buffer->price);

  int cmp = strcmp(word, nodeWord);
  if (cmp == 0) {
    return 1;
  }
  else if (cmp > 0) {
    // go right
    return binSearch(fp, word, buffer->right_child, buffer);
  }
  else {
    // go left
    return binSearch(fp, word, buffer->left_child, buffer);
  }
}

