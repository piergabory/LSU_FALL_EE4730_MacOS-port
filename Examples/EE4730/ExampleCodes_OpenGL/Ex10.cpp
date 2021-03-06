#include <iostream>
#include <GL/glut.h>


void init(void)
{
	std::cout << "Executing the init() function. \n";
	//Define Material Properties for the Objects in the Scene
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	
	//The following color components specify the intensity for each type of lights.
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);


	//Material properties determine how it reflects light
	//You can specify a material's ambient, diffuse, and specular colors and how shiny it is.
	//Here only the last two material properties are explicitly specified
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
		
	
	//This enables lighting calculations
	glEnable(GL_LIGHTING);
	
	//Remember to enable the light you just defined 
	glEnable(GL_LIGHT0);
	
	glEnable(GL_DEPTH_TEST);
}

void display(void)
{
	std::cout << "Executing the display() function. \n";
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glutSolidSphere(1.0, 20, 16);
	//the normals for the sphere are defined as part of the glutSolidSphere() routine
	
	glFlush();
}

void reshape(int w, int h)
{
	std::cout << "Executing the reshape() function. \n";
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
		glOrtho(-1.5, 1.5, -1.5*(GLfloat)h / (GLfloat)w,
		1.5*(GLfloat)h / (GLfloat)w, -10.0, 10.0);
	else
		glOrtho(-1.5*(GLfloat)w / (GLfloat)h,
		1.5*(GLfloat)w / (GLfloat)h, -1.5, 1.5, -10.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
		
	//To keep the light stationary:
	// Set the light position after whatever viewing and/or modeling transformation you use. 
	// The viewport and projection matrices are established first. 
	// Then, the identity matrix is loaded as the modelview matrix, after which the light position is set.
	// Since the identity matrix is used, the originally specified light position(1.0, 1.0, 1.0) isn't changed by being multiplied by the modelview matrix. 
	// The direction of the light remains (1.0, 1.0, 1.0).
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMainLoop();
	return 0;
}