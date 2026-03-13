#pragma once
// -*- coding: utf-8 -*-
// vim: ai ts=4 sts=4 et sw=4 ft=cpp foldmethod=marker
// author : Prakash [प्रकाश]
// date   : 2025-05-03
// version 2.0.1

/**
  * This is `util.h` and has a lot of utility functions that I use very often.
  * The stuff here are used by a lot of other scripts in analysis of the simulation
  * data
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <functional>
#include <utility>


#include "TFile.h"
#include "TChain.h"
#include "TTree.h"
#include "TLatex.h"
#include "TLine.h"
#include "TPolyLine.h"
#include "TArc.h"

#include "ROOT/RDataFrame.hxx"

#include "comptontypes.hh"


namespace PID {
    const int ELECTRON =  11;
    const int POSITRON = -11;
    const int PHOTON   =  22;
    const int PROTON   =  2212;
}

/** The type of `hit` object stored in the compton simulation root file is
    `comptonGenericDetectorHit_t` which is a bit lot to remember and type. So I
    typedefed this to ComptonHit which is easy to remember

    All the compton types are defined in comptontypes.hh file.
*/
typedef comptonGenericDetectorHit_t ComptonHit;

/**
  Frequently we need a `std::vector` of \ref ComptonHit. This is just an
  alias to std::vector<ComptonHit> which in turn is an alias so it effect
  this is  std::vector<comptonGenericDetectorHit_t> which would be a handful
  to type. So you could just use `hit_list`. `hit_vector` would've been better name.
*/
typedef std::vector<ComptonHit> hit_list;
typedef std::function<bool(const ComptonHit&)> hitfunc;
typedef std::function<bool(const ComptonHit&)> hit_cut;

/** The type of `ipart` object stored in the compton simulation root file is
    `comptonEventParticle_t` which is a bit lot to remember and type. So I
    typedefed this to  @ref `ComptonPart` which is easy to remember

    All the compton types are defined in comptontypes.hh file.
*/
//typedef comptonEventParticle_t ComptonPart;
//typedef std::vector<ComptonPart> part_list;

typedef std::function<std::pair<double,double>(ComptonHit)> pair_func;

//typedef std::tuple<hit_list,part_list,double> ComptonHitPartRate;

