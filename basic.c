/* singer.c */
/* One sings can the other find a harmony? */

#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#define SA struct sockaddr 
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

/* here are our X variables */
Display *display;
XColor    color[100];
int screen;
Window win;
GC gc;
unsigned long black,white;
#define X_SIZE 1920 
//#define TX_SIZE 3840
#define TX_SIZE 20000 
#define Y_SIZE 1080 

/* here are our X routines declared! */
void init_x();
void close_x();
void redraw();

#define MAX 80 
#define PORT 9080 

/* sound */
void *spkr();
void *comms();
void *control();

static float notes[108]={
16.35,17.32,18.35,19.45,20.60,21.83,23.12,24.50,25.96,27.50,29.14,30.87,
32.70,34.65,36.71,38.89,41.20,43.65,46.25,49.00,51.91,55.00,58.27,61.74,
65.41,69.30,73.42,77.78,82.41,87.31,92.50,98.00,103.8,110.0,116.5,123.5,
130.8,138.6,146.8,155.6,164.8,174.6,185.0,196.0,207.7,220.0,233.1,246.9,
261.6,277.2,293.7,311.1,329.6,349.2,370.0,392.0,415.3,440.0,466.2,493.9,
523.3,554.4,587.3,622.3,659.3,698.0,740.0,784.0,830.6,880.0,932.3,987.8,
1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951,
4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902 }; 

//static float notes[11]={0,277.2,311.1,370.0,415.3,466.2,554.4,622.3,740.0,830.6,932.3};

struct output { short *waveform; long where; float lno; float rno; float dt; float dlpf; float dw; float dth; int vamp;};

void usage ()
{
	printf("usage: singer\n");
	exit (1);
}

