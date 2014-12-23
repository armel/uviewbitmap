#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_MEMALIGN
    #include <malloc.h> // for memalign
#endif

unsigned char *read_bmp_file(char *file_name, int* _width, int* _height) {
    unsigned char head[54];
    FILE *f = fopen(file_name,"rb");

    // BMP header is 54 bytes
    fread(head, 1, 54, f);

    int width = head[18] + ( ((int)head[19]) << 8) + ( ((int)head[20]) << 16) + ( ((int)head[21]) << 24);
    int height = head[22] + ( ((int)head[23]) << 8) + ( ((int)head[24]) << 16) + ( ((int)head[25]) << 24);

    // lines are aligned on 4-byte boundary
    int line_size = (width / 8 + (width / 8) % 4);
    int file_size = line_size * height;

    unsigned char *img = malloc(width * height), *data = malloc(file_size);

    // skip the header
    fseek(f,54,SEEK_SET);

    // skip palette - two rgb quads, 8 bytes
    fseek(f, 8, SEEK_CUR);

    // read data
    fread(data,1,file_size,f);

    // decode bits
    int i, rev_i, j, k;
    for(i = 0, rev_i = height - 1; i < height ; i++, rev_i--) {
        for(j = 0 ; j < width / 8; j++) {
            int fpos = i * line_size + j, pos = rev_i * width + j * 8;
            for(k = 0 ; k < 8 ; k++)
                img[pos + (7 - k)] = (data[fpos] >> k ) & 1;
        }
    }

    free(data);
    *_width = width; *_height = height;
    return img;
}

void print_usage() {
    printf("Usage: uviewbitmap [-a] bmp_file\n");
}

int main(int argc, char *argv[]) {
    FILE *file;
    int i, j, width, height;
    char* file_name;
    unsigned char* bmp;

    if (argc < 2 || argc > 3) {
        print_usage();
        exit(0);
    }
    else {
        if(argc == 3)
            file_name = argv[2];
        else
            file_name = argv[1];

        file = fopen(file_name, "r");

        if (file == 0) { // fopen returns 0, the NULL pointer, on failure
            printf( "No such file\n" );
            exit(0);
        }

        bmp = read_bmp_file(file_name, &width, &height);

        if(strcmp(argv[1], "-a")) {
            printf("static const unsigned char bmp [] PROGMEM = {\n");
            printf("  ");
        }

        int new_line = 0;

        for(i = 0 ; i < height ; i++) {
            for(j = 0 ; j < width ; j++) {
                if(!strcmp(argv[1], "-a")) {
                    printf("%c ", bmp[i * width + j] ? '.' : ' ' );
                }
                else {
                    if(bmp[i * width + j] == 1) {
                        printf("0x%02x, 0x%02x, ", j, i);
                        new_line += 1;
                    }

                    if(new_line == 15) {
                        new_line = 0;
                        printf("\n  ");
                    }
                }
            }
            if(!strcmp(argv[1], "-a"))
                printf("\n");
        }
        if(strcmp(argv[1], "-a"))
            printf("};\n");
    }

    return 0;
}