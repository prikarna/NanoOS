/*
 * File    : DkObject.h
 * Remark  : C++ class and template for Event and Property object.
 *           Event :
 *               To represent when something happened on an object. Note that event machanism defined
 *               here may violate any C++ standard.
 *           Property :
 *               To represent what an object has and this property can do something when it changes.
 *               Note that although this property can 'behave' like a builtin type but C functions 
 *               may not understand and will treat this as a C structure.
 *
 */

#pragma once

/*
 * EventConsumer class.
 *
 * A class that consume an event (or do something when event is triggered) must inherit from this class.
 * If that class has multiple class to be inherited, EventConsumer class must be put as first item in the 
 * inheritance list class definition. This allow C++ compiler to do reinterpret_cast() between member 
 * function pointer safely (please see EventHandler class template definition) when assign an new instance 
 * of event handler on an event object.
 */
class EventConsumer
{
public :
	EventConsumer() {}
	~EventConsumer() {}
};

/*
 * EventHandler class template.
 *
 * As the name suggest, this is an event handler. Event handler must be assigned to Event when
 * a class need to do something about it. This event handler class template is used to assign a 
 * void member function of a class or an object.
 */
template <typename ArgumentType=int>
class EventHandler
{
public:
	EventConsumer *			_Consumer;
	void (EventConsumer:: *	_ConsumerHandler)(ArgumentType);
	void (EventConsumer:: *	_ConsumerHandlerNoArg)();

	/* 
	 * C'tor for consumer object with its void member function pointer with argument.
	 * Note that assigment below may violate any C++ standard.
	 */
	template <typename Consumer>
	EventHandler<ArgumentType>(Consumer * ConsumerObject, void (Consumer:: * ConsumerHandler)(ArgumentType))
	{
		_Consumer					= dynamic_cast<EventConsumer *>(ConsumerObject);
		_ConsumerHandler			= reinterpret_cast<void (EventConsumer:: *)(ArgumentType)>(ConsumerHandler);

		_ConsumerHandlerNoArg		= 0;

	};

	/*
	 * C'tor for comsumer object with its void member function pointer without argument. 
	 * Note that assigment below may violate any C++ standard.
	 */
	template <typename Consumer>
	EventHandler<ArgumentType>(Consumer * ConsumerObject, void (Consumer:: * ConsumerHandler)())
	{
		_Consumer					= dynamic_cast<EventConsumer *>(ConsumerObject);
		_ConsumerHandlerNoArg		= reinterpret_cast<void (EventConsumer:: *)()>(ConsumerHandler);

		_ConsumerHandler			= 0;
	};

	/* D'tor */
	~EventHandler<ArgumentType>() {};
};

/*
 * Event class template.
 *
 * A class that generate an event use this as event generator (trigger). In order to use it a class
 * put it as a class member and when class want to trigger (fire) an event use event class as function 
 * call with or without EventArgumentType (this argument type can be any type or can be a pointer to 
 * other class or structure).
 * An event handler can be assigned to event object and this event handler can be:
 *    1. Void function (function that return nothing) with or without an argument in global space.
 *    2. New instance of EventHandler class template. This new instance must be instantiated with 
 *       parameters of 'this' of consumer object and its void member function pointer.
 *
 * Note:
 *    - For point 2, the class of consumer object must inherit from EventConsumer class. (Please see 
 *      EventConsumer class).
 * 
 */
template <typename Owner, typename ArgumentType=int>
class Event
{
private:
	EventConsumer *			_Consumer;
	void (EventConsumer:: *	_ConsumerHandler)(ArgumentType);
	void (EventConsumer:: *	_ConsumerHandlerNoArg)();
	void (*					_GlobalHandler)(ArgumentType);
	void (*					_GlobalHandlerNoArg)();

	bool Fire(ArgumentType Arg)
	{
		bool	fRes = false;

		if (_GlobalHandler) {
			(* _GlobalHandler)(Arg);
			fRes = true;
		}

		if ((_Consumer != 0) &&
			(_ConsumerHandler != 0))
		{
			(_Consumer->*(_ConsumerHandler))(Arg);
			fRes = true;
		}

		return fRes;
	}

	bool Fire()
	{
		bool	fRes = false;

		if (_GlobalHandlerNoArg) {
			(* _GlobalHandlerNoArg)();
			fRes = true;
		}

		if ((_Consumer != 0) &&
			(_ConsumerHandlerNoArg != 0))
		{
			(_Consumer->*(_ConsumerHandlerNoArg))();
			fRes = true;
		}

		return fRes;
	}

	/* Fire or trigger an event with argument, only owner can fire an event */
	bool operator() (ArgumentType Argument)
	{
		return Fire(Argument);
	}

	/* Fire or trigger an event wihtout argument, only owner can fire an event */
	bool operator() ()
	{
		return Fire();
	}

public:
	/* Assignment with a global void function with an argument */
	Event<Owner, ArgumentType>& operator= (void (* GlobalHandler)(ArgumentType))
	{
		_GlobalHandler = GlobalHandler;
		return (*this);
	}

