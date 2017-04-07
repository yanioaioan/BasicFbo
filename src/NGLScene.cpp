#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>

#include <QtOpenGL/QGLWidget>


//////////////////////////////////////////////////////////////////////////





//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
const static float INCREMENT=0.01;
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
const static float ZOOM=0.1;


GLuint          program1;
GLuint          program2;
GLuint          vao;
GLuint          position_buffer;
GLuint          index_buffer;
GLuint          fbo;
GLuint          color_texture;
GLuint          depth_texture;
GLint           mv_location;
GLint           proj_location;
GLuint          mv_location2;
GLuint          proj_location2;


GLuint texSamplerID;

NGLScene::NGLScene()
{
  setTitle("Basic Framebuffer Object Demo");
  m_rotate=false;
  // mouse rotation values set to 0
  m_spinXFace=0;
  m_spinYFace=0;
  startTimer(1);
  m_width=width();
  m_height=height();

}


NGLScene::~NGLScene()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program1);
    glDeleteProgram(program2);
    glDeleteBuffers(1, &position_buffer);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &color_texture);
}

void NGLScene::resizeGL(int _w , int _h)
{
  m_cam.setShape(45.0f,(float)_w/_h,0.05f,350.0f);
  m_width=_w*devicePixelRatio();
  m_height=_h*devicePixelRatio();
}


