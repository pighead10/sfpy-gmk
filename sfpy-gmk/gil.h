#pragma once

#include <boost/python.hpp>

/*
ReleaseGIL and AcquireGIL:
RAII classes for easy store/release of the Python Global Interpreter Lock
Adapted from a StackOverflow post
*/

class ReleaseGIL{
public:
	inline ReleaseGIL(){
		state = PyEval_SaveThread();
	}
	inline ~ReleaseGIL(){
		PyEval_RestoreThread(state);
	}
private:
	PyThreadState* state;
};

class AcquireGIL{
public:
	inline AcquireGIL(){
		state = PyGILState_Ensure();
	}
	inline ~AcquireGIL(){
		PyGILState_Release(state);
	}
private:
	PyGILState_STATE state;
};