	/* Assignment with a global void function without argument */
	Event<Owner, ArgumentType>& operator= (void (* GlobalHandler)())
	{
		_GlobalHandlerNoArg = GlobalHandler;
		return (*this);
	}

	/*
	 * Assignment with a new instance of EventHandler<EventArgumentType> class.
	 */
	Event<Owner, ArgumentType>& operator= (EventHandler<ArgumentType> * NewHandlerObject)
	{
		if (NewHandlerObject) {
			if (NewHandlerObject->_ConsumerHandler) 
			{
				_Consumer				= NewHandlerObject->_Consumer;
				_ConsumerHandler		= NewHandlerObject->_ConsumerHandler;
			} 
			else if (NewHandlerObject->_ConsumerHandlerNoArg) 
			{
				_Consumer				= NewHandlerObject->_Consumer;
				_ConsumerHandlerNoArg	= NewHandlerObject->_ConsumerHandlerNoArg;
			} 
			else 
			{
				throw "No event handler assigned!";
			}

			delete NewHandlerObject;
		}
		return (*this);
	}

	// C'tor
	Event<Owner, ArgumentType>()
	{
		_Consumer				= 0;
		_ConsumerHandler		= 0;
		_ConsumerHandlerNoArg	= 0;
		_GlobalHandler			= 0;
		_GlobalHandlerNoArg		= 0;
	}

	// D'tor
	~Event<Owner, ArgumentType>() {}

	/* Owner can access private members */
	friend typename Owner;	// friend typename ..., may not comply with C++ standard
};

/*
 * Property class template.
 *
 * Owner or class user can be any type of class who use this class template as class member. VariableType can be any 
 * variable type and its pointer to those types.
 * Property class must be set to a setter and getter function of an owner or a class user (a class that use Property 
 * class template as its member). This operation can be done in class user's constructor. Note that these setter and 
 * getter function can be a functions in global space or class user's member functions.
 * The setter function must be parameterized with VariableType. This setter function is called when the property
 * of class user is assigned to a varable type of VariableType.
 * The getter function must return a value with the type of VariableType. This getter function is called when
 * a variable with type of VariableType is assign to class user property.
 * 
 * Note this class template may not work on some (untested) situations.
 *
 */
template <typename Owner, typename VariableType=int>
class Property
{
private:
	Owner *						_Owner;
	void (Owner:: *				_OwnerSetter)(VariableType);
	VariableType (Owner:: *		_OwnerGetter)();
	void (*						_GlobalSetter)(VariableType);
	VariableType (*				_GlobalGetter)();
	bool						_IsGlobal;

	/*
	 * Set a setter and a getter function of global space. 
	 */
	void Set(void (* GlobalSetter)(VariableType), VariableType (* GlobalGetter)())
	{
		_GlobalSetter = GlobalSetter;
		_GlobalGetter = GlobalGetter;

		_IsGlobal = true;

		_Owner			= 0;
		_OwnerSetter	= 0;
		_OwnerGetter	= 0;
	}

	/*
	 * Set a setter and a getter function of class user's member function.
	 */
	void Set(
			Owner * OwnerObject, 
			void (Owner:: * OwnerSetter)(VariableType), 
			VariableType (Owner:: * OwnerGetter)()
			)
	{
		_Owner			= OwnerObject;
		_OwnerSetter	= OwnerSetter;
		_OwnerGetter	= OwnerGetter;

		_IsGlobal = false;

		_GlobalSetter	= 0;
		_GlobalGetter	= 0;
	}

public:
	/*
	 * Assigment with a variable with type of VariableType.
	 */
	Property<Owner, VariableType>& operator= (VariableType Variable)
	{
		if (_IsGlobal) 
		{
			if (_GlobalSetter) {
				(* _GlobalSetter)(Variable);
			}
		}
		else
		{
			if ((_Owner) && (_OwnerSetter))
			{
				(_Owner->*(_OwnerSetter))(Variable);
			}
		}

		return *this;
	}

	/*
	 * To be assigned to a variable with type of VariableType
	 */
	operator VariableType() const
	{
		VariableType	Var;

		if (_IsGlobal) 
		{
			if (_GlobalGetter) {
				Var = (* _GlobalGetter)();
			} else {
				throw "Global getter has not been set!";
			}
		}
		else
		{
			if ((_Owner) && (_OwnerGetter))
			{
				Var = (_Owner->*(_OwnerGetter))();
			} 
			else 
			{
				throw "Owner or class user getter has not been set!";
			}
		}

		return Var;
	}

	/* C'tor */
	Property<Owner, VariableType>()
	{
		_Owner			= 0;
		_OwnerSetter	= 0;
		_OwnerGetter	= 0;

		_IsGlobal = false;

		_GlobalSetter	= 0;
		_GlobalGetter	= 0;
	}

	/* D'tor */
	~Property<Owner, VariableType>() {}

	/* Owner can access private members */
	friend typename Owner;
};
