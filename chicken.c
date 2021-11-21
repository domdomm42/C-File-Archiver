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
#include <string.h>

#include "chicken.h"

#define NAME_LENGTH 100
#define PERMISSION 11


int check_egglet_format(int format);


// Subset 0 
void list_egg(char *egg_pathname, int long_listing) {
    FILE *read_file = fopen(egg_pathname, "r");

    // Error Checking
    if (read_file == NULL) {
        perror(egg_pathname);
        exit(1);
    }

    if (long_listing) {
        int ch = 0;
        fseek(read_file, 11, SEEK_CUR);

        while ((ch = fgetc(read_file)) != EOF) {
            
        char name[NAME_LENGTH];
        char permission[PERMISSION];
        fseek(read_file, -11, SEEK_CUR);

        int egglet_var = fgetc(read_file);
        egglet_var = check_egglet_format(egglet_var);

        int permission_count = 0;
        while (permission_count < 10) {
            permission[permission_count] = fgetc(read_file);
            permission_count++;
        }
        permission[10] = '\0';

        int counter1 = 0;
        uint16_t length_path = 0;
        uint16_t mask = 0;

        // Finding length of name, converted from little endian.
        while (counter1 < 2) {
            length_path = length_path << 8;
            ch = fgetc(read_file);
            mask = ch;
            length_path = length_path | mask;
            counter1++;

        }
        length_path = (length_path >> 8) | (length_path << 8);

        // Filename 
        int i = 0;
        while (i < length_path) {
            ch = fgetc(read_file);
            name[i] = ch;
            i++;
        }
        name[i] = '\0';

        int counter2 = 0;
        uint64_t content_length = 0;
        uint64_t mask2 = 0;
        
        // Finding Content_Length from little endian.
        ch = fgetc(read_file);
        while (counter2 < 6) {
            mask2 = ch;
            mask2 = mask2 << (8 * counter2);
            content_length = content_length | mask2;
            ch = fgetc(read_file);

            counter2++;
        }

        // Jump to start of next egglet.
        fseek(read_file, content_length + 11, SEEK_CUR);
    
        printf("%s %2d  %5lu  %s\n",permission, egglet_var, content_length, name);
        
    }
    }

    else {
        int ch = 0;
        fseek(read_file, 11, SEEK_CUR);
        while ((ch = fgetc(read_file)) != EOF) {

            int counter1 = 0;
            uint16_t length_path = 0;
            uint16_t mask = 0;

            // File length from little endian.
            while (counter1 < 2) {
                length_path = length_path << 8;
                ch = fgetc(read_file);
                mask = ch;
                length_path = length_path | mask;
                counter1++;

            }
            length_path = (length_path >> 8) | (length_path << 8);

            // Printing out file name.
            int i = 0;
            while (i < length_path) {
                ch = fgetc(read_file);
                printf("%c", ch);
                i++;
            }
            printf("\n");

            int counter2 = 0;
            uint64_t content_length = 0;
            uint64_t mask2 = 0;
            
            // Finding content length so we know exactly how much to fseek.
            ch = fgetc(read_file);
            while (counter2 < 6) {
                mask2 = ch;
                mask2 = mask2 << (8 * counter2);
                content_length = content_length | mask2;
                ch = fgetc(read_file);

                counter2++;
            }

            // Jump to start of next egglet
            fseek(read_file, content_length + 11, SEEK_CUR);
   
    }
}

    fclose(read_file);
}



