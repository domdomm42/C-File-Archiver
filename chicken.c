////////////////////////////////////////////////////////////////////////
// COMP1521 21T3 --- Assignment 2: `chicken', a simple file archiver
// <https://www.cse.unsw.edu.au/~cs1521/21T3/assignments/ass2/index.html>
//
// Written by Lim Oudom (z5329509) on 9/Nov/21.
//
// 2021-11-08   v1.1    Team COMP1521 <cs1521 at cse.unsw.edu.au>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

#include "chicken.h"

// ADD ANY extra #defines HERE
#define NAME_LENGTH 100
#define PERMISSION 11

// ADD YOUR FUNCTION PROTOTYPES HERE
int check_egglet_format(int format);
int decimal_to_octal(int num);

// print the files & directories stored in egg_pathname (subset 0)
//
// if long_listing is non-zero then file/directory permissions, formats & sizes are also printed (subset 0)

void list_egg(char *egg_pathname, int long_listing) {
    FILE *filename = fopen(egg_pathname, "r");
        
    if (filename == NULL) {
        perror(egg_pathname);
        exit(1);
    }

    if (long_listing) {
        int ch = 0;
        fseek(filename, 11, SEEK_CUR);

        while ((ch = fgetc(filename)) != EOF) {
        char name[NAME_LENGTH];
        char permission[PERMISSION];
        fseek(filename, -11, SEEK_CUR);

        int egglet_var = fgetc(filename);
        egglet_var = check_egglet_format(egglet_var);

        int permission_count = 0;
        while (permission_count < 10) {
            permission[permission_count] = fgetc(filename);
            permission_count++;
        }
        permission[10] = '\0';

        int counter1 = 0;
        uint16_t length_path = 0;
        uint16_t mask = 0;

        while (counter1 < 2) {
            length_path = length_path << 8;
            ch = fgetc(filename);
            mask = ch;
            length_path = length_path | mask;
            counter1++;

        }
        length_path = (length_path >> 8) | (length_path << 8);

        int i = 0;
        while (i < length_path) {
            ch = fgetc(filename);
            name[i] = ch;
            i++;
        }
        name[i] = '\0';

        int counter2 = 0;
        uint64_t content_length = 0;
        uint64_t mask2 = 0;
        
        ch = fgetc(filename);
        while (counter2 < 6) {
            mask2 = ch;
            mask2 = mask2 << (8 * counter2);
            content_length = content_length | mask2;
            ch = fgetc(filename);

            counter2++;
        }

        fseek(filename, content_length + 11, SEEK_CUR);
    
        printf("%s %2d  %5lu  %s\n",permission, egglet_var, content_length, name);
        
    }
    }

    else {
        int ch = 0;
        fseek(filename, 11, SEEK_CUR);
        while ((ch = fgetc(filename)) != EOF) {

            int counter1 = 0;
            uint16_t length_path = 0;
            uint16_t mask = 0;

            while (counter1 < 2) {
                length_path = length_path << 8;
                ch = fgetc(filename);
                mask = ch;
                length_path = length_path | mask;
                counter1++;

            }
            length_path = (length_path >> 8) | (length_path << 8);

            int i = 0;
            while (i < length_path) {
                ch = fgetc(filename);
                printf("%c", ch);
                i++;
            }
            printf("\n");

            int counter2 = 0;
            uint64_t content_length = 0;
            uint64_t mask2 = 0;
            
            ch = fgetc(filename);
            while (counter2 < 6) {
                mask2 = ch;
                mask2 = mask2 << (8 * counter2);
                content_length = content_length | mask2;
                ch = fgetc(filename);

                counter2++;
            }

            fseek(filename, content_length + 11, SEEK_CUR);
   
    }


}
}


// check the files & directories stored in egg_pathname (subset 1)
//
// prints the files & directories stored in egg_pathname with a message
// either, indicating the hash byte is correct, or
// indicating the hash byte is incorrect, what the incorrect value is and the correct value would be

void check_egg(char *egg_pathname) {
    FILE *filename = fopen(egg_pathname, "r");

    if (filename == NULL) {
        perror(egg_pathname);
        exit(1);
    }
    int ch = 0;

    while ((ch = fgetc(filename)) != EOF) {
        int hash_value = 0;
        int byte_value = 0;
        
        fseek(filename, -1, SEEK_CUR);
        ch = fgetc(filename);
        hash_value = egglet_hash(hash_value, ch);

        int magic_number = ch;
            if (magic_number != 'c') {
                fprintf(stderr, "error: incorrect first egglet byte: 0x%02x should be 0x63\n", magic_number);
                exit(1);
            }

        char name[NAME_LENGTH];
        char permission[PERMISSION];

        int egglet_var = ch;

        egglet_var = check_egglet_format(egglet_var);

        int permission_count = 0;

        while (permission_count < 10) {
            permission[permission_count] = fgetc(filename);
            hash_value = egglet_hash(hash_value, permission[permission_count]);
            permission_count++;
        }

        permission[10] = '\0';

        int counter1 = 0;
        uint16_t length_path = 0;
        uint16_t mask = 0;

        ch = fgetc(filename);
        hash_value = egglet_hash(hash_value, ch);

        while (counter1 < 2) {
            length_path = length_path << 8;

            ch = fgetc(filename);
            hash_value = egglet_hash(hash_value, ch);

            mask = ch;
            length_path = length_path | mask;
            counter1++;

        }

        length_path = (length_path >> 8) | (length_path << 8);

        int i = 0;
        while (i < length_path) {

            ch = fgetc(filename);
            hash_value = egglet_hash(hash_value, ch);

            name[i] = ch;
            i++;
        }
        name[i] = '\0';

        int counter2 = 0;
        uint64_t content_length = 0;
        uint64_t mask2 = 0;
        
        ch = fgetc(filename);
        hash_value = egglet_hash(hash_value, ch);

        while (counter2 < 6) {
            mask2 = ch;
            mask2 = mask2 << (8 * counter2);
            content_length = content_length | mask2;

            ch = fgetc(filename);
            hash_value = egglet_hash(hash_value, ch);

            counter2++;
        }

        int counter3 = 0;
        while (counter3 < content_length - 1) {
            byte_value = fgetc(filename);
            hash_value = egglet_hash(hash_value, byte_value);
            counter3++;
        }

        int hash_value2 = fgetc(filename);

        if (hash_value == hash_value2) {
            printf("%s - correct hash\n", name);
        }

        else {
            printf("%s - incorrect hash 0x%02x should be 0x%02x\n", name, hash_value, hash_value2);
        }

        }
    
    // REPLACE THIS PRINTF WITH YOUR CODE

    // printf("check_egg called to check egg: '%s'\n", egg_pathname);
}


