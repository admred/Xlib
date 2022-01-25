#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <png.h>

#define WIDTH 960
#define HEIGHT 480

#define FILENAME "path835.png"


static Display* display=NULL;
static const int screen_num=0;
static Window window=0;

void create_window() {
    uint64_t black=0x00ffffff;
    uint32_t border_width=0;
    uint64_t border=0;
    int x=0;
    int y=0;


    if(display == NULL){
        display=XOpenDisplay(getenv("DISPLAY"));
    }
    assert(display != NULL);

    window=XCreateSimpleWindow(display ,RootWindow(display,screen_num),x,y,WIDTH,HEIGHT,border_width,border,black);
    assert(window != 0);
    

    XMapWindow(display,window);
    XFlush(display);
}


void destroy_window(){
    if(display != NULL && window != 0){
        XDestroyWindow(display,window);
        window=0;
    }
    if(display != NULL){
        XCloseDisplay(display);
        display=NULL;
    }
}

/* TODO: here is when are proccesed all events */
void mainloop(){

    sleep(3); /* wait 3 secs */
}

int  load_png(char *fname,char **buf,int *width,int  *height)
{
	png_structp png_ptr;
	png_infop  info_ptr;
	unsigned char hdr[8];
	uint8_t color_type;
	uint32_t bytes_per_pixel=3;
	FILE *fp;
	int x,y;
	char  *row;
		
	fp=fopen(fname,"rb");
	if(fp==NULL)
		return True;
	
	fread(hdr,1,8,fp);
	if(png_sig_cmp(hdr,0,8)){
		fclose(fp);
		return True;
	}
		
	png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
	
	if(!png_ptr){
		fclose(fp);
		return True;
    }
	
    info_ptr=png_create_info_struct(png_ptr);

	if(!info_ptr){
	    png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
		fclose(fp);
		return True;
    }
		
	if(setjmp(png_jmpbuf(png_ptr))){
	    png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
		fclose(fp);
		return True;
    }
	
		
	png_init_io(png_ptr,fp);
	png_set_sig_bytes(png_ptr,8);
	png_read_info(png_ptr,info_ptr);

	color_type=png_get_color_type(png_ptr,info_ptr);
	if(color_type&PNG_COLOR_MASK_PALETTE){
		png_set_expand(png_ptr);  /* 8 bit  -> 24/32 bit RGB */
		png_read_update_info(png_ptr,info_ptr);
		color_type=png_get_color_type(png_ptr,info_ptr);
	}
	if(color_type&PNG_COLOR_MASK_ALPHA){
		bytes_per_pixel=4;  /* RGBA 32bit*/
	}else{ 
		bytes_per_pixel=3;  /* RGB 24 bit*/
	}

	*width=png_get_image_width(png_ptr,info_ptr);
	*height=png_get_image_height(png_ptr,info_ptr);
	*buf=(char*)malloc((*width)*(*height)*sizeof(int));
	
	row=(char*)malloc((*width)*4);
	
	for(y=0;y<(*height);y++){
		png_read_row(png_ptr,(png_bytep)row,NULL);  /* RGBRGBRGBRGB ... */
		for(x=0;x<(*width);x++){
			*(uint32_t*)(*buf+y*(*width)*4+x*4)=(uint32_t)(row[x*bytes_per_pixel]<<16|row[x*bytes_per_pixel+1]<<8|row[x*bytes_per_pixel+2]); /* 0RGB0RGB0RGB ... */			
		}
	}
	free((void*)row);	
	png_read_end(png_ptr,info_ptr);
	png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
	fclose(fp);	
    return False;
}

void draw_image(char *path,char *buf){
    int width;
    int height;
    int err;
    XImage *ximage;

    err=load_png(path,&buf,&width,&height);
    assert(err == False);

    ximage=XCreateImage(display,DefaultVisual(display,screen_num),DefaultDepth(display,screen_num),ZPixmap,0,buf,width,height,32,0);
    assert( ximage != NULL );

    XPutImage(display,window,DefaultGC(display,screen_num),ximage,0,0,0,0,width,height);

    XDestroyImage(ximage);
}

int main(){
    char *buf=NULL;
    create_window();
    draw_image(FILENAME,buf);
    sleep(3);  /* wait 3 secs */
    destroy_window();
    if(buf != NULL)
        free((void*)buf);
    return 0;
}
