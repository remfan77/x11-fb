#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>

int width, height;
int *framebuf;
Window win;
Display *dpy;
XImage *ximage;
pthread_t th;
GC NormalGC;

void *fn(void *dummy)
{
	while (1) {
		int i;
		static unsigned char cnt;
		for (i = 0; i < (width * height); i++)
		{
			framebuf[i] = cnt<<8;
		}
		cnt++;

		// XLockDisplay(dpy);
		XPutImage(dpy, win, NormalGC, ximage, 0, 0, 0, 0, width, height);
		// XUnlockDisplay(dpy);
		usleep(10000);
	}
}

int main(int argc, char **argv)
{

	XVisualInfo vinfo;
	int depth;
	XVisualInfo *visual_list;
	XVisualInfo visual_template;
	int nxvisuals;
	int i;
	XSetWindowAttributes attrs;
	Window parent;
	Visual *visual;

	XEvent event;

	XInitThreads();
	dpy = XOpenDisplay(NULL);

	nxvisuals = 0;
	visual_template.screen = DefaultScreen(dpy);
	visual_list = XGetVisualInfo(dpy, VisualScreenMask, &visual_template, &nxvisuals);

	if (!XMatchVisualInfo(dpy, XDefaultScreen(dpy), 24, TrueColor, &vinfo))
	{
		fprintf(stderr, "no such visual\n");
		return 1;
	}

	parent = XDefaultRootWindow(dpy);

	XSync(dpy, True);

	printf("creating RGBA child\n");

	visual = vinfo.visual;
	depth = vinfo.depth;

	attrs.colormap =
	    XCreateColormap(dpy, XDefaultRootWindow(dpy), visual, AllocNone);
	attrs.background_pixel = 0;
	attrs.border_pixel = 0;

	width = 320;
	height = 240;

	framebuf = (int *)malloc((width * height) * 4);

	for (i = 0; i < (width * height); i++) {
		framebuf[i] = 0xFFFF0000;
	}

	win =
	    XCreateWindow(dpy, parent, 100, 100, width, height, 0, depth,
			  InputOutput, visual,
			  CWBackPixel | CWColormap | CWBorderPixel, &attrs);

	//Change to this line
	//ximage = XCreateImage(dpy, vinfo.visual, 32, XYPixmap, 0, (char *)framebuf, width, height, 8, width*4);
	ximage = XCreateImage(dpy, vinfo.visual, depth, ZPixmap, 0, (char *)framebuf,
		              width, height, 8, width * 4);

	if (ximage == 0) {
		printf("ximage is null!\n");
	}

	XSync(dpy, True);

	XSelectInput(dpy, win,
		     ExposureMask | KeyPressMask | ButtonPressMask |
		     ButtonReleaseMask | PointerMotionMask);

	XGCValues gcv;
	unsigned long gcm;

	//gcm = GCForeground | GCBackground | GCGraphicsExposures;
	//gcv.foreground = BlackPixel(dpy, parent);
	//gcv.background = WhitePixel(dpy, parent);
	gcm = GCGraphicsExposures;
	gcv.graphics_exposures = 0;
	NormalGC = XCreateGC(dpy, parent, gcm, &gcv);

	XMapWindow(dpy, win);

	pthread_create(&th, 0, fn, 0);
	while (!XNextEvent(dpy, &event)) {
		printf("event.type=%d  ", event.type);
		switch (event.type) {
		case Expose:
			printf("I have been exposed!\n");
			XLockDisplay(dpy);
			XPutImage(dpy, win, NormalGC, ximage, 0, 0, 0, 0, width,
				  height);
			XUnlockDisplay(dpy);
			break;

		case ButtonPress:
			switch (event.xbutton.button) {
			case Button1:
				printf("Button1 pressed x=%d y=%d\n",
				       event.xmotion.x, event.xmotion.y);
				break;
			case Button2:
				printf("Button2 pressed x=%d y=%d\n",
				       event.xmotion.x, event.xmotion.y);
				break;
			case Button3:
				printf("Button3 pressed x=%d y=%d\n",
				       event.xmotion.x, event.xmotion.y);
				break;
			case Button4:	/* Scrolled up */
				printf("Scroll up x=%d y=%d\n", event.xmotion.x,
				       event.xmotion.y);
				break;
			case Button5:	/* Scrolled down */
				printf("Scroll down x=%d y=%d\n",
				       event.xmotion.x, event.xmotion.y);
				break;
			default:
				printf("unhandled button press : %d",
				       event.xbutton.button);
			}
			break;
		case ButtonRelease:
			switch (event.xbutton.button) {
			case Button1:
				printf("Button1 release x=%d y=%d\n",
				       event.xmotion.x, event.xmotion.y);
				break;
			case Button2:
				printf("Button2 released x=%d y=%d\n",
				       event.xmotion.x, event.xmotion.y);
				break;
			case Button3:
				printf("Button3 released x=%d y=%d\n",
				       event.xmotion.x, event.xmotion.y);
				break;
			}
			break;
		case MotionNotify:
			printf("x %d y %d\n", event.xmotion.x, event.xmotion.y);
			break;
		}
	}

	printf("No error\n");

	return 0;
}
