#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "types.h"

//colures
#define CLR_RESET   "\033[0m"
#define CLR_RED     "\033[1;31m"
#define CLR_GREEN   "\033[1;32m"
#define CLR_YELLOW  "\033[1;33m"
#define CLR_BLUE    "\033[1;34m"
#define CLR_CYAN    "\033[1;36m"

#define BORDER "--------------------------------------------------"

// Function: read_and_validate_encode_args
Status read_and_validate_encode_args(int argc, char *argv[], EncodeInfo *encInfo)
{
    if (argc < 4 || argc > 5)
        return e_failure;

    char *dot_pos = strchr(argv[2], '.');

    if (dot_pos && strcmp(dot_pos, ".bmp") == 0)
        encInfo->src_image_fname = argv[2];
    else
    {
        printf(CLR_RED "[✘ ERROR] Source image must be .bmp\n" CLR_RESET);
        return e_failure;
    }

    dot_pos = strchr(argv[3], '.');
    if (dot_pos && strcmp(dot_pos, ".txt") == 0)
    {
        encInfo->secret_fname = argv[3];
        strcpy(encInfo->extn_secret_file, dot_pos);
    }
    else
    {
        printf(CLR_RED "[✘ ERROR] Secret file must be .txt\n" CLR_RESET);
        return e_failure;
    }

    if (argv[4] == NULL)
        encInfo->stego_image_fname = "stego.bmp";
    else
    {
        dot_pos = strchr(argv[4], '.');
        if (dot_pos && strcmp(dot_pos, ".bmp") == 0)
            encInfo->stego_image_fname = argv[4];
        else
        {
            printf(CLR_YELLOW "[! WARNING] Output image must be .bmp\n" CLR_RESET);
            return e_failure;
        }
    }

    return e_success;
}

//Function: open_files
Status open_files(EncodeInfo *encInfo)
{
    // Open source image file in read-binary mode
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, CLR_RED "Error: Unable to open source image file %s\n" CLR_RESET, encInfo->src_image_fname);
        return e_failure;
    }

    // Open secret file 
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, CLR_RED "Error: Unable to open secret file %s\n" CLR_RESET, encInfo->secret_fname);
        return e_failure;
    }

    // Open stego image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, CLR_RED "Error: Unable to create stego image file %s\n" CLR_RESET, encInfo->stego_image_fname);
        return e_failure;
    }

    printf(CLR_GREEN "All files opened successfully\n" CLR_RESET);
    return e_success;
}

//Function: check_capacity
Status check_capacity(EncodeInfo *encInfo)
{
    printf(CLR_CYAN "\nCHECKING IMAGE CAPACITY\n%s\n" CLR_RESET, BORDER);
    
    // Get usable image capacity
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    
    //Get size of the secret file in bytes 
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    if (encInfo->image_capacity >
                                8 * (encInfo->size_secret_file +
                                strlen(MAGIC_STRING) +
                                4 +
                                strlen(encInfo->extn_secret_file) +4))
    {
        printf(CLR_GREEN "Image capacity is sufficient\n" CLR_RESET);
        return e_success;
    }

    printf(CLR_RED "Image capacity insufficient\n" CLR_RESET); //image does not have enoughf space
    return e_failure;
}

//Function: copy_bmp_header
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54]; //BMP header size is 54 bytes

    rewind(fptr_src_image);  //move file pointer to start
    
    //read BMP header from sourc image
    fread(buffer, sizeof(char), 54, fptr_src_image);

    //write BMP header from source image
    fwrite(buffer, sizeof(char), 54, fptr_dest_image);

   // printf(CLR_GREEN "BMP header copied successfully\n" CLR_RESET);
    return e_success;
}

//Function: encode_magic_string
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    printf(CLR_CYAN "\nENCODING MAGIC STRING\n%s\n" CLR_RESET, BORDER);

    if(encode_data_to_image((char *)magic_string,
                          strlen(magic_string),
                          encInfo->fptr_src_image,
                          encInfo->fptr_stego_image)==e_failure)
    {
        printf(CLR_RED "Failed to encode magic string\n" CLR_RESET);
        return e_failure;
    }

    printf(CLR_GREEN "Magic string encoded\n" CLR_RESET);
    return e_success;
}

//Function: Encode secret file extension size 
Status encode_secret_file_extn_size(long file_size, EncodeInfo *encInfo)
{
    printf(CLR_CYAN "\nENCODING EXTENSION SIZE\n%s\n" CLR_RESET, BORDER);
    
    char buffer[32]; // 4 bytes * 8 bits
    
    // Read 32 bytes from source image 
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
         printf(CLR_RED "[ERROR] Failed to read image data\n" CLR_RESET);
        return e_failure;
    }

    //Encode size into LSBs of image bytes
    encode_size_to_lsb(file_size, buffer);

    // Write modified bytes to stego image 
    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        printf(CLR_RED "[ERROR] Failed to write stego image data\n" CLR_RESET);
        return e_failure;
    }
    
    printf(CLR_GREEN "Extension size encoded\n" CLR_RESET);
    return e_success;
}

