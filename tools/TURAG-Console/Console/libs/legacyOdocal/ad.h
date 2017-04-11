#ifndef _LECAGYODOCAL_AD_H_
#define _LECAGYODOCAL_AD_H_

#include <cmath>
#include <cstdarg>

namespace LegacyOdocal {

extern double quad(double);
extern double sinc(double);

template<typename T,int dim>
class AD{
protected:
	T _val;
	T _grad[dim];
public:
	AD(){}
	AD(const T& val):_val(val){
		for(int i=0;i<dim;++i)this->_grad[i]=T(0);
	}
	AD(const T& val,const T* const grad):_val(val){
		if(grad){
			for(int i=0;i<dim;++i)this->_grad[i]=grad[i];
		}
	}
	AD(const T& val,int d,...):_val(val){
		va_list grad;
		va_start(grad,d);
		for(int i=0;i<d;++i)this->_grad[i]=va_arg(grad,T);
		for(int i=d;i<dim;++i)this->_grad[i]=0;
		va_end(grad);
	}
	
	inline const T& operator()()const{
		return this->_val;
	}
	inline const T& operator[](int index)const{
		return this->_grad[index];
	}
	inline bool operator==(const T& x)const{
		return this->_val==x;
	}
	inline bool operator==(const AD<T,dim>& x)const{
		return this->_val==x._val;
	}
	//...
	
	inline const AD<T,dim>& operator+()const{
		return *this;
	}
	inline const AD<T,dim>& operator+=(const T& x){
		this->_val+=x;
		return *this;
	}
	inline const AD<T,dim>& operator+=(const AD<T,dim>& x){
		this->_val+=x._val;
		for(int i=0;i<dim;++i)this->_grad[i]+=x._grad[i];
		return *this;
	}
	inline AD<T,dim> operator+(const T& x)const{
		return AD<T,dim>(this->_val+x,this->_grad);
	}
	inline AD<T,dim> operator+(const AD<T,dim>& x)const{
		AD<T,dim> ans(this->_val+x._val,0);
		for(int i=0;i<dim;++i)ans._grad[i]=this->_grad[i]+x._grad[i];
		return ans;
	}
	template<typename U,int d> friend AD<U,d> operator+(const T&,const AD<U,d>&);
	
	inline AD<T,dim> operator-()const{
		AD<T,dim> ans(-this->_val,0);
		for(int i=0;i<dim;++i)ans._grad[i]=-this->_grad[i];
		return ans;
	}
	inline const AD<T,dim>& operator-=(const T& x){
		this->_val-=x;
		return *this;
	}
	inline const AD<T,dim>& operator-=(const AD<T,dim>& x){
		this->_val-=x._val;
		for(int i=0;i<dim;++i)this->_grad[i]-=x._grad[i];
		return *this;
	}
	inline AD<T,dim> operator-(const T& x)const{
		return AD<T,dim>(this->_val-x,this->_grad);
	}
	inline AD<T,dim> operator-(const AD<T,dim>& x)const{
		AD<T,dim> ans(this->_val-x._val,0);
		for(int i=0;i<dim;++i)ans._grad[i]=this->_grad[i]-x._grad[i];
		return ans;
	}
	template<typename U,int d> friend AD<U,d> operator-(const T&,const AD<U,d>&);
	
	inline const AD<T,dim>& operator*=(const T& x){
		this->_val*=x;
		for(int i=0;i<dim;++i)this->_grad[i]*=x;
		return *this;
	}
	inline const AD<T,dim>& operator*=(const AD<T,dim>& x){
		for(int i=0;i<dim;++i){
			this->_grad[i]*=x._val;
			this->_grad[i]+=this->_val*x._grad[i];
		}
		this->_val*=x._val;
		return *this;
	}
	inline AD<T,dim> operator*(const T& x)const{
		AD<T,dim> ans(this->_val*x,0);
		for(int i=0;i<dim;++i)ans._grad[i]=this->_grad[i]*x;
		return ans;
	}
	inline AD<T,dim> operator*(const AD<T,dim>& x)const{
		AD<T,dim> ans(this->_val*x._val,0);
		for(int i=0;i<dim;++i)ans._grad[i]=this->_grad[i]*x._val+this->_val*x._grad[i];
		return ans;
	}
	template<typename U,int d> friend AD<U,d> operator*(const U&,const AD<U,d>&);
	