// extract the files/directories stored in egg_pathname (subset 2 & 3)

void extract_egg(char *egg_pathname) {

    FILE *filename = fopen(egg_pathname, "r");
    if (filename == NULL) {
        perror(egg_pathname);
        exit(1);
    }

    int ch = 0;
    // fseek(filename, 11, SEEK_CUR);

    while ((ch = fgetc(filename)) != EOF) {
        char name[NAME_LENGTH];
        // char permission[PERMISSION];
        // char binary[11];
        // mode_t mode = 0;
        // char *ptr;
    
        int egglet_var = fgetc(filename);
        egglet_var = check_egglet_format(egglet_var);

        int mode = 0;
    
        if ((ch = fgetc(filename)) == 'd') {
            mode |= S_ISDIR(ch);
        }

        if ((ch = fgetc(filename)) == 'r') {
            mode |= S_IRUSR;
            
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        }

        if ((ch = fgetc(filename)) == 'w') {
            mode |= S_IWUSR;
            
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        }

        if ((ch = fgetc(filename)) == 'x') {
            mode |= S_IXUSR;
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        }

        if ((ch = fgetc(filename)) == 'r') {
            mode |= S_IRGRP;
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        }

        if ((ch = fgetc(filename)) == 'w') {
            mode |= S_IWGRP;
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        }

        if ((ch = fgetc(filename)) == 'x') {
            mode |= S_IXGRP;
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        } 

        if ((ch = fgetc(filename)) == 'r') {
            mode |= S_IROTH;
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        } 
        

        if ((ch = fgetc(filename)) == 'w') {
            mode |= S_IWOTH;
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        } 

        if ((ch = fgetc(filename)) == 'x') {
            mode |= S_IXOTH;
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        } 



        int counter1 = 0;
        uint16_t length_path = 0;
        uint16_t mask = 0;

        // FINDING FILE NAME LENGTH
        while (counter1 < 2) {
            length_path = length_path << 8;
            ch = fgetc(filename);
            mask = ch;
            length_path = length_path | mask;
            counter1++;

        }
        length_path = (length_path >> 8) | (length_path << 8);

        // STORING FILE NAME
        int i = 0;
        while (i < length_path) {
            ch = fgetc(filename);
            name[i] = ch;
            i++;
        }
        name[i] = '\0';

        FILE *fPtr = fopen(name, "w");

        if (fPtr == NULL) {
            perror(name);
            exit(1);
        }

        if (chmod(name, mode) != 0) {
            perror(name);  // prints why the chmod failed
            exit(1);
        }


        int counter2 = 0;
        uint64_t content_length = 0;
        uint64_t mask2 = 0;
        
        // FINDING CONTENT LENGTH
        ch = fgetc(filename);
        while (counter2 < 6) {
            mask2 = ch;
            mask2 = mask2 << (8 * counter2);
            content_length = content_length | mask2;
            ch = fgetc(filename);

            counter2++;
        }
        // printf("%ld\n", content_length);

        // WRITING CONTENT
        printf("Extracting: %s\n", name);
        int counter3 = 0;

        while (counter3 < content_length) {
            fputc(ch, fPtr);
            ch = fgetc(filename);
            // fputc(ch, fPtr);
            counter3++;
        }

        
    }

    // REPLACE THIS PRINTF WITH YOUR CODE

    // printf("extract_egg called to extract egg: '%s'\n", egg_pathname);
}


// create egg_pathname containing the files or directories specified in pathnames (subset 3)
//
// if append is zero egg_pathname should be over-written if it exists
// if append is non-zero egglets should be instead appended to egg_pathname if it exists
//
// format specifies the egglet format to use, it must be one EGGLET_FMT_6,EGGLET_FMT_7 or EGGLET_FMT_8

void create_egg(char *egg_pathname, int append, int format,
                int n_pathnames, char *pathnames[n_pathnames]) {

    // REPLACE THIS CODE PRINTFS WITH YOUR CODE

    printf("create_egg called to create egg: '%s'\n", egg_pathname);
    printf("format = %x\n", format);
    printf("append = %d\n", append);
    printf("These %d pathnames specified:\n", n_pathnames);
    for (int p = 0; p < n_pathnames; p++) {
        printf("%s\n", pathnames[p]);
    }
}


// ADD YOUR EXTRA FUNCTIONS HERE
int check_egglet_format(int format) {

    if (format == 54) {
        return 6;
    }

    else if (format == 55) {
        return 7;
    }

    else {
        return 8;
    }

    return 0;
}

int decimal_to_octal(int num) {
    int octalnum = 0; 
    int tmp = 1;

    while (num != 0)
    {
    	octalnum = octalnum + (num % 8) * tmp;
    	num = num / 8;
        tmp = tmp * 10;
    }

    return octalnum;
}
	