// Subset 1
void check_egg(char *egg_pathname) {
    FILE *read_file = fopen(egg_pathname, "r");

    // Error checking when opening file.
    if (read_file == NULL) {
        perror(egg_pathname);
        exit(1);
    }
    int ch = 0;

    // This loop loops through every single egglet format to find hash.
    while ((ch = fgetc(read_file)) != EOF) {
        int hash_value = 0;
        int byte_value = 0;
        
        fseek(read_file, -1, SEEK_CUR);
        ch = fgetc(read_file);
        hash_value = egglet_hash(hash_value, ch);

        // Byte 0 
        int magic_number = ch;
            if (magic_number != 'c') {
                fprintf(stderr, "error: incorrect first egglet byte: 0x%02x should be 0x63\n", magic_number);
                exit(1);
            }

        char name[NAME_LENGTH];
        char permission[PERMISSION];

        // Byte 1
        int egglet_var = ch;
        egglet_var = check_egglet_format(egglet_var);

        int permission_count = 0;
        // Byte 2-11
        while (permission_count < 10) {
            permission[permission_count] = fgetc(read_file);
            hash_value = egglet_hash(hash_value, permission[permission_count]);
            permission_count++;
        }

        permission[10] = '\0';

        int counter1 = 0;
        uint16_t length_path = 0;
        uint16_t mask = 0;

        ch = fgetc(read_file);
        hash_value = egglet_hash(hash_value, ch);

        // File name length convered from little endian
        while (counter1 < 2) {
            length_path = length_path << 8;

            ch = fgetc(read_file);
            hash_value = egglet_hash(hash_value, ch);

            mask = ch;
            length_path = length_path | mask;
            counter1++;

        }
        length_path = (length_path >> 8) | (length_path << 8);

        // Hashing characters from file name
        int i = 0;
        while (i < length_path) {

            ch = fgetc(read_file);
            hash_value = egglet_hash(hash_value, ch);

            name[i] = ch;
            i++;
        }
        name[i] = '\0';

        int counter2 = 0;
        uint64_t content_length = 0;
        uint64_t mask2 = 0;
        
        ch = fgetc(read_file);
        hash_value = egglet_hash(hash_value, ch);

        // Finding content length from little endian.
        while (counter2 < 6) {
            mask2 = ch;
            mask2 = mask2 << (8 * counter2);
            content_length = content_length | mask2;

            ch = fgetc(read_file);
            hash_value = egglet_hash(hash_value, ch);

            counter2++;
        }


        int counter3 = 0;
        while (counter3 < content_length - 1) {
            byte_value = fgetc(read_file);
            hash_value = egglet_hash(hash_value, byte_value);
            counter3++;
        }

        int hash_value2 = fgetc(read_file);

        if (hash_value == hash_value2) {
            printf("%s - correct hash\n", name);
        }

        else {
            printf("%s - incorrect hash 0x%02x should be 0x%02x\n", name, hash_value, hash_value2);
        }

        }

        fclose(read_file);
    
}

