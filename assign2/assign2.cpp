/*
  CSCI 480
  Assignment 2
 */

#include <stdlib.h>
#include <cstdlib>
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


void showpoint(point *a){
	cout<<a->x<<endl;
  cout<<a->y<<endl;
  cout<<a->z<<endl;
}


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
int first_flag =1;
int nsplinepoint=0;

/*The parameter of the ground*/
Pic * g_pHeightData;
Pic * groundimage;
GLuint _textureId; //The id of the texture


/*The parameter of camera*/
int cam_index=0;
int speed = 100;
point campos;
point camdir;
point camtow;
int stereo=1;



//Makes the image into a texture, and returns the id of the texture
GLuint loadTexture(Pic* image) {
	GLuint textureId;
	glGenTextures(1, &textureId); //Make room for our texture
	glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
    gluPerspective(45.0, (double)w / (double)h, 0.1, 200.0);
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


vector<point> cont_point;
vector<point> line_left;
vector<point> line_right;
vector<point> dir_up;
vector<point> dir_for;
vector<point> dir_right;

void slinecal(point *r,int i,float t)
{
  r->x = cont_point[i].x + 0.5*t*(-cont_point[i-1].x+cont_point[i+1].x)
      + t*t*(cont_point[i-1].x - 2.5*cont_point[i].x + 2*cont_point[i+1].x - 0.5*cont_point[i+2].x)
      + t*t*t*(-0.5*cont_point[i-1].x + 1.5*cont_point[i].x - 1.5*cont_point[i+1].x + 0.5*cont_point[i+2].x);
  r->y = cont_point[i].y + 0.5*t*(-cont_point[i-1].y+cont_point[i+1].y)
      + t*t*(cont_point[i-1].y - 2.5*cont_point[i].y + 2*cont_point[i+1].y - 0.5*cont_point[i+2].y)
      + t*t*t*(-0.5*cont_point[i-1].y + 1.5*cont_point[i].y - 1.5*cont_point[i+1].y + 0.5*cont_point[i+2].y);
  r->z = cont_point[i].z + 0.5*t*(-cont_point[i-1].z+cont_point[i+1].z)
      + t*t*(cont_point[i-1].z - 2.5*cont_point[i].z + 2*cont_point[i+1].z - 0.5*cont_point[i+2].z)
      + t*t*t*(-0.5*cont_point[i-1].z + 1.5*cont_point[i].z - 1.5*cont_point[i+1].z + 0.5*cont_point[i+2].z);
}

void dircal(point *r,int i,int t)
{
  r->x =  0.5*(-cont_point[i-1].x+cont_point[i+1].x)
      + 2*t*(cont_point[i-1].x - 2.5*cont_point[i].x + 2*cont_point[i+1].x - 0.5*cont_point[i+2].x)
      + 3*t*t*(-0.5*cont_point[i-1].x + 1.5*cont_point[i].x - 1.5*cont_point[i+1].x + 0.5*cont_point[i+2].x);
  r->y =0.5*(-cont_point[i-1].y+cont_point[i+1].y)
      + 2*t*(cont_point[i-1].y - 2.5*cont_point[i].y + 2*cont_point[i+1].y - 0.5*cont_point[i+2].y)
      + 3*t*t*(-0.5*cont_point[i-1].y + 1.5*cont_point[i].y - 1.5*cont_point[i+1].y + 0.5*cont_point[i+2].y);
  r->z = 0.5*(-cont_point[i-1].z+cont_point[i+1].z)
      + 2*t*(cont_point[i-1].z - 2.5*cont_point[i].z + 2*cont_point[i+1].z - 0.5*cont_point[i+2].z)
      + 3*t*t*(-0.5*cont_point[i-1].z + 1.5*cont_point[i].z - 1.5*cont_point[i+1].z + 0.5*cont_point[i+2].z);
}


void Subdivision(float u0,float u1,int i,float maxlineseg)
{
  point x0;
  slinecal(&x0,i,u0);
  point x1;
  slinecal(&x1,i,u1);
  float umid = (u0+u1)/2.0f;


  float d =distance(&x0,&x1);


  if (d>maxlineseg)
  {
    Subdivision(u0,umid,i,maxlineseg);
    Subdivision(umid,u1,i,maxlineseg);
  }
  else {

    if (first_flag==1){
      line_left.push_back(x0);
      line_left.push_back(x1);




      point dir1,dir2;
      dircal(&dir1,i,u0);
      normalize(&dir1);
      dircal(&dir2,i,u1);
      normalize(&dir2);
      dir_for.push_back(dir1);
      dir_for.push_back(dir2);

      point right1,right2;
      point up1,up2;
      if (nsplinepoint==0){
        setpoint(&right1,0,0,1);

      } else{
        right1 = dir_right[nsplinepoint-1];

      }
      prod_vect(&up1,&dir1,&right1);
      normalize(&up1);
      dir_up.push_back(up1);
      prod_vect(&right1,&up1,&dir1);
      normalize(&right1);
      dir_right.push_back(right1);

      right2 = dir_right[nsplinepoint];
      prod_vect(&up2,&dir2,&right2);
      normalize(&up2);
      dir_up.push_back(up2);
      prod_vect(&right2,&up2,&dir2);
      normalize(&right2);
      dir_right.push_back(right2);

      float tail_widht=2;
      add_mult(&x0,tail_widht,&right1);
      add_mult(&x1,tail_widht,&right2);
      line_right.push_back(x0);
      line_right.push_back(x1);

      nsplinepoint+=2;

    }

  }
}

void Drawtrail()
{
  float maxlineseg=0.001;
  int i;
  if (first_flag==1)
  {
    for (i =0 ;i<g_Splines->numControlPoints;i++)
    {
    cont_point.push_back(*((g_Splines->points+i)));
    }
  }

  for (i =1 ;i<g_Splines->numControlPoints-2;i++)
  {
    Subdivision(0,1,i,maxlineseg);
  }

  glBegin(GL_LINES);
  glLineWidth(10);
  for (i=0;i<nsplinepoint;i++)
  glVertex3f(line_left[i].x,line_left[i].y,line_left[i].z);
  glEnd();


  glBegin(GL_LINES);
  glLineWidth(10);
  for (i=0;i<nsplinepoint;i++)
  glVertex3f(line_right[i].x,line_right[i].y,line_right[i].z);
  glEnd();

}


void display()
{
    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity(); // reset transformation

    // Camera PARAMETER
    if (stereo){
      glTranslatef(0.0f, 0.0f, -max_distance-10);
      glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
      glScalef(0.5,0.5,-.5);
    }
    else{
      gluLookAt(campos.x,campos.y,campos.z,camtow.x,camtow.y,camtow.z,camdir.x,camdir.y,camdir.z);
    }



    Drawgroud();//Draw the Terrain of the ground
    //DrawSky();//Draw the Sky using SkyBox
    Drawtrail();
    first_flag=0;

    glutSwapBuffers(); // double buffer flush
}


void update(int value) {

  setpoint(&campos,(line_left[cam_index].x+line_right[cam_index].x)/2,(line_left[cam_index].y+line_right[cam_index].y)/2,
                    (line_left[cam_index].z+line_right[cam_index].z)/2);
  setpoint(&camdir,dir_up[cam_index].x,dir_up[cam_index].y,dir_up[cam_index].z);

  point dirnow;
  dirnow = dir_for[cam_index];
  add(&camtow,&campos,&dirnow);
/*
showpoint(&campos);
showpoint(&camdir);
getchar();
*/
//cout<<nsplinepoint<<endl;
  cam_index += speed;
	if (cam_index > nsplinepoint-2) {
		cam_index=0;
	}
	glutPostRedisplay();
	glutTimerFunc(50, update, 0);
}


void doIdle()
{

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
  glutTimerFunc(50, update, 0);
  /* replace with any animate code */
  glutIdleFunc(doIdle);


  // start GLUT program
  glutMainLoop();

  return 0;
}
