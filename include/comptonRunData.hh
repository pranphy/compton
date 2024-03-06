#ifndef __REMOLLRUNDATA_HH
#define __REMOLLRUNDATA_HH

#include "TObject.h"

#include <vector>
#include <string>

#include "comptontypes.hh"
#include "comptonTextFile.hh"

/*!
 * All the information on the run
 * This will get put into the output
 * stream
*/

class comptonRunData : public TObject {
  using TObject::Print;
    public:
	comptonRunData() { };
	virtual ~comptonRunData() { };

        void Init();

	void Print();

    private:
        std::vector<filedata_t> fMagData;
    public:
	void AddMagData(filedata_t d) { fMagData.push_back(d); }

    private:
	comptonTextFile fMacro;
    public:
	void SetMacroFile(const char *fn) { fMacro = comptonTextFile(fn); }

    private:
	std::vector<comptonTextFile> fGDMLFiles;
    public:
	void AddGDMLFile(const char *fn);
	comptonTextFile GetGDMLFile(int i) const { return fGDMLFiles[i]; }
	void ClearGDMLFiles(){ fGDMLFiles.clear(); }
	void RecreateGDML(const char *adir = NULL, bool clobber = false);

    private:
	long int fNthrown;
    public:
	void SetNthrown(unsigned long long int n) { fNthrown = n; }
	unsigned long long int GetNthrown() const { return fNthrown; }

    private:
	long int fSeed;
    public:
	void SetSeed(unsigned long int seed) { fSeed = seed; }
        unsigned long long int GetSeed() const { return fSeed; }

    private:
        std::string fGitInfo;
    public:
        std::string GetGitInfo() const { return fGitInfo; }

    private:
	TTimeStamp fRunTime;
    public:
        TTimeStamp GetRunTime() const { return fRunTime; }

    private:
	std::string fRunPath;
    public:
        std::string GetRunPath() const { return fRunPath; }

    private:
        std::string fHostName;
    public:
        std::string GetHostName() const { return fHostName; }

	ClassDef(comptonRunData, 2);
};

#endif//__REMOLLRUNDATA_HH