// ===================== CPP STUFFS ====================
namespace utl {

// standard cpp{{{

int log_level = 3;
void log(std::string message,int level=2){
    if(level < log_level) std::cout<<"[Log] "<<message<<std::endl;
}

constexpr double pi = 3.1415926535897932384626433832795;
/**
  Given a std::string, split it by the delimiter and return the vector of tokens.
  \param full the full string to split
  \param delimiter the delimiter to split the full string with
  \return `std::vector<std::string>` of each token after splitting by delimiter.
*/
std::vector<std::string> split(std::string full, char delimiter=',') {
    std::string segment; std::stringstream fullstream(full);
    std::vector<std::string> seglist;

    while(std::getline(fullstream, segment, delimiter)) {
        seglist.push_back(segment);
    }
    return seglist;
}

std::string get_extension(std::string str){
    auto npos = str.find_last_of('.');
    return npos == str.npos ? "" : str.substr(npos+1,str.size());
}



/**
  A handy shortcut function to check if the element elem is inside a container ( a std::vector ).
  \param container a std::vector of any type
  \param elem an object of the sme type as the vector objects.
  \returns true of elem is contained in container else false.
*/
template <typename T>
bool contains(std::vector<T>& container,const T& elem) {
    return container.end() != std::find(container.begin(),container.end(),elem);
}

template <typename T>
std::vector<T> logspace(int min, int max, int bins, T offset){
    T bw = (1.0*max - min) / bins;
    std::vector<T> vals(bins+1);
    for(int i = 0; i <= bins; ++i) vals.at(i) = offset + std::pow(10, i*bw + min);
    return vals;
}

template <typename T>
std::vector<T> logspace(T min, T max, int bins){
    T xe = std::log10(max/min); T bw = xe / bins;
    std::vector<T> vals(bins+1);
    for(int i = 0; i <= bins; ++i) vals.at(i) =  min  * std::pow(10, i*bw);
    return vals;
}


template <typename T>
bool is_inside_pgm(const T& x, const T& y,std::vector<T> box){
    T x0 = box[0], x1 = box[4], y0 = box[2],  y1 = box[6],
      x2 = box[1], x3 = box[5], y2 = box[3],  y3 = box[7];
    bool L0up   =  (y-y0) > ( (y1-y0)/(x1-x0)*(x-x0) );
    bool L1down =  (y-y0) < ( (y2-y1)/(x2-x1)*(x-x1) );
    bool L2down =  (y-y2) < ( (y2-y3)/(x2-x3)*(x-x3) );
    bool L3up   =  (y-y3) > ( (y3-y0)/(x3-x0)*(x-x0) );
    return L0up and L1down and L2down and L3up;
}


template <typename T>
bool is_inside(const T& x, const T& y,std::vector<T> box){
    if(box.size() <= 4)
        return (x > box[0] and x <= box[1] and y > box[2] and y <= box[3]); //box = [x0,x1, y0, y1]
    else
        return is_inside_pgm(x,y, box);
}

/**
  Merges std::vector of type <T> into a single std::vector.
*/
template <typename T, typename... Vectors>
std::vector<T> merge(Vectors&&... vecs) {
    std::vector<T> result;
    result.reserve((vecs.size() + ...));
    (result.insert(result.end(), std::make_move_iterator(vecs.begin()), std::make_move_iterator(vecs.end())), ...);
    return result;
}


template <typename T>
void print(std::vector<T>& container, const std::string formatter="%8.1f, "){
    printf("[ ");
    std::for_each(container.begin(),container.end(),[&](T c){ printf(formatter.c_str(),c);});
    printf("] ");
}

template <typename T>
void println(std::vector<T>& container,const std::string formatter="%8.1f, "){
    print(container,formatter);
    printf("\n");
}

// General utilities

double hypot(double x, double y) { return std::hypot(x, y); }
double to_degree(double a) { return a*180/pi;}
//! Given a angle a in degrees, returns the radian value of the angle in radians
double to_radian(double a) { return a*pi/180;}

// inverse tangent in degrees
double atan_deg(double y, double x){
    double rad = std::atan2(y,x);
    return to_degree(rad);
}

double neg_deg(double a){
    int sign =  a < 0 ? -1 : 1;
    return std::fmod(360-sign*(180.0-std::abs(a)),360);
}

/**
  Returns the difference between two angles in degrees
    \param a the first angle
    \param b the second angle
  This is not as simple as it sounds because of the fact that
  it wraps around from -180 to 180 degrees.
*/
double diff_deg(double a, double b){
    double d1 = std::abs(a - b);
    double d2 = std::abs(utl::neg_deg(a) - utl::neg_deg(b));
    return std::min(d1,d2);
}

// inverse tangent in degrees
double atan_rad(double y, double x){
    return std::atan2(y,x);
}

/**
  Reads the text file line by line and returns all lines as a vector of strings.
  \param filename The path of text file to read
  \returns `std::vector<std::string>` of each line in file.
*/
std::vector<std::string> readlines(const std::string filename,int nol=-1){
    std::vector<std::string> all_lines;
    std::string line; std::ifstream fileobj(filename);
    while(std::getline(fileobj,line)){
        all_lines.push_back(std::move(line));
        if(all_lines.size() >= nol) break;
    }
    return all_lines;
}

TChain* make_chain(std::string filelist,int nof = 0, bool verbose=false){
    std::ifstream flist(filelist);
    std::string filename;
    TChain* TC = new TChain("T");
    int cnt = 0;
    while(std::getline(flist,filename)){
        if(++cnt > nof && nof > 0)  break;
        if(filename.length()<2) continue;
        if(verbose) std::cout<<"Adding "<<filename<<std::endl;
        TC->Add(filename.c_str());
    }
    return TC;
}



// }}} CPP helpers

/// {{{ ROOT Stuffs

/**
  This function saves  objects into rootfile inside a directory. The file is created if it doesn't exist.
    \param Obj (usually histograms right?)
    \param name Name of the object in the root file
    \param directory the directory in the rootfile where the object is saved
    \param rootfile the path to root file to save the object to
*/

void save(TObject* Obj, std::string name, std::string directory="", std::string rootfile="__saved-root-objects.root"){
    auto F = std::make_unique<TFile>(rootfile.c_str(),"UPDATE");
    if(directory != ""){
        TDirectory* objdir = F->GetDirectory(directory.c_str());
        if(!objdir) objdir = F->mkdir(directory.c_str());
        F->cd(directory.c_str());
    }
    Obj->Write(name.c_str());
    F->Close();
}

/**
  This function saves  std::vector into rootfile inside a directory. The file is created if it doesn't exist.
    \param treename the name of the tree to save data to. Usually this is not very important
    \param data a std::vector any type. usually std::vector<float>
    \param name Name of the object in the root file
    \param directory the directory in the rootfile where the object is saved
    \param rootfile the path to root file to save the object to
*/
template<typename T> void save(std::string treename, std::vector<T>& data, std::string name, const std::string directory="", std::string rootfile="__saved-root-data.root"){
    utl::log("saving Generic ",2);
    auto F = std::make_unique<TFile>(rootfile.c_str(),"UPDATE");
    if(directory != ""){
        TDirectory* objdir = F->GetDirectory(directory.c_str());
        if(!objdir) objdir = F->mkdir(directory.c_str());
        F->cd(directory.c_str());
    }

    std::unique_ptr<TTree> tree;
    if (F->Get(treename.c_str()) == nullptr) tree = std::make_unique<TTree>(treename.c_str(), "Data Tree");
    else tree = std::make_unique<TTree>(dynamic_cast<TTree*>(F->Get(treename.c_str())));

    T elem;
    tree->Branch(name.c_str(), &elem);
    F->cd();
    for(auto dt: data) {elem = dt; tree->Fill(); }
    F->Write();
    F->Close();
}

//! Same as other data save function but omits the treename parameter to set to "T", imitating the compton tree name.
template<typename T>
void save(std::vector<T>& data, std::string name, const std::string directory="", std::string rootfile="__saved-root-objects.root"){
    utl::log("saving Generic ",2);
    save("T",data,name,directory,rootfile);
}

/**
  Returns a RDataFrame object for the given filename. It looks for a tree named "T" (the default tree name for compton(
  doesn't care if the the filename points to a zombie file. It doesn't make sure that the file exists
  so have to pase a existing tree file.
  \param filename The path to the root file.
  \param treename The name of the tree to read ("T" by default)
  \returns RDataFrame object.
*/
ROOT::RDataFrame  get_df(const std::string filename,const std::string treename="T"){
    return ROOT::RDataFrame(treename,filename);
}

/**
  Returns a TTree object for the given filename. It looks for a tree named "T" (the default tree name for compton(
  doesn't care if the the filename points to a zombie file. It doesn't make sure that the file exists
  so have to pase a existing tree file.
  \param filename The path to the root file.
  \param treename The name of the tree to read ("T" by default)
  \returns Pointer to TTree object.
*/

TTree* get_tree(std::string filename,const std::string treename="T"){
    TFile* F = new TFile(filename.c_str());
    TTree* T = F->Get<TTree>(treename.c_str());
    return T;
}


/**
  For a given filename checks if the root file is a zombie file. If it is a zombie file
  it will return false. Otherwie it will update the reference to the tree and return true.
  \param filename The path to root file.
  \param T The address of the pointer to a Ttree object which has to be updated.
  \param treename The name of the Tree in the root file ("T" by default)
  returns true if the file is not a zombie, false if it is.

  */
bool get_tree(std::string filename, TTree** T, const std::string treename="T"){
    TFile* F = new TFile(filename.c_str());
    if(F->IsZombie())
        return false;
    else
        *T = F->Get<TTree>(treename.c_str());
    return true;
}



/// {{{ Graphics stuffs
namespace plt {
void line(std::vector<double>& p0, std::vector<double>& p1){
    auto line = new TLine(p0[0],p0[1], p1[0],p1[1]);
    line->SetLineColor(kBlue); line->SetLineColor(2); line->SetLineWidth(1);
    line->Draw();
}

/**
  Adds a vertical line on the screen. The colour is red.
  \param xval The x coordinate at the current axis where the vertical line is drawn
*/
void vline(double xval){
    auto line = new TLine(xval, gPad->GetUymin(), xval, gPad->GetUymax());
    line->SetLineColor(kRed);
    line->SetLineStyle(2);
    line->SetLineWidth(1);
    line->Draw();
}

/**
  Adds a horizontal line on the screen. The colour is red.
  \param yval The y coordinate at the current axis where the horizontal line is drawn
*/
void hline(double yval){
    auto line = new TLine( gPad->GetUxmin(), yval, gPad->GetUxmax(),yval);
    line->SetLineColor(kRed);
    line->SetLineStyle(2);
    line->SetLineWidth(1);
    line->Draw();
}

/**
  Draws a circle of given radius, and center (optionally)
  \param radius The radius of the circle to draw
  \param x The x coordinate of the center
  \param y The y coordinate of the center
*/
void circle(float radius, float x = 0, float y = 0,int colour=kBlue,int style=1,int width=1){
    auto circle = new TArc(x, y, radius);
    circle->SetLineColor(colour); circle->SetLineStyle(style); circle->SetLineWidth(width);
    circle->SetFillStyle(0); // fillstyle 0 is transparent
    circle->Draw();
}

void polygon(std::vector<float> x, std::vector<float> y,int colour=kRed, int thickness=2,int style=2){
    TPolyLine* poly = new TPolyLine(x.size(),x.data(),y.data());
    poly->SetLineWidth(thickness); poly->SetLineColor(colour); poly->SetLineStyle(style); poly->Draw();
}

void quad(std::vector<float> box,int colour=kRed, int thickness=2,int style=2){
    std::vector<float> x={box[0],box[0],box[1],box[1],box[0]};
    std::vector<float> y={box[2],box[3],box[3],box[2],box[2],};
    polygon(x, y, colour, thickness, style);
}

void rectangle(std::vector<float> box,int colour=kRed, int thickness=2,int style=2) {
    std::vector<float> x={box[0],box[0],box[1],box[1],box[0]};
    std::vector<float> y={box[2],box[3],box[3],box[2],box[2],};
    polygon(x, y, colour, thickness, style);
}

void text(std::string text, Double_t normx, Double_t normy, int colour=kRed, Double_t size = 0.04, int NDC=0) {
  TLatex* label = new TLatex();
  label->SetNDC(NDC);
  label->SetTextAlign(22);
  label->SetTextSize(size);
  label->SetTextColor(colour);
  label->DrawLatex(normx, normy, text.c_str());
}

} // plt::

// }}} Graphics Stuffs


///}}} Root Stuffs

// {{{ Compton Stuffs


/// {{{ Cut
namespace cut{

