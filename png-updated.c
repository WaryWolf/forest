#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "forest.h"    
    
/* Given "bitmap", this returns the pixel of bitmap at the point 
   ("x", "y"). */
/*
static pixel_t * pixel_at (bitmap_t * bitmap, int x, int y)
{
    return bitmap->pixels + bitmap->width * y + x;
}
*/

/* Write "bitmap" to a PNG file specified by "path"; returns 0 on
   success, non-zero on error. */

void out_png (forest* f) {

    FILE * fp;
    char path[50];
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_byte ** row_pointers = NULL;
    /* The following number is set by trial and error only. I cannot
       see where it it is documented in the libpng manual.
    */
    int pixel_size = 3;
    int depth = 8;
    

    sprintf(path, "out/img%05d.png",f->time);

    fp = fopen (path, "wb");
    if (! fp) {
        return;
        //goto fopen_failed;
    }

    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        fclose (fp);
        return;
        //goto png_create_write_struct_failed;
    }
    
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL) {

        png_destroy_write_struct (&png_ptr, &info_ptr);
        fclose (fp);
        return;
        //goto png_create_info_struct_failed;
    }
    
    /* Set up error handling. */

    if (setjmp (png_jmpbuf (png_ptr))) {

        png_destroy_write_struct (&png_ptr, &info_ptr);
        fclose (fp);
        return;
        //goto png_failure;
    }
    
    /* Set image attributes. */

    png_set_IHDR (png_ptr,
                  info_ptr,
                  f->dimX,
                  f->dimY,
                  depth,
                  PNG_COLOR_TYPE_RGB,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);
    
    /* Initialize rows of PNG. */

    row_pointers = png_malloc (png_ptr, f->dimY * sizeof (png_byte *));
    for (y = 0; y < f->dimY; ++y) {
        png_byte *row = 
            png_malloc (png_ptr, sizeof (uint8_t) * f->dimY * pixel_size);
        row_pointers[y] = row;
        for (x = 0; x < f->dimX; ++x) {
            //pixel_t * pixel = pixel_at (bitmap, x, y);
            switch(f->newGrid[(y * f->dimY) + x].status) {
                
                case EMPTY:
                    *row++ = 0;
                    *row++ = 0;
                    *row++ = 0;
                    break;

                case TREE:
                    *row++ = 0;
                    *row++ = f->newGrid[(y * f->dimY) + x].age;
                    *row++ = 0;
                    break;

                case BURN:
                    *row++ = 255;
                    *row++ = 0;
                    *row++ = 0;
                    break;

                default:
                    *row++ = 0;
                    *row++ = 0;
                    *row++ = 255;
            }
            //*row++ = pixel->red;
            //*row++ = pixel->green;
            //*row++ = pixel->blue;
        }
    }
    
    /* Write the image data to "fp". */

    png_init_io (png_ptr, fp);
    png_set_rows (png_ptr, info_ptr, row_pointers);
    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    
    for (y = 0; y < f->dimY; y++) {
        png_free (png_ptr, row_pointers[y]);
    }
    png_free (png_ptr, row_pointers);
   


    // horrible gotos....
    /* 
    png_failure:
    png_create_info_struct_failed:
        png_destroy_write_struct (&png_ptr, &info_ptr);
     png_create_write_struct_failed:
        fclose (fp);
    fopen_failed:
        return status;
    */
}
/*
void out_png(forest* f) {

    int x;
    int y;
    char buff[50];
    bitmap_t* out = f->png;


    for (y = 0; y < out->height; y++) {
        for (x = 0; x < out->width; x++) {
            
            pixel_t * pixel = pixel_at (out, x, y);
            
            switch(f->newGrid[x][y]->status) {
                case EMPTY:
                    pixel->red = 0;
                    pixel->green = 0;
                    pixel->blue = 0;
                    break;
                
                case TREE:
                    pixel->red = 0;
                    pixel->green = f->newGrid[x][y]->age;
                    //pixel->green = 255;
                    pixel->blue = 0;
                    break;
              
                case BURN:
                    pixel->red = 255;
                    pixel->green = 0;
                    pixel->blue = 0;
                    break;

                default:
                    pixel->red = 0;
                    pixel->blue = 255;
                    pixel->green = 0;
            }
        }
    }

    sprintf(buff, "out/img%05d.png",f->time);
    save_png_to_file (out, buff);

}
*/
