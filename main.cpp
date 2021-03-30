#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include "GL\glew.h"
#include "GL\freeglut.h"
#include <chrono>
#include <iostream>
#include <cmath>
using namespace std::chrono;
using namespace std;

char* shaderLoadSource( const char* filePath );
unsigned int loadComputeShader( const char* filePath );

int screen_width = 640;
int screen_height = 480;


#define PI_COUNT 100000000

#define USE_GPU


void calculatePi();
void display();


int main( int argc, char** argv )
{
	glEnable( GL_PROGRAM_POINT_SIZE );
	glutInit( &argc, argv );
	glutInitContextVersion( 4, 3 );
	glutInitDisplayMode( GLUT_RGB | GLUT_DEPTH );
	glutInitWindowSize( screen_width, screen_height );
	glutInitWindowPosition( 0, 0 );
	glutCreateWindow( "Pierwszy prog" );

	glewInit(); //init rozszerzeszeñ OpenGL z biblioteki GLEW
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


	calculatePi();
	

	glutSwapBuffers();

	glutMainLoop();

	return 0;
}

void display()
{
}

void calculatePi()
{
	unsigned int computeProgram = loadComputeShader( "compute_shader_pi.glsl" );
	
	glUseProgram( computeProgram );


	unsigned int computeBufferId;
	glGenBuffers( 1, &computeBufferId );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, computeBufferId );
	glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( double ) * PI_COUNT, NULL, GL_DYNAMIC_DRAW );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, computeBufferId );


	auto start = high_resolution_clock::now();

	double sum = 0;


#ifdef USE_GPU

	glDispatchCompute( PI_COUNT/ 1024, 1, 1 );
	glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );

	double* dataRead = ( double* )glMapBufferRange(
		GL_SHADER_STORAGE_BUFFER, 0, sizeof( double ) * PI_COUNT, GL_MAP_READ_BIT );


	for ( int i = 0 ; i < PI_COUNT ; i++ )
	{
		sum += dataRead[i];
	}
	glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );
#else


	for ( int i = 0 ; i < PI_COUNT ; i++ )
	{
		sum += pow( -1, i ) / ( 2.0 * i + 1 );
	}

#endif
	printf( "%.10lf\n", sum * 4 );

	auto stop = high_resolution_clock::now();

	auto duration = duration_cast<microseconds>( stop - start );
	printf( "%lf", duration.count() / 1000000.0 );
	glDeleteBuffers( 1, &computeBufferId );
}


unsigned int loadComputeShader( const char* filePath )
{
	unsigned int computeShader = glCreateShader( GL_COMPUTE_SHADER );

	char* fragmentShaderSource = shaderLoadSource( filePath );
	if ( !fragmentShaderSource )
		return -1;

	glShaderSource( computeShader, 1, ( const char** )&fragmentShaderSource, NULL );
	glCompileShader( computeShader );

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader( shaderProgram, computeShader );
	glLinkProgram( shaderProgram );

	return shaderProgram;
}

unsigned int loadDisplayProgram( const char* vertex, const char* fragment )
{
	unsigned int vertexShader = glCreateShader( GL_VERTEX_SHADER );
	unsigned int fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

	char* vertexShaderSource = shaderLoadSource( vertex );
	char* fragmentShaderSource = shaderLoadSource( fragment );
	
	if ( !fragmentShaderSource && !vertexShaderSource )
		return -1;

	glShaderSource( vertexShader, 1, ( const char** ) &vertexShaderSource, NULL );
	glShaderSource( fragmentShader, 1, ( const char** ) &fragmentShaderSource, NULL );
	glCompileShader( vertexShader );
	glCompileShader( fragmentShader );

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader( shaderProgram, vertexShader );
	glAttachShader( shaderProgram, fragmentShader );
	glLinkProgram( shaderProgram );

	return shaderProgram;
}

char* shaderLoadSource( const char* filePath )
{
	const size_t blockSize = 512;
	FILE* fp;
	char buf[512];
	char* source = NULL;
	size_t tmp, sourceLength = 0;
	fopen_s( &fp, filePath, "r" );
	if ( !fp )
	{
		fprintf( stderr, "shaderLoadSource(): Unable to open %s for reading\n", filePath );
		return NULL;
	}

	while ( ( tmp = fread( buf, 1, blockSize, fp ) ) > 0 )
	{
		char* newSource = ( char* )malloc( sourceLength + tmp + 1 );
		if ( !newSource )
		{
			fprintf( stderr, "shaderLoadSource(): malloc failed\n" );
			if ( source )
				free( source );
			return NULL;
		}

		if ( source )
		{
			memcpy( newSource, source, sourceLength );
			free( source );
		}
		memcpy( newSource + sourceLength, buf, tmp );

		source = newSource;
		sourceLength += tmp;
	}

	fclose( fp );
	if ( source )
		source[sourceLength] = '\0';

	return source;
}
