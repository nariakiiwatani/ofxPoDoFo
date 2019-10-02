#pragma once

#include "podofo.h"
#include "ofPath.h"

namespace ofx { namespace podofo {
namespace parse {
class Parser {
public:
	struct Context {
		glm::mat4 mat = glm::mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
		ofPath path, clipping;
		bool clipping_enabled=false;
		float stroke_width=1;
		glm::vec2 start_pos, current_pos;
		Context();
		ofPath getClippedPath() const;
	};
	std::vector<ofPath> parse(PoDoFo::PdfContentsTokenizer *tokenizer, Parser::Context context=Parser::Context());
};
class Extractor {
public:
	Extractor(const std::string &token)
	:token_(token) { }
	bool extract(const char *chk, Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
		if(strcmp(token_.c_str(), chk)==0) {
			run(context, vars);
			return true;
		}
		return false;
	}
protected:
	virtual void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars){}
	std::string token_;
};
}
}}
