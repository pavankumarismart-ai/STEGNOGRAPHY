/*

 Language: C
 Name:P.V.PAVAN KUMAR
 Date: 27/01/2026
 
 * Project: LSB Steganography for BMP Images
 * ------------------------------------------
 *
 * Description:
 * This project implements a simple steganography tool that can hide and retrieve
 * secret files inside BMP images using the Least Significant Bit (LSB) technique.
 *
 * Functionality:
 * 1. **Encoding Mode (-e)**:
 *    - Hides a secret file (e.g., text, code) inside a BMP image.
 *    - Embeds a magic string to identify valid stego images.
 *    - Stores the secret file's extension and size.
 *    - Writes the modified image as a stego image.
 *
 * 2. **Decoding Mode (-d)**:
 *    - Extracts the hidden secret file from a stego BMP image.
 *    - Verifies the magic string to ensure the image contains hidden data.
 *    - Retrieves the secret file extension and size.
 *    - Reconstructs the original secret file and saves it.
 *
 * Technical Details:
 * - Uses 24-bit BMP images for simplicity.
 * - Each byte of secret data is stored in the LSBs of 8 image bytes.
 * - Supports files with small extensions (up to 5 characters).
 *
 * Usage:
 *  - Encoding: ./steg -e <source_bmp> <secret_file> <output_stego>
 *  - Decoding: ./steg -d <stego_image> [output_file_prefix]
 *
 * This project demonstrates how digital information can be hidden in images
 * while preserving the original image visually.
 * 
 ***************************************************************************

 */


#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

//COLORS
#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define CYAN    "\033[1;36m"

#define BORDER  "************************************************"

//Determines whether the user selected encode or decode mode
OperationType check_operation_type(int argc,char *argv[])
{
    if (argc < 2) 
        return e_unsupported;

    if (strcmp(argv[1], "-e") == 0)
        return e_encode;

    else if (strcmp(argv[1], "-d") == 0)
        return e_decode;
    
    return e_unsupported;
}


//Main function for steganography application

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;

    //print project header
    printf(CYAN "▶ LSB Steganography Tool - Hide & Retrieve Secret Files in BMP Images\n" RESET);

    //ENCODING MODE 
    if (check_operation_type(argc,argv) == e_encode)
    {
        printf(YELLOW "MODE SELECTED : ENCODING\n\n" RESET);

        if (read_and_validate_encode_args(argc, argv, &encInfo) == e_success)
        {
            if (do_encoding(&encInfo) == e_success)
            {
                printf(GREEN "\n%s\n" RESET, BORDER);
                printf(GREEN "ENCODING COMPLETED SUCCESSFULLY\n" RESET);
                printf(GREEN "\n%s\n" RESET, BORDER);
            }
            else
            {
                printf(RED "\n%s\n" RESET, BORDER);
                printf(RED "[Error] ENCODING FAILED\n" RESET);
            }
        }
        else
        {
            printf(RED "\n%s\n" RESET, BORDER);
            printf(RED "INVALID ENCODING ARGUMENTS\n" RESET);
            return e_failure;
        }
    }

    // DECODING MODE
    else if (check_operation_type(argc,argv) == e_decode)
    {
        printf(YELLOW "MODE SELECTED : DECODING\n\n" RESET);

        if (read_and_validate_decode_args(argc, argv, &decInfo) == e_success)
        {
            if (do_decoding(&decInfo) == e_success)
            {
                printf(GREEN "\n%s\n" RESET, BORDER);
                printf(GREEN "DECODING COMPLETED SUCCESSFULLY\n" RESET);
                printf(GREEN "\n%s\n" RESET, BORDER);
            }
            else
            {
                printf(RED "\n%s\n" RESET, BORDER);
                printf(RED "[Error] DECODING FAILED\n" RESET);
            }
        }
        else
        {
            printf(RED "\n%s\n" RESET, BORDER);
            printf(RED "[Error]INVALID DECODING ARGUMENTS\n" RESET);

            return e_failure;
        }
    }

    //INVALID MODE
    else
    {
        printf(RED "\n%s\n" RESET, BORDER);
        printf(RED "<UNSUPPORTED OPERATION>\n" RESET);
        printf(RED "Use -e for Encoding or -d for Decoding\n" RESET);
        printf(RED "%s\n\n" RESET, BORDER);
        return e_failure;
    }
    return 0;
}