    auto E(float E1, float E2 =  12000){
        return [E1,E2](const ComptonHit& hit)->bool { return hit.e > E1 and hit.e <= E2; };
    };

    bool E1(const ComptonHit& hit) { return E(1)(hit); }
    //! Returns true if the hit pid is electron or positron.

    /** (e) (p)lus/(m)inus cut; only exists for backward compatibility \ref cut::epm
      \param hit The Comptonhit object
      \returns true if the hit is a particle with pid \f$ \pm 11\f$  otherwise false
    */
    bool epm(const ComptonHit& hit) { return hit.pid == PID::ELECTRON || hit.pid == PID::POSITRON; }
    //! Returns true if the hit pid is photon
    bool photon(const ComptonHit& hit) {return hit.pid == PID::PHOTON; }

    auto type(std::string ptype){
        return [ptype](const ComptonHit& hit)->bool { if(ptype=="epm") return epm(hit); else if(ptype=="photon") return photon(hit); else return false; };
    }

    //! Returns true if it hits ring 5 of Main detector and has energy > 1MeV and also is a photon.
    auto det = [](int det){ return [det](const ComptonHit& hit)->bool { return hit.det == det; }; };
    //auto ring = [](int nring){ return [nring](ComptonHit& hit)->bool { return md_ring(hit,nring); }; };

