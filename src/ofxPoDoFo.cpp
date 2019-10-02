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
		parse::Parser::Context context;
		float top = page->GetMediaBox().GetBottom()+page->GetMediaBox().GetHeight();
		context.mat[1][1] = -1;
		context.mat[3][1] = top;
		auto paths = parse::Parser().parse(&tokenizer, &context);
		for(auto &&path : paths) {
			page_[i].addPath(path);
		}
	}
}

