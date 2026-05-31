#include "decode.h"
#include <stdlib.h>

//colures 
#define CLR_RESET   "\033[0m"
#define CLR_RED     "\033[1;31m"
#define CLR_GREEN   "\033[1;32m"
#define CLR_YELLOW  "\033[1;33m"
#define CLR_BLUE    "\033[1;34m"
#define CLR_CYAN    "\033[1;36m"

#define BORDER "----------------------------------------"


//Function: read_and_validate_decode_args
Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo)
{
    if (argc < 3 || argc > 4)     // Validate number of arguments

        return e_failure;

    // Check if input BMP filename is valid
    char *dot_pos = strchr(argv[2], '.');  //find dot

    if (dot_pos && strcmp(dot_pos, ".bmp") == 0)
    {
        decInfo->output_fname = argv[2];
    }
    else
    {
        printf(CLR_RED "[ERROR] Input file must be a .bmp image\n" CLR_RESET);
        return e_failure;
    }

    decInfo->stego_image_fname = (char *)malloc(15);
    if (decInfo->stego_image_fname == NULL)
    {
        fprintf(stderr, CLR_RED "[ERROR] Memory allocation failed\n" CLR_RESET);
        return e_failure;
    }

    if (argv[3] == NULL)
    {
        strcpy(decInfo->stego_image_fname, "decode_secret");
    }
    else
    {
        if (strchr(argv[3], '.') == NULL)
        {
            strcpy(decInfo->stego_image_fname, argv[3]);
        }
        else
        {
            printf(CLR_YELLOW "[! WARNING] Do not include file extension\n" CLR_RESET);
            return e_failure;
        }
    }

    return e_success;
}


//Function: open_decode_files
Status open_decode_files(DecodeInfo *decInfo)
{
    // Open the stego image file in read-binary mode
    decInfo->fptr_output = fopen(decInfo->output_fname, "rb");

    if (decInfo->fptr_output == NULL)
    {
        perror("fopen"); //print error
        fprintf(stderr, CLR_RED "Unable to open file %s\n" CLR_RESET, decInfo->output_fname);
        return e_failure;
    }

    printf(CLR_GREEN "Stego image opened successfully\n" CLR_RESET);
    return e_success;
}

//Function: decode_magic_string
Status decode_magic_string(DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_output, 54L, SEEK_SET);
    char buffer[3];

    printf(CLR_CYAN "\nDECODING MAGIC STRING\n%s\n" CLR_RESET, BORDER);

    if (decode_image_to_data(buffer, 2, decInfo->fptr_output) == e_failure)
    {
        printf(CLR_RED "Magic string decoding failed\n" CLR_RESET);
        return e_failure;
    }

    buffer[2]='\0'; //NUll terminate the string

    printf(CLR_GREEN "Magic string decoded successfully\n" CLR_RESET);
    printf("Magic String : %s\n",buffer);

    if (strcmp(buffer, MAGIC_STRING) != 0)
    {
        printf(CLR_RED "Magic string mismatch\n" CLR_RESET);
        return e_failure;
    }

    return e_success;
}

//Function: decode_secret_file_extn
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[decInfo->extn_size + 1];  //temparary buffer
    memset(buffer, 0, sizeof(buffer));

    printf(CLR_CYAN "\nDECODING FILE EXTENSION\n%s\n" CLR_RESET, BORDER);

    if (decode_image_to_data(buffer, decInfo->extn_size, decInfo->fptr_output) == e_failure)
    {
        printf(CLR_RED "Extension decoding failed\n" CLR_RESET);
        return e_failure;
    }

    strcpy(decInfo->extn_secret_file, buffer);
    printf(CLR_GREEN "File extension decoded: %s\n" CLR_RESET, buffer);

    return e_success;
}

// Function: decode_secret_file_extn_size
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];  //temapary buffer

    printf(CLR_CYAN "\nDECODING EXTENSION SIZE\n%s\n" CLR_RESET, BORDER);

    // Read 32 bytes from stego image file
    if (fread(buffer, sizeof(char), 32, decInfo->fptr_output) != 32)
    {
        printf(CLR_RED "[ERROR] Unable to read extension size\n" CLR_RESET);
        return e_failure;
    }

    decode_lsb_to_size(&decInfo->extn_size, buffer);
    printf(CLR_GREEN "Extension size : %d bytes\n" CLR_RESET, decInfo->extn_size);

    return e_success;
}

