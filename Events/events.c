#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#define WIDTH 960
#define HEIGHT 480

static Display* display=NULL;
static const int screen_num=0;
static Window window=0;
static Atom wm_delete_window;


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

    /* thanks: https://stackoverflow.com/questions/1157364/intercept-wm-delete-window-on-x11 */
    wm_delete_window=XInternAtom(display,"WM_DELETE_WINDOW",False);
    XSetWMProtocols(display,window,&wm_delete_window,1);

    XSelectInput(display,window,(ExposureMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|KeyPressMask|KeyReleaseMask|EnterWindowMask|LeaveWindowMask|ClientMessage));



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

void mainloop(){
    XEvent xevent;
    
    while (1) {
        XNextEvent(display,&xevent);
        switch(xevent.type){
            case GraphicsExpose:{
                XGraphicsExposeEvent* xev=(XGraphicsExposeEvent*)&xevent;
                printf("GraphicsExpose x = %d , y = %d , width = %d , height = %d\n",xev->x,xev->y,xev->width,xev->height);
                break;
            }
            case Expose:{
                XExposeEvent* xev=(XExposeEvent*)&xevent;
                printf("Expose x = %d , y = %d , width = %d , height = %d\n",xev->x,xev->y,xev->width,xev->height);
                break;
            }
            case KeyPress:{
                XKeyEvent* xev=(XKeyEvent*)&xevent;
                printf("KeyPress keycode = %u\n",xev->keycode);
                break;
            }
            case KeyRelease:{
                XKeyEvent* xev=(XKeyEvent*)&xevent;
                printf("KeyRelease keycode = %u\n",xev->keycode);
                break;
            }
            case ButtonPress:{
                XButtonEvent* xev=(XButtonEvent*)&xevent;
                printf("ButtonPress x = %d , y = %d , button = %d\n",xev->x,xev->y,xev->button);
                break;
            }
            case ButtonRelease:{
                XButtonEvent* xev=(XButtonEvent*)&xevent;
                printf("ButtonRelease x = %d , y = %d , button = %d\n",xev->x,xev->y,xev->button);
                break;
            }
            case MotionNotify:{
                XMotionEvent* xev=(XMotionEvent*)&xevent;
                printf("MotionNotify x = %d , y = %d\n",xev->x,xev->y);
                break;
            }
            /* thanks: https://stackoverflow.com/questions/1157364/intercept-wm-delete-window-on-x11 */
            case ClientMessage:{
                XClientMessageEvent* xev=(XClientMessageEvent*)&xevent;
                if( xev->data.l[0]  == wm_delete_window)
                    goto finloop;
                break;
            }
            default:{
                printf("Unknow event type = %x\n",xevent.type);
                break;
            }
        }
    }
finloop:
    return;
    /* */
}

int main(){
    create_window();
    mainloop();
    destroy_window();
    return 0;
}
