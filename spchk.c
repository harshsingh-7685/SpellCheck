#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
//------------------------------------------------------------------------------------------------------

//reading files using read()
#define BUFFER_SIZE 4096

//file buffer
typedef struct File{
    int fd;
    char buffer[BUFFER_SIZE];
    int bufferLen;
    int bufferPos;
    char pLine[BUFFER_SIZE];
} FileBuffer;

//open file
int openFile(FileBuffer *fileBuffer, const char * filePath){
    fileBuffer->fd = open(filePath, O_RDONLY);

    if(fileBuffer->fd == -1){
        printf("Could not open file: %s\n", filePath);
        return -1;
    }

    fileBuffer->bufferLen = 0;
    fileBuffer->bufferPos = 0;
    fileBuffer->pLine[0] = '\0';

    return 0;
}

//read line from file
ssize_t readLine(FileBuffer *fileBuffer, char *line, size_t maxLength){
    size_t lLen = 0;
    ssize_t bRead;
    char c;
    // bool lineNotEmpty = false;

    while((bRead = read(fileBuffer->fd, &c, 1)) > 0){
        //check if end of line or file
        if(c == '\n')
            break;


        //check if line is full
        if(lLen < maxLength - 1)
            line[lLen++] = c;
    }

    line[lLen] = '\0';
    return (lLen > 0 || bRead > 0) ? lLen : bRead;
}

//closing file
void closeFile(FileBuffer *fileBuffer){
    if(fileBuffer->fd != -1)
        close(fileBuffer->fd);
}

//------------------------------------------------------------------------------------------------------

//structure for trie
#define CHAR_TO_INDEX(c) ((int)(unsigned char)c)

//preventing error for DT_DIR
#ifndef DT_DIR
#define DT_DIR 4
#endif

//trie strucutre to store dictionary(limited to 256 characters)
typedef struct Node{
    struct Node *child[256];
    bool isEnd;
} Node;

//create a new node
Node* getNode(){
    Node *newNode = (Node*)malloc(sizeof(Node));

    if(newNode){
        newNode->isEnd = false;
        for(int i = 0; i < 256; i++)
            newNode->child[i] = NULL;
    }

    return newNode;
}

//insert function
void insert(Node *root, char *word){
    int index;

    Node *temp = root;

    for(int level = 0; level<strlen(word); level++){
        index = CHAR_TO_INDEX(word[level]);   //case sensitive

        if(!temp->child[index])
            temp->child[index] = getNode();

        temp = temp->child[index];
    }

    temp->isEnd = true;
}

//search function
bool search(Node *root, const char *word){
    int index;

    Node *temp = root;

    for(int level = 0; level<strlen(word); level++){
        index = CHAR_TO_INDEX(word[level]);

        if(!temp->child[index])
            return false;

        temp = temp->child[index];
    }

    return (temp != NULL && temp->isEnd);
}
//------------------------------------------------------------------------------------------------------

//structure for checking words against dictionary

//delete common punctuation
void trimPunctuation(char *word) {
    size_t len = strlen(word);

    //remove trailing punctuation
    while(len > 0 && ispunct(word[len - 1]))
        word[--len] = '\0';

    //remove leading punctuation
    size_t start = 0;
    if(len > 0 && (word[start] == '\'' || word[start] == '"' || word[start] == '('
                                || word[start] == '[' || word[start] == '{')) {
        start++;
        memmove(word, word + start, len - start + 1);
    }
}

//searches for case sensitive word in dictionary
bool checkWord(const char *word, Node *root){
    //check if word is in dictionary as is
    if(search(root, word))
        return true;

    size_t len = strlen(word);

    //checks if word is in all caps
    bool isUpper = true;
    for(int i = 0; i<len; i++){
        if(!isupper(word[i])){
            isUpper = false;
            break;
        }
    }

    if(isUpper){
        char uWord[len + 1];
        char lWord[len + 1];

        for(int i = 0; i<len; i++){
            uWord[i] = toupper(word[i]);
            lWord[i] = tolower(word[i]);
        }

        uWord[len] = '\0';
        lWord[len] = '\0';

        return search(root, uWord) || search(root, lWord);
    }

    //checks if first letter is upper case and rest of the word is not all upper case
    bool isFirstUpper = isupper(word[0]);
    if(isFirstUpper && !isUpper){
        char lowerWord[len+1];
        lowerWord[0] = tolower(word[0]);

        for(int i = 1; i<len; i++)
            lowerWord[i] = word[i];

        lowerWord[len] = '\0';

        return search(root, lowerWord);
    }

    return false;
}