int main(int argc,char *argv[])
{
 
	int *fhead,len,chan,sample_rate,bits_pers,byte_rate,ba,size,seed;
	int number,along,osc,note,leg;
	char stop;
	struct output *out;

	init_x();

        out=(struct output *)malloc(sizeof(struct output ));

	//seed=atoi(argv[1]);

        fhead=(int *)malloc(sizeof(int)*11);

        len=60*60; //1 hour
        chan=2;
        sample_rate=44100;
        bits_pers=16;
        byte_rate=(sample_rate*chan*bits_pers)/8;
        ba=((chan*bits_pers)/8)+bits_pers*65536;
        size=chan*len*sample_rate;
	leg=4;

	//srand(seed);

        out->waveform=(short *)malloc(sizeof(short)*size);

	//printf ("waiting \n");
	//scanf("%c",&stop);

       pthread_t spkr_id,comms_id,control_id;

       struct timespec tim, tim2;
               tim.tv_sec = 0;
        tim.tv_nsec = 100L;

        pthread_create(&spkr_id, NULL, spkr, out);
        pthread_create(&control_id, NULL, control, out);
        //pthread_create(&comms_id, NULL, comms, out);

	long my_point,ahead,chunk,psum,dtot,avg;
	chunk=2206;
	//ahead=4410;
	ahead=2206;
	my_point=3*ahead;
	int vamp,lamp,ramp,lnote,llnote,rnote,mnote,lcount,tnote,minnote,maxnote,lchoice,a,lattack,rattack;
	float max,min,lp,rp,fl,fr,fd,ft,thresh,fm;

	lnote=48;lcount=0;
	rnote=48;tnote=1;
	mnote=1;
	lamp=0;ramp=0;
	lchoice=10;
	a=38820;
	lattack=1;rattack=5;
	out->vamp=0;
	out->lno=48;
	out->rno=49;

	thresh=1.01;

	fr=2*M_PI*notes[lnote]/44100;

	//float mosc,losc,rosc,tosc,ph,lpf;
	double mosc,losc,rosc,tosc,ph,lpf;
	losc=0;rosc=0;tosc=0;
	avg=0; fd=0;
	min=100000;
	max=0;
	ph=0;
	lpf=2;
	int lpp,aplus;
	float ll,rr,mm,t,w;
	out->dth=1;
	out->dt=1;
	out->dlpf=2;
	out->dw=0;
	ll=0;rr=0; lpp=-5;aplus=2;
	t=1;w=0;

printf (" z: C s: C# x: D d: E flat c: E v: F g: F# b: G h: G# n: A j: B flat m: B \n q: width w: threshold point r: filter e: threshold value t: octave \n");


	while (1>0)
	{
		long trigger,diff;
		int wcount;
		trigger=my_point-ahead;
		wcount=0;
		while (out->where<trigger)
		{
                       nanosleep(&tim , &tim2);
		       wcount++;

		}
		if (wcount < 20){printf ("%d\n",wcount);}

		lcount++;

		long from,to,sum;
		from=my_point;
		to=my_point+chunk;

		//fl=out->lno;
		//fr=out->rno;
		for (my_point=from;my_point<to;my_point+=2)
		{
			short left,right;
			float leftf,rightf,midf;

			losc+=(out->lno);
			rosc+=(out->rno);
			leftf=(sin(losc+ph));
			rightf=(sin(rosc-ph));
			t+=0.000039;
			ph+=0.0001;
			if (t>1){t=0;};

			//width it random
		/*	
			if (leftf<out->dw && leftf>0) { leftf=((float)(rand()%8000))/8000;}
			if (leftf>-out->dw && leftf<0) { leftf=-((float)(rand()%8000))/8000;}
			if (rightf<out->dw && rightf>0) { rightf=((float)(rand()%8000))/8000;}
			if (rightf>-out->dw && rightf<0) { rightf=-((float)(rand()%8000))/8000;} 
			*/
		
		/*	
			// width it to 0
			if (leftf<out->dw && leftf>0) { leftf=0;}
			if (leftf>-out->dw && leftf<0) { leftf=0;}
			if (rightf<out->dw && rightf>0) { rightf=0;}
			if (rightf>-out->dw && rightf<0) { rightf=0;} 
			*/
			

			// width it rolling t.
			if (leftf<out->dw && leftf>0) { leftf=t;}
			else if (leftf>-out->dw && leftf<0) { leftf=-t;}
			if (rightf<out->dw && rightf>0) { rightf=t;}
			else if (rightf>-out->dw && rightf<0) { rightf=-t;} 


			//threshold it
			if (leftf>(out->dth) ){ leftf=out->dt;}
			else if (leftf<-(out->dth) ){ leftf=-out->dt;}
			if (rightf>(out->dth) ){rightf=out->dt;}
			else if (rightf<-(out->dth) ){rightf=-out->dt;}

			//filter it
			float dl,dr;
			dl=leftf-ll;
			dr=rightf-rr;
			if (dl>out->dlpf){ leftf=ll+out->dlpf;}
			else if (-dl>out->dlpf){ leftf=ll-out->dlpf;}
			if (dr>out->dlpf){ rightf=rr+out->dlpf;}
			else if (-dr>out->dlpf){ rightf=rr-out->dlpf;}
			ll=leftf;
			rr=rightf;


			left=out->vamp*leftf;
			right=out->vamp*rightf;
			// verb 
			out->waveform[my_point]=left/4+(4*out->waveform[my_point-a+1]/5);
			out->waveform[my_point+1]=right/4+(4*out->waveform[my_point-a]/5);
			//no verb
			//out->waveform[my_point]=left;
			//out->waveform[my_point+1]=right;
			if (out->vamp>0 && my_point%rattack==0  ){out->vamp--;}
		}
		if (lcount%leg==0){
			float v;
			long reached;
			int xd;
			reached=out->where;
			XSetForeground(display,gc,color[0].pixel);
			XFillRectangle(display, win, gc, 0, 0, X_SIZE, Y_SIZE);

			long slide,lfrom,rfrom;
			lfrom=0;rfrom=0;
			for (slide=reached-(2*X_SIZE);slide<reached;slide+=2)
			{
				if (out->waveform[slide]<0 && lfrom==0){lfrom=1;}
				if (out->waveform[slide+1]<0 && rfrom==0){rfrom=1;}
				if (lfrom==1 && out->waveform[slide]>0){ lfrom=slide;} 
				if (rfrom==1 && out->waveform[slide+1]>0){ rfrom=slide+1;} 
			}

			XSetForeground(display,gc,color[1].pixel);
			for (xd=0;xd<(X_SIZE);xd+=2)
			{
			int rp,xxx,wp;
			//wp=reached-xd-X_SIZE;
			xxx=xd;
			lp=510+(out->waveform[lfrom+xd]/111);
			XDrawPoint(display, win, gc, xxx,lp);
			XDrawPoint(display, win, gc, xxx,lp+2);
			XDrawPoint(display, win, gc, xxx,lp-2);
			XDrawPoint(display, win, gc, xxx+1,lp);
			XDrawPoint(display, win, gc, xxx-1,lp);
			XDrawPoint(display, win, gc, xxx,lp+1);
			XDrawPoint(display, win, gc, xxx-1,lp+1);
			XDrawPoint(display, win, gc, xxx+1,lp+1);
			XDrawPoint(display, win, gc, xxx,lp-1);
			XDrawPoint(display, win, gc, xxx-1,lp-1);
			XDrawPoint(display, win, gc, xxx+1,lp-1);
			}

			XSetForeground(display,gc,color[2].pixel);
			for (xd=0;xd<(X_SIZE);xd+=2)
			{
			int rp,xxx,wp;
			xxx=xd;
			rp=490+(out->waveform[rfrom+xd]/111);
			XDrawPoint(display, win, gc, xxx,rp-1);
			XDrawPoint(display, win, gc, xxx+1,rp-1);
			XDrawPoint(display, win, gc, xxx-1,rp-1);
			XDrawPoint(display, win, gc, xxx,rp);
			XDrawPoint(display, win, gc, xxx,rp-2);
			XDrawPoint(display, win, gc, xxx,rp+2);
			XDrawPoint(display, win, gc, xxx-1,rp);
			XDrawPoint(display, win, gc, xxx+1,rp);
			XDrawPoint(display, win, gc, xxx,rp+1);
			XDrawPoint(display, win, gc, xxx-1,rp+1);
			XDrawPoint(display, win, gc, xxx+1,rp+1);
			}
			XFlush(display);
		}
	} 
}