void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::instance();


  static const char * vs_source[] =
       {
           "#version 410 core                                                  \n"
           "                                                                   \n"
           "layout (location = 0) in vec4 position;                            \n"
           "layout (location = 1) in vec2 texcoord;                            \n"
           "                                                                   \n"
           "out VS_OUT                                                         \n"
           "{                                                                  \n"
           "    vec4 color;                                                    \n"
           "    vec2 texcoord;                                                 \n"
           "} vs_out;                                                          \n"
           "                                                                   \n"
           "uniform mat4 mv_matrix;                                            \n"
           "uniform mat4 proj_matrix;                                          \n"
           "                                                                   \n"
           "void main(void)                                                    \n"
           "{                                                                  \n"
           "    gl_Position = proj_matrix * mv_matrix * position;              \n"
           "    vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);      \n"
           "    vs_out.texcoord = texcoord;                                    \n"
           "}                                                                  \n"
       };

       static const char * fs_source1[] =
       {
           "#version 410 core                                                              \n"
           "                                                                               \n"
           "in VS_OUT                                                                      \n"
           "{                                                                              \n"
           "    vec4 color;                                                                \n"
           "    vec2 texcoord;                                                             \n"
           "} fs_in;                                                                       \n"
           "                                                                               \n"
           "out vec4 color;                                                                \n"
           "                                                                               \n"
           "void main(void)                                                                \n"
           "{                                                                              \n"
           "    color = vec4(1,0,0,1);//sin(fs_in.color * vec4(40.0, 20.0, 30.0, 1.0)) * 0.5 + vec4(0.5);  \n"
           "}                                                                              \n"
       };

       static const char * fs_source2[] =
       {
           "#version 420 core                                                              \n"
           "                                                                               \n"
           "uniform sampler2D tex;                                                         \n"
           "                                                                               \n"
           "out vec4 color;                                                                \n"
           "                                                                               \n"
           "//The  in VS_OUT is the shared vertex shaders outputs                          \n"
           "in VS_OUT                                                                      \n"
           "{                                                                              \n"
           "    vec4 color;                                                                \n"
           "    vec2 texcoord;                                                             \n"
           "} fs_in;                                                                       \n"
           "                                                                               \n"
           "void main(void)                                                                \n"
           "{                                                                              \n"
           "    color = mix(fs_in.color, texture(tex, fs_in.texcoord), 0.7);               \n"
           "}                                                                              \n"
       };

       program1 = glCreateProgram();
       GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
       glShaderSource(fs, 1, fs_source1, NULL);
       glCompileShader(fs);

       GLuint vs = glCreateShader(GL_VERTEX_SHADER);
       glShaderSource(vs, 1, vs_source, NULL);
       glCompileShader(vs);

       glAttachShader(program1, vs);
       glAttachShader(program1, fs);

       glLinkProgram(program1);

       glDeleteShader(vs);
       glDeleteShader(fs);

       program2 = glCreateProgram();
       fs = glCreateShader(GL_FRAGMENT_SHADER);
       glShaderSource(fs, 1, fs_source2, NULL);
       glCompileShader(fs);

       vs = glCreateShader(GL_VERTEX_SHADER);
       glShaderSource(vs, 1, vs_source, NULL);
       glCompileShader(vs);

       glAttachShader(program2, vs);
       glAttachShader(program2, fs);

       glLinkProgram(program2);

       glDeleteShader(vs);
       glDeleteShader(fs);

       mv_location = glGetUniformLocation(program1, "mv_matrix");
       proj_location = glGetUniformLocation(program1, "proj_matrix");
       mv_location2 = glGetUniformLocation(program2, "mv_matrix");
       proj_location2 = glGetUniformLocation(program2, "proj_matrix");

       texSamplerID = glGetUniformLocation(program2, "tex");

       glGenVertexArrays(1, &vao);
       glBindVertexArray(vao);

       static const GLushort vertex_indices[] =
       {
           0, 1, 2,
           2, 1, 3,
           2, 3, 4,
           4, 3, 5,
           4, 5, 6,
           6, 5, 7,
           6, 7, 0,
           0, 7, 1,
           6, 0, 2,
           2, 4, 6,
           7, 5, 3,
           7, 3, 1
       };

       static const GLfloat vertex_data[] =
       {
            // Position                 Tex Coord
           -0.25f, -0.25f,  0.25f,      0.0f, 1.0f,
           -0.25f, -0.25f, -0.25f,      0.0f, 0.0f,
            0.25f, -0.25f, -0.25f,      1.0f, 0.0f,

            0.25f, -0.25f, -0.25f,      1.0f, 0.0f,
            0.25f, -0.25f,  0.25f,      1.0f, 1.0f,
           -0.25f, -0.25f,  0.25f,      0.0f, 1.0f,

            0.25f, -0.25f, -0.25f,      0.0f, 0.0f,
            0.25f,  0.25f, -0.25f,      1.0f, 0.0f,
            0.25f, -0.25f,  0.25f,      0.0f, 1.0f,

            0.25f,  0.25f, -0.25f,      1.0f, 0.0f,
            0.25f,  0.25f,  0.25f,      1.0f, 1.0f,
            0.25f, -0.25f,  0.25f,      0.0f, 1.0f,

            0.25f,  0.25f, -0.25f,      1.0f, 0.0f,
           -0.25f,  0.25f, -0.25f,      0.0f, 0.0f,
            0.25f,  0.25f,  0.25f,      1.0f, 1.0f,

           -0.25f,  0.25f, -0.25f,      0.0f, 0.0f,
           -0.25f,  0.25f,  0.25f,      0.0f, 1.0f,
            0.25f,  0.25f,  0.25f,      1.0f, 1.0f,

           -0.25f,  0.25f, -0.25f,      1.0f, 0.0f,
           -0.25f, -0.25f, -0.25f,      0.0f, 0.0f,
           -0.25f,  0.25f,  0.25f,      1.0f, 1.0f,

           -0.25f, -0.25f, -0.25f,      0.0f, 0.0f,
           -0.25f, -0.25f,  0.25f,      0.0f, 1.0f,
           -0.25f,  0.25f,  0.25f,      1.0f, 1.0f,

           -0.25f,  0.25f, -0.25f,      0.0f, 1.0f,
            0.25f,  0.25f, -0.25f,      1.0f, 1.0f,
            0.25f, -0.25f, -0.25f,      1.0f, 0.0f,

            0.25f, -0.25f, -0.25f,      1.0f, 0.0f,
           -0.25f, -0.25f, -0.25f,      0.0f, 0.0f,
           -0.25f,  0.25f, -0.25f,      0.0f, 1.0f,

           -0.25f, -0.25f,  0.25f,      0.0f, 0.0f,
            0.25f, -0.25f,  0.25f,      1.0f, 0.0f,
            0.25f,  0.25f,  0.25f,      1.0f, 1.0f,

            0.25f,  0.25f,  0.25f,      1.0f, 1.0f,
           -0.25f,  0.25f,  0.25f,      0.0f, 1.0f,
           -0.25f, -0.25f,  0.25f,      0.0f, 0.0f,
       };

       glGenBuffers(1, &position_buffer);
       glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
       glBufferData(GL_ARRAY_BUFFER,
                    sizeof(vertex_data),
                    vertex_data,
                    GL_STATIC_DRAW);

       //as program2 was linked last, we use program2 since glGetAttribLocation works on which program was linked last
       //NOTE both program1 & program2 are using the same vertex shader thought, so all good
       GLuint posVert = glGetAttribLocation(program2, "position");
       GLuint texCoorVert = glGetAttribLocation(program2, "texcoord");

       glVertexAttribPointer(posVert, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), NULL);
       glEnableVertexAttribArray(posVert);
       glVertexAttribPointer(texCoorVert, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
       glEnableVertexAttribArray(texCoorVert);

       glGenBuffers(1, &index_buffer);
       glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
       glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                    sizeof(vertex_indices),
                    vertex_indices,
                    GL_STATIC_DRAW);

       glEnable(GL_CULL_FACE);

       glEnable(GL_DEPTH_TEST);
       glDepthFunc(GL_LEQUAL);

       glGenFramebuffers(1, &fbo);
       glBindFramebuffer(GL_FRAMEBUFFER, fbo);


       // Load up the data from the image using the Qt image functions
//       QImage texImage = QImage("mandril.png");
//       QImage texData = QGLWidget::convertToGLFormat(texImage);


       glGenTextures(1, &color_texture);
       glBindTexture(GL_TEXTURE_2D, color_texture);
       glTexStorage2D(GL_TEXTURE_2D, 9, GL_RGBA8, 512, 512);
       //unccoment the following line, to load texture instead of just color.
       //Make sure to also uncomment glFramebufferTexture with color_texture for it to work, also commend glTexStorage2D
