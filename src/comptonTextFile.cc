#include "comptonTextFile.hh"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

comptonTextFile::comptonTextFile()
: TObject() {
    fFilenameSize = 0;
    fFilename = NULL;

    fBufferSize = 1;
    fBuffer = new char[1];
    fBuffer[0] = '\0';
}

comptonTextFile::comptonTextFile(const char *fn)
: TObject() {
    fFilenameSize = 0;
    fFilename = NULL;

    fBufferSize = 1;
    fBuffer = new char[1];
    fBuffer[0] = '\0';

    copyFileIn(fn);
}

comptonTextFile::comptonTextFile(const comptonTextFile& r)
: TObject(r) {
    fFilenameSize = r.fFilenameSize;
    fFilename = new char[r.fFilenameSize];
    strncpy(fFilename, r.fFilename, fFilenameSize);

    fBufferSize = r.fBufferSize;
    fBuffer = new char[r.fBufferSize];
    memcpy(fBuffer, r.fBuffer, fBufferSize);
}

const comptonTextFile& comptonTextFile::operator=(const comptonTextFile& r){
    TObject::operator=(r);

    delete[] fFilename; 
    fFilenameSize = r.fFilenameSize;
    fFilename = new char[r.fFilenameSize];
    strncpy(fFilename, r.fFilename, fFilenameSize);

    delete[] fBuffer; 
    fBufferSize = r.fBufferSize;
    fBuffer = new char[r.fBufferSize];
    memcpy(fBuffer, r.fBuffer, fBufferSize);

    return *this;
}

comptonTextFile::~comptonTextFile(){
    delete[] fFilename; 
      delete[] fBuffer; 
}

void comptonTextFile::copyFileIn(const char *fn){
    if( strlen(fn) > __STRLEN ){
	fprintf(stderr, "%s %d: ERROR filename too long", __PRETTY_FUNCTION__, __LINE__ );
	exit(1);
    }

    // Get file info
    struct stat filedata;
    stat(fn, &filedata);

    FILE *fd = fopen(fn, "r");
    if( fd != NULL ){
	fFilenameSize = strlen(fn)+1; // +1 so we pick up \0
	fFilename = new char[fFilenameSize];
	strncpy(fFilename, fn, fFilenameSize);

	fBufferSize = filedata.st_size+1;
	fBuffer = new char[fBufferSize];
	size_t size = fread(fBuffer, sizeof(char), filedata.st_size, fd);
	fBuffer[fBufferSize-1] = '\0';
	if( (long int) size != filedata.st_size ){
	    fprintf(stderr, "%s line %d: ERROR file %s cannot be fully read (%lld of %lld read)\n",
		    __PRETTY_FUNCTION__, __LINE__, fn,(unsigned long long int)  size, (unsigned long long int)  filedata.st_size);
	    exit(1);
	}
    } else {
	fprintf(stderr, "%s line %d: ERROR file %s cannot be opened\n", __PRETTY_FUNCTION__, __LINE__, fn);
	exit(1);
    }
    fclose(fd);
}

void comptonTextFile::RecreateInDir(const char *adir, bool clobber ){
    char *thisdir;

    if( adir == NULL ){
	thisdir = new char[2];
	strcpy(thisdir, ".");
    } else {
	thisdir = new char[strlen(adir)];
	strcpy(thisdir, adir);
    }

    int dirlen = strlen(thisdir);

    char *catpath = new char[dirlen+ strlen(GetBaseFile()) + 2];
    strcpy(catpath, thisdir);
    strcat(catpath, "/"); // Add slash
    strcat(catpath, GetBaseFile());

    int ret = mkdir(thisdir, 0755); // rwx for owner, rx for everyone else

    if( ret == -1 && errno != EEXIST ){ 
	fprintf(stderr, "%s - %s\n", thisdir, strerror(errno) );
	delete thisdir;
	delete[] catpath;
	return;
    }

    Recreate(catpath, clobber);

    delete thisdir;
    delete[] catpath;
}

void comptonTextFile::Recreate(const char *fn, bool clobber ){
    // Behavior 
    //    don't clobber, end with error if we are asked to do this
    //    If directory structure doesn't exist, make file in present directory        

    if( fn == NULL ){ fn = fFilename; }

    struct stat fdata;
    int ret = stat(fn, &fdata);

    if( ret == 0 && !clobber ){
	fprintf(stderr, "%s  Will not create file %s - already exists\n", __PRETTY_FUNCTION__, fn);
	return;
    }

    FILE *fd;
    fd = fopen(fn, "w");

    if( fd == NULL ){
	printf("errno = %d\n", errno ); 
	printf("%s - %s\n", fn, strerror(errno) ); 
	printf("Attempting to write %s to present directory\n", GetBaseFile(fn) );
	fd = fopen(GetBaseFile(fn), "w");
    }

    if( fd == NULL ){
	printf("Failed %s - %s\n", GetBaseFile(), strerror(errno) ); 
	return;
    }

    printf("Recreating %s\n", GetBaseFile(fn));

    fwrite(fBuffer, sizeof(char), fBufferSize, fd);

    fclose(fd);
}

void comptonTextFile::Print(){
    if( fBufferSize > 1024 ){
	printf("Stored file %s (%lld kB)\n", fFilename, fBufferSize/1024);
    } else {
	printf("Stored file %s (%lld bytes)\n", fFilename, fBufferSize);
    }
    char *tmpbuf = new char[fBufferSize+1];
    memcpy( tmpbuf, fBuffer, fBufferSize );
    tmpbuf[fBufferSize] = '\0';  // Make a string and manually terminate

    printf("%s\n", tmpbuf);

    delete[] tmpbuf;
}

const char *comptonTextFile::GetBaseFile(const char *fp){
    if( fp == NULL ) fp = fFilename;

    int idx = strlen(fp)-1;

    while( fp[idx] != '/' && idx != 0 ) idx--;
    if( fp[idx] == '/' ) idx++;

    return &(fp[idx]);
}

ClassImp(comptonTextFile)
