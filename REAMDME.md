Harsh Singh, NETID: hks59
Assignment 2: Spelling Checker(spchk.c)

File Handling:
spchk.c uses low level I/O to read files line by line. Specifically, it uses open(), read() and close()
as specified in P2.pdf.

struct FileBuffer:
Handles buffering of file contents for line-by-line processing.

openFile(FileBuffer *fileBuffer, const char * filePath):
This function opens the file using open() and stores the file descriptor in the fileBuffer.
It prints an error message if the file cannot be opened.

readLine(FileBuffer *fileBuffer, char *line, size_t maxLength):
This function reads a line from a file specified by fileBuffer until it encounters a newline character '\n'
Once the end of the line is reached, it return the number of characters read, excluding '\n'.

closeFile(FileBuffer *fileBuffer):
This function closes the file specified by fileBuffer using close().

------------------------------------------------------------------------------------------------------------

#define CHAR_TO_INDEX(c) ((int)(unsigned char)c):
The Macro CHAR_TO_INDEX(c) takes a character c and returns its index in the alphabet array.

struct Trie:
Effectively stores and search for words for spell-checking.

getNode():
This function creates a new node for the Trie and returns it.
It uses malloc() to allocate memory for the node.
It also initializes the isEnd flag to false and sets all children to NULL.

insert(Trie *root, char *word):
Inserts a word into the Trie.
It iterates over each character in the input word and uses the CHAR_TO_INDEX macro to calculate
the index of the character.
If a child node corresponding to the character does not exist, it creates a new node and moves it to
the child node for the current character.
Finally, it sets the isEnd flag of the last node to true to represent the end of the word.

search(Node *root, const char *word):
Searches for a word in the Trie.
For each character, it calculates the index using the CHAR_TO_INDEX macro.
If a child node corresponding to the character does not exist, it returns false.
If the current node is an end node, it marks isEnd as true.
The function will only return true if the entire word is present in the Trie.

------------------------------------------------------------------------------------------------------------

Structure for checking words in the dictionary:

trimPunctuation(char *word):
Removes trailing punctuation from a word using a while loop and the ispunct() function.
It also removes the following leading punctuation using memmove():
    \, ", ', (, [, {(not inclusing commas)

checkWord(const char *word, Node *root):
If the current word is in the dictionary as-is, returns true.
Otherwise, it checks if the word is in all-caps, if so, it checks both the lower case word and upper case word
int the dictionary, if either is present, it returns true.
    This works within the specifications of the assignment because an all caps version of the word should
    be accepted if it is in the dictionary according to p2.pdf.
If the word is not in all-caps, it checks if only the first letter of the word is captilized. If it is,
it checks for the lowercase word in the dictionary and if it is present, it returns true.
    This also works within the specifications of the assignment because a word with only the first letter
    capitalized should be accepted if it is in the dictionary according to p2.pdf.
If the word is still not in the dictionary, it returns false.

processWord(char* wordStart, int wordLen, const char* filePath,
                                            int lineNum, int col, Node* root, int *errorFlag):
This function creates a new array called word and copies the characters from wordStart into it using strncpy().
It then calls trimPunctuation(word) to remove punctuation.
It then calls checkWord(word, root) to check if the word is in the dictionary.
If the word is not in the dictionary, it prints an error message and increments the errorFlag.

checkFile(const char* filePath, Node* root, int *errorFlag):
This function reads the contents of the file specified by filePath through readLine()
and calls processWord() for each word. If the file cannot be opened, it prints an error message.
checkFile also handles keeping track of line and column numbers for error messages.

------------------------------------------------------------------------------------------------------------

Structure for reading dictionary and processing paths for files:

oDict(const char* dictPath, Node *root, int *errorFlag):
This function is used to read the dictionary specified by dictPath and inserts each word into the Trie.
It initializes a fileBuffer and calls openFile() to open the dictionary file. If the file cannot be opened,
it prints an error.
It reads the dictionary file using readLine() and inserts each word into the Trie using insert().
To handle tricky cases like "MacDonald", it also inserts a full caps version of the word into the Trie
if the word contains any upper case letters.

processPath(const char* path, Node *root, int *errorFlag):
This function is responsible for processing any given path which can be a file or a directory.
If the path cannot be opened using stat(), it prints an error.
If the path is a directory, it checks if that directory can be opened using opendir().
If the directory cannot be opened, it prints an error.
It then traverses the directory using readdir() and for each entry, it skips the current and parent directories.
It then constructs the full path using the path and the entry name and recursively calls processPath() to
process the full path.
If the path is a file, it checks if the file can be opened and checked using checkFile().

------------------------------------------------------------------------------------------------------------

freeTrie(Node *root):
This function frees the memory allocated for the Trie recursively using free().

main():
This is the main function which calls each function as needed.
If the command line arguments are incorrect, it prints an error message.
It then saves the path to the dictionary in dictPath, creates a root node for the Trie and calls oDict to
create the Trie dictionary.
It then checks for multiple files/directories and calls processPath() for each path.
Finally, it frees the memory allocated for the Trie using freeTrie() and exits the program.

------------------------------------------------------------------------------------------------------------

The test scenarios for this program are provided in test1.txt, test2.txt, ...
They are simple tests designed to test the program on its proofreading capabilities
and directory traversing capabilities. "test2.txt" contains a test text obtained from copilot after "what's."

One limitation of this program is that if there is an empty line in the test file, the program will
not continue after that line and acts as if the file ended there. Any words after the empty line will
be ignored. This is a big limitation of the program but I would need to overhaul the readLine() function to fix
this and unfortunately, I do not have the time to make those changes.

A known error of this program is that the column number is not always correct. Especially if the
word is the first word in the line and in the first column. The column number is sometimes 7 instead of 0.
I'm not sure how to fix this since I am subtracting a char* from a char array to get the column number.

A known error of the Makefile is that I need to run "touch spchk.c" each time before running the Makefile
regardless if the file has been changed or not.