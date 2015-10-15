/* 
 * File:   render.cpp
 * Author: ivan
 * 
 * Created on October 12, 2015, 11:04 PM
 */

#include "render.hpp"

#include <cmath>

#include <cairo/cairo.h>

#include <utki/util.hpp>

using namespace svgren;



namespace{

class CairoMatrixPush{
	cairo_matrix_t m;
	cairo_t* cr;
public:
	CairoMatrixPush(cairo_t* cr) :
			cr(cr)
	{
		ASSERT(this->cr)
		cairo_get_matrix(this->cr, &this->m);
	}
	~CairoMatrixPush()noexcept{
		cairo_set_matrix(this->cr, &this->m);
	}
};


class Renderer : public svgdom::Renderer{
	cairo_t* cr;
	
	void applyTransformations(const svgdom::Transformable& transformable)const{
		for(auto& t : transformable.transformations){
			switch(t.type){
				case svgdom::Transformation::EType::TRANSLATE:
					cairo_translate(this->cr, t.x, t.y);
					break;
				case svgdom::Transformation::EType::MATRIX:
					{
						cairo_matrix_t matrix;
						matrix.xx = t.a;
						matrix.yx = t.b;
						matrix.xy = t.c;
						matrix.yy = t.d;
						matrix.x0 = t.e;
						matrix.y0 = t.f;
						cairo_transform(this->cr, &matrix);
					}
					break;
				case svgdom::Transformation::EType::SCALE:
					cairo_scale(this->cr, t.x, t.y);
					break;
				case svgdom::Transformation::EType::ROTATE:
					cairo_translate(this->cr, t.x, t.y);
					cairo_rotate(this->cr, t.angle);
					cairo_translate(this->cr, -t.x, -t.y);
					break;
				case svgdom::Transformation::EType::SKEWX:
					{
						cairo_matrix_t matrix;
						matrix.xx = 1;
						matrix.yx = 0;
						matrix.xy = std::tan(t.angle);
						matrix.yy = 1;
						matrix.x0 = 0;
						matrix.y0 = 0;
						cairo_transform(this->cr, &matrix);
					}
					break;
				case svgdom::Transformation::EType::SKEWY:
					{
						cairo_matrix_t matrix;
						matrix.xx = 1;
						matrix.yx = std::tan(t.angle);
						matrix.xy = 0;
						matrix.yy = 1;
						matrix.x0 = 0;
						matrix.y0 = 0;
						cairo_transform(this->cr, &matrix);
					}
					break;
				default:
					ASSERT(false)
					break;
			}
		}
	}
	
public:
	Renderer(cairo_t* cr) :
			cr(cr)
	{}
	
	void render(const svgdom::GElement& e)override{
		CairoMatrixPush cairoMatrixPush(this->cr);
		
		this->applyTransformations(e);
		
		e.Container::render(*this);
	}
	
	void render(const svgdom::SvgElement& e)override{
		e.Container::render(*this);
	}
	