void *control(void *o) {
        // This handles the speakers.

  struct output *out;
  out=(struct output *)o;
  int rec;
  long start;
  rec=0;
  char c;
  char text[255];
  XEvent event;           /* the XEvent declaration !!! */
  KeySym key;             /* a dealie-bob to handle KeyPress Events */


	while (1>0)
	{
		int oct,no;
                XNextEvent(display, &event);
                if (event.type==KeyPress && XLookupString(&event.xkey,text,255,&key,0)==1)
                {
                        //printf ("got %c \n",text[0]);
			//out->vamp=32760;

			switch (text[0])
			{
		        case 'z': no=0; break; //C
		        case 's': no=1; break; //C#
		        case 'x': no=2; break; //D
		        case 'd': no=3; break; //Eflat
		        case 'c': no=4; break; //E
		        case 'v': no=5; break; //F
		        case 'g': no=6; break; //F#
		        case 'b': no=7; break; //G
		        case 'h': no=8; break; //G#
		        case 'n': no=9; break; //A
		        case 'j': no=10; break; //B flat
		        case 'm': no=11; break; //B
		        case 'q': out->dw+=0.01; if(out->dw>1){out->dw=0;}break; //width
		        case 'w': out->dth-=0.01; if(out->dth<0){out->dth=1;}break; //threshold point
		        case 'e': out->dt-=0.01; if(out->dt<-1){out->dt=1;}break; //thresholdi value
		        case 'r': out->dlpf-=0.005; if(out->dlpf<0.001){out->dlpf=0.3;}break; //filter
		        case 't': oct++; if(oct>7){oct=0;}break; //octave
		        case 'y': out->dw=0;out->dth=1;out->dt=1;out->dlpf=0.4;break; //reset
                        }
			if (out->vamp<29700){out->vamp+=3000;}
			out->lno=(M_PI*notes[no+(oct*12)])/22050;
			out->rno=(M_PI*notes[no+(oct*12)])/22000;
		}
	}
/*
		scanf ("%c",&c);
		if (c == 'r')
		{
		if (rec==0){rec=1;printf("Recording starting \n");start=out->where;}
		else if (rec==1){rec=0;
			printf ("Recording complete\n");

        int *fhead,chan,sample_rate,bits_pers,byte_rate,ba,size;
        fhead=(int *)malloc(sizeof(int)*11);

        chan=2;
        sample_rate=44100;
        bits_pers=16;
        byte_rate=(sample_rate*chan*bits_pers)/8;
        ba=((chan*bits_pers)/8)+bits_pers*65536;
        size=out->where-start;

        fhead[0]=0x46464952;
        fhead[1]=36;
        fhead[2]=0x45564157;
        fhead[3]=0x20746d66;
        fhead[4]=16;
        fhead[5]=65536*chan+1;
        fhead[6]=sample_rate;
        fhead[7]=byte_rate;
        fhead[8]=ba;
        fhead[9]=0x61746164;
        fhead[10]=(size*chan*bits_pers)/8;


        FILE *record;
        record=fopen("record1.wav","wb");
        fwrite(fhead,sizeof(int),11,record);
        fwrite(out->waveform+start,sizeof(short),size,record);
        fclose (record);
        free(fhead);
		}
	}
		if (c == 't'){ printf ("Gimmie threshold point 0->1  \n"); scanf("%f",&out->dth); }
		if (c == 'v'){ printf ("Gimmie threshold value -1->1 \n"); scanf("%f",&out->dt); }
		if (c == 'f'){ printf ("Gimmie filter 0->2 \n"); scanf("%f",&out->dlpf); }
		if (c == 'w'){ printf ("Gimmie width 0->1 \n"); scanf("%f",&out->dw); }
		printf ("threshold point (t) %f threshold value (v) %f filter (f) %f width (w) %f\n",out->dth,out->dt,out->dlpf,out->dw);
	}
	*/
}



