#ifndef ENCODE_H
#define ENCODE_H

#include "types.h"    // Contains user-defined data types like Status, uint, enum
#include "common.h"   // Contains common macros and magic string 

/*
 * EncodeInfo Structure
 
 * This structure contains all the data required
 * to hide a secret file inside a BMP image.

 * It stores information related to:
 *  - Input image file
 *  - Secret file to be hidden
 *  - Output stego image
 *  - Buffers used during encoding
 */

// Buffer Size macros 

#define MAX_SECRET_BUF_SIZE 1                 // One byte of secret data 
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8) // 8 image bytes per secret byte(1 char)
#define MAX_FILE_SUFFIX 5                     // Max extention size

typedef struct _EncodeInfo
{
    //Source Image Information

    char *src_image_fname;            // Source BMP image filename
    FILE *fptr_src_image;             // File pointer to source image
    uint image_capacity;              // Total usable image capacity
    uint bits_per_pixel;              // Number of bits per pixel in the BMP (usually 24)
    char image_data[MAX_IMAGE_BUF_SIZE]; // Temporary buffer for reading/writing image data

    //Secret File Information 

    char *secret_fname;               // Secret file filename
    FILE *fptr_secret;                // File pointer to secret file
    char extn_secret_file[MAX_FILE_SUFFIX]; // extension of the secret file
    char secret_data[MAX_SECRET_BUF_SIZE];  // Buffer for secret data to encode
    long size_secret_file;             //  total Size of secret file in bytes

    //Stego Image Information

    char *stego_image_fname;           // Output stego image filename
    FILE *fptr_stego_image;            // File pointer to stego image

} EncodeInfo;


/*
 * Encoding Function Prototypes
 * These functions provide all the necessary operations to hide a secret
*/

/* Identify whether encode or decode operation is requested */
OperationType check_operation_type(int argc,char *argv[]);

/* Validate command-line arguments for encoding */
Status read_and_validate_encode_args(int argc, char *argv[], EncodeInfo *encInfo);

/* Main controller function for encoding process */
Status do_encoding(EncodeInfo *encInfo);

/* Open all required files (source, secret, stego) */
Status open_files(EncodeInfo *encInfo);

/* Verify whether image can hold the secret data */
Status check_capacity(EncodeInfo *encInfo);

/* Calculate usable image size from BMP file */
uint get_image_size_for_bmp(FILE *fptr_image);

/* Determine size of a file in bytes */
uint get_file_size(FILE *fptr);

/* Copy BMP header without modification */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);

/* Encode predefined magic string into image */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo);

/* Encode secret file extension length */
Status encode_secret_file_extn_size(long file_size, EncodeInfo *encInfo);

/* Encode secret file extension string */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);

/* Encode size of secret file */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo);

/* Encode actual secret file content */
Status encode_secret_file_data(EncodeInfo *encInfo);

/* Core function that embeds data into image LSBs */
Status encode_data_to_image(char *data, int size,FILE *fptr_src_image,FILE *fptr_stego_image);

/* Embed a single byte into 8 image bytes using LSB */
Status encode_byte_to_lsb(char data, char *image_buffer);

/* Embed a 32-bit integer into 32 image bytes */
Status encode_size_to_lsb(int data, char *image_buffer);

//Copy remaining image data after encoding 
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);

#endif
