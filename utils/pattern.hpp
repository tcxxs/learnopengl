#pragma once

class NoCopy {
protected:
	NoCopy() = default;
private:
	NoCopy(NoCopy const&) = delete;
	NoCopy(NoCopy const&&) = delete;
	const NoCopy& operator=(const NoCopy&) = delete;
	const NoCopy& operator=(const NoCopy&&) = delete;
};

template <typename T> 
class Singleton: private NoCopy {
public:
	static T& inst() {
		static T _inst;
		return _inst;
	}
};