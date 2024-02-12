#include "ofxPoDoFo.h"
#include "ofxPoDoFoParse.h"

using namespace PoDoFo;
using std::string;
using std::vector;

void Document::load(const std::string &filepath) {
	PoDoFo::PdfMemDocument doc;
    doc.Load(ofToDataPath(filepath).c_str());
	
	page_.clear();
	page_.resize(doc.GetPages().GetCount());
	for(int i = 0; i < doc.GetPages().GetCount(); ++i) {
        PdfPage & page = doc.GetPages().GetPageAt(i);
//        PdfTokenizer tokenizer;
//        tokenizer.
		Parser::Context context;
        float top = page.GetMediaBox().GetBottom() + page.GetMediaBox().Height;
		context.mat[1][1] = -1;
		context.mat[3][1] = top;
        
//		auto paths = parse::Parser().parse(&tokenizer, context);
//		for(auto &&path : paths) {
//			page_[i].addPath(path);
//		}
	}
}