//process a word
void processWord(char* wordStart, int wordLen, const char* filePath,
                                        int lineNum, int col, Node* root, int *errorFlag) {
    //ensure the word array is clear
    char word[BUFFER_SIZE] = {0};
    strncpy(word, wordStart, wordLen);
    word[wordLen] = '\0'; //null-terminate the word

    //trim punctuation
    trimPunctuation(word);

    //check if word is in dictionary
    if (!checkWord(word, root)){
        printf("%s (%d, %d): %s\n", filePath, lineNum, col, word);
        *errorFlag = 1;
    }
}

//checking the file for spelling errors
void checkFile(const char *filePath, Node *root, int *errorFlag) {
    FileBuffer fb;

    //if can't open file
    if (openFile(&fb, filePath) != 0) {
        printf("Could not open file: %s\n", filePath);
        *errorFlag = 1;
        return;
    }

    char line[BUFFER_SIZE];
    ssize_t lineLen;
    int lineNum = 1;

    //read the file line by line
    while ((lineLen = readLine(&fb, line, sizeof(line))) > 0) {
        char *wordStart = line;
        char *wordEnd;
        int wordLen;
        int col;

        //process each word in the line
        while ((wordEnd = strpbrk(wordStart, " \t\n-")) != NULL) {
            wordLen = wordEnd - wordStart;
            col = (wordStart - line) + 1;

            //process the word
            if (wordLen > 0)
                processWord(wordStart, wordLen, filePath, lineNum, col, root, errorFlag);

            //move past the delimiter
            wordStart = wordEnd + 1;
        }

        //handle the last word in the line
        if (*wordStart != '\0')
            processWord(wordStart, strlen(wordStart), filePath, lineNum, col, root, errorFlag);

        lineNum++;
    }

    closeFile(&fb);
}
//------------------------------------------------------------------------------------------------------

//structure for reading files and directories

//open dictionary
void oDict(const char* dictPath, Node *root, int *errorFlag){
    FileBuffer fb;

    //if can't open dictionary
    if(openFile(&fb, dictPath) != 0){
        printf("Could not open dictionary file: %s\n", dictPath);
        *errorFlag = 1;
        exit(EXIT_FAILURE);
    }

    char line[BUFFER_SIZE];

    //read dictionary and insert every word into trie
    while(readLine(&fb, line, sizeof(line)) > 0){
        insert(root, line);

        //add a full uppercase version of word to dictionary if there are any uppercase letters
        bool isUpper = false;
        for(int i = 0; i < strlen(line); i++){
            if(isupper(line[i])){
                isUpper = true;
                break;
            }
        }

        if(isUpper){
            char uLine[strlen(line) + 1];
            for(int i = 0; i < strlen(line); i++)
                uLine[i] = toupper(line[i]);

            uLine[strlen(line)] = '\0';
            insert(root, uLine);
        }
    }


    closeFile(&fb);
}

//process a path recursively
void processPath(const char *path, Node *root, int *errorFlag){
    struct stat pathStat;

    //if can't open path
    if(stat(path, &pathStat) != 0){
        printf("Could not open path: %s\n", path);
        *errorFlag = 1;
        return;
    }

    //if path is a directory
    if(S_ISDIR(pathStat.st_mode)){
        DIR *dir = opendir(path);

        //if can't open directory
        if(!dir){
            printf("Could not open directory: %s\n", path);
            *errorFlag = 1;
            return;
        }

        struct dirent *entry;

        //traverse directory
        while((entry = readdir(dir)) != NULL){
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            char fullPath[BUFFER_SIZE];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);

            //check if path is a file or directory recursively
            processPath(fullPath, root, errorFlag);
        }

        closedir(dir);
    }
    //if path is a file
    else if(S_ISREG(pathStat.st_mode))
        checkFile(path, root, errorFlag);

}
//------------------------------------------------------------------------------------------------------

//frees memory allocated recursively
void freeTrie(Node *root){
    if(root == NULL)
        return;
    for(int i = 0; i<256; i++){
        if(root->child[i] != NULL)
            freeTrie(root->child[i]);
    }

    free(root);
}

int main(int argc, char *argv[]){
    //check arguments
    if(argc < 3){
        printf("Usage: %s <dictionary> <file/directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    //flag to check if any errors were found
    int errorFlag = 0;

    //build trie with dictionary file
    const char* dictPath = argv[1];
    Node *root = getNode();
    oDict(dictPath, root, &errorFlag);

    //check multiple files/directories
    for(int i = 2; i < argc; i++){
        const char* path = argv[i];
        processPath(path, root, &errorFlag);
    }

    //free memory
    freeTrie(root);

    //check if any errors were found
    return errorFlag ? EXIT_FAILURE : EXIT_SUCCESS;
}
