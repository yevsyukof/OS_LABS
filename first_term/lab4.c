#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct node {
    char *value;
    struct node* next;
} Node;

Node *initNode() {
    Node *head = (Node*)malloc(sizeof(Node));
    head->next = NULL;
    head->value = NULL;
    return head;
}

void freeNode(Node* currentNode) {
    free(currentNode->value);
    free(currentNode);
}

Node *initNodeWithStr(char *str) {
    Node *newNode = NULL;
    newNode = (Node*)malloc(sizeof(Node));
    newNode->next = NULL;
    newNode->value = (char*)malloc(strlen(str) + 1);
    strcpy(newNode->value, str);
    return(newNode);
}


int main() {
    char line[BUFSIZ];
    Node *head = NULL, *currentNode = NULL, *iterator = NULL;
    head = initNode();
    currentNode = head;
    printf("Enter lines of text. To end entering, put '.' in the beginning of line.\n");
    while (fgets(line, BUFSIZ, stdin) != NULL) { // забираем строки
        if (line[0] == '.') {
            break;
        }
        currentNode->next = initNodeWithStr(line); // line уже с символом переноса строки
        currentNode = currentNode->next;
    }
    for (iterator = head->next; iterator != NULL; iterator = iterator->next) {
        puts(iterator->value); //выводим строки в стандартный поток вывода
    }
    iterator = head->next;
    struct node *j = NULL;
    while (iterator != NULL) {
        j = iterator->next;
        freeNode(iterator);
        iterator = NULL;
        iterator = j;
    }
    free(head);
    return 0;
}

