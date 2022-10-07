/**************************************************************
* Class:  CSC-415-0# Summer 2022
* Name: Jiayi Gu
* Student ID: 920024739
* GitHub ID: Jiayi079
* Project: Assignment 4 – Word Blast
*
* File: Gu_Jiayi_HW4_main.c
*
* Description:
*
*       This program is to read a text file and count and sort each of the words 
*       that are 6 or more characters long, then will print the top ten most repetitive 
*       words. I set up a marco difinition about the number of display, user can easily 
*       change the number of words they want to display.
*
**************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>


#define MAX_CHAR 6 // struct only hold the words if letters bigger and equal to six
#define NUMBER_OF_DISPLAY 10 // user can change the number of words deplay here easily
#define MAX_WORDS_COUNT 14000 // total words count: 13972

// You may find this Useful
char * delim = "\"\'.“”‘’?:;-,—*($%)! \t\n\x0A\r";

int fd;
int divided_buffer_size;
int number_of_words = 0;
pthread_mutex_t word_count_lock;


struct Words{
    char * word;
    size_t count;
} Words;

struct Words words_list[839889]; // ensure big enought to hold words

void * storeWords()
{
    char * buff;

    // printf("divided_buffer_size: %d\n", divided_buffer_size);

    buff = malloc(divided_buffer_size);

    int read_return_value;

    // read function returns the number of bytes readed
    read_return_value = read(fd, buff, divided_buffer_size);
    
    // printf("read function return value: %d\n", read_return_value);

    if (read_return_value == -1) // error occurs on return -1
    {
        printf("ERROR OCCUR: READ FILE FAILED\n");
        exit(1);
    }
    

    int check = -1;
    int mutex_lock_return_value, mutex_unlock_return_value;

    // break each words in buff into ptr using the delimiter
    char * ptr = strtok(buff, delim);


    /* the while loop should run until every words store in to the ptr,
       a null pointer is returned if there are no ptr left in the buff*/
    while (ptr != NULL)
    {
        int length_of_word = strlen(ptr);

        // keep words into struct if its letters are larger and equal than MAX_CHAR
        if (length_of_word >= MAX_CHAR)
        {
            // starting searching each words in the words_list contains ptr or not
            for (int i = 0; i < number_of_words; i++)
            {
                /* strcasesmp will compare two string without case sensitivity,
                   return value 0 -> two string equivalent with each other*/
                check = strcasecmp(words_list[i].word, ptr);
                if (check == 0) // word already inside of the array
                {

                    /* in case mutex currently is locked by another thread,
                       this mutex lock simply to ensure an atomic update of the shared variable
                       it will block until other thread doing count words and unclock mutex*/
                    mutex_lock_return_value = pthread_mutex_lock(&word_count_lock);

                    /* pthread_mutex_lock() will return zero if completing sucessfully,
                       if-statement to check if it failed */
                    if (mutex_lock_return_value != 0)
                    {
                        printf("ERROR OCCUR: MUTEX LOCK FAILED\n");
                        exit(1);
                    }

                    // count plus one for this words
                    words_list[i].count = words_list[i].count + 1;
                    
                    // finish counting, unlock mutex 
                    mutex_unlock_return_value = pthread_mutex_unlock(&word_count_lock);

                    /* pthread_mutex_unlock() will return zero if completing sucessfully,
                       if-statement to check if it failed */
                    if (mutex_unlock_return_value != 0)
                    {
                        printf("ERROR OCCUR: MUTEX UNLOCK FAILED\n");
                        exit(1);
                    }

                    /* since we've already found the words do in the words_list,
                       break out of the for loop and get next ptr(word) 
                       by using while loop agian */
                    break;
                }
            }

            // return non-zero value -> two string differenct between each other
            if (check != 0) // word is not in the array 
            {

                mutex_lock_return_value = pthread_mutex_lock(&word_count_lock);

                if (mutex_lock_return_value != 0) // if failed
                {
                    printf("ERROR OCCUR: MUTEX LOCK FAILED\n");
                    exit(1);
                }

                /* input this new words into words_list
                   count the number plus one */
                strcpy(words_list[number_of_words].word, ptr);
                words_list[number_of_words].count = 1;

                mutex_unlock_return_value = pthread_mutex_unlock(&word_count_lock);

                if (mutex_unlock_return_value != 0) // if failed
                {
                    printf("ERROR OCCUR: MUTEX UNLOCK FAILED\n");
                    exit(1);
                }

                number_of_words++; // count total number of words in the words_list
            }
        }
        ptr = strtok(NULL, delim); //get next words in the file and while loop again
    }

    // printf("total number of words: %d\n", number_of_words);  // 13972 total words

    // dynamically de-allocate the memory
    free(buff);
    buff = NULL;
}



