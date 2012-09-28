#include <cgv/media/video/capture_buffer.h>
#include <cgv/media/video/capture_renderer_file.h>
#include <iostream>
#include <math.h>


using namespace cgv::media::capture;


// a buffer for storing data in RGB24
// this buffer will be used to store the image data in that will
// then be sent to the rendering device
unsigned char buffer[640*480*3];


// method to alter the buffer by creating some kind of
// water effect. The definition is put to the end of the
// file because it is not neccessary for demonstrating
// the buffer capturing
void animate_water(int i);


void main() {

	// create a new buffer capture device
	capture_buffer *capture = new capture_buffer();


	// attach the buffer to the device with a resolution of 640x480 and 20fps
	if (!capture->attach(buffer, 640, 480, 20)) {
		std::cout<<"Attaching a buffer failed!. Error code is "<<capture->get_last_error()<<std::endl;
		delete capture;
		return;
	}

	// set the renderer to be a video renderer
	// and let the renderer automagically choose the best codec
	capture->set_renderer(new capture_renderer_file("output.avi"));

	// start capturing
	if (!capture->start()) {
		std::cout<<"Starting capturing failed. Error code is "<<capture->get_last_error()<<std::endl;
		delete capture;
		return;
	}


	// make 100 frames
	for (int i=0; i<100; i++) {

		// somehow alter the buffer
		animate_water(i);

		std::cout<<"Sending Frame "<<(i+1)<<"/100... ";

		// when the frame is rendered you can send it to the
		// capturing device calling this method.
		capture->send_frame();

		std::cout<<"done"<<std::endl;
	}

	// stop capturing
	capture->stop();

	// clean up
	delete capture;

}




// some more buffers used for this doubtful water effect
static unsigned char waterbuffer1[640*480*3];
static unsigned char waterbuffer2[640*480*3];
static unsigned char waterbuffer3[640*480*3];


void process_water_step() {
	// current color
	int col;

	// move the stack of water buffers
	memcpy(waterbuffer3, waterbuffer2, sizeof(waterbuffer3));
	memcpy(waterbuffer2, waterbuffer1, 640*480*3);

	// populate information
	for (int x=1; x<640-1; x++)
		for (int y=1; y<480-1; y++)
			for (int z=0; z<3; z++) {
				col = 0;
				col += waterbuffer2[((x-1)+y*640)*3+z];
				col += waterbuffer2[((x+1)+y*640)*3+z];
				col += waterbuffer2[((x)+(y-1)*640)*3+z];
				col += waterbuffer2[((x)+(y+1)*640)*3+z];

				col = col/2;

				if (col > 255)
					col = 255;

				col -= waterbuffer3[(x+y*640)*3+z];

				if (col<0)
					col = 0;

				waterbuffer1[(x+y*640)*3+z] = col;
		}

		// our buffer is the difference between the first and the third water buffer
		// ... and some hole filling
		int pos;
		for (int x=1; x<639; x++)
			for (int y=1; y<479; y++)
				for (int z=0; z<3; z++) {
					pos = (x+y*640)*3+z;
					if (waterbuffer1[pos] == 0 && 
						waterbuffer1[pos-3]>0 && waterbuffer1[pos+3]>0 && 
						waterbuffer1[pos+640*3]>0 && waterbuffer1[pos-640*3]>0)
						buffer[pos] = (waterbuffer1[pos-3] + waterbuffer1[pos+3] +
										waterbuffer1[pos+640*3] + waterbuffer1[pos-640*3])/2;
					else
						buffer[pos] = waterbuffer1[pos]*2;
				}

		// smooth the image
		for (int x=1; x<639; x++)
			for (int y=1; y<479; y++)
				for (int z=0; z<3; z++) {
					pos = (x+y*640)*3+z;
					col = buffer[pos];
					col += buffer[pos-3];
					col += buffer[pos+3];
					col += buffer[pos-3*640];
					col += buffer[pos+3*640];
					buffer[pos] = col / 5;
				}
}



void animate_water(int i) {
	// set some random water drops
	for (int k=0; k<2; k++)
		waterbuffer1[(rand()*rand())%(640*480*3)] = 255;

	// also make a diagonal drop from all sides
	waterbuffer1[(i*4+i*4*640)*3+rand()%3] = 255;
	waterbuffer1[(639-i*4+i*4*640)*3+rand()%3] = 255;
	waterbuffer1[(i*4+(479-i*4)*640)*3+rand()%3] = 255;
	waterbuffer1[(639-i*4+(479-i*4)*640)*3+rand()%3] = 255;

	// and from down, above, left, right
	waterbuffer1[(i+240*640)*3+rand()%3] = 255;
	waterbuffer1[(639-i+240*640)*3+rand()%3] = 255;
	waterbuffer1[(320+i*640)*3+rand()%3] = 255;
	waterbuffer1[(320+(479-i)*640)*3+rand()%3] = 255;

	// and to complete things a twist
	waterbuffer1[ ((int)(cos((float)i*0.1f)*(i)+320) + (int)(sin((float)i*0.1f)*(i)+240)*640)*3 + rand()%3] = 255;

	// alter the buffer (twice for more animation speed
	process_water_step();
	process_water_step();
}



