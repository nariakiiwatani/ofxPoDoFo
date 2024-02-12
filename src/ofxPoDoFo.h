#pragma once

#include "podofo.h"
#include "ofPath.h"
#include <string>

using std::vector;
using std::string;
using namespace PoDoFo;

class Page {
public:
	void addPath(const ofPath &path) {
		path_.push_back(path);
	}
	void draw() const {
		for(auto &&p : path_) {
			p.draw();
		}
	}
private:
	std::vector<ofPath> path_;
};
class Document {
public:
	void load(const std::string &filepath);
	void draw() const {
		for(auto &&p : page_) {
			p.draw();
		}
	}
private:
	std::vector<Page> page_;
};

//using ofxPoDoFo = podofo::Document;