// Encode secret file extension 
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    printf(CLR_CYAN "\nENCODING FILE EXTENSION\n%s\n" CLR_RESET, BORDER);
    
    if(encode_data_to_image((char *)file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image)==e_failure)
    {
        printf(CLR_RED "Failed to encode file extension\n" CLR_RESET);
        return e_failure;
    }
    
    printf(CLR_GREEN "File extension encoded\n" CLR_RESET);
    return e_success;
}

//Encode size of secret file 
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    printf(CLR_CYAN "\nENCODING SECRET FILE SIZE\n%s\n" CLR_RESET, BORDER);
    
    char buffer[32];
    
    //Read 32 bytes from source image 
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        printf(CLR_RED "[ERROR] Failed to read image data\n" CLR_RESET);
        return e_failure;
    }
    
    //Encode file size into LSBs
    encode_size_to_lsb(file_size, buffer);

    //Write modified bytes to stego image 
    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        printf(CLR_RED "[ERROR] Failed to write stego image data\n" CLR_RESET);
        return e_failure;
    }

    printf(CLR_GREEN "Secret file size encoded\n" CLR_RESET);
    return e_success;
}

//Function: encode_secret_file_data
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    rewind(encInfo->fptr_secret); //move file pointer to start

    // Allocate memory to store entire secret file data
    char *secret_data = malloc(encInfo->size_secret_file);
    
    if (secret_data == NULL)  // Check if memory allocation failed
    {
        printf(CLR_RED "[ERROR] Memory allocation failed\n" CLR_RESET);
        return e_failure;
    }

    fread(secret_data, 1, encInfo->size_secret_file, encInfo->fptr_secret);

    printf(CLR_CYAN "\nENCODING SECRET DATA\n%s\n" CLR_RESET, BORDER);

    if(encode_data_to_image(secret_data,
                          encInfo->size_secret_file,
                          encInfo->fptr_src_image,
                          encInfo->fptr_stego_image)==e_failure)
    {
        printf(CLR_RED "[ERROR] Failed to encode secret data\n" CLR_RESET);
        free(secret_data);
        return e_failure;
    }
    
    // Free allocated memory after encoding
    free(secret_data);

    printf(CLR_GREEN "Secret data encoded successfully\n" CLR_RESET);
    return e_success;
}



//Function: get_file_size
uint get_file_size(FILE *fptr)
{
    uint size;
    fseek(fptr, 0, SEEK_END); // Move file pointer to the end of the file
    size = ftell(fptr);  // Get current position of file pointer
    rewind(fptr);    // Reset file pointer 
    return size;
}

//Function: get_image_size_for_bmp
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    fseek(fptr_image, 18, SEEK_SET);
    fread(&width, sizeof(int), 1, fptr_image);
    fread(&height, sizeof(int), 1, fptr_image);

    printf("Image Width  : %u\n",width);
    printf("Image Height : %u\n",height);

    rewind(fptr_image);
    return width * height * 3;
}

// Function: encode_size_to_lsb
Status encode_size_to_lsb(int data, char *image_buffer)
{
    for (int i = 0; i < 32; i++)
    {
        int bit = (data >> (31 - i)) & 1;
        image_buffer[i] = (image_buffer[i] & ~1) | bit;
    }
    return e_success;
}

//Function: encode_byte_to_lsb
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        int bit = (data >> (7 - i)) & 1;
        image_buffer[i] = (image_buffer[i] & ~1) | bit;
    }
    return e_success;
}

//Function: encode_data_to_image
Status encode_data_to_image(char *data, int size,
                            FILE *fptr_src_image,
                            FILE *fptr_stego_image)
{
    char buffer[8];

    for (int i = 0; i < size; i++)
    {
        fread(buffer, sizeof(char), 8, fptr_src_image);
        encode_byte_to_lsb(data[i], buffer);
        fwrite(buffer, sizeof(char), 8, fptr_stego_image);
    }

    return e_success;
}

//Function: copy_remaining_img_data
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) == 1)
        fwrite(&ch, 1, 1, fptr_dest);

    return e_success;
}

//Function: do_encoding//
/*
 * Steps performed:
 * 1. Open all required files
 * 2. Check whether image has enough capacity
 * 3. Copy BMP header to stego image
 * 4. Encode magic string
 * 5. Encode secret file extension size
 * 6. Encode secret file extension
 * 7. Encode secret file size
 * 8. Encode actual secret file data
 * 9. Copy remaining image data
 */

Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_failure) return e_failure;
    if (check_capacity(encInfo) == e_failure) return e_failure;

    copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image);
    encode_magic_string(MAGIC_STRING, encInfo);
    encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo);
    encode_secret_file_extn(encInfo->extn_secret_file, encInfo);
    encode_secret_file_size(encInfo->size_secret_file, encInfo);
    encode_secret_file_data(encInfo);
    copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image);

    //close all open files
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    //printf(CLR_GREEN "\n%s\n ENCODING COMPLETED SUCCESSFULLY \n%s\n" CLR_RESET, BORDER, BORDER);
    return e_success;
}
