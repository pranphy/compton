#if defined(__CINT__) || defined(__CLING__)

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class comptonRunData+;
#pragma link C++ class comptonTextFile+;
#pragma link C++ struct filedata_t+;

#pragma link C++ struct comptonUnits_t+;

#pragma link C++ struct comptonSeed_t+;
#pragma link C++ struct comptonEvent_t+;
#pragma link C++ struct comptonBeamTarget_t+;
#pragma link C++ struct comptonEventParticle_t+;
#pragma link C++ class vector<comptonEventParticle_t>+;

#pragma link C++ struct comptonGenericDetectorHit_t+;
#pragma link C++ class vector<comptonGenericDetectorHit_t>+;

#pragma link C++ struct comptonGenericDetectorSum_t+;
#pragma link C++ class vector<comptonGenericDetectorSum_t>+;
#pragma link C++ struct comptonGenericDetectorSumByPID_t+;
#pragma link C++ class vector<comptonGenericDetectorSumByPID_t>+;

#endif