    auto r(float r1,float r2=3500){
        return [r1,r2](const ComptonHit& hit)->bool { return hit.r > r1 and hit.r <= r2; };
    };

    /**
      Takes a vector of hit_cut and returns a hit_cut such that it
      only evaluates to true if each of the cut in the vector evaluates
      to true
      \param cuts std::vector<hit_cut>, the list of cuts to and
      \return lambda that evaluates to true if all the cuts pass when passed hit.
    */
    auto all(std::vector<hit_cut> cuts){
        return [cuts](const ComptonHit& hit) ->bool {
            bool anded = true;
            for(const auto& cut: cuts) anded = cut(hit) and anded;
            return anded;
        };
    }


} // cut::
// }}} Cut

typedef std::function<hit_list(hit_list&)> lookup_func;
bool __pass(ComptonHit) { return true; }

typedef std::function<double(const ComptonHit&)> __remhit_attrib_d;
auto __attrib_trid = [](const ComptonHit& hit)->double { return hit.trid; };

/// {{{ Compton Data Manip

/**
  Given hit array, this function looks first for hits that pass the first cut `cut_s`. For those hits,
  it looks again in the hit array to select those hits that pass the second cut `cut_d` and also have
  the same trackid as the first set of hit.
  \param hits the array of hits of type std::vector<ComptonHit> which is what is saved in hit branch of compton tree.
  \param cut_s The source cut;
  \param cut_d The destination cut (default value __pass) which returns true
  \param param The parameter of the hit object to look for, trid for trackid and mtrid for mother track
  */
hit_list select_tracks(hit_list& hits, const hit_cut& cut_s, const hit_cut& cut_d = __pass, const __remhit_attrib_d& param=__attrib_trid){
    utl::log("Now selecting tracks on size "+std::to_string(hits.size()),3);
    std::vector<int> trids;
    std::vector<ComptonHit>  rev_hits;
    int pass = 0;
    for(const auto& hit: hits){
        utl::log("Pass "+std::to_string(++pass),4);
        if(cut_s(hit)){
            utl::log("Cut passed ",4);
            trids.push_back(param(hit));
        } else {
            utl::log("Cut failed ",4);
        }
    }
    utl::log("First pass done ",4);
    for(auto& hit: hits) if(contains(trids,hit.trid) and cut_d(hit)) rev_hits.push_back(hit);
    return rev_hits;
}

/**
  Same implementation as \ref select_tracks but looks for mtrid for mother tracks instead for trid for tracks.
  */
hit_list select_mother_tracks(hit_list& hits, const hit_cut& cut_s, const hit_cut& cut_d = __pass){
    const __remhit_attrib_d param = [](const ComptonHit& hit)->double { return hit.mtrid; };
    return select_tracks(hits,cut_s,cut_d,param);
}


bool __get_det_hit(hit_list& hits, int trid, hit_cut cut, ComptonHit& dethit){
    for(auto hit : hits) if(cut(hit) and hit.trid == trid) { dethit = hit; return true; }
    return false;
}

/**
   Given a list of cuts and a list of hits, looks up those specific tracks that pass all the cuts.
   It returns a vector of all those hit_list where each hit_list is the hit corresponding to each
   cut in the cut list.
   \param hits The hits to look up from, this is typically for an event
   \param cuts The list of cut function, to select those hits that pass
   \returns a matrix of hits corresponding to the cuts
*/
std::vector<hit_list> lookup_tracks(hit_list& hits, std::vector<hit_cut> cuts){
    std::set<int> trids; for(auto hit: hits) if(cuts[0](hit)) trids.insert(hit.trid);
    std::vector<hit_list> trackhits;
    for(int trid : trids){
        bool all = true; hit_list cuthits;
        for(auto cut : cuts){
            ComptonHit tmphit;
            all = all and __get_det_hit(hits, trid, cut, tmphit);
            cuthits.push_back(tmphit);
        }
        if(all) trackhits.push_back(cuthits);
    }
    return trackhits;
}

/**
  This function takes 2 detector ids, (and a basecut) and returns a lambda.
  The lambda will take a hit_list and a TH2* pointer. Now it will loop through the
  hits fills the histogram with hit.r at det1 vs hit.r at det2 with the same trackid
  and passing the cut passed as the third parameter.
  \param det1 The detid of first detector
  \param det2 The detid of second detector
  \param basecut The cut that each of the tracks has to pass (usually utl::cut::epm).
  \returns lambda(hit_list, TH2*) where TH2* will be filled with hit.r at det1 vs hit.r at det2
*/
auto rr_correlate(int det1, int det2, hit_cut basecut = utl::cut::epm){
    return [basecut,det1,det2](hit_list& hl, TH2D* hist)->void{
        std::vector<hit_cut> cuts{basecut};
        std::vector<int> dets = {det1,det2}; for(auto detid : dets) cuts.push_back(cut::det(detid));
        std::vector<hit_list> those_tracks = lookup_tracks(hl, cuts);
        for(hit_list& cuthits : those_tracks){
            ComptonHit h0 = cuthits[1], h1 = cuthits[2];
            hist->Fill(h0.r, h1.r);
        }
    };
}

const lookup_func track_selector(const hit_cut& cut){
    utl::log("Selecting tracks that pass the cut",3);
    const lookup_func track_selector = [&cut](hit_list& hits)->hit_list {
        utl::log("Inside selector Selecting tracks that pass the cut",3);
        return select_tracks(hits,cut);
    };
    return track_selector;
}

const lookup_func mother_track_selector(const hit_cut& cut){
    return [&cut](hit_list& hits) { return select_mother_tracks(hits,cut); };
}

// }}} Compton data Manip



typedef std::function<double(const ComptonHit&)> __attrib_f;

const lookup_func identity_lookup = [](hit_list& hits)->hit_list { return hits;};
//{{{ att stuff
/**
  So the idea behind this att stuff is that I want to be able to get any of the parameters
  or some function of individual hit object. This att objects are basically functions that
  extract out what i want. The ones named x, y etc pull hit.x hit.y etc but there is a need
  of getting some function of them. Also user can define their own attrib functions and get
  custom function during usage, possibly to make histogram.
  */
namespace att{
    const __attrib_f x    = [](const ComptonHit& hit){ return hit.x; };
    const __attrib_f y    = [](const ComptonHit& hit){ return hit.y; };
    const __attrib_f z    = [](const ComptonHit& hit){ return hit.z; };
    const __attrib_f r    = [](const ComptonHit& hit){ return hit.r; };
    const __attrib_f e    = [](const ComptonHit& hit){ return hit.e; };
    const __attrib_f edep = [](const ComptonHit& hit){ return hit.edep; };
    const __attrib_f phi  = [](const ComptonHit& hit){ return utl::atan_deg(hit.y,hit.x); };
    const __attrib_f th   = [](const ComptonHit& hit){ return utl::atan_deg(std::hypot(hit.x,hit.y),hit.z);};
    const __attrib_f pphi = [](const ComptonHit& hit){ return utl::atan_deg(hit.py,hit.px); };
    const __attrib_f pth  = [](const ComptonHit& hit){ return utl::atan_deg(std::hypot(hit.px,hit.py),hit.pz);};

