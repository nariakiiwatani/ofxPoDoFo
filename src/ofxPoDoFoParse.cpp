#include "ofxPoDoFoParse.h"


using namespace PoDoFo;
using std::string;

namespace {
//	std::string print(const PoDoFo::PdfVariant &var) {
////		std::string ss;
////		switch(var.GetDataType()) {
////            case PdfLiteralDataType::Bool:			ss = std::string() + "bool: " + (var.GetBool()?"true":"false"); break;
////			case ePdfDataType_Number:		ss = std::string() + "number: " + ofToString(var.GetNumber()); break;
////			case ePdfDataType_Real:			ss = std::string() + "real: " + ofToString(var.GetReal()); break;
////			case ePdfDataType_String:
////			case ePdfDataType_HexString:	ss = std::string() + "string: " + var.GetString().GetString(); break;
////			case ePdfDataType_Name:			ss = std::string() + "name: " + var.GetName().GetName(); break;
////			case ePdfDataType_Array:
////			case ePdfDataType_Dictionary:
////			case ePdfDataType_Null:
////			case ePdfDataType_Reference:
////			case ePdfDataType_RawData:
////			case ePdfDataType_Unknown: ss = "unhandled"; break;
////		}
//		return ss;
//	}
}
namespace {
	class AffineTransform : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			glm::vec4 rs(vars[0].GetReal(), vars[1].GetReal(), vars[2].GetReal(), vars[3].GetReal());
			glm::vec2 t(vars[4].GetReal(), vars[5].GetReal());
			ofLogVerbose("ofxPoDoFoParser") << "affine transform: " << rs << "," << t;
			glm::mat4 mat(rs[0], rs[1], 0, 0,
						  rs[2], rs[3], 0, 0,
						  0, 0, 1, 0,
						  t[0], t[1], 0, 1);
			context.mat *= mat;
		}
	};
	class MoveTo : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			glm::vec2 pos(vars[0].GetReal(), vars[1].GetReal());
			context.current_pos =
			context.start_pos = pos;
			ofLogVerbose("ofxPoDoFoParser") << "move to: " << pos;
			pos = context.mat * glm::vec4(pos,0,1);
			context.path.moveTo(pos);
		}
	};
	class LineTo : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			glm::vec2 pos(vars[0].GetReal(), vars[1].GetReal());
			context.current_pos = pos;
			ofLogVerbose("ofxPoDoFoParser") << "line to: " << pos;
			pos = context.mat * glm::vec4(pos,0,1);
			context.path.lineTo(pos);
		}
	};
	class BezierTo : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			glm::vec2 cp1(vars[0].GetReal(), vars[1].GetReal());
			glm::vec2 cp2(vars[2].GetReal(), vars[3].GetReal());
			glm::vec2 cp3(vars[4].GetReal(), vars[5].GetReal());
			context.current_pos = cp3;
			ofLogVerbose("ofxPoDoFoParser") << "bezier to: " << cp1 << " " << cp2 << " " << cp3;
			cp1 = context.mat * glm::vec4(cp1,0,1);
			cp2 = context.mat * glm::vec4(cp2,0,1);
			cp3 = context.mat * glm::vec4(cp3,0,1);
			context.path.bezierTo(cp1, cp2, cp3);
		}
	};
	class EaseInTo : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			glm::vec2 cp1(vars[0].GetReal(), vars[1].GetReal());
			glm::vec2 cp2(vars[2].GetReal(), vars[3].GetReal());
			glm::vec2 cp3(vars[2].GetReal(), vars[3].GetReal());
			context.current_pos = cp3;
			ofLogVerbose("ofxPoDoFoParser") << "ease in to: " << cp1 << " " << cp2 << " " << cp3;
			cp1 = context.mat * glm::vec4(cp1,0,1);
			cp2 = context.mat * glm::vec4(cp2,0,1);
			cp3 = context.mat * glm::vec4(cp3,0,1);
			context.path.bezierTo(cp1, cp2, cp3);
		}
	};
	class EaseOutTo : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			glm::vec2 cp1(context.current_pos);
			glm::vec2 cp2(vars[0].GetReal(), vars[1].GetReal());
			glm::vec2 cp3(vars[2].GetReal(), vars[3].GetReal());
			context.current_pos = cp3;
			ofLogVerbose("ofxPoDoFoParser") << "ease out to: " << cp1 << " " << cp2 << " " << cp3;
			cp1 = context.mat * glm::vec4(cp1,0,1);
			cp2 = context.mat * glm::vec4(cp2,0,1);
			cp3 = context.mat * glm::vec4(cp3,0,1);
			context.path.bezierTo(cp1, cp2, cp3);
		}
	};
	class Rectangle : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			glm::vec4 xywh(vars[0].GetReal(), vars[1].GetReal(), vars[2].GetReal(), vars[3].GetReal());
			ofLogVerbose("ofxPoDoFoParser") << "rect: " << xywh;
			glm::vec2 pos[4] = {
				{xywh[0], xywh[1]},
				{xywh[0]+xywh[2], xywh[1]},
				{xywh[0]+xywh[2], xywh[1]+xywh[3]},
				{xywh[0], xywh[1]+xywh[3]},
			};
			for(auto &&v : pos) {
				v = context.mat * glm::vec4(v,0,1);
			}
			context.path.moveTo(pos[0]);
			context.path.lineTo(pos[1]);
			context.path.lineTo(pos[2]);
			context.path.lineTo(pos[3]);
			context.path.lineTo(pos[0]);
			context.path.close();
			context.current_pos =
			context.start_pos = pos[0];
		}
	};
	class Close : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			auto &&commands = context.path.getCommands();
			if(commands.empty() || commands.back().type == ofPath::Command::close) {
				return;
			}
			glm::vec2 pos = context.start_pos;
			context.current_pos = pos;
			ofLogVerbose("ofxPoDoFoParser") << "close";
			pos = context.mat * glm::vec4(pos, 0, 1);
			context.path.lineTo(pos);
			context.path.close();
		}
	};
	class StrokeColor : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			ofFloatColor color = vars.size() > 1 ? ofFloatColor(vars[0].GetReal(), vars[1].GetReal(), vars[2].GetReal()): ofFloatColor(vars[0].GetReal());
			ofLogVerbose("ofxPoDoFoParser") << "stroke color: " << color;
			context.path.setStrokeColor(color);
		}
	};
	class StrokeWidth : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			float width = vars[0].GetReal();
			ofLogVerbose("ofxPoDoFoParser") << "stroke width: " << width;
			context.stroke_width = width;
		}
	};
	class FillColor : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			ofFloatColor color = vars.size() > 1 ? ofFloatColor(vars[0].GetReal(), vars[1].GetReal(), vars[2].GetReal()): ofFloatColor(vars[0].GetReal());
			ofLogVerbose("ofxPoDoFoParser") << "fill color: " << color;
			context.path.setFillColor(color);
		}
	};
	class Drawer : public Extractor {
	public:
		Drawer(const std::string &token, ofPolyWindingMode winding_mode, bool close_path)
		:Extractor(token),winding_mode_(winding_mode),close_path_(close_path) { }
		virtual void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			context.path.setPolyWindingMode(winding_mode_);
			if(close_path_) {
				context.path.close();
			}
		}
	protected:
		ofPolyWindingMode winding_mode_;
		bool close_path_;
	};
	class Stroke : public Drawer {
	public:
		using Drawer::Drawer;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			ofLogVerbose("ofxPoDoFoParser") << "draw stroke";
			Drawer::run(context, vars);
			context.path.setFilled(false);
			context.path.setStrokeWidth(context.stroke_width);
		}
	};
	class Fill : public Drawer {
	public:
		using Drawer::Drawer;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			ofLogVerbose("ofxPoDoFoParser") << "draw fill";
			Drawer::run(context, vars);
			context.path.setFilled(true);
			context.path.setStrokeWidth(0);
		}
	};
	class StrokeFill : public Drawer {
	public:
		using Drawer::Drawer;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			ofLogVerbose("ofxPoDoFoParser") << "draw fill stroke";
			Drawer::run(context, vars);
			context.path.setFilled(true);
			context.path.setStrokeWidth(context.stroke_width);
		}
	};
	class NoDraw : public Drawer {
	public:
		using Drawer::Drawer;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			ofLogVerbose("ofxPoDoFoParser") << "no draw";
			Drawer::run(context, vars);
			context.path.setFilled(false);
			context.path.setStrokeWidth(0);
		}
	};
	class BeginGroup : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			ofLogVerbose("ofxPoDoFoParser") << "begin group";
		}
	};
	class EndGroup : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			ofLogVerbose("ofxPoDoFoParser") << "end group";
		}
	};
	class Ignore : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			auto &&log = ofLogNotice("ofxPoDoFoParser");
			log << "ignore: " << token_;
			log << "(";