int main (int argc, char *argv[])
{
    //***TO DO***  Look at arguments, open file, divide by threads
    //             Allocate and Initialize and storage structures

    // look at arguments
    // for (int i = 0; i < strlen(argv); i++)
    // {
    //     printf("argv %d: %s\n", i, argv[i]);
    // }
    /*
        argument result:
            argv 0: ./Gu_Jiayi_HW4_main
            argv 1: WarAndPeace.txt
            argv 2: 2
            argv 3: (null)
            argv 4: DESKTOP_SESSION=ubuntu
            argv 5: XDG_SESSION_TYPE=x11

    */

    /* argv[2] present the number of thread we need to use in this program
       atoi() function is used to convert char string to integer*/
    int thread_num = atoi(argv[2]);

    /* print out the text file's name by calling the command line arguments
       after looking at the output (shows above) found that argv[1] is presenting the file name*/
    printf("\n\nWord Frequency Count on %s with %d threads\n",argv[1], thread_num);
   
    // printf("test open function: %d\n", open(argv[1], O_RDONLY));  // file descriptor of the file
    // O_RDONLY -> open for reading only
    // fd -> file descriptor

    fd = open(argv[1], O_RDONLY);

    /* open() will return a non-negative integer representing 
       the lowest numbered unused file descriptor */
    if (fd < 0)
    {
        printf("ERROR OCCUR: FAILD FILE DESCRIPTOR\n");
        exit(1);
    }

    // get size of the file plus offset(0) bytes
    int fs = lseek(fd, 0, SEEK_END);

    // printf("file size: %d\n", fs); // file size -> 3359549

    // set to the offset(0) bytes which means set to the beginning of the file
    lseek(fd, 0, SEEK_SET);


    // devide file by thread
    /* make sure the buffer size is bigger than the result if it has decimal,
       since we may have remainder when we are doing devide by thread_num,
       decided to set up the size plus one to make sure every characters can be stored*/
    divided_buffer_size = ((fs / thread_num) + 1);

    // printf("buffer size: %d\n", divided_buffer_size);


    // initialize words struct to clean the garbage value
    for (int i = 0; i < MAX_WORDS_COUNT; i++)
    {
        /* initial the malloc size(in bytes) is big enough to hold the word
           the longest word in the world has 45 letters -> 45 char = 45 byte */
        words_list[i].word = malloc(45);
    }

    // initial mutex to its defaul value
    int ret = pthread_mutex_init(&word_count_lock, NULL);
    /* ret return 0 after initial mutex completing succesfully
       other returned value indicates that an error occured */
    if (ret != 0)
    {
        printf("ERROR OCCUR: INITIAL MUTEX\n");
        exit(1);
    }


    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Time stamp start
    struct timespec startTime;
    struct timespec endTime;

    clock_gettime(CLOCK_REALTIME, &startTime);
    //**************************************************************
    // *** TO DO ***  start your thread processing
    //                wait for the threads to finish


    /* create array of threads since we're going to have multiple threads
       number of the threads are depends on the number in command line arg,
       so initial the size of thread array by calling thread_num -> argv[2] (shows line 185)*/
    pthread_t pthreads[thread_num];


    int p_create, p_join;
    // create independent threads, each of those will execute storeWords()
    for (int i = 0; i < thread_num; i++)
    {
        p_create = pthread_create(&pthreads[i], NULL, storeWords, NULL);

        /* pthread_create function return 0 if successfully
           check if pthread_create failed -> reutrn non-zero number*/
        if (p_create != 0)
        {
            printf("ERROR OCCUR: PTHREAD_CREATE %d FAILD\n", i);
            exit(1);
        }

        /* Wait for all threads to finish before moving on to main. If we don't wait, 
           we face the risk of executing an exit that kills the process and all running 
           threads before they've finished. */
        p_join = pthread_join(pthreads[i], NULL);

        /* pthread_join function return 0 if successfully
           check if pthread join faild -> return non-zero number */
        if (p_join != 0)
        {
            printf("ERROR OCCUR: PTHREAD_JION %d FAILD\n", i);
            exit(1);
        }
    }


    for (int i = 0; i < thread_num; i++)
    {
        
    }


    // ***TO DO *** Process TOP 10 and display


    // sort words counts highest to lowest

    int i;
    int biggest_i;
    char * word_keep;


    for (int j = 0; j < NUMBER_OF_DISPLAY; j++) {
        int count = 0;
        for (i = 0; i < number_of_words; i++)
        {
            /* find the highest count words
               store the value and the index of the word in words_list */
		    if (words_list[i].count > count)
		    {
			    count = words_list[i].count;
			    biggest_i = i;
                word_keep = words_list[i].word;
            }
        }

        // print out the result
        printf("Number %d is %s with a count of %d\n", j+1, word_keep, count);

        // set this word's count become zero to ensure not search this word become the biggest count word again
        words_list[biggest_i].count = 0;
    }


    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Clock output
    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec)
        {
        --sec;
        n_sec = n_sec + 1000000000L;
        }

    printf("Total Time was %ld.%09ld seconds\n", sec, n_sec);
    //**************************************************************


    // ***TO DO *** cleanup

    // close file descriptor
    close(fd);

    /* since we initialized words struct to clean the garbage value 
       by using memory location,
       do dynamically de-allocate the memory */
    for (int i = 0; i < MAX_WORDS_COUNT; i++)
    {
        free(words_list[i].word);
        words_list[i].word = NULL;
    }
    
    // desotry the mutex that mutex can no longer used
    pthread_mutex_destroy(&word_count_lock);
}