    const __attrib_f vx   = [](const ComptonHit& hit){ return hit.vx; };
    const __attrib_f vy   = [](const ComptonHit& hit){ return hit.vy; };
    const __attrib_f vz   = [](const ComptonHit& hit){ return hit.vz; };
    const __attrib_f vr   = [](const ComptonHit& hit){ return std::hypot(hit.vx,hit.vy); };
    const __attrib_f rate = [](const ComptonHit&){ return -1; };

}
// }}} att stuff

// {{{ Histogram Stuffs

struct hist;
typedef std::function<void(hit_list&, TH2D*)> __cbackf;
//typedef std::function<void(hit_list&, TH1D*)> __cback1f;
//typedef void(*)(hit_list&, hist&) __cbackf;

void fill(ROOT::RDataFrame&, std::vector<hist>&&);

struct hist{
    std::string name;
    lookup_func lookup;
    hit_cut cut;
    std::string title;
    std::vector<__attrib_f> params;
    std::vector<float> bins;
    TObject* obj;
    TH1D* h1; TH2D* h2;
    int dim;
    __cbackf callback; bool calls;

    hist(int dim, TObject* h,hit_cut cut, std::vector<__attrib_f> params, lookup_func lookup=identity_lookup)
        :lookup(lookup), cut(cut), params(params), bins({}), dim(dim),calls(false)
    {
        if(dim == 1) {
            h1 = (TH1D*) h; name = std::string(h1->GetName());
            bins = { float(h1->GetXaxis()->GetNbins()),float(h1->GetXaxis()->GetXmin()),float(h1->GetXaxis()->GetXmax()) };

        } else if(dim == 2) {
            h2 = (TH2D*) h; name = std::string(h2->GetName());
            bins = {
                float(h2->GetXaxis()->GetNbins()), float(h2->GetXaxis()->GetXmin()), float(h2->GetXaxis()->GetXmax()),
                float(h2->GetYaxis()->GetNbins()), float(h2->GetYaxis()->GetXmin()), float(h2->GetYaxis()->GetXmax())
            };
        }
    }

