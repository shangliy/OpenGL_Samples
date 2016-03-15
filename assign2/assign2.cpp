/*
  CSCI 480
  Assignment 2
 */

#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <pic.h>
#include <vector>
#include <iostream>



using namespace std;

/* represents one control point along the spline */
struct point {
   double x;
   double y;
   double z;
};
#include "point.c";

/* spline struct which contains how many control points, and an array of control points */
struct spline {
   int numControlPoints;
   struct point *points;
};

/* the spline array */
struct spline *g_Splines;

/* total number of splines */
int g_iNumOfSplines;

/* Total length of the space*/
float max_distance=30.0f;


/*The parameter of the ground*/
Pic * g_pHeightData;
Pic * groundimage;
GLuint _textureId; //The id of the texture
//Makes the image into a texture, and returns the id of the texture
GLuint loadTexture(Pic* image) {
	GLuint textureId;
	glGenTextures(1, &textureId); //Make room for our texture
	glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit
	//Map the image to the texture
  // specify texture parameters (they affect whatever texture is active)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
 // repeat pattern in s
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// repeat pattern in t
// use linear filter both for magnification and minification
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
				 0,                            //0 for now
				 GL_RGB,                       //Format OpenGL uses for image
				 image->nx, image->ny,  //Width and height
				 0,                            //The border of the image
				 GL_RGB, //GL_RGB, because pixels are stored in RGB format
				 GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
				                   //as unsigned numbers
				 image->pix);               //The actual pixel data
	return textureId; //Returns the id of the texture
}

// called before main loop
void init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);   // set background color
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    g_pHeightData = jpeg_read("GrandTeton-768.jpg", NULL);

    groundimage = jpeg_read("jajlands1_dn.jpg", NULL);

}

// called every time window is resized to update projection matrix
void reshape(int w, int h)
{
    // setup image size
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // setup camera
    gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

vector<point> groundnorm;
vector<float> heightvalue;
void GroundNormalComp(int width,int height){

  for(int z = 1; z < height-1; z++) {
    for(int x = 1; x < width-1; x++) {
      point out,in,right,left;
      setpoint(&out,0.0f, heightvalue[(z - 1)*width+x] - heightvalue[(z)*width+x] , -1.0f);
      setpoint(&in,0.0f, heightvalue[(z + 1)*width+x] - heightvalue[(z)*width+x] , 1.0f);
      setpoint(&right,1.0f, heightvalue[(z)*width+x+1] - heightvalue[(z)*width+x] , -1.0f);
      setpoint(&left,-1.0f, heightvalue[(z)*width+x-1] - heightvalue[(z)*width+x] , -1.0f);

      point temor,temil;
      add(&temor,&out,&right);
      normalize(&temor);
      add(&temil,&in,&left);
      normalize(&temil);

      point norm;
      prod_vect(&norm,&temor,&temil);
      normalize(&norm);
      groundnorm.push_back(norm);
    }
  }
}

void Drawgroud()
{
  int width = g_pHeightData->nx;
  int height = g_pHeightData->ny;

  for(int z = 0; z < height; z++) {
    for(int x = 0; x < width; x++) {
      heightvalue.push_back(*(g_pHeightData->pix+z*width+x)*1.0f/255.0f);

    }
  }

  _textureId = loadTexture(groundimage);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glEnable(GL_TEXTURE_2D);

  GroundNormalComp(width,height);

  float scale = max_distance*2/ (width);

  for(float z = 1; z < height - 1; z++) {
		//Makes OpenGL draw a triangle at every three consecutive vertices
		glBegin(GL_TRIANGLE_STRIP);
		for(float x = 1; x < width-1; x++) {
			point normal = groundnorm[(z)*width+x];
			glNormal3f(normal.x, normal.y, normal.z);
      glTexCoord2f(x/width, z/height);
			glVertex3f((x-width/2)*scale, heightvalue[(z)*width+x]*10, (z-height/2)*scale);
			normal = groundnorm[(z+1)*width+x];
      glTexCoord2f(x/width, (z+1)/height);
			glNormal3f(normal.x, normal.y, normal.z);
			glVertex3f((x-width/2)*scale, heightvalue[(z+1)*width+x]*10, (z + 1-height/2)*scale);
		}
		glEnd();

  }

  // turn off texture mapping
 glDisable(GL_TEXTURE_2D);
}





void display()
{
    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity(); // reset transformation

    // Camera PARAMETER
    glTranslatef(0.0f, 0.0f, -max_distance-10);
    glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.5,0.5,-.5);

    Drawgroud();//Draw the Terrain of the ground
    DrawSky();//Draw the Sky using SkyBox
    DrawSpline();

    glutSwapBuffers(); // double buffer flush
}

int loadSplines(char *argv) {
  char *cName = (char *)malloc(128 * sizeof(char));
  FILE *fileList;
  FILE *fileSpline;
  int iType, i = 0, j, iLength;


  /* load the track file */
  fileList = fopen(argv, "r");
  if (fileList == NULL) {
    printf ("can't open file\n");
    exit(1);
  }

  /* stores the number of splines in a global variable */
  fscanf(fileList, "%d", &g_iNumOfSplines);

  g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

  /* reads through the spline files */
  for (j = 0; j < g_iNumOfSplines; j++) {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL) {
      printf ("can't open file\n");
      exit(1);
    }

    /* gets length for spline file */
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    /* allocate memory for all the points */
    g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
    g_Splines[j].numControlPoints = iLength;

    /* saves the data to the struct */
    while (fscanf(fileSpline, "%lf %lf %lf",
	   &g_Splines[j].points[i].x,
	   &g_Splines[j].points[i].y,
	   &g_Splines[j].points[i].z) != EOF) {
      i++;
    }
  }

  free(cName);

  return 0;
}


int main (int argc, char ** argv)
{
  if (argc<2)
  {
  printf ("usage: %s <trackfile>\n", argv[0]);
  exit(0);
  }

  loadSplines(argv[1]);

  // initialize GLUT
  glutInit(&argc, argv);

  // initialize states
  init();

  // request double buffer
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);

  // set window size
  glutInitWindowSize(500, 500);

  // set window position
  glutInitWindowPosition(0, 0);

  // creates a window
  glutCreateWindow("My Roller Coaster");



  // GLUT callbacks
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);


  // start GLUT program
  glutMainLoop();

  return 0;
}
