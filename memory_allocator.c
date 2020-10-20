#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#define MAXCHAR 1000

char const* const allocStrat[] = {"first", "best", "worst"};

typedef struct _Alloc {
	char * wpt;
	int wsize;
    LIST_ENTRY(_Alloc) alloc_pointers;
    LIST_ENTRY(_Alloc) freed_pointers;
} Alloc;

struct LinkedList {
    LIST_HEAD(alloc_list, _Alloc) head;
};

FILE* openFile(char const* const filename) {
    FILE *fp;

    fp = fopen(filename, "r");
	if (fp == NULL){
		printf("Could not open file %s\n",filename);
		return NULL;
	}

    return fp;
}

int findLines(FILE *fp) {
    int lines = 1;
    while (EOF != (fscanf(fp, "%*[^\n]"), fscanf(fp,"%*c")))
        ++lines;
    rewind(fp);
    return lines;
}

int countList(struct LinkedList linkedlist, int list) {
    int count = 0;
    Alloc *currAlloc = malloc(sizeof(Alloc));
    if(list == 0) {
        LIST_FOREACH(currAlloc, &linkedlist.head, alloc_pointers) count++;
    } else {
        LIST_FOREACH(currAlloc, &linkedlist.head, freed_pointers) count++;
    }
    
    return count;
}