    //hist(std::string name, __cback1f callback, std::string title,  std::vector<float> bins)
    //    :name(name), title(title), bins(bins), callback(callback),  calls(true)
    //{
    //    dim = 1;
    //    obj = init_hist();
    //}


    hist(std::string name, __cbackf callback, std::string title,  std::vector<float> bins)
        :name(name), title(title), bins(bins), callback(callback),  calls(true)
    {
        dim = 2;
        obj = init_hist();
    }

    hist(std::string name, lookup_func lookup, hit_cut cut, std::vector<__attrib_f> params, std::string title,  std::vector<float> bins)
        :name(name), lookup(lookup), cut(cut), title(title), params(params), bins(bins),  calls(false)
    {
        dim = params.size();
        obj = init_hist();
    }

    hist(std::string name, lookup_func lookup, hit_cut cut, __attrib_f param, std::string title,  std::vector<float> bins)
        :name(name), lookup(lookup), cut(cut), title(title), params({param}), bins(bins), calls(false)
    {
        dim = params.size();
        obj = init_hist();
    }

    hist(std::string name, hit_cut cut, __attrib_f param, std::string title,  std::vector<float> bins)
        :name(name), cut(cut), title(title), params({param}), bins(bins),  calls(false)
    {
        dim = params.size();
        obj = init_hist();
        lookup = identity_lookup;
    }
    hist(std::string name, hit_cut cut, std::vector<__attrib_f> params, std::string title,  std::vector<float> bins)
        :name(name), cut(cut), title(title), params(params), bins(bins), calls(false)
    {
        dim = params.size();
        lookup = identity_lookup;
        obj = init_hist();
    }

