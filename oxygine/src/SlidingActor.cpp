#include "SlidingActor.h"
#include "ClipRectActor.h"
#include "PointerState.h"
#include "UpdateState.h"
#include "Draggable.h"
#include "initActor.h"
#include "Serialize.h"

namespace oxygine
{
	float _defaultTouchThreshold = 15;
	void SlidingActor::setDefaultTouchThreshold(float val)
	{
		_defaultTouchThreshold = val;
	}

	SlidingActor::SlidingActor():
		_sliding(false),
		_rad(_defaultTouchThreshold), 
		_maxSpeed(250),
		_downTime(0),  
		_downPos(0, 0),
		_speed(0, 0),  
		_lastTime(0), _current(0), _lastIterTime(0)
	{
		_clip = initActor(new ClipRectActor, 
			arg_attachTo = this);

		_clip->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &SlidingActor::_newEvent));
		_clip->addEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &SlidingActor::_newEvent));
		_clip->addEventListener(TouchEvent::MOVE, CLOSURE(this, &SlidingActor::_newEvent));
	}
	
	void SlidingActor::destroy()
	{
		_clip->removeChildren();

		_drag.destroy();
		_content = 0; 
	}

	SlidingActor::~SlidingActor()
	{
		destroy();		
	}

	void SlidingActor::setTouchThreshold(float rad)
	{
		_rad = rad;
	}

	void SlidingActor::sizeChanged(const Vector2 &size)
	{
		if (_clip)
			_clip->setSize(size);

		updateDragBounds();
	}

	void SlidingActor::snap()
	{
		updateDragBounds();
		_drag.snapClient2Bounds();
		_sliding = false;
	}

	void SlidingActor::stop()
	{
		_speed = Vector2(0, 0);
	}

	void SlidingActor::setContent(spActor content)
	{
		if (_content)
		{
			_drag.destroy();
			_content->detach();
		}

		_speed = Vector2(0, 0);

		_content = content;
		_drag.init(content.get());
		
		_clip->addChild(_content);

		updateDragBounds();
	}

	void SlidingActor::setLocked(bool locked)
	{
		_drag.setDragEnabled(!locked);
	}

	void SlidingActor::updateDragBounds()
	{
		if (!_content)
			return;
		float w = std::max(0.0f, _content->getWidth() * _content->getScaleX() - _clip->getWidth());
		float h = std::max(0.0f, _content->getHeight() * _content->getScaleY() - _clip->getHeight());
		RectF bounds(-w, -h, w, h);		

		_drag.setDragBounds(bounds);
	}

	const timeMS fdt = 1000/60;
	//const float fdt = 20;

	void SlidingActor::doUpdate(const UpdateState &us)
	{
		if (!_content)
			return;

		//static float ml = 0;
		//ml = max(_speed.length(), ml);
		//log::messageln("sp: %.2f", ml);

		int ct = getTimeMS();
		if (_lastIterTime + NUM * fdt < ct)
			_lastIterTime = ct;

		if (_drag.isDragging())
		{
			Vector2 pos = _content->getPosition();
			//log::messageln("%d) pos %.2f %.2f", _current, pos.x, pos.y);
			_prev[_current].pos = pos;
			_prev[_current].tm = ct;
			_current = (_current + 1) % NUM;

			return;
		}

		if (_sliding)
		{
			const RectF &bounds = _drag.getDragBounds();
			while (_lastIterTime + fdt <= ct)
			{
				Vector2 pos = _content->getPosition();
				Vector2 newpos = pos + _speed * (fdt / 1000.0f);
				if (newpos.x < bounds.getLeft())
				{
					newpos.x = bounds.getLeft();
					_speed.x = 0;
				}else
				if (newpos.x > bounds.getRight())
				{
					newpos.x = bounds.getRight();
					_speed.x = 0;
				}

				if (newpos.y < bounds.getTop())
				{
					newpos.y = bounds.getTop();
					_speed.y = 0;
				}else
				if (newpos.y > bounds.getBottom())
				{
					newpos.y = bounds.getBottom();
					_speed.y = 0;
				}


				_speed *= 0.97f;
				_content->setPosition(newpos);

				_lastIterTime += fdt;
			}


			SlidingEvent sl(SlidingEvent::SLIDING);
			sl.speed = _speed;
			dispatchEvent(&sl);
			_speed = sl.speed;

			if (_speed.sqlength() < 8)
			{
				_sliding = false;
				SlidingEvent ev(SlidingEvent::END);
				dispatchEvent(&ev);
			}
		}

	}

	void SlidingActor::handleEvent(Event *event)
	{		
		Actor::handleEvent(event);
	}

	void SlidingActor::_newEvent(Event *event)
	{
		TouchEvent *te = safeCast<TouchEvent*>(event);
		timeMS tm = getTimeMS();
		switch(te->type)
		{
		case TouchEvent::TOUCH_DOWN:
			{
				_current = 0;
				_lastIterTime = tm;
				
				_prev[0].pos = _content->getPosition();
				_prev[0].tm = tm;

				for (int i = 1; i < NUM; ++i)
					_prev[i].tm = 0;

				_holded = event->target;
				_downPos = te->localPosition;
				_downTime = tm;
			}
			break;

		case TouchEvent::TOUCH_UP:
			{		
				if (_drag.getDragEnabled())
				{
					_downTime = 0;
					Vector2 pos = _content->getPosition();
	
					_holded = 0;

					const iter *old = 0;
					const iter *mid = 0;
					const iter *last = _prev + _current;

					for (int i = 1; i < NUM; ++i)
					{
						int n = (_current + NUM - i) % NUM;
						if (_prev[n].tm)					
							last = _prev + n;
						else
							break;
						if (!mid && (last->tm + 50 <= tm))
							mid = last;
						if (last->tm + 150 <= tm)
						{
							old = last;
							break;
						}
					}
					if (!old)
						old = last;
					if (!mid)
						mid = last;
								
					Vector2 midpos = mid->pos;
					Vector2 dir = pos - midpos;
					if (dir.sqlength() < 10 * 10)				
						_speed = Vector2(0,0);
					else
					{
						Vector2 dr = pos - old->pos;
						timeMS v = tm - old->tm;
						OX_ASSERT(v);
						if (v)
						{
							Vector2 ns = (dr * 1000.0f) / v;

							/*
							int d = tm - _downTime;
							Vector2 dr2 = pos - _downPos;
							Vector2 ts = dr2 / float(d) * 1000.0f;
							ts.x = 0;

							log::messageln("fs: %.2f %d ns: %.2f %d", ns.y, int(tm), ts.y, d);
							*/
							//ns = ts;


							if (_speed.dot(ns) < 0)
								_speed = ns;
							else
								_speed += ns;
						}
					}

				
					if (!_sliding)
					{
						_sliding = true;
					}

					SlidingEvent sd(SlidingEvent::BEGIN);
					sd.speed = _speed;
					dispatchEvent(&sd);
					_speed = sd.speed;

					_lastIterTime = tm;
				}				
			}
			break;

		case TouchEvent::MOVE:
			{
				Vector2 offset = _downPos - te->localPosition;
				float d = offset.dot(offset);
				if (_holded && (d >= _rad * _rad))
				{
					TouchEvent ev(TouchEvent::TOUCH_UP, false, Vector2(-10000, -10000));
					spActor act = safeSpCast<Actor>(_holded);
					while(act && act.get() != _content.get())
					{
						Actor::setPressed(act.get(), 0);
						//act->setPressed(0);
						//act->setOvered(0);
						act = act->getParent();
					}

					Actor::setPressed(_content.get(), te->index);
					_holded = 0;
				}
			}			
			break;
		}
	}

	void SlidingActor::serialize(serializedata* data)
	{
		Actor::serialize(data);

		data->node.set_name("SlidingActor");
	}

	void SlidingActor::deserialize(const deserializedata* data)
	{
		Actor::deserialize(data);
	}
}