int allocate(char const* const filename, int stratNo, char const* const outputname) {
    // opening file
    FILE *fp = openFile(filename);
    if(fp == NULL) {
        return EXIT_FAILURE;
    }

    // initialising linked lists
    struct LinkedList allocMBList;
    struct LinkedList freedMBList;
    LIST_INIT(&allocMBList.head);
    LIST_INIT(&freedMBList.head);

    char str[MAXCHAR];
    char * token;
    int currLine = 0;
    Alloc *previousAlloc = malloc(sizeof(Alloc));

    int lines = findLines(fp);

    // filling first 1000 entries
    while(fgets(str, MAXCHAR, fp) != NULL) {
        currLine++;
        token = strtok(str, "\n");
        size_t wsize = strlen(token) + 1;

        void *request;
        request = sbrk(wsize);
        strcpy((char *) request, token);

        Alloc *newAlloc = malloc(sizeof(Alloc));
        newAlloc->wpt = request;
        newAlloc->wsize = wsize;

        if(currLine == 1) {
            LIST_INSERT_HEAD(&allocMBList.head, newAlloc, alloc_pointers);
        } else {
            LIST_INSERT_AFTER(previousAlloc, newAlloc, alloc_pointers);
        }

        previousAlloc = newAlloc;

        if(currLine == 1000) {
            break;
        }
    }

    // randomly deleting 500 entries
    srand((unsigned int)time(NULL));
    for(int i = 0; i < 500; i++) {
        Alloc *deAlloc = malloc(sizeof(Alloc));
        Alloc *currAlloc = malloc(sizeof(Alloc));
        while(deAlloc == NULL || deAlloc->wpt == NULL || !deAlloc->wpt[0]) {
            int randNum = (rand() % ((countList(allocMBList, 0) - 1) - 1 + 1)) + 1;
            for(int j = 0; j <= randNum; j++) {
                if(j == 0) {
                    currAlloc = LIST_FIRST(&allocMBList.head);
                } else {
                    currAlloc = LIST_NEXT(previousAlloc, alloc_pointers);
                }

                if(j != randNum) {
                    previousAlloc = currAlloc;
                }
            }
            deAlloc = currAlloc;
        }
        
        // deleting data
        *deAlloc->wpt = '\0';

        Alloc *nextAlloc = LIST_NEXT(deAlloc, alloc_pointers);
        int allocated = 0;
                
        if (nextAlloc != NULL && !nextAlloc->wpt[0]) {
            // checking if next block is empty
            // merging with next block
            deAlloc->wsize += nextAlloc->wsize;
            LIST_REMOVE(nextAlloc, alloc_pointers);
            LIST_INSERT_BEFORE(nextAlloc, deAlloc, freed_pointers);
            LIST_REMOVE(nextAlloc, freed_pointers);
            allocated = 1;
        } 

        // checking if previous block is empty
        if(LIST_FIRST(&allocMBList.head)->wpt != deAlloc->wpt && !previousAlloc->wpt[0]) {
            // merging with previous block
            previousAlloc->wsize += deAlloc->wsize;
            LIST_REMOVE(deAlloc, alloc_pointers);
            if(allocated) {
                LIST_REMOVE(deAlloc, freed_pointers);
            }
            allocated = 1;
        }

        if(!allocated) {
            // if no adjacent blocks are empty
            // add to freed linked list
            if(LIST_FIRST(&freedMBList.head) == NULL) {
                LIST_INSERT_HEAD(&freedMBList.head, deAlloc, freed_pointers);
            } else {
                if(deAlloc < LIST_FIRST(&freedMBList.head)) {
                    LIST_INSERT_HEAD(&freedMBList.head, deAlloc, freed_pointers);
                } else {
                    LIST_FOREACH(currAlloc, &freedMBList.head, freed_pointers) {
                        if(deAlloc < currAlloc) {
                            LIST_INSERT_BEFORE(currAlloc, deAlloc, freed_pointers);
                            break;
                        } else if (LIST_NEXT(currAlloc, freed_pointers) == NULL) {
                            LIST_INSERT_AFTER(currAlloc, deAlloc, freed_pointers);
                            break;
                        }
                    }
                }
            }
        }
    }

    // while haven't loaded all lines
    while(currLine != lines) {
        // calculate how many lines to load
        int finalLine;
        if(currLine + 1000 < lines) {
            finalLine = currLine + 1000;
        } else {
            finalLine = lines;
        }
        
        // read in each line
        Alloc *currAlloc = malloc(sizeof(Alloc));
        while(fgets(str, MAXCHAR, fp) != NULL) {
            currLine++;
            token = strtok(str, "\n");
            size_t wsize = strlen(token) + 1;

            int foundBlock = 0;
            Alloc *newAlloc = malloc(sizeof(Alloc));

            if(stratNo == 0) {
                // find the first block to load into
                LIST_FOREACH(currAlloc, &freedMBList.head, freed_pointers) {
                    // if the block is larger than needed, split it
                    if(currAlloc->wsize > wsize) {
                        newAlloc->wpt = currAlloc->wpt + wsize;
                        *newAlloc->wpt = '\0';
                        newAlloc->wsize = currAlloc->wsize - wsize;
                        currAlloc->wpt = strcpy((char *)currAlloc->wpt, token);
                        currAlloc->wsize = wsize;
                        LIST_INSERT_AFTER(currAlloc, newAlloc, alloc_pointers);
                        LIST_INSERT_AFTER(currAlloc, newAlloc, freed_pointers);
                        LIST_REMOVE(currAlloc, freed_pointers);
                        foundBlock = 1;
                        break;
                    } else if (currAlloc->wsize == wsize) {
                        // else if the block is the right size assign to block
                        currAlloc->wpt = strcpy((char *)currAlloc->wpt, token);
                        LIST_REMOVE(currAlloc, freed_pointers);
                        foundBlock = 1;
                        break;
                    }
                }
            } else if (stratNo == 1) {
                // find closest size block
                Alloc *closestAlloc = malloc(sizeof(Alloc));
                LIST_FOREACH(currAlloc, &freedMBList.head, freed_pointers) {
                    if((currAlloc->wsize - (int)wsize) >= 0) {
                        if(closestAlloc->wpt == NULL) {
                            closestAlloc = currAlloc;
                        } else if ((closestAlloc->wsize - (int)wsize) > (currAlloc->wsize - (int)wsize)) {
                            closestAlloc = currAlloc;
                        }
                    }
                }

                if(closestAlloc->wpt != NULL) {
                    if(closestAlloc->wsize - (int)wsize > 0) {
                        newAlloc->wpt = closestAlloc->wpt + wsize;
                        *newAlloc->wpt = '\0';
                        newAlloc->wsize = closestAlloc->wsize - wsize;
                        LIST_INSERT_AFTER(closestAlloc, newAlloc, alloc_pointers);
                        LIST_INSERT_AFTER(closestAlloc, newAlloc, freed_pointers);
                    }
                    closestAlloc->wpt = strcpy((char *)closestAlloc->wpt, token);
                    closestAlloc->wsize = wsize;
                    LIST_REMOVE(closestAlloc, freed_pointers);
                    foundBlock = 1;
                }
            } else if (stratNo == 2) {
                // find largest size block
                Alloc *largestAlloc = malloc(sizeof(Alloc));
                LIST_FOREACH(currAlloc, &freedMBList.head, freed_pointers) {
                    if(largestAlloc->wpt == NULL) {
                        largestAlloc = currAlloc;
                    } else if (largestAlloc->wsize < currAlloc->wsize) {
                        largestAlloc = currAlloc;
                    }
                }

                if(largestAlloc->wpt != NULL && largestAlloc->wsize >= (int)wsize) {
                    if(largestAlloc->wsize - (int)wsize > 0) {
                        newAlloc->wpt = largestAlloc->wpt + wsize;
                        *newAlloc->wpt = '\0';
                        newAlloc->wsize = largestAlloc->wsize - wsize;
                        LIST_INSERT_AFTER(largestAlloc, newAlloc, alloc_pointers);
                        LIST_INSERT_AFTER(largestAlloc, newAlloc, freed_pointers);
                    }
                    largestAlloc->wpt = strcpy((char *)largestAlloc->wpt, token);
                    largestAlloc->wsize = wsize;
                    LIST_REMOVE(largestAlloc, freed_pointers);
                    foundBlock = 1;
                }
            }

            // if no block big enough was found, add to the end of the linked list
            if(!foundBlock) {
                void *request;
                request = sbrk(wsize);
                strcpy((char *) request, token);

                newAlloc->wpt = request;
                newAlloc->wsize = wsize;

                LIST_FOREACH(currAlloc, &allocMBList.head, alloc_pointers) {
                    if(LIST_NEXT(currAlloc, alloc_pointers) == NULL) {
                        previousAlloc = currAlloc;
                    }
                }

                LIST_INSERT_AFTER(previousAlloc, newAlloc, alloc_pointers);
            }

            if(currLine == finalLine) {
                break;
            }
        }
        if(finalLine < lines) {
            for(int i = 0; i < 500; i++) {
                Alloc *deAlloc = malloc(sizeof(Alloc));
                while(deAlloc == NULL || deAlloc->wpt == NULL || !deAlloc->wpt[0]) {
                    int randNum = (rand() % ((countList(allocMBList, 0) - 1) - 1 + 1)) + 1;
                    for(int j = 0; j <= randNum; j++) {
                        if(j == 0) {
                            currAlloc = LIST_FIRST(&allocMBList.head);
                        } else {
                            currAlloc = LIST_NEXT(previousAlloc, alloc_pointers);
                        }
                
                        if(j != randNum) {
                            previousAlloc = currAlloc;
                        }
                    }
                    deAlloc = currAlloc;
                }
                
                // deleting data
                *deAlloc->wpt = '\0'; 

                Alloc *nextAlloc = LIST_NEXT(deAlloc, alloc_pointers);
                int allocated = 0;
                
                if (nextAlloc != NULL && !nextAlloc->wpt[0]) {
                    // checking if next block is empty
                    // merging with next block
                    deAlloc->wsize += nextAlloc->wsize;
                    LIST_REMOVE(nextAlloc, alloc_pointers);
                    LIST_INSERT_BEFORE(nextAlloc, deAlloc, freed_pointers);
                    LIST_REMOVE(nextAlloc, freed_pointers);
                    allocated = 1;
                } 

                // checking if previous block is empty
                if(LIST_FIRST(&allocMBList.head)->wpt != deAlloc->wpt && !previousAlloc->wpt[0]) {
                    // merging with previous block
                    previousAlloc->wsize += deAlloc->wsize;
                    LIST_REMOVE(deAlloc, alloc_pointers);
                    if(allocated) {
                        LIST_REMOVE(deAlloc, freed_pointers);
                    }
                    allocated = 1;
                }

                if(allocated == 0) {
                    // if no adjacent blocks are empty
                    // add to freed linked list
                    if(LIST_FIRST(&freedMBList.head) == NULL) {
                        LIST_INSERT_HEAD(&freedMBList.head, deAlloc, freed_pointers);
                    } else {
                        if(deAlloc < LIST_FIRST(&freedMBList.head)) {
                            LIST_INSERT_HEAD(&freedMBList.head, deAlloc, freed_pointers);
                        } else {
                            LIST_FOREACH(currAlloc, &freedMBList.head, freed_pointers) {
                                if(deAlloc < currAlloc) {
                                    LIST_INSERT_BEFORE(currAlloc, deAlloc, freed_pointers);
                                    break;
                                } else if (LIST_NEXT(currAlloc, freed_pointers) == NULL) {
                                    LIST_INSERT_AFTER(currAlloc, deAlloc, freed_pointers);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    fclose(fp);

    fp = fopen(outputname, "w");

    int totalSize = 0;
    Alloc *currAlloc = malloc(sizeof(Alloc));
    LIST_FOREACH(currAlloc, &allocMBList.head, alloc_pointers) {
        totalSize += currAlloc->wsize;
    }

    fprintf(fp,"Total memory allocated: %d\n",totalSize);

    fprintf(fp,"\n-----freedMBList-----\n");
    LIST_FOREACH(currAlloc, &freedMBList.head, freed_pointers) {
        fprintf(fp,"%p\t%d\n", currAlloc->wpt, currAlloc->wsize);
    }

    fprintf(fp,"\n-----allocMBList-----\n");
    LIST_FOREACH(currAlloc, &allocMBList.head, alloc_pointers) {
        fprintf(fp,"%p\t%d\t%s\n", currAlloc->wpt, currAlloc->wsize, (char *) currAlloc->wpt);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if(argc != 4){
		printf("The input file/allocation strategy/output file is not specified.\n");
		return EXIT_FAILURE;
    }

    char const* const filename = argv[1];
    char const* const allocType = argv[2];
    char const* const outputname = argv[3];

    if(strcmp(allocType, allocStrat[0]) == 0) {
        return allocate(filename, 0, outputname);
    } else if (strcmp(allocType, allocStrat[1]) == 0) {
        return allocate(filename, 1, outputname);
    } else if (strcmp(allocType, allocStrat[2]) == 0) {
        return allocate(filename, 2, outputname);
    } else {
        printf("Invalid allocation stragegy.\nTry: \"first\", \"best\" or \"worst\"\n");
    }
}