    TObject* init_hist(){
        if(dim == 1){
            if (bins.size() > 6){ // well actually this is bit of a hack, but should work
                h1 = new TH1D(name.c_str(),title.c_str(),bins.size()-1, bins.data()); // for explicit bins passed
                return h1;
            }else{
                h1 = new TH1D(name.c_str(),title.c_str(), bins[0],bins[1],bins[2]);
                return h1;
            }
        } else if(dim == 2){
            h2 = new TH2D(name.c_str(),title.c_str(), bins[0],bins[1],bins[2],bins[3],bins[4],bins[5]);
            return h2;
        }
        return nullptr;
    }

    void fillit(ComptonHit hit,double rate=1){
        if(dim == 1){
            //TH1D* hist = (TH1D*) obj;
            auto val = params[0](hit);
            if(val > h1->GetXaxis()->GetXmin() and val <= h1->GetXaxis()->GetXmax()) h1->Fill(val,rate);
        }
        else if (dim == 2){
            //TH2D* hist = (TH2D*) obj;
            auto val0 = params[0](hit); auto val1 = params[1](hit);
            if(val0 > bins[1] and val0 <= bins[2] and val1 > bins[4] and val1 <= bins[5]) h2->Fill(val0,val1,rate);
        }
    }
    hist yscale(double scale){
        if(dim == 1) h1->Scale(scale);
        else if(dim == 2) h2->Scale(scale);
        return *this;
    }

