///
///  hw1.cpp
///  EE 4730 programming 1
///
///  Created by Pierre Gabory on 04/02/2020.
///  Copyright © 2020 Pierre Gabory. All rights reserved.
///
///  Modified to support MacOS.
///  I have reimplemented the camera controls to a simple orbit mechanic
///  I have also added a display of the bounding box of a mesh. Along with origin axis of the space.
///
///  # COMPILE
///  MacOs:
///  gpp hw1.cpp -o hw1 -framework OpenGL -framework GLUT
///
///  # USAGE:
///  ./hw1 [obj_file_1] [obj_file_2] ...
///  Obj files are consecutively loaded, the camera will be centered on the last one.
///
/// # 
///
///

// MacOs
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>

// Windows / Linux
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

#include "Vertex.h"
#include "Point.h"
#include "Halfedge.h"
#include "Edge.h"
#include "Mesh.h"
#include "Iterators.h"



// MARK: - CLASS HEADERS -

/// Box aligned to the space axes.
/// Represented by two opposites points, min and max
/// - Max is the ccrner  facing direction (+X, +Y, +Z)
/// - Min is the corner facing direction (-X, -Y, -Z)
struct BoundingBox {
    Point min;
    Point max;
    BoundingBox(Point min, Point max): min(min), max(max) {}
    
    inline Point center();
    inline float diagonal();
};



/// Base Object Class
/// Object tree in the scene
/// Inherited by the camera, meshes, and eventually lights..
struct Object {
    Point position;
    Point rotation;
    Point scale = Point(1, 1, 1);
    std::vector<Object *> childrens;
    
    virtual void render() const;
};



/// RenderableObject Class
/// Provides bounding box containing all the vertices
class RenderableObject: public Object {

private:
    GLenum type = GL_TRIANGLES;
    Mesh *mesh;
    BoundingBox *bounds = nullptr;
    void computeBoundingBox();
    
public:
    Point color = Point(1, 1, 1);
    
    RenderableObject(Mesh *mesh): mesh(mesh) {}
    ~RenderableObject();
    
    inline void setRenderingStyle(GLenum mode);
    inline const BoundingBox& getBounds();
    inline void showBounds();
    void render() const override;
};



/// Point of view of the scene
/// Defines and updates the matrix projection
class Camera: public Object {
private:
    float fieldOfView = 45.0f;
    int width = 400;
    int height = 400;
    float nearField = 1.0;
    float farField = 1000.0;
    
    void updateView() const;
    
public:
    void resize(int newWidth, int newHeight);
    
    // Getters
    inline const int getWidth() const { return width; }
    inline const int getHeight() const { return height; }
};



/// Renderer static class
/// Provides the draw and update loop to OpenGL
/// Usage: Init is called in `main()` first.
/// second comes `createWindow()` where the app window will appear
/// finally `start()` launches the render loop.
///
/// The camera property is the point of view and root of the scene graph.
class Renderer {
private:
    static void draw();
    static void update(int value);
    static void handleResize(int w, int h);
    
public:
    static Camera camera;
    
    static void init(int &argc, char** &argv);
    static void createWindow(int width, int height, const char title[]);
    static void start();
};



/// Orbital Camera controls
/// Simple and classic orbital camera control scheme using mouse input.
/// Takes the Renderer::camera and rotates it around the (0,0,0) scene coordinates.
/// The camera up direction is always alined to the Z axis
class OrbitControls {
private:
    static void mouseMove(int screenX, int screenY);
public:
    static void init();
};


// MARK: - MAIN -

int main(int argc, char** argv) {
    
    Object *scene = new Object();
    Renderer::camera.childrens.push_back(scene);
    
//    Object *origin = makeOrigin();
//    scene->childrens.push_back(origin);
    
    RenderableObject *lastAddedObject = nullptr;
    for (int arg = 1; arg < argc; arg ++) {
        
        Mesh *mesh = new Mesh();
        // try to load obj, and push into the scene
        if (mesh->readOBJFile(argv[arg])) {
            RenderableObject *object = new RenderableObject(mesh);
            object->setRenderingStyle(GL_TRIANGLES);
            object->showBounds();
            scene->childrens.push_back(object);
            lastAddedObject = object;
        }
        
        // Free memory if failed to open file
        else {
            delete mesh;
        }
    }
    
    // center camera point of view on the last mesh
    // we move the scene so the orbit is around the object
    // then we translate the camera back so it fits in the fov.
    if (lastAddedObject != nullptr) {
        BoundingBox bounds = lastAddedObject->getBounds();
        scene->position = bounds.center() * -1;
        Renderer::camera.position.v[2] = -1.5 * bounds.diagonal();
    }
    
    Renderer::init(argc, argv);
    Renderer::createWindow(800, 800, "Prog1");
    OrbitControls::init();
    Renderer::start();
    
    return 0;
}








// MARK: - CLASS IMPLEMENTATION -



// MARK: - RenderableObject

// MARK: RenderableObject Render

