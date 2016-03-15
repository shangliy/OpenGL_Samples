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

  GroundNormalComp(width,height);

  float scale = 5.0f / (width-1);
  cout<<scale<<endl;
	glScalef(scale, scale, scale);


  for(int z = 1; z < height - 1; z++) {
		//Makes OpenGL draw a triangle at every three consecutive vertices
		glBegin(GL_TRIANGLE_STRIP);
		for(int x = 1; x < width-1; x++) {
			point normal = groundnorm[(z)*width+x];
			glNormal3f(normal.x, normal.y, normal.z);
      glColor3f(0,0,heightvalue[(z)*width+x]);
			glVertex3f(x-width/2, heightvalue[(z)*width+x]/scale, z-height/2);
			normal = groundnorm[(z+1)*width+x];
      glColor3f(0,0,heightvalue[(z+1)*width+x]);
			glNormal3f(normal.x, normal.y, normal.z);
			glVertex3f(x-width/2, heightvalue[(z+1)*width+x]/scale, z + 1-height/2);
		}
		glEnd();
  }
}