	void render(const svgdom::PathElement& e)override{
		CairoMatrixPush cairoMatrixPush(this->cr);
		
		this->applyTransformations(e);
		
//		const svgdom::PathElement::Step* prev = nullptr;
		for(auto& s : e.path){
			switch(s.type){
				case svgdom::PathElement::Step::EType::MOVE_ABS:
					cairo_move_to(this->cr, s.x, s.y);
					break;
				case svgdom::PathElement::Step::EType::MOVE_REL:
					cairo_rel_move_to(this->cr, s.x, s.y);
					break;
				case svgdom::PathElement::Step::EType::LINE_ABS:
					cairo_line_to(this->cr, s.x, s.y);
					break;
				case svgdom::PathElement::Step::EType::LINE_REL:
					cairo_rel_line_to(this->cr, s.x, s.y);
					break;
				case svgdom::PathElement::Step::EType::HORIZONTAL_LINE_ABS:
					{
						double x, y;
						if(cairo_has_current_point(this->cr)){
							cairo_get_current_point(this->cr, &x, &y);
						}else{
							y = 0;
						}
						cairo_line_to(this->cr, s.x, y);
					}
					break;
				case svgdom::PathElement::Step::EType::HORIZONTAL_LINE_REL:
					{
						double x, y;
						if(cairo_has_current_point(this->cr)){
							cairo_get_current_point(this->cr, &x, &y);
						}else{
							y = 0;
						}
						cairo_rel_line_to(this->cr, s.x, y);
					}
					break;
				case svgdom::PathElement::Step::EType::VERTICAL_LINE_ABS:
					{
						double x, y;
						if(cairo_has_current_point(this->cr)){
							cairo_get_current_point(this->cr, &x, &y);
						}else{
							x = 0;
						}
						cairo_line_to(this->cr, x, s.y);
					}
					break;
				case svgdom::PathElement::Step::EType::VERTICAL_LINE_REL:
					{
						double x, y;
						if(cairo_has_current_point(this->cr)){
							cairo_get_current_point(this->cr, &x, &y);
						}else{
							x = 0;
						}
						cairo_rel_line_to(this->cr, x, s.y);
					}
					break;
				case svgdom::PathElement::Step::EType::CLOSE:
					cairo_close_path(this->cr);
					break;
				case svgdom::PathElement::Step::EType::CUBIC_ABS:
					cairo_curve_to(this->cr, s.x1, s.y1, s.x2, s.y2, s.x, s.y);
					break;
				case svgdom::PathElement::Step::EType::CUBIC_REL:
					cairo_rel_curve_to(this->cr, s.x1, s.y1, s.x2, s.y2, s.x, s.y);
					break;
//				case svgdom::PathElement::Step::EType::CUBIC_SMOOTH_ABS:
//					{
//						double x, y;
//						if(cairo_has_current_point(this->cr)){
//							cairo_get_current_point(this->cr, &x, &y);
//						}else{
//							x = 0;
//							y = 0;
//						}
//						double x1, y1;
//						if(prev){
//							x1 = -(prev->x2 - x) + x;
//							y1 = -(prev->y2 - y) + y;
//						}else{
//							x1 = x;
//							y1 = y;
//						}
//						cairo_curve_to(this->cr, x1, y1, s.x2, s.y2, s.x, s.y);
//					}
//					break;
				default:
					ASSERT(false)
					break;
			}
//			prev = &s;
		}
		
		auto fill = e.getStyleProperty(svgdom::EStyleProperty::FILL);
		auto stroke = e.getStyleProperty(svgdom::EStyleProperty::STROKE);
		
		if(fill && fill->effective){
			svgdom::real opacity;
			if(auto fillOpacity = e.getStyleProperty(svgdom::EStyleProperty::FILL_OPACITY)){
				opacity = fillOpacity->floating;
			}else{
				opacity = 1;
			}

			auto fillRgb = fill->getRgb();

			cairo_set_source_rgba(this->cr, fillRgb.r, fillRgb.g, fillRgb.b, opacity);
			if(stroke && stroke->effective){
				cairo_fill_preserve(this->cr);
			}else{
				cairo_fill(this->cr);
			}
		}
		
		if(stroke && stroke->effective){
			if(auto strokeWidth = e.getStyleProperty(svgdom::EStyleProperty::STROKE_WIDTH)){
				cairo_set_line_width(cr, strokeWidth->length.value);
			}else{
				cairo_set_line_width(cr, 1);
			}

			auto rgb = stroke->getRgb();

			cairo_set_source_rgb(this->cr, rgb.r, rgb.g, rgb.b);
			cairo_stroke(this->cr);
		}
	}
};

}



std::vector<std::uint32_t> svgren::render(const svgdom::SvgElement& svg, unsigned width, unsigned height){
//	int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
	int stride = width * 4;
	
	TRACE(<< "width = " << width << " stride = " << stride / 4 << std::endl)
	
	std::vector<std::uint32_t> ret((stride / sizeof(std::uint32_t)) * height);
	
	for(auto& c : ret){
		c = 0xffffffff;//TODO: fill 0
	}
	
	cairo_surface_t* surface = cairo_image_surface_create_for_data(
			reinterpret_cast<unsigned char*>(&*ret.begin()),
			CAIRO_FORMAT_ARGB32,
			width,
			height,
			stride
		);
	if(!surface){
		ret.clear();
		return ret;
	}
	utki::ScopeExit scopeExitSurface([&surface](){
		cairo_surface_destroy(surface);
	});
	
	cairo_t* cr = cairo_create(surface);
	if(!cr){
		ret.clear();
		return ret;
	}
	utki::ScopeExit scopeExitContext([&cr](){
		cairo_destroy(cr);
	});
	
	Renderer r(cr);
	
	svg.render(r);
	
	return ret;
}