// Subset 2 and 3
void extract_egg(char *egg_pathname) {

    FILE *read_file = fopen(egg_pathname, "r");

    // Error checking when reading
    if (read_file == NULL) {
        perror(egg_pathname);
        exit(1);
    }

    int ch = 0;
    while ((ch = fgetc(read_file)) != EOF) {
        char name[NAME_LENGTH];
    
        int egglet_var = fgetc(read_file);
        egglet_var = check_egglet_format(egglet_var);

        int mode = 0;
    
        // Permissions with else if statement, that is for example, the first set of conditions,
        // If character is not 'd' or '-' then print an error!
        if ((ch = fgetc(read_file)) == 'd') {
            mode |= S_ISDIR(ch);
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        }

        if ((ch = fgetc(read_file)) == 'r') {
            mode |= S_IRUSR;
            
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        }

        if ((ch = fgetc(read_file)) == 'w') {
            mode |= S_IWUSR;
            
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        }

        if ((ch = fgetc(read_file)) == 'x') {
            mode |= S_IXUSR;
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        }

        if ((ch = fgetc(read_file)) == 'r') {
            mode |= S_IRGRP;
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        }

        if ((ch = fgetc(read_file)) == 'w') {
            mode |= S_IWGRP;
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        }

        if ((ch = fgetc(read_file)) == 'x') {
            mode |= S_IXGRP;
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        } 

        if ((ch = fgetc(read_file)) == 'r') {
            mode |= S_IROTH;
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        } 
        

        if ((ch = fgetc(read_file)) == 'w') {
            mode |= S_IWOTH;
        }

        else if (ch != '-') {
            fprintf(stderr, "Invalid Permission %c \n", ch);
            exit(1);

        } 

        if ((ch = fgetc(read_file)) == 'x') {
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
            ch = fgetc(read_file);
            mask = ch;
            length_path = length_path | mask;
            counter1++;

        }
        length_path = (length_path >> 8) | (length_path << 8);

        // STORING FILE NAME
        int i = 0;
        while (i < length_path) {
            ch = fgetc(read_file);
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
        ch = fgetc(read_file);
        while (counter2 < 6) {
            mask2 = ch;
            mask2 = mask2 << (8 * counter2);
            content_length = content_length | mask2;
            ch = fgetc(read_file);

            counter2++;
        }

        // WRITING CONTENT
        printf("Extracting: %s\n", name);
        int counter3 = 0;

        while (counter3 < content_length) {
            fputc(ch, fPtr);
            ch = fgetc(read_file);
            // fputc(ch, fPtr);
            counter3++;
        }

        fclose(fPtr);

        
    }

    fclose(read_file);

}



// Subset 3
void create_egg(char *egg_pathname, int append, int format,
                int n_pathnames, char *pathnames[n_pathnames]) {

    // Stored as NULL early on so no error will be given due to if-else statements
    FILE *filename = NULL;

    if (append) {
        filename = fopen(egg_pathname, "a+");
        if (filename == NULL) {
            perror(egg_pathname);
            exit(1);
        }

    }

    else {
        filename = fopen(egg_pathname, "w+");
        if (filename == NULL) {
            perror(egg_pathname);
            exit(1);
        }
    }

    // Loop, every step has to be hashed to provide a valid 
    // and correct Hashing value at the final byte of egg.

    int ch = 0;
    for (int p = 0; p < n_pathnames; p++) {

        struct stat s;
        if (stat(pathnames[p], &s) != 0) {
            perror(pathnames[p]);
            exit(1);
        }

        int hash_value = 0;

        // magic num
        ch = fputc('c', filename);
        hash_value = egglet_hash(hash_value, ch);

        // egglet format
        ch = fputc(format, filename);
        hash_value = egglet_hash(hash_value, ch);

        mode_t m = s.st_mode;
        char permission = 0;


        // Permissions
        if (S_ISREG(m)) { 
            fputc('-', filename);
            hash_value = egglet_hash(hash_value, '-');
        } 
        else if (S_ISDIR(m)) {
            fputc('d', filename);
            hash_value = egglet_hash(hash_value, 'd');
        }
    
        permission = (m & S_IRUSR) ? 'r' : '-'; 
        hash_value = egglet_hash(hash_value, permission);
        fputc(permission, filename);
        
        permission = (m & S_IWUSR) ? 'w' : '-'; 
        hash_value = egglet_hash(hash_value, permission);
        fputc(permission, filename);
        
        permission = (m & S_IXUSR) ? 'x' : '-'; 
        hash_value = egglet_hash(hash_value, permission);
        fputc(permission, filename);
    
        // group permission
        permission = (m & S_IRGRP) ? 'r' : '-'; 
        hash_value = egglet_hash(hash_value, permission);
        fputc(permission, filename);
        
        permission = (m & S_IWGRP) ? 'w' : '-'; 
        hash_value = egglet_hash(hash_value, permission);
        fputc(permission, filename);
        
        permission = (m & S_IXGRP) ? 'x' : '-'; 
        hash_value = egglet_hash(hash_value, permission);
        fputc(permission, filename);
        
        // other permission
        permission = (m & S_IROTH) ? 'r' : '-'; 
        hash_value = egglet_hash(hash_value, permission);
        fputc(permission, filename);
        
        permission = (m & S_IWOTH) ? 'w' : '-'; 
        hash_value = egglet_hash(hash_value, permission);
        fputc(permission, filename);
        
        permission = (m & S_IXOTH) ? 'x' : '-'; 
        hash_value = egglet_hash(hash_value, permission);
        fputc(permission, filename);
        
        // Pathname length
        int length = strlen(pathnames[p]);
        unsigned char byte1 = (length & 255);
        unsigned char byte2 = ((length >> 8) & 255);
        ch = fputc(byte1, filename);
        hash_value = egglet_hash(hash_value, ch);
        ch = fputc(byte2, filename);
        hash_value = egglet_hash(hash_value, ch);

        // Pathname with hashes.
        int counter = 0;
        printf("Adding: %s\n", pathnames[p]);
        while (pathnames[p][counter] != '\0') {
            ch = fputc(pathnames[p][counter], filename);
            hash_value = egglet_hash(hash_value, ch);
            counter++;
        }

        // FINDING CONTENT LENGTH
        unsigned long counter1 = 0;
        FILE *content = fopen(pathnames[p], "r");
        if (content == NULL) {
            perror(pathnames[p]);
            exit(1);
        }
        while (fgetc(content) != EOF) {
            counter1++;
        }

        fclose(content);


        // Content Length, Given at this order so that it is in Little Endian.
        int bytes6 = counter1 & 0xFF;
        ch = fputc(bytes6, filename);
        hash_value = egglet_hash(hash_value, ch);

        int bytes5 = (counter1 >> 8) & 0xFF;
        ch = fputc(bytes5, filename);
        hash_value = egglet_hash(hash_value, ch);

        int bytes4 = (counter1 >> 16) & 0xFF;
        ch = fputc(bytes4, filename);
        hash_value = egglet_hash(hash_value, ch);

        int bytes3 = (counter1 >> 24) & 0xFF;
        ch = fputc(bytes3, filename);
        hash_value = egglet_hash(hash_value, ch);

        int bytes2 = (counter1 >> 32) & 0xFF;
        ch = fputc(bytes2, filename);
        hash_value = egglet_hash(hash_value, ch);

        int bytes1 = (counter1 >> 40) & 0xFF;
        ch = fputc(bytes1, filename);
        hash_value = egglet_hash(hash_value, ch);



        // Writing content.
        FILE *content2 = fopen(pathnames[p], "r");
        if (content2 == NULL) {
            perror(pathnames[p]);
            exit(1);
        }   
        int counter2 = 0;
        while (counter2 < counter1) {
            ch = fgetc(content2);
            fputc(ch, filename);
            hash_value = egglet_hash(hash_value, ch);
            counter2++;
        }

        fputc(hash_value, filename);
        fclose(content2);
        
        
    }

    fclose(filename);

}



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