//			for(auto &&v : vars) {
//				log << print(v) << ", ";
//			}
			log << ")";
		}
	};
	class ShouldHandle : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			auto &&log = ofLogWarning("ofxPoDoFoParser");
			log << "unhandled: " << token_;
			log << "(";
//			for(auto &&v : vars) {
//				log << print(v) << ", ";
//			}
			log << ")";
		}
	};
	class Any : public Extractor {
	public:
		using Extractor::Extractor;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			auto &&log = ofLogWarning("ofxPoDoFoParser");
			log << "unknown: " << token_;
			log << "(";
//			for(auto &&v : vars) {
//				log << print(v) << ", ";
//			}
			log << ")";
		}
	};
	class Clipping : public Drawer {
	public:
		using Drawer::Drawer;
		void run(Parser::Context &context, const std::vector<PoDoFo::PdfVariant> &vars) {
			ofLogVerbose("ofxPoDoFoParser") << "clipping";
			Drawer::run(context, vars);
			context.clipping.append(context.path);
			context.clipping_enabled = true;
		}
	};
};

Parser::Context::Context() {
}

ofPath Parser::Context::getClippedPath() const
{
	ofPath c = clipping, p = path;
	const float upscale = 1000000;
	c.scale(upscale, upscale);
	p.scale(upscale, upscale);
//	ofxClipper clipper;
//	clipper.addPath(c, OFX_CLIPPER_CLIP);
//	clipper.addPath(p, OFX_CLIPPER_SUBJECT);
	std::vector<ofPolyline> polys;
//	clipper.clip(OFX_CLIPPER_INTERSECTION, polys);
	ofPath ret = path;
	ret.clear();
	for(auto &&poly : polys) {
		if(poly.size() < 2) continue;
		poly.scale(1/upscale, 1/upscale);
		ret.moveTo(poly[0]);
		for(int i = 1; i < poly.size(); ++i) {
			ret.lineTo(poly[i]);
		}
		auto &&commands = path.getCommands();
		if(commands.empty() || commands.back().type == ofPath::Command::close) {
			ret.close();
		}
	}
	return ret;
}