void RenderableObject::render() const  {
    Object::render();
    glBegin(type);
    glColor3dv(color.v);
    for (MeshVertexIterator it(mesh); !it.end(); ++it) {
        glVertex3dv((*it)->point().v);
    }
    glEnd();
}

// MARK: Compute Bounding Box

void RenderableObject::computeBoundingBox() {
    MeshVertexIterator it(mesh);
    
    Point min = it.value()->point();
    Point max = it.value()->point();
    
    for (MeshVertexIterator it(mesh); !it.end(); ++it) {
        min.v[0] = std::min(min.v[0], (*it)->point().v[0]);
        min.v[1] = std::min(min.v[1], (*it)->point().v[1]);
        min.v[2] = std::min(min.v[2], (*it)->point().v[2]);
        
        max.v[0] = std::max(max.v[0], (*it)->point().v[0]);
        max.v[1] = std::max(max.v[1], (*it)->point().v[1]);
        max.v[2] = std::max(max.v[2], (*it)->point().v[2]);
    }
    
    bounds = new BoundingBox(min, max);
}

// MARK: Mesh extras

RenderableObject::~RenderableObject() {
    for (Object *child: childrens) { delete child; }
    if (mesh != nullptr) {
        delete mesh;
    }
}

void RenderableObject::setRenderingStyle(GLenum mode) {
    type = mode;
}

const BoundingBox& RenderableObject::getBounds() {
    if (bounds == nullptr) { computeBoundingBox(); }
    return *bounds;
}


void RenderableObject::showBounds() {
    if (bounds == nullptr) { computeBoundingBox(); }
    Mesh *boundsMesh = new Mesh();
    
    //{
//        bounds->min,                                            // 0
//        Vertex(bounds->max.x, bounds->min.y, bounds->min.z),    // 1
//        Vertex(bounds->max.x, bounds->max.y, bounds->min.z),    // 2
//        Vertex(bounds->min.x, bounds->max.y, bounds->min.z),    // 3
//        bounds->max,                                            // 4
//        Vertex(bounds->min.x, bounds->max.y, bounds->max.z),    // 5
//        Vertex(bounds->min.x, bounds->min.y, bounds->max.z),    // 6
//        Vertex(bounds->max.x, bounds->min.y, bounds->max.z)     // 7
//    }, { 0, 1, 2, 3, 4, 5, 6, 7, 0, 6, 1, 7, 2, 4, 3, 5, 0, 3, 1, 2, 4, 7, 5, 6});
//
    RenderableObject *boundsRenderable = new RenderableObject(boundsMesh);
    boundsRenderable->setRenderingStyle(GL_LINES);
    boundsRenderable->color = Point(0, 1, 0);
    
    childrens.push_back(boundsRenderable);
}





// MARK: - BoundingBox

Point BoundingBox::center() {
    return (min + max) / 2;
}

float BoundingBox::diagonal() {
    return (max - min).norm();
}




// MARK: - Renderer

Camera Renderer::camera;

void Renderer::init(int &argc, char** &argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
}

void Renderer::createWindow(int width, int height, const char title[]) {
    glutInitWindowSize(width, height);
    glutCreateWindow(title);
    
    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
    handleResize(width, height);
}

void Renderer::start() {
    glutDisplayFunc(Renderer::draw);
    glutReshapeFunc(Renderer::handleResize);
    glutTimerFunc(25, Renderer::update, 0); //Add a timer
    glutMainLoop();
}

void Renderer::handleResize(int w, int h) {
    camera.resize(w, h);
    glutPostRedisplay();
}

void Renderer::draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    camera.render();
    glPopMatrix();
    glutSwapBuffers();
}

void Renderer::update(int value) {
    glutPostRedisplay();
    glutTimerFunc(25, Renderer::update, 0);
}




// MARK: - Orbit Controls

void OrbitControls::mouseMove(int screenX, int screenY) {
    float viewX = (float)screenX / (float)Renderer::camera.getWidth();
    float viewY = (float)screenY / (float)Renderer::camera.getHeight();
    
    Point rotation = Point((viewY * 2) - 1, (viewX * 2) - 1, 0);
    Renderer::camera.rotation = rotation * 180;
}

void OrbitControls::init() {
    glutMotionFunc(OrbitControls::mouseMove);
}



// MARK: - Object

void Object::render() const {
    glTranslatef(position.v[0], position.v[1], position.v[2]);
    glRotatef(rotation.v[0], 1, 0, 0);
    glRotatef(rotation.v[1], 0, 1, 0);
    glRotatef(rotation.v[2], 0, 0, 1);
    glScaled(scale.v[0], scale.v[1], scale.v[2]);
    
    for (Object *child: childrens) {
        glPushMatrix();
        child->render();
        glPopMatrix();
    }
}




// MARK: - Camera

void Camera::updateView() const {
    float aspectRatio = (float)width / (float)height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fieldOfView, aspectRatio, nearField, farField);
}

void Camera::resize(int newWidth, int newHeight){
    width = newWidth;
    height = newHeight;
    updateView();
}
