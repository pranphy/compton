#ifndef __REMOLLTEXTFILE_HH
#define __REMOLLTEXTFILE_HH

#define __STRLEN 1024

#include "TObject.h"

class comptonTextFile : public TObject {
  using TObject::Print;
     public:       
	 comptonTextFile();
	 comptonTextFile(const comptonTextFile &);
	 const comptonTextFile& operator=(const comptonTextFile &);
	 comptonTextFile(const char *);
	 virtual ~comptonTextFile();

	 void copyFileIn(const char *);

	void Print();

	const char *GetFilename(){ return fFilename; }
	unsigned long long int GetBufferSize(){ return fBufferSize; }
	
	void Recreate(const char *fn = NULL, bool clobber = false);
	void RecreateInDir(const char *path, bool clobber = false);

    private:
	unsigned int fFilenameSize;
	char *fFilename;

	unsigned long long int fBufferSize;
	char *fBuffer;

	const char *GetBaseFile(const char *fp = NULL);

	ClassDef(comptonTextFile, 1);
};

#endif//__REMOLLTEXTFILE_HH
