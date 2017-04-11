#ifndef _LECAGYODOCAL_POSE_H_
#define _LECAGYODOCAL_POSE_H_

#include <ostream>

namespace LegacyOdocal {

template<class T>
class Pose{
protected:
	T _x,_y,_phi;
public:
    /// Default construtor
    Pose():_x(0),_y(0),_phi(0){}
    /// Generate pose with x, y, phi
    Pose(const T& x,const T& y,const T& phi):_x(x),_y(y),_phi(phi){}
    /// Generate pose with combination of trans- and rot-movement
    Pose(const T& L,const T& phi): _x(L*sinc(phi)), _y(.5*_x*sin(phi)), _phi(phi){
		//std::cout<<"L "<<L<<std::endl<<"phi "<<phi<<std::endl;
	}
	
	inline const T& x()const{
		return this->_x;
	}
	inline const T& y()const{
		return this->_y;
	}
	inline const T& phi()const{
		return this->_phi;
	}
	
	//TODO: some operators

    /// Posenaddition wie im Wiki
	const Pose<T>& operator+=(const Pose<T>& p){
		this->_x+=cos(this->_phi)*p._x-sin(this->_phi)*p._y;
		this->_y+=sin(this->_phi)*p._x+cos(this->_phi)*p._y;
		this->_phi+=p._phi;
		return *this;
	}
	
	template<class U> friend std::ostream& operator<<(std::ostream&,const Pose<U>&);
};

template<class T>
std::ostream& operator<<(std::ostream& stream,const Pose<T>& p){
	stream<<"("<<p._x<<","<<p._y<<","<<p._phi<<")";
	return stream;
}

} // namespace LeacyOdocal

#endif