    void save(std::string dir, std::string filename){
        if(dim == 1){
            //TH1D* hist = (TH1D*) obj;

            utl::save(h1,name,dir,filename);
        }
        else if(dim == 2){
            //TH2D* hist = (TH2D*) obj;
            utl::save(h2,name,dir,filename);
        }
    }

   void draw(std::string opt=""){
        if(dim == 1 and opt=="") opt="hist";
        if(dim == 2 and opt=="") opt="colz";
        if(dim == 1) h1->Draw(opt.c_str()); else h2->Draw(opt.c_str());
    }

    hist loop(ROOT::RDataFrame& df){
        fill(df,{*this});
        return *this;
    }

}; // hist

void fill(ROOT::RDataFrame& df, std::vector<hist>& histograms){
    auto fill_hists = [&](hit_list hits)->void {
        for(auto& hist: histograms){
            if(hist.calls){
                hist.callback(hits,hist.h2);
            } else {
                hit_list looked_up = hist.lookup(hits);
                for(auto hit: looked_up){
                    if(hist.cut(hit)) hist.fillit(hit);
                }
            }
        }
    };
    df.Foreach(fill_hists,{"hit"});
}

void fill_rate(ROOT::RDataFrame& df, std::vector<hist>& histograms){
    auto fill_hists = [&](hit_list hits,double rate)->void {
        for(auto& hist: histograms){
            if(hist.calls){
                hist.callback(hits,hist.h2);
            } else {
                hit_list looked_up = hist.lookup(hits);
                for(auto hit: looked_up){
                    if(hist.cut(hit)) hist.fillit(hit,rate);
                }
            }
        }
    };
    df.Foreach(fill_hists,{"hit","rate"});
}

void fill(ROOT::RDataFrame& df,std::vector<hist>&& histograms){
    fill(df,histograms);
}

void fill_rate(ROOT::RDataFrame& df,std::vector<hist>&& histograms){
    fill_rate(df,histograms);
}

void save(std::vector<hist>& hists,std::string dir, std::string filename){
    for(auto& hist: hists){
        hist.save(dir,filename);
    }
}


// }}} Histogram Stuffs


// }}} Compton Stuffs


} // utl::


namespace att = utl::att;
