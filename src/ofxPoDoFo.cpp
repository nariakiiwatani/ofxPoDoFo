#include "ofxPoDoFo.h"
#include "ofxPoDoFoParse.h"

using namespace ofx::podofo;
using namespace PoDoFo;
using namespace std;

void Document::load(const std::string &filepath) {
	PoDoFo::PdfMemDocument doc(ofToDataPath(filepath).c_str());
	
	page_.clear();
	page_.resize(doc.GetPageCount());
	for(int i = 0; i < doc.GetPageCount(); ++i) {
		auto page = doc.GetPage(i);
		PdfContentsTokenizer tokenizer(page);
		auto paths = parse::Parser().parse(&tokenizer);
		for(auto &&path : paths) {
			page_[i].addPath(path);
		}
	}
}