//       glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, texData.width(), texData.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texData.bits());

       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

       // the texture wraps over at the edges (repeat)
       glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
       glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );


       glGenTextures(1, &depth_texture);
       glBindTexture(GL_TEXTURE_2D, depth_texture);
       glTexStorage2D(GL_TEXTURE_2D, 9, GL_DEPTH_COMPONENT32F, 512, 512);

       // unccoment to loads the texture wrapped, use in combination with glTexImage2D to load textrue
       // These commands attach a selected mipmap level or image of a texture object as one of the logical buffers of the specified framebuffer object. Textures cannot be attached to the default draw and read framebuffer, so they are not valid targets of these comm
       glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0);
       glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);

       static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
       glDrawBuffers(1, draw_buffers);

}

//http://www.codeincodeblock.com/2013/05/modern-opengl-3x-perspective-projection.html
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <ctime>

static float fi;
void NGLScene::paintGL()
{
    static const GLfloat blue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
           static const GLfloat one = 1.0f;

           glm::mat4 proj_matrix = glm::perspective(50.0f,(float)640 / (float)480,
                                                        0.1f,
                                                        1000.0f);

           std::time_t currentTime = std::time(nullptr);
           float f = (float)currentTime * 0.3f;
           fi+=0.01;
           glm::mat4 mv_matrix = glm::lookAt(
                       glm::vec3(sin(fi),3,3), // Camera is at (4,3,3), in World Space
                       glm::vec3(0,0,0), // and looks at the origin
                       glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                       );
//           vmath::translate(0.0f, 0.0f, -4.0f) *
//           vmath::translate(sinf(2.1f * f) * 0.5f,
//                               cosf(1.7f * f) * 0.5f,
//                               sinf(1.3f * f) * cosf(1.5f * f) * 2.0f) *
//           vmath::rotate((float)currentTime * 45.0f, 0.0f, 1.0f, 0.0f) *
//           vmath::rotate((float)currentTime * 81.0f, 1.0f, 0.0f, 0.0f);

           //bind the framebuffer and
           glBindFramebuffer(GL_FRAMEBUFFER, fbo);

           glViewport(0, 0, 512, 512);
           static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };

//           glClearColor(green[0], green[1], green[2], green[3]);
//           glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
           //Alternative clearing of depth and color
           glClearBufferfv(GL_COLOR, 0, green);
           glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

           glUseProgram(program1);

           glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));
           glUniformMatrix4fv(mv_location, 1, GL_FALSE, glm::value_ptr(mv_matrix));
           glDrawArrays(GL_TRIANGLES, 0, 36);

           glBindFramebuffer(GL_FRAMEBUFFER, 0);

           glViewport(0, 0, 800, 600);
           glClearBufferfv(GL_COLOR, 0, blue);
           glClearBufferfv(GL_DEPTH, 0, &one);

           //bind the color texture at GL_COLOR_ATTACHMENT0 (because of fbo rendering to color_texture),
           //to whatever is drawn next (in this case a Cube)
           glBindTexture(GL_TEXTURE_2D, color_texture);

           //use program 2 on it
           glUseProgram(program2);

           glUniform1i(texSamplerID, 0);

           glUniformMatrix4fv(proj_location2, 1, GL_FALSE, glm::value_ptr(proj_matrix));
           glUniformMatrix4fv(mv_location2, 1, GL_FALSE, glm::value_ptr(mv_matrix));

           glDrawArrays(GL_TRIANGLES, 0, 36);
           glBindTexture(GL_TEXTURE_2D, 0);
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent (QMouseEvent * _event)
{
  // note the method buttons() is the button state when event was called
  // this is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if(m_rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx=_event->x()-m_origX;
    int diffy=_event->y()-m_origY;
    m_spinXFace += (float) 0.5f * diffy;
    m_spinYFace += (float) 0.5f * diffx;
    m_origX = _event->x();
    m_origY = _event->y();
    update();

  }
        // right mouse translate code
  else if(m_translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = (int)(_event->x() - m_origXPos);
    int diffY = (int)(_event->y() - m_origYPos);
    m_origXPos=_event->x();
    m_origYPos=_event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();

   }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent ( QMouseEvent * _event)
{
  // this method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if(_event->button() == Qt::LeftButton)
  {
    m_origX = _event->x();
    m_origY = _event->y();
    m_rotate =true;
  }
  // right mouse translate mode
  else if(_event->button() == Qt::RightButton)
  {
    m_origXPos = _event->x();
    m_origYPos = _event->y();
    m_translate=true;
  }

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent ( QMouseEvent * _event )
{
  // this event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_rotate=false;
  }
        // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_translate=false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent(QWheelEvent *_event)
{

	// check the diff of the wheel position (0 means no change)
	if(_event->delta() > 0)
	{
		m_modelPos.m_z+=ZOOM;
	}
	else if(_event->delta() <0 )
	{
		m_modelPos.m_z-=ZOOM;
	}
	update();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // turn on wirframe rendering
  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
  // turn off wire frame
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;
  default : break;
  }
  // finally update the GLWindow and re-draw
    update();
}
void NGLScene::timerEvent(QTimerEvent *_event)
{
  NGL_UNUSED(_event);
  update();
}