	inline const AD<T,dim>& operator/=(const T& x){
		this->_val/=x;
		for(int i=0;i<dim;++i)this->_grad[i]/=x;
		return *this;
	}
	inline const AD<T,dim>& operator/=(const AD<T,dim>& x){
		this->_val/=x._val;
		for(int i=0;i<dim;++i){
			this->_grad[i]-=this->_val*x._grad[i];
			this->_grad[i]/=x._val;
		}
		return *this;
	}
	inline AD<T,dim> operator/(const T& x)const{
		AD<T,dim> ans(this->_val/x,0);
		for(int i=0;i<dim;++i)ans._grad[i]=this->_grad[i]/x;
		return ans;
	}
	inline AD<T,dim> operator/(const AD<T,dim>& x)const{
		AD<T,dim> ans(this->_val/x._val,0);
		for(int i=0;i<dim;++i)ans._grad[i]=(this->_grad[i]-ans._val*x._grad[i])/x._val;
		return ans;
	}
	template<typename U,int d> friend AD<U,d> operator/(const T&,const AD<U,d>&);
	
	//...
	
	template<typename U,int d> friend AD<U,d> quad(const AD<U,d>&);
	
	template<typename U,int d> friend AD<U,d> cos(const AD<U,d>&);
	template<typename U,int d> friend AD<U,d> sin(const AD<U,d>&);
	template<typename U,int d> friend AD<U,d> sinc(const AD<U,d>&);
	
	template<typename U,int d> friend std::ostream& operator<<(std::ostream&,const AD<U,d>&);
};

template<typename T,int dim>
AD<T,dim> operator+(const T& x,const AD<T,dim>& y){
	return AD<T,dim>(x+y._val,y._grad);
}

template<typename T,int dim>
AD<T,dim> operator-(const T& x,const AD<T,dim>& y){
	AD<T,dim> ans(x-y._val,0);
	for(int i=0;i<dim;++i)ans._grad[i]=-y._grad[i];
	return ans;
}

template<typename T,int dim>
AD<T,dim> operator*(const T& x,const AD<T,dim>& y){
	AD<T,dim> ans(x*y._val,0);
	for(int i=0;i<dim;++i)ans._grad[i]=x*y._grad[i];
	return ans;
}

template<typename T,int dim>
AD<T,dim> operator/(const T& x,const AD<T,dim>& y){
	AD<T,dim> ans(x/y._val,0);
	for(int i=0;i<dim;++i)ans._grad[i]=-ans._val/y._val*y._grad[i];
	return ans;
}

template<typename T,int dim>
AD<T,dim> quad(const AD<T,dim>& x){
	AD<T,dim> ans(x._val*x._val,0);
	for(int i=0;i<dim;++i)ans._grad[i]=2.0*x._val*x._grad[i];
	return ans;
}

template<typename T,int dim>
AD<T,dim> cos(const AD<T,dim>& x){
    AD<T,dim> ans(::cos(x._val),0);
	for(int i=0;i<dim;++i)ans._grad[i]=-sin(x._val)*x._grad[i];
	return ans;
}

template<typename T,int dim>
AD<T,dim> sin(const AD<T,dim>& x){
    AD<T,dim> ans(::sin(x._val),0);
    for(int i=0;i<dim;++i)ans._grad[i]=::cos(x._val)*x._grad[i];
	return ans;
}

template<typename T,int dim>
AD<T,dim> sinc(const AD<T,dim>& x){
	if(std::abs(x._val)<.1){
		AD<T,dim> q(-quad(x));
		AD<T,dim> ans(1.0/5040.0);
		ans*=q;
		ans+=1.0/120;
		ans*=q;
		ans+=1.0/6.0;
		ans*=q;
		ans+=1.0;
		return ans;
	}else{
		return sin(x)/x;
	}
}

template<typename T,int dim>
std::ostream& operator<<(std::ostream& stream,const AD<T,dim>& x){
	stream<<x._val<<" [";
	for(int i=0;i<dim;++i)stream<<" "<<x._grad[i];
	stream<<"]";
	return stream;
}

} // namespace LeacyOdocal

#endif