void *spkr(void *o) {
        // This handles the speakers.

  struct output *out;
  int rc;
  int size;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;

  out=(struct output *)o;

  //char *buffer;

  /* Open PCM device for playback. */
  rc = snd_pcm_open(&handle, "default",
                    SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {
    fprintf(stderr,
            "unable to open pcm device: %s\n",
            snd_strerror(rc));
    exit(1);
  }
  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);
  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);
  /* Set the desired hardware parameters. */
  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(handle, params, 2);
  /* 44100 bits/second sampling rate (CD quality) */
  val = 44100;
  snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
  /* Set period size to 32 frames. */
  frames = 64;
  snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
  /* Write the parameters to the aux */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    fprintf(stderr,
            "unable to set hw parameters: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params, &frames, &dir);
  //size = frames * 4; /* 2 bytes/sample, 2 channels 
  // size as in number of data points along
  size = frames * 2;

  snd_pcm_hw_params_get_period_time(params, &val, &dir);

  while (1 > 0) {
    rc = snd_pcm_writei(handle, (out->waveform+out->where), frames);
    if (rc == -EPIPE) {
      /* EPIPE means underrun */
      fprintf(stderr, "underrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      fprintf(stderr,
              "error from writei: %s\n",
              snd_strerror(rc));
    }  else if (rc != (int)frames) {
      fprintf(stderr,
              "short write, write %d frames\n", rc);
    }
    out->where+=size;
  }

  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  //free(buffer);

  return 0;
}

void init_x()
{
/* get the colors black and white (see section for details) */
        XInitThreads();
        //x_buffer=(unsigned char *)malloc(sizeof(unsigned char)*4*VX_SIZE*VY_SIZE);
        display=XOpenDisplay((char *)0);
        screen=DefaultScreen(display);
        black=BlackPixel(display,screen),
        white=WhitePixel(display,screen);
        win=XCreateSimpleWindow(display,DefaultRootWindow(display),0,0, X_SIZE, Y_SIZE, 5, white,black);
        XSetStandardProperties(display,win,"PC scope","PC scope",None,NULL,0,NULL);
        XSelectInput(display, win, ExposureMask|ButtonPressMask|KeyPressMask|ButtonReleaseMask|ButtonMotionMask);
        //XSelectInput(display, vwin, ExposureMask|ButtonPressMask|KeyPressMask|ButtonReleaseMask|ButtonMotionMask);
        gc=XCreateGC(display, win, 0,0);
        XSetBackground(display,gc,black); XSetForeground(display,gc,white);
        XClearWindow(display, win); 
        XMapRaised(display, win);
        XMoveWindow(display, win,0,0);
        Visual *visual=DefaultVisual(display, 0);
        Colormap cmap;
        cmap = DefaultColormap(display, screen);
        color[0].red = 0; color[0].green = 0; color[0].blue = 0;

        color[1].red = 65535; color[1].green = 0; color[1].blue = 0;
        color[2].red = 0; color[2].green = 65535; color[2].blue = 0;
        color[3].red = 0; color[3].green = 0; color[3].blue = 65535;

        color[4].red = 0; color[4].green = 65535; color[4].blue = 65535;
        color[5].red = 65535; color[5].green = 65535; color[5].blue = 0;
        color[6].red = 65535; color[6].green = 0; color[6].blue = 65535;

        color[7].red = 32768; color[7].green = 65535; color[7].blue = 0;
        color[8].red = 65535; color[8].green = 32768; color[8].blue = 0;
        color[9].red = 0; color[9].green = 65535; color[9].blue = 32768;

        color[10].red = 65535; color[10].green = 65535; color[10].blue = 32768;
        color[11].red = 65535; color[11].green = 32768; color[11].blue = 65535;
        color[12].red = 32768; color[12].green = 65535; color[12].blue = 65535;

        color[13].red = 32768; color[13].green = 0; color[13].blue = 0;
        color[14].red = 0; color[14].green = 32768 ; color[14].blue = 0;
        color[15].red = 45535; color[15].green = 45535 ; color[15].blue = 45535;

        XAllocColor(display, cmap, &color[0]);
        XAllocColor(display, cmap, &color[1]);
        XAllocColor(display, cmap, &color[2]);
        XAllocColor(display, cmap, &color[3]);
        XAllocColor(display, cmap, &color[4]);
        XAllocColor(display, cmap, &color[5]);
        XAllocColor(display, cmap, &color[6]);
        XAllocColor(display, cmap, &color[7]);
        XAllocColor(display, cmap, &color[8]);
        XAllocColor(display, cmap, &color[9]);
        XAllocColor(display, cmap, &color[10]);
        XAllocColor(display, cmap, &color[11]);
        XAllocColor(display, cmap, &color[12]);
        XAllocColor(display, cmap, &color[13]);
        XAllocColor(display, cmap, &color[14]);
        XAllocColor(display, cmap, &color[15]);
}

