#pragma once

#include <boost/python.hpp>

//RAII classes for easy control of the GIL

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