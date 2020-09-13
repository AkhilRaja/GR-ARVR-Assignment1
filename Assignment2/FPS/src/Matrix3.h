//
//  Matrix3.h
//  SimpleFPS
//
//  Created by Akhil Raja on 12/09/20.
//

#ifndef __SimpleFPS__Matrix3__
#define __SimpleFPS__Matrix3__

#include <iostream>
#include <GLFW/glfw3.h>

typedef struct
{
    GLfloat m00;
    GLfloat m01;
    GLfloat m02;
    GLfloat m10;
    GLfloat m11;
    GLfloat m12;
    GLfloat m20;
    GLfloat m21;
    GLfloat m22;
    
} Matrix3;

Matrix3 scalerMultiplyMatrix3(Matrix3 matrix, GLfloat scalerValue);
Matrix3 addMatrix3(Matrix3 matrixA, Matrix3 matrixB);

const Matrix3 identityMatrix3 = {  1.0f, 0.0f, 0.0f,
                            0.0f, 1.0f, 0.0f,
                            0.0f, 0.0f, 1.0f};

#endif /* defined(__SimpleFPS__Matrix3__) */
