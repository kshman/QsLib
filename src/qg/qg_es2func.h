﻿DEF_GL_FUNC(void, glActiveTexture, (GLenum))
DEF_GL_FUNC(void, glAttachShader, (GLuint, GLuint))
DEF_GL_FUNC(void, glBindBuffer, (GLenum, GLuint))
DEF_GL_FUNC(void, glBindTexture, (GLenum, GLuint))
DEF_GL_FUNC(void, glBlendEquationSeparate, (GLenum, GLenum))
DEF_GL_FUNC(void, glBlendFuncSeparate, (GLenum, GLenum, GLenum, GLenum))
DEF_GL_FUNC(void, glBufferData, (GLenum, GLsizeiptr, const GLvoid *, GLenum))
DEF_GL_FUNC(void, glClear, (GLbitfield))
DEF_GL_FUNC(void, glClearColor, (GLclampf, GLclampf, GLclampf, GLclampf))
DEF_GL_FUNC(void, glClearDepthf, (GLclampf))
DEF_GL_FUNC(void, glColorMask, (GLboolean, GLboolean, GLboolean, GLboolean))
DEF_GL_FUNC(void, glCompileShader, (GLuint))
DEF_GL_FUNC(GLuint, glCreateProgram, (void)) 
DEF_GL_FUNC(GLuint, glCreateShader, (GLenum)) 
DEF_GL_FUNC(void, glCullFace, (GLenum))
DEF_GL_FUNC(void, glDeleteBuffers, (GLsizei, const GLuint *))
DEF_GL_FUNC(void, glDeleteProgram, (GLuint))
DEF_GL_FUNC(void, glDeleteShader, (GLuint))
DEF_GL_FUNC(void, glDepthFunc, (GLenum))
DEF_GL_FUNC(void, glDepthMask, (GLboolean))
DEF_GL_FUNC(void, glDetachShader, (GLuint, GLuint))
DEF_GL_FUNC(void, glDisable, (GLenum))
DEF_GL_FUNC(void, glDisableVertexAttribArray, (GLuint))
DEF_GL_FUNC(void, glDrawArrays, (GLenum, GLint, GLsizei))
DEF_GL_FUNC(void, glDrawElements, (GLenum, GLsizei, GLenum, const void*))
DEF_GL_FUNC(void, glEnable, (GLenum))
DEF_GL_FUNC(void, glEnableVertexAttribArray, (GLuint))
DEF_GL_FUNC(void, glFlush, (void))
DEF_GL_FUNC(void, glFrontFace, (GLenum))
DEF_GL_FUNC(void, glGenBuffers, (GLsizei, GLuint *))
DEF_GL_FUNC(void, glGetActiveAttrib, (GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*))
DEF_GL_FUNC(void, glGetActiveUniform, (GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*))
DEF_GL_FUNC(GLint, glGetAttribLocation, (GLuint, const GLchar *))	 
DEF_GL_FUNC(void, glGetIntegerv, (GLenum, GLint *))
DEF_GL_FUNC(void, glGetProgramInfoLog, (GLuint, GLsizei, GLsizei *, GLchar *))
DEF_GL_FUNC(void, glGetProgramiv, (GLuint, GLenum, GLint *))
DEF_GL_FUNC(void, glGetShaderInfoLog, (GLuint, GLsizei, GLsizei *, char *))
DEF_GL_FUNC(void, glGetShaderiv, (GLuint, GLenum, GLint *))
DEF_GL_FUNC(const GLubyte *, glGetString, (GLenum))	 
DEF_GL_FUNC(GLint, glGetUniformLocation, (GLuint, const char *))	 
DEF_GL_FUNC(void, glLinkProgram, (GLuint))
DEF_GL_FUNC(void, glPixelStorei, (GLenum, GLint))
DEF_GL_FUNC(void, glShaderSource, (GLuint, GLsizei, const GLchar* const*, const GLint *))
DEF_GL_FUNC(void, glStencilMaskSeparate, (GLenum, GLuint))
DEF_GL_FUNC(void, glUniform1i, (GLint, GLint))
DEF_GL_FUNC(void, glUniform4fv, (GLint, GLsizei, const GLfloat *))
DEF_GL_FUNC(void, glUniformMatrix4fv, (GLint, GLsizei, GLboolean, const GLfloat *))
DEF_GL_FUNC(void, glUseProgram, (GLuint))
DEF_GL_FUNC(void, glVertexAttrib4fv, (GLuint, const GLfloat *))
DEF_GL_FUNC(void, glVertexAttribPointer, (GLuint, GLint, GLenum, GLboolean, GLsizei, const void *))

// 아직 안쓰고 있는거
//DEF_GL_FUNC(void, glBindAttribLocation, (GLuint, GLuint, const char *))
//DEF_GL_FUNC(void, glBindFramebuffer, (GLenum, GLuint))
//DEF_GL_FUNC(void, glBufferSubData, (GLenum, GLintptr, GLsizeiptr, const GLvoid *))
//DEF_GL_FUNC(GLenum, glCheckFramebufferStatus, (GLenum))	
//DEF_GL_FUNC(void, glDeleteFramebuffers, (GLsizei, const GLuint *))
//DEF_GL_FUNC(void, glDeleteTextures, (GLsizei, const GLuint *))
//DEF_GL_FUNC(void, glFinish, (void))
//DEF_GL_FUNC(void, glFramebufferTexture2D, (GLenum, GLenum, GLenum, GLuint, GLint))
//DEF_GL_FUNC(void, glGenFramebuffers, (GLsizei, GLuint *))
//DEF_GL_FUNC(void, glGenTextures, (GLsizei, GLuint *))
//DEF_GL_FUNC(GLenum, glGetError, (void))		 
//DEF_GL_FUNC(void, glReadPixels, (GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *))
//DEF_GL_FUNC(void, glScissor, (GLint, GLint, GLsizei, GLsizei))
//DEF_GL_FUNC(void, glShaderBinary, (GLsizei, const GLuint *, GLenum, const void *, GLsizei))
//DEF_GL_FUNC(void, glTexImage2D, (GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *))
//DEF_GL_FUNC(void, glTexParameteri, (GLenum, GLenum, GLint))
//DEF_GL_FUNC(void, glTexSubImage2D, (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *))
//DEF_GL_FUNC(void, glUniform4f, (GLint, GLfloat, GLfloat, GLfloat, GLfloat))
//DEF_GL_FUNC(void, glViewport, (GLint, GLint, GLsizei, GLsizei))