//Function: decode_lsb_to_size
Status decode_lsb_to_size(int *data, char *image_buffer)
{
    *data = 0;  //intialize to 0

    for (int i = 0; i < 32; i++)
    {
        int bit = image_buffer[i] & 1;
        *data = (*data << 1) | bit;  // Shift previous bits left and OR with current bit

    }
    return e_success;
}

//Function: decode_secret_file_size
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32]; //temparary buffer

    printf(CLR_CYAN "\nDECODING SECRET FILE SIZE\n%s\n" CLR_RESET, BORDER);

    // Read 32 bytes from the stego image
    if (fread(buffer, sizeof(char), 32, decInfo->fptr_output) != 32)
    {
        printf(CLR_RED "[ERROR] Unable to read secret file size\n" CLR_RESET);
        return e_failure;
    }
    
    // Decode the 32 LSBs in buffer to get the file siz
    decode_lsb_to_size((int *)&decInfo->size_output_file, buffer);

    printf(CLR_GREEN "Secret file size : %ld bytes\n" CLR_RESET, decInfo->size_output_file);

    return e_success;
}

// Function: decode_secret_file_data
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    strcat(decInfo->stego_image_fname, decInfo->extn_secret_file);
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "w");

    if (decInfo->fptr_stego_image == NULL)
    {
        printf(CLR_RED "[ERROR] Unable to create output file\n" CLR_RESET);
        return e_failure;
    }

    char buffer[decInfo->size_output_file + 1];

    printf(CLR_CYAN "\nDECODING SECRET DATA\n%s\n" CLR_RESET, BORDER);

    if (decode_image_to_data(buffer, decInfo->size_output_file, decInfo->fptr_output) == e_failure)
    {
        printf(CLR_RED "[ERROR] Secret data decoding failed\n" CLR_RESET);
        return e_failure;
    }
    
    // Write the decoded data to the output file
    fwrite(buffer, sizeof(char), decInfo->size_output_file, decInfo->fptr_stego_image);
    printf(CLR_GREEN "Secret data written successfully\n" CLR_RESET);

    return e_success;
}

//Function: decode_image_to_data
Status decode_image_to_data(char *data, int size, FILE *fptr_stego_image)
{
    char buffer[8];

    for (int i = 0; i < size; i++)
    {
        // Read 8 bytes from the stego image
        if (fread(buffer, sizeof(char), 8, fptr_stego_image) != 8)
        {
            printf(CLR_RED "[ERROR] Unable to read pixel data\n" CLR_RESET);
            return e_failure;
        }

        // Decode the 8 bytes into a single secret byte
        decode_lsb_to_byte(&data[i], buffer);
    }

    return e_success;
}

//Function: decode_lsb_to_byte
Status decode_lsb_to_byte(char *data, char *image_buffer)
{
    *data = 0; // Initialize output byt

    for (int i = 0; i < 8; i++)
    {
        int bit = image_buffer[i] & 1;
        *data = (*data << 1) | bit; // Shift left and set LSB
    }
    
    return e_success;
}

/*
 * Function: do_decoding
 * ---------------------
 * Orchestrates the complete decoding process of a secret file from a stego BMP image.
 * Steps performed:
 * 1. Open the stego image file.
 * 2. Verify magic string to confirm correct stego image.
 * 3. Decode the secret file extension size.
 * 4. Decode the secret file extension.
 * 5. Decode the secret file size.
 * 6. Decode the actual secret file data and write it to output.
 * 7. Close all files and free dynamically allocated memory.
*/

Status do_decoding(DecodeInfo *decInfo)
{
    if (open_decode_files(decInfo) == e_failure) 
    return e_failure;

    if (decode_magic_string(decInfo) == e_failure) 
    return e_failure;

    if (decode_secret_file_extn_size(decInfo) == e_failure) 
    return e_failure;

    if (decode_secret_file_extn(decInfo) == e_failure) 
    return e_failure;

    if (decode_secret_file_size(decInfo) == e_failure) 
    return e_failure;

    if (decode_secret_file_data(decInfo) == e_failure) 
    return e_failure;

    fclose(decInfo->fptr_output);
    fclose(decInfo->fptr_stego_image);
    free(decInfo->stego_image_fname);

    // printf(CLR_GREEN "\nDECODING COMPLETED SUCCESSFULLY\n%s\n" CLR_RESET,BORDER);
    return e_success;
}
