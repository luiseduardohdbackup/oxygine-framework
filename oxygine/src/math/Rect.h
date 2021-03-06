#pragma once
#include "oxygine_include.h"
#include "vector2.h"
#include <algorithm>

namespace oxygine
{
	template<typename point2>
	class RectT
	{		
	public:
		typedef typename point2::type T;
		typedef point2 type;

		RectT():pos(0,0), size(0,0){}
		RectT(const point2 &Pos, const point2 &Size):pos(Pos), size(Size){}
		RectT(T x, T y, T w, T h):pos(x, y), size(w, h){}

		bool operator == (const RectT &r) const
		{
			return r.pos == pos && r.size == size;
		}

		bool operator != (const RectT &r) const
		{
			return !(*this == r);
		}

		bool isEmpty()const
		{
			if (size.x <= 0 || size.y <= 0)
				return true;
			return false;
		}

		bool isIntersecting(const RectT &r) const
		{
			RectT l = *this;
			l.clip(r);
			return !l.isEmpty();
		}

		bool pointIn(const point2 &p) const
		{
			point2 rb;
			rb = pos + size;
			return (p.x >= pos.x) && (p.x < rb.x ) && (p.y >= pos.y) && (p.y < rb.y);
		}

		void clip(const RectT &rect)
		{
			point2 pt = pos + size;
			
			if (pos.x < rect.pos.x)
				pos.x = rect.pos.x;
			if (pos.y < rect.pos.y)
				pos.y = rect.pos.y;

			point2 rb = rect.pos + rect.size;
			
			if (pt.x > rb.x)
				pt.x = rb.x;
			if (pt.y > rb.y)
				pt.y = rb.y;

			size.x = pt.x - pos.x;
			size.y = pt.y - pos.y;
		}

		void unite(const RectT &rect)
		{
			point2 rbA = pos + size;
			point2 rbB = rect.pos + rect.size;

			pos.x = std::min(rect.pos.x, pos.x);
			pos.y = std::min(rect.pos.y, pos.y);

			size.x = std::max(rbA.x, rbB.x) - pos.x;
			size.y = std::max(rbA.y, rbB.y) - pos.y;
		}

		point2 getCenter() const {return pos + size/2;}
		point2 getSize() const {return size;}
		point2 getLeftTop() const {return pos;}
		point2 getRightBottom() const {return pos + size;}
		point2 getRightTop() const {return point2(getRight(), getTop());}
		point2 getLeftBottom() const {return point2(getLeft(), getBottom());}

		T getX() const{return pos.x;}
		T getY() const{return pos.y;}
		T getLeft() const{return pos.x;}
		T getTop() const{return pos.y;}
		T getWidth() const{return size.x;}
		T getHeight() const{return size.y;}

		T getRight() const{return pos.x + size.x;}
		T getBottom() const{return pos.y + size.y;}

		void setPosition(const point2 &pos_){pos = pos_;}
		void setSize(const point2 &size_){size = size_;}
		void setX(T v) {pos.x = v;}
		void setY(T v) {pos.y = v;}


		void setWidth(T v) {size.x = v;}
		void setHeight(T v) {size.y = v;}

		void moveLeft(T v) {T p = pos.x; pos.x = v; size.x += p - v;}
		void moveTop(T v) {T p = pos.y; pos.y = v; size.y += p - v;}
		void moveRight(T v) {size.x = v - pos.x;}
		void moveBottom(T v) {size.y = v - pos.y;}

		void expand(const point2 &v1, const point2 &v2){pos -= v1; size += v1 + v2;}

		template<class R>
		RectT operator * (const VectorT2<R> &v) const
		{
			RectT r(*this);
			r.pos.x *= v.x;
			r.pos.y *= v.y;
			r.size.x *= v.x;
			r.size.y *= v.y;
			return r;
		}

		template<class R>
		RectT operator / (const VectorT2<R> &v) const
		{
			RectT r(*this);
			r.pos.x /= v.x;
			r.pos.y /= v.y;
			r.size.x /= v.x;
			r.size.y /= v.y;
			return r;
		}

		template<class R>
		RectT operator * (const R &v) const
		{
			RectT r(*this);
			r.pos.x *= v;
			r.pos.y *= v;
			r.size.x *= v;
			r.size.y *= v;
			return r;
		}

		template<class R>
		RectT operator / (const R &v) const
		{
			RectT r(*this);
			r.pos.x /= v;
			r.pos.y /= v;
			r.size.x /= v;
			r.size.y /= v;
			return r;
		}


		template<typename R>
		R cast() const
		{
			typedef R rect;
			typedef typename R::type rect2type;
			return rect(pos.template cast<rect2type>(), size.template cast<rect2type>());
		}
		/*
		template<class R>
		R cast() const
		{
			R r = R(pos.cast<R::type>(), size.cast<R::type>());
			return r;
		}
		*/
		

		point2 pos;
		point2 size;
	};

	typedef RectT<Point> Rect;
	typedef RectT<Vector2> RectF;
}
