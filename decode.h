

#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include <string.h>
#include "types.h"   // user-defined data types (Status, etc.)
#include "common.h"  // common macros like MAGIC_STRING

/*
 * DecodeInfo Structure
 * Stores all information required for decoding a secret file from a stego BMP image
 
 * It keeps track of:
 *  - The stego image being read
 *  - The output file being generated
 *  - Information needed to reconstruct the secret data
 */

typedef struct _DecodeInfo
{
    /* Secret Output File Information */

    char *output_fname;            // Name of the stego image file
    FILE *fptr_output;             // File pointer to stego image
    int extn_size;                 // Size of the secret file extension
    char extn_secret_file[5];      // Secret file extension (e.g., .txt, .c)
    long size_output_file;         // Size of the secret file data

    /* Decoded Secret File Information */

    char *stego_image_fname;       // Name of the decoded output file
    FILE *fptr_stego_image;        // File pointer for decoded secret file

} DecodeInfo;



/* Macros for decoding buffer sizes */

#define MAX_SECRET_BUF_SIZE 1                  // Number of bytes decoded at a time
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8) // 8 image bytes per secret byte
#define MAX_FILE_SUFFIX 5                      // Maximum extension length


/*
  Function Prototypes:
 
  These functions implement the decoding process step by step.
*/

/* Validate command-line arguments for decoding */
Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo);

/* Main decoding controller function */
Status do_decoding(DecodeInfo *decInfo);

/* Open the stego image file for reading */
Status open_decode_files(DecodeInfo *decInfo);

/* Decode and verify the magic string */
Status decode_magic_string(DecodeInfo *decInfo);

/* Decode the size of the secret file extension */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decode the secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode the size of the secret file */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode the actual secret file data and store it */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Extract hidden data from image LSBs */
Status decode_image_to_data(char *data, int size, FILE *fptr_stego_image);

/* Convert 8 image LSBs into a single character */
Status decode_lsb_to_byte(char *data, char *image_buffer);

/* Convert 32 image LSBs into an integer value */
Status decode_lsb_to_size(int *data, char *image_buffer);

#endif