std::vector<ofPath> Parser::parse(PdfContentsTokenizer *tokenizer, Parser::Context context)
{
	std::vector<ofPath> ret;
	const char *token = NULL;
	EPdfContentsType type;
	PdfVariant var;
	std::vector<PoDoFo::PdfVariant> vars;

	while(tokenizer->ReadNext(type, token, var))
	{
		bool escape = false;
		switch(type) {
			case ePdfContentsType_Keyword: {
				if(false) {}
				else if(Ignore("j").extract(token, context, vars)) {}
				else if(Ignore("J").extract(token, context, vars)) {}
				else if(Ignore("M").extract(token, context, vars)) {}
				else if(Ignore("d").extract(token, context, vars)) {}
				else if(Ignore("ri").extract(token, context, vars)) {}
				else if(Ignore("i").extract(token, context, vars)) {}
				else if(Ignore("MP").extract(token, context, vars)) {}
				else if(Ignore("DP").extract(token, context, vars)) {}
				else if(Ignore("BMC").extract(token, context, vars)) {}
				else if(Ignore("BDC").extract(token, context, vars)) {}
				else if(Ignore("EMC").extract(token, context, vars)) {}
				
				else if(ShouldHandle("cs").extract(token, context, vars)) {}
				else if(ShouldHandle("CS").extract(token, context, vars)) {}
				else if(ShouldHandle("gs").extract(token, context, vars)) {}
				
				else if(BeginGroup("q").extract(token, context, vars)) {
					auto &&paths = parse(tokenizer, context);
					ret.insert(end(ret), begin(paths), end(paths));
				}
				else if(EndGroup("Q").extract(token, context, vars)) {
					escape = true;
				}
				else if(AffineTransform("cm").extract(token, context, vars)) {}
				else if(MoveTo("m").extract(token, context, vars)) {}
				else if(LineTo("l").extract(token, context, vars)) {}
				else if(BezierTo("c").extract(token, context, vars)) {}
				else if(EaseInTo("y").extract(token, context, vars)) {}
				else if(EaseOutTo("v").extract(token, context, vars)) {}
				else if(Rectangle("re").extract(token, context, vars)) {}
				else if(Close("h").extract(token, context, vars)) {}
				else if(StrokeWidth("w").extract(token, context, vars)) {}
				else if(StrokeWidth("LW").extract(token, context, vars)) {}
				else if(StrokeColor("G").extract(token, context, vars)) {}
				else if(StrokeColor("RG").extract(token, context, vars)) {}
				else if(StrokeColor("SCN").extract(token, context, vars)) {}
				else if(FillColor("g").extract(token, context, vars)) {}
				else if(FillColor("rg").extract(token, context, vars)) {}
				else if(FillColor("scn").extract(token, context, vars)) {}
				else if(Clipping("W", OF_POLY_WINDING_NONZERO, true).extract(token, context, vars)) {}
				else if(Clipping("W*", OF_POLY_WINDING_ODD, true).extract(token, context, vars)) {}
				else if(Stroke("s", OF_POLY_WINDING_ODD, true).extract(token, context, vars) ||
						Stroke("S", OF_POLY_WINDING_ODD, false).extract(token, context, vars) ||
						Fill("f", OF_POLY_WINDING_NONZERO, true).extract(token, context, vars) ||
						Fill("f*", OF_POLY_WINDING_ODD, true).extract(token, context, vars) ||
						StrokeFill("b", OF_POLY_WINDING_NONZERO, true).extract(token, context, vars) ||
						StrokeFill("b*", OF_POLY_WINDING_ODD, true).extract(token, context, vars) ||
						StrokeFill("B", OF_POLY_WINDING_NONZERO, false).extract(token, context, vars) ||
						StrokeFill("B*", OF_POLY_WINDING_ODD, false).extract(token, context, vars) ||
						NoDraw("n", OF_POLY_WINDING_NONZERO, true).extract(token, context, vars)
						)
				{
					ret.push_back(context.clipping_enabled ? context.getClippedPath(): context.path);
					context.path.clear();
				}
				else if(Any(token).extract(token, context, vars)) {}
				else {
					ofLogError("ofxPoDoFoParser") << "something is wrong";
				}
				vars.clear();
			}	break;
			case ePdfContentsType_Variant: {
				vars.push_back(var);
			}	break;
			case ePdfContentsType_ImageData: {
			}	break;
			default:
				PODOFO_RAISE_ERROR(ePdfError_InternalLogic);
				break;
		}
		if(escape) {
			break;
		}
	}
	return ret;
}
