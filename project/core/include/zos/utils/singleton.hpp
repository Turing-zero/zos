#ifndef __ZOS_SINGLETON_H__
#define __ZOS_SINGLETON_H__

template <typename SingletonClass >
class Singleton{
public:
	static SingletonClass& GetInstance(){
		static SingletonClass instance;
		return instance;
	}
	// SingletonClass& operator ->() { return Instance(); }
	// const SingletonClass& operator ->() const { return Instance(); }
private:
	Singleton() = default;
	~Singleton() = default;
	Singleton(Singleton &other) = delete;
	void operator=(const Singleton &) = delete;
};

#endif // __ZOS_SINGLETON